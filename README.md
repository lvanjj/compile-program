# 编译原理课程设计项目

本项目是一个按编译流程拆分的课程设计示例，包含词法分析、语法分析、语义分析和目标代码生成四个阶段。仓库内已经集成了 `WinFlexBison` 工具链，适合在 Windows 环境下直接构建和演示。

## 项目目标

- 使用 Flex 完成词法分析器生成
- 使用 Bison 完成语法分析器生成
- 实现基础的语义检查与符号表管理
- 基于语法树输出简单的伪汇编目标代码

## 目录结构

```text
.
├─ lexical_analyzer/      词法分析模块
├─ syntax_analyzer/       语法分析模块
├─ semantic_analyzer/     语义分析模块
├─ target_code/           目标代码生成模块
├─ WinFlexBison/          WinFlex / WinBison 工具
├─ GnuWin32/              Flex 相关运行库
└─ .github/               仓库配置
```

## 环境要求

- Windows
- `gcc` 或 `cl`（任选其一）
- 命令行可执行 `.bat` 脚本

仓库已自带：

- `WinFlexBison/win_flex.exe`
- `WinFlexBison/win_bison.exe`

因此通常不需要额外安装 Flex / Bison。

## 快速开始

建议按“词法分析 -> 语法分析 -> 语义分析 -> 目标代码生成”的顺序体验。

### 1. 词法分析

进入目录并执行：

```bat
cd lexical_analyzer
build.bat
lexer.exe
```

说明：

- `build.bat` 会根据 `lexer.l` 生成 `lexer.yy.c`
- 如果系统安装了 `gcc` 或 `cl`，会继续生成 `lexer.exe`

### 2. 语法分析

进入目录并执行：

```bat
cd syntax_analyzer
build.bat
syntax.exe
```

也可以直接解析测试文件：

```bat
syntax.exe test1_success.txt
```

说明：

- `build.bat` 会先生成 `parser.tab.c / parser.tab.h`
- 再由 `lexer.l` 生成 `lexer.yy.c`
- 成功后输出 `syntax.exe`

### 3. 语义分析

进入目录并执行：

```bat
cd semantic_analyzer
build.bat
semantic.exe 1
```

支持的测试编号：

- `semantic.exe 1`：简单声明与赋值
- `semantic.exe 2`：嵌套作用域
- `semantic.exe 3`：未声明标识符
- `semantic.exe 4`：类型混合检查

测试用例位于 `semantic_analyzer/tests/`。

### 4. 目标代码生成

进入目录并执行：

```bat
cd target_code
build.bat
target_code.exe
```

也可以指定输入输出文件：

```bat
target_code.exe source.c my_output.asm
```

说明：

- 默认会生成 `target_code.asm`
- 当前输出为便于展示和调试的伪汇编格式

## 各模块说明

### `lexical_analyzer`

负责将源程序切分为记号（Token），当前支持：

- 关键字
- 标识符
- 整型常量
- 浮点常量
- 字符串和字符常量
- 运算符与分隔符
- 单行和多行注释

### `syntax_analyzer`

负责对 Token 序列进行语法分析，当前覆盖：

- 变量声明
- 赋值语句
- 复合语句
- `if / else`
- `while`
- `for`
- `return / break / continue`
- 算术、关系、相等、逻辑和一元表达式

成功解析后可输出语法树，失败时会给出带行号的错误信息。

### `semantic_analyzer`

负责语义检查，当前重点包括：

- 符号表建立与查询
- 多层作用域管理
- 重复声明检查
- 未声明标识符检查
- 表达式类型检查
- 赋值类型兼容性检查

### `target_code`

负责把前端分析结果转换成伪汇编形式的目标代码，便于课程设计展示与后续扩展。当前已覆盖：

- 声明与初始化
- 表达式计算
- 条件判断
- 循环控制
- `return / break / continue`

## 重要文件

- `lexical_analyzer/lexer.l`：词法规则
- `syntax_analyzer/parser.y`：语法规则
- `syntax_analyzer/lexer.l`：语法分析阶段使用的词法规则
- `semantic_analyzer/semantic_analyzer.c`：语义分析实现
- `target_code/target_code.c`：目标代码生成实现

## 现有文档导航

- `lexical_analyzer/README.md`
- `syntax_analyzer/README.md`
- `syntax_analyzer/测试执行指南.md`
- `syntax_analyzer/实验报告材料.md`
- `semantic_analyzer/COMPLETION_REPORT.md`
- `target_code/README.md`

## 后续可完善方向

- 统一各子模块 README 的中文说明风格
- 增加项目级测试样例说明
- 补充编译流程图和模块调用关系图
- 将伪汇编继续扩展为更具体的目标机指令
