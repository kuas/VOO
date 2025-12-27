# AI Clip 编译和测试指南

## ✅ CMakeLists.txt 已更新

已完成以下修改：
1. ✅ 添加 Qt Network 模块到 `find_package`（第 119 行）
2. ✅ 添加 Qt Network 到 `target_link_libraries`（第 343 行）
3. ✅ AI 相关的 .cpp 文件会被 `GLOB_RECURSE` 自动包含（第 155-160 行）

---

## ✅ 编译已完成

**编译状态**: ✅ 成功
**构建时间**: 2025-12-27
**构建目录**: `build/unknown-Debug`
**可执行文件**: `build/unknown-Debug/VOO.app/Contents/MacOS/VOO` (6.2MB)

所有 AI Clip 相关代码已成功编译：
- ✅ AIGeneratedClip 基类
- ✅ AIImageClip 图片生成类
- ✅ AIGenerationTask 任务系统
- ✅ AIResourceCache 缓存管理
- ✅ AIGenerationControlsComponent UI 组件

**已修复的编译问题**:
1. ✅ 添加了 `Q_DECLARE_METATYPE(AIGenerationState)` 注册枚举类型
2. ✅ 修复了 SkyVariant API 调用（使用直接构造函数而非 make* 方法）
3. ✅ 修复了 QJsonObject::value() 调用（使用 contains() + [] 访问）
4. ✅ 添加了 QFile 头文件

---

## 🔧 编译前的准备

### 1. 配置 API Key（必须）

**位置**: `src/timeline/models/ai/aigenerationtask.cpp:107`

**当前代码**:
```cpp
request.setRawHeader("Authorization", "Bearer XXX");  // TODO: 从配置读取 API Key
```

**需要修改为**:
```cpp
request.setRawHeader("Authorization", "Bearer 你的真实API Key");
```

**推荐方案**：创建配置文件
```cpp
// 方案 A: 从环境变量读取
QString apiKey = qgetenv("VOO_AI_API_KEY");
if (apiKey.isEmpty()) {
    apiKey = "sk-xxxx";  // 默认 Key
}
request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

// 方案 B: 从配置文件读取
QSettings settings("VOO", "AIConfig");
QString apiKey = settings.value("api_key", "sk-xxxx").toString();
request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
```

### 2. 验证文件完整性

运行以下命令检查所有 AI 文件是否存在：

```bash
# 检查 AI Models
ls -la src/timeline/models/ai/
# 应该看到 8 个文件：
# aigeneratedclip.h/cpp
# aiimageclip.h/cpp
# aigenerationtask.h/cpp
# airesourcecache.h/cpp

# 检查 AI Components
ls -la src/timeline/controller/components/AIGenerationControlsComponent.*
# 应该看到 2 个文件：
# AIGenerationControlsComponent.h/cpp

# 检查修改的文件
grep -n "AIImageResource" src/timeline/models/TimelineDef.h
# 应该看到第 120 行：AIImageResource = 10

grep -n "AIGenerationControls" src/timeline/controller/components/ComponentBean.h
# 应该看到第 52 行和第 76 行
```

---

## 🏗️ 编译步骤

### macOS 编译

```bash
cd /Users/huwei/Code/github/VOO

# 1. 清理旧的构建（如果需要）
rm -rf build/unknown-Debug

# 2. 创建构建目录
mkdir -p build/unknown-Debug
cd build/unknown-Debug

# 3. 配置 CMake
cmake ../..

# 4. 编译
make -j$(sysctl -n hw.ncpu)

# 或者使用 ninja（如果安装了）
# ninja
```

### 常见编译问题

#### 问题 1: Qt Network 模块找不到
```
CMake Error: Could NOT find Qt5Network
```

**解决方案**：
```bash
# 检查 Qt 安装
brew list qt@5

# 确保 QT_DIR 环境变量正确
export QT_DIR="/usr/local/Cellar/qt@5/5.15.16_2/lib/cmake/Qt5"
```

#### 问题 2: 找不到 AI 头文件
```
fatal error: 'aigeneratedclip.h' file not found
```

**解决方案**：检查文件路径是否正确，CMake 应该自动包含 `src` 目录。

#### 问题 3: QNetworkAccessManager 未定义
```
use of undeclared identifier 'QNetworkAccessManager'
```

**解决方案**：确保 CMakeLists.txt 已添加 Qt::Network 模块。

---

## 🧪 测试 AI 功能

### 1. 运行应用

```bash
# macOS
./build/unknown-Debug/VOO.app/Contents/MacOS/VOO

# 或者
open ./build/unknown-Debug/VOO.app
```

### 2. 创建 AI Clip 测试

在代码中添加测试：

```cpp
// 在某个合适的位置（如 TimelineModel 或测试代码中）
void testAIImageClip() {
    // 1. 创建 AIImageClip
    SkyResourceBean bean("", "AI Test Image", AIImageResource);
    AIImageClip* aiClip = new AIImageClip(bean, nullptr);

    // 2. 配置生成参数
    QJsonObject params;
    params["ai_model"] = "gemini-3-pro-image-preview";
    params["ai_prompt"] = "一只可爱的猫咪在海边玩耍";
    params["ai_image_size"] = "1024x1024";
    params["ai_style_preset"] = "photorealistic";
    params["ai_steps"] = 30;
    params["ai_seed"] = -1;

    // 3. 启动生成
    bool success = aiClip->startGeneration(params);
    qDebug() << "Generation started:" << success;

    // 4. 监听完成信号
    connect(aiClip, &AIImageClip::generationCompleted, [](const QString &path) {
        qDebug() << "Image generated successfully:" << path;
    });

    connect(aiClip, &AIImageClip::generationFailed, [](const QString &error) {
        qWarning() << "Generation failed:" << error;
    });
}
```

