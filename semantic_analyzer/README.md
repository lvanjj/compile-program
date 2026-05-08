# 语义分析器

这是编译器课程设计中的**语义分析阶段**的独立实现。

## 功能

### 1. 符号表管理
- 符号表建立与管理
- 多级作用域支持（嵌套块作用域）
- 符号查找和插入

### 2. 类型检查
- 类型推导
- 类型兼容性检查
- 支持的类型：int, float, char, void

### 3. 语义检查
- 变量重复声明检查
- 未声明使用检查
- 表达式类型检查
- 赋值类型兼容性检查

## 编译

```bash
cd semantic_analyzer
build.bat
```

## 运行

```bash
semantic.exe 1  # 测试用例1
semantic.exe 2  # 测试用例2
semantic.exe 3  # 测试用例3
semantic.exe 4  # 测试用例4
```

## 文件说明

- `semantic_analyzer.h` - 头文件
- `semantic_analyzer.c` - 实现文件
- `main.c` - 主程序和测试
- `build.bat` - 编译脚本
