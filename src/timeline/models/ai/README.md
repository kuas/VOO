# AI Clip 实现说明

## 已完成的核心功能

### 1. 基础架构

#### TimelineDef.h 扩展
- ✅ 添加了 3 个 AI ResourceType（AIImageResource, AIVideoResource, AITTSResource）
- ✅ 添加了 4 个 AI TrackRole（State, Progress, Error, Params）

#### 核心类
- ✅ **AIGeneratedClip** - AI Clip 基类，提供状态管理、缓存、异步任务管理
- ✅ **AIGenerationTask** - 异步任务基类及三个子类（Image, Video, TTS）
- ✅ **AIResourceCache** - 单例缓存管理器，基于参数哈希的智能缓存
- ✅ **AIImageClip** - AI 生图实现，完整的 UIConfig

### 2. LiteLLM API 集成

#### AIImageGenerationTask
- ✅ 调用 LiteLLM Gemini API（`http://litellm.test.bloomeverybody.work/v1/images/generations`）
- ✅ 支持两个模型：
  - `gemini-3-pro-image-preview`
  - `vertex_ai/gemini-2.5-flash-image`
- ✅ 解析 base64 响应格式
- ✅ 自动保存生成的图片到临时目录

### 3. 编辑面板组件

#### AIGenerationControlsComponent
- ✅ C++ 组件类实现
- ✅ 状态监听和同步
- ✅ 参数收集逻辑
- ✅ 生成控制方法（Start, Cancel, Retry）

### 4. AIImageClip UIConfig

完整的参数配置：
1. 模型选择（Dropdown）
2. 提示词输入（Text）
3. 参考图片（Image）- 用于图生图
4. 图片尺寸（Dropdown）- 5种预设
5. 生成风格（Dropdown）- 6种风格
6. 生成强度（Slider）- 0.0-1.0
7. 生成步数（Slider）- 10-100
8. 随机种子（Slider）
9. AIGenerationControls 组件
10. Transform 组件

---

## 待完成的工作

### P0 - 必须完成（才能编译测试）

1. **更新 CMakeLists.txt**
   - 添加所有 AI 相关的 .cpp 文件到编译列表

2. **扩展 ComponentBean 字典**
   - 在 `src/timeline/controller/components/ComponentBean.h` 中添加：
   ```cpp
   {"AIGenerationControls", "AIGenerationControlsProperty.qml"}
   ```

3. **创建 QML 文件**（可选，但建议）
   - `src/ui/property/ai/AIGenerationControlsProperty.qml`

4. **创建占位符资源**（可选）
   - `assets/ai_image_placeholder.png`

5. **API Key 配置**
   - 目前硬编码为 "Bearer XXX"
   - 建议改为从配置文件或环境变量读取

### P1 - 重要（扩展功能）

1. **实现 AIVideoClip 和 AITTSClip**
   - 类似 AIImageClip 的实现
   - 各自的 UIConfig
   - 特定的组件（AITTSVoiceSelector, AIAudioWaveform）

2. **完善 API 调用**
   - 图生图支持（将参考图片转为 base64）
   - 错误重试机制
   - 更详细的进度信息

3. **TimelineModel 集成**
   - 添加创建 AI Clip 的逻辑

### P2 - 优化（锦上添花）

1. **性能优化**
   - 并发生成支持
   - 内存管理优化

2. **用户体验**
   - 批量生成
   - 历史记录
   - 参数预设模板

---

## 文件清单

### 已创建的文件

**Models (AI Clip)**:
- ✅ src/timeline/models/ai/aigeneratedclip.h
- ✅ src/timeline/models/ai/aigeneratedclip.cpp
- ✅ src/timeline/models/ai/aiimageclip.h
- ✅ src/timeline/models/ai/aiimageclip.cpp
- ✅ src/timeline/models/ai/aigenerationtask.h
- ✅ src/timeline/models/ai/aigenerationtask.cpp
- ✅ src/timeline/models/ai/airesourcecache.h
- ✅ src/timeline/models/ai/airesourcecache.cpp

**Components (编辑面板)**:
- ✅ src/timeline/controller/components/AIGenerationControlsComponent.h
- ✅ src/timeline/controller/components/AIGenerationControlsComponent.cpp

**Modified Files**:
- ✅ src/timeline/models/TimelineDef.h

### 待创建的文件

**QML UI**:
- ⏳ src/ui/property/ai/AIGenerationControlsProperty.qml（可选，系统会自动处理）

**Assets**:
- ⏳ assets/ai_image_placeholder.png（可选）

**Build Configuration**:
- ⏳ CMakeLists.txt 修改（必须）

---

## 使用流程

### 1. 创建 AIImageClip

```cpp
SkyResourceBean bean("", "AI Generated Image", AIImageResource);
AIImageClip* aiClip = new AIImageClip(bean, nullptr);
```

### 2. 配置参数

通过编辑面板配置：
- 选择模型
- 输入提示词
- 选择图片尺寸
- 调整其他参数

### 3. 生成图片

点击 "Generate" 按钮：
1. 组件收集所有参数
2. 调用 `AIImageClip::startGeneration(params)`
3. 创建 `AIImageGenerationTask`
4. 调用 LiteLLM API
5. 解析 base64 响应
6. 保存图片
7. 更新 Clip 资源

### 4. 状态监听

- Idle → Preparing → Generating → Completed
- 实时进度更新（0.0 - 1.0）
- 错误信息显示

---

## API 配置

### 当前配置

```cpp
// 在 aigenerationtask.cpp 中
QNetworkRequest request(QUrl("http://litellm.test.bloomeverybody.work/v1/images/generations"));
request.setRawHeader("Authorization", "Bearer XXX");  // TODO: 从配置读取
```

### 建议改进

创建配置类：
```cpp
class AIConfig {
public:
    static QString getApiEndpoint();
    static QString getApiKey();
    static QString getDefaultModel();
};
```

---

## 缓存机制

### 缓存目录
```
~/.cache/VOO/AIGenerated/
├── cache_index.json          # 缓存索引
├── ai_image_xxx.png          # 生成的图片
└── ...
```

### 缓存策略
- 基于参数 SHA256 哈希作为 Key
- 相同参数自动命中缓存
- 支持按时间清理（默认 7 天）
- 支持按大小清理

---

## 调试建议

### 1. 检查网络请求
```cpp
qDebug() << "Sending request to LiteLLM:" << requestBody;
```

### 2. 检查响应数据
```cpp
qDebug() << "Received response, size:" << responseData.size() << "bytes";
```

### 3. 检查状态变化
```cpp
qDebug() << "Generation state changed to:" << state;
```

### 4. 查看缓存
```cpp
AIResourceCache::instance()->getCacheCount();
AIResourceCache::instance()->getTotalCacheSize();
```

---

## 常见问题

### Q: 编译错误 "QObject file not found"
A: 需要在 CMakeLists.txt 中添加 AI 相关文件。

### Q: 生成失败 "Network error"
A: 检查 API 端点和 API Key 是否正确。

### Q: 图片无法显示
A: 检查 base64 解码是否成功，文件是否正确保存。

### Q: 缓存不生效
A: 确保参数完全相同（包括顺序和格式）。

---

## 下一步计划

1. 完成编译配置（CMakeLists.txt）
2. 集成测试 AIImageClip
3. 实现 AIVideoClip
4. 实现 AITTSClip
5. 完善 UI 和用户体验

---

更新时间：2025-12-25