### 3. 检查日志输出

运行应用后，查看控制台输出：

```
AIGeneratedClip created: 0x...
AIImageClip created: 0x...
AIGeneratedClip::startGeneration {...}
AIImageGenerationTask::executeTask {...}
Sending request to LiteLLM: {"model":"gemini-3-pro-image-preview","prompt":"..."}
Received response, size: XXXX bytes
Image saved to: /var/folders/.../ai_image_xxx.png
AIGeneratedClip::applyGeneratedResource /var/folders/.../ai_image_xxx.png
AIImageClip resource updated successfully
```

### 4. 验证缓存功能

```bash
# 查看缓存目录
ls -la ~/Library/Caches/VOO/AIGenerated/

# 应该看到：
# cache_index.json - 缓存索引
# ai_image_xxx.png - 生成的图片
```

### 5. 测试不同场景

#### 测试 A: 文生图
```cpp
params["ai_prompt"] = "一片美丽的樱花林";
params["ai_reference_image"] = "";  // 不设置参考图
```

#### 测试 B: 不同模型
```cpp
params["ai_model"] = "vertex_ai/gemini-2.5-flash-image";
```

#### 测试 C: 不同尺寸
```cpp
params["ai_image_size"] = "1792x1024";  // 横屏
```

#### 测试 D: 缓存命中
```cpp
// 使用相同参数再次生成，应该立即返回缓存结果
aiClip->startGeneration(params);  // 第二次应该很快
```

#### 测试 E: 取消生成
```cpp
aiClip->startGeneration(params);
QTimer::singleShot(1000, [aiClip]() {
    aiClip->cancelGeneration();  // 1秒后取消
});
```

---

## 📊 性能测试

### 1. 测试生成时间

```cpp
QElapsedTimer timer;
timer.start();

connect(aiClip, &AIImageClip::generationCompleted, [&timer](const QString &path) {
    qint64 elapsed = timer.elapsed();
    qDebug() << "Generation took:" << elapsed << "ms";
});
```

### 2. 测试缓存效果

```cpp
// 第一次生成（无缓存）
QElapsedTimer timer1;
timer1.start();
aiClip->startGeneration(params);
// 记录时间...

// 第二次生成（有缓存）
QElapsedTimer timer2;
timer2.start();
aiClip->startGeneration(params);  // 应该立即返回
// 对比时间...
```

### 3. 测试内存使用

```bash
# macOS
top -pid $(pgrep VOO)

# 或者使用 Instruments
instruments -t Leaks -D trace.trace VOO.app
```

---

## 🐛 调试技巧

### 1. 启用详细日志

所有 AI 相关代码都包含了 qDebug 输出，编译时使用 Debug 模式：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ../..
```

### 2. 使用断点

在以下关键位置设置断点：
- `AIGeneratedClip::startGeneration()` - 生成开始
- `AIImageGenerationTask::executeTask()` - API 调用
- `AIImageGenerationTask::onNetworkReplyFinished()` - 响应处理
- `AIGeneratedClip::onTaskCompleted()` - 完成处理

### 3. 检查网络请求

使用工具抓包：
```bash
# macOS - 使用 Charles Proxy 或 Wireshark
# 监听 http://litellm.test.bloomeverybody.work
```

### 4. 验证 JSON 格式

```cpp
// 在 AIImageGenerationTask::executeTask() 中添加
qDebug() << "Request JSON:" << QJsonDocument(requestBody).toJson(QJsonDocument::Indented);

// 在 onNetworkReplyFinished() 中添加
qDebug() << "Response JSON:" << QString::fromUtf8(responseData);
```

---

## ✅ 验收清单

编译和基本测试完成后，检查以下项目：

- [ ] 编译成功，无错误和警告
- [ ] 应用可以正常启动
- [ ] 创建 AIImageClip 不崩溃
- [ ] 可以配置生成参数
- [ ] API 调用成功（检查网络日志）
- [ ] base64 图片正确解码
- [ ] 图片保存到临时目录
- [ ] Clip 资源正确更新
- [ ] 缓存机制工作正常
- [ ] 第二次相同参数使用缓存
- [ ] 状态变化正确（Idle → Generating → Completed）
- [ ] 进度更新正常（0.0 → 1.0）
- [ ] 错误处理正常（网络错误、API 错误等）
- [ ] 取消功能正常
- [ ] 重试功能正常
- [ ] 项目保存/加载后 AI Clip 仍可用

---

## 🚀 下一步

编译测试成功后：

1. **集成到 TimelineModel**
   - 添加创建 AI Clip 的菜单或按钮
   - 实现从资源面板添加 AI Clip

2. **完善 UI**
   - 创建 AIGenerationControlsProperty.qml
   - 美化编辑面板

3. **实现其他 AI Clip**
   - AIVideoClip（视频生成）
   - AITTSClip（语音合成）

4. **优化用户体验**
   - 添加批量生成
   - 添加历史记录
   - 添加参数预设

---

## 📞 遇到问题？

1. 检查所有文件是否正确创建
2. 检查 API Key 是否配置
3. 检查网络连接
4. 查看详细的 qDebug 日志
5. 参考 AI_CLIP_IMPLEMENTATION_SUMMARY.md

---

**更新时间**: 2025-12-25
**编译环境**: macOS (Intel x86_64), Qt 5.15.16, C++11
