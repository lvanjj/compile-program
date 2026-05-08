# 目标代码生成模块

这是编译原理课程设计里的**目标代码生成阶段**模块。

## 功能

- 先调用语法分析器生成真实的 `ASTNode` 语法树，再做目标代码生成
- 将表达式、赋值、条件语句和循环语句转换为伪目标代码
- 支持 `int` 声明、初始化、`if / else`、`while`、`for`、`return`、`break`、`continue`
- 输出可读的汇编风格文本，方便后续替换为真实 x86 或 MIPS 指令

## 输出形式

当前版本输出的是**伪汇编**，例如：

- `DECL int a`
- `MOV a, 1`
- `ADD t0, a, b`
- `CMP t0, 0`
- `JE L_else_0`

这适合课程设计展示，也方便后续继续扩展到真正的目标机器代码。

## 文件说明

- `target_code.h`：模块接口
- `target_code.c`：目标代码生成实现
- `main.c`：入口程序，读取源文件、调用语法分析器并输出目标代码
- `build.bat`：一键编译脚本

## 编译

在 `target_code` 目录下执行：

```bat
build.bat
```

## 运行

编译成功后会生成 `target_code.exe`。默认读取 `..\\semantic_analyzer\\tests\\test1.c` 并输出到 `target_code.asm`：

```bat
target_code.exe
```

也可以手动指定源文件和输出文件：

```bat
target_code.exe source.c my_output.asm
```

## 说明

这个模块复用了语义分析阶段定义的 `ASTNode` 结构，并直接链接语法分析器生成 AST，所以后续只要语法规则继续扩展，目标代码生成也会同步受益。