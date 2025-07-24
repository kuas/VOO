for framework in *.framework; do
    framework_name=$(basename "$framework" .framework)
    main_lib="$framework/$framework_name"
    
    # 跳过已经正常的framework
    if [[ "$framework_name" == "Protobuf" ]]; then
        echo "跳过 $framework (已正常)"
        continue
    fi
    
    # 检查是否为ASCII文本(说明符号链接破损)
    if file "$main_lib" | grep -q "ASCII text"; then
        echo "修复 $framework..."
        
        # 修复Versions/Current符号链接
        cd "$framework/Versions"
        if [[ -f "Current" ]]; then
            rm Current
        fi
        ln -s A Current
        cd ..
        
        # 修复主库文件符号链接
        if [[ -f "$framework_name" ]]; then
            rm "$framework_name"
        fi
        ln -s "Versions/Current/$framework_name" "$framework_name"
        
        cd ..
        
        # 验证修复结果
        echo "验证: $(file "$main_lib" | cut -d: -f2)"
    else
        echo "$framework 无需修复"
    fi
    echo
done