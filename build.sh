#!/bin/bash

echo "rpc 一键编译开始..."

# 如果 build 目录存在，则删除
if [ -d "build" ]; then
    echo "发现 build 目录，正在删除..."
    rm -rf build
fi

# 创建 build 并进入
mkdir build
cd build

# 运行 CMake
echo "正在执行 cmake .."
cmake ..

# 编译
echo "正在执行 make -j$(nproc)"
make -j$(nproc)

echo "编译完成！"