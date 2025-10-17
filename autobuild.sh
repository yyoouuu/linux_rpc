#!/bin/bash

set -e

# 获取当前工作目录
CURRENT_DIR=$(pwd)

# 清理构建目录
rm -rf "$CURRENT_DIR/build/*"

# 创建构建目录（如果它不存在的话）
mkdir -p "$CURRENT_DIR/build"

# 构建项目
cd "$CURRENT_DIR/build" &&
    cmake .. &&
    make

# 返回上级目录
cd "$CURRENT_DIR"

# 复制 include 目录到 lib
cp -r "$CURRENT_DIR/src/include" "$CURRENT_DIR/lib"
