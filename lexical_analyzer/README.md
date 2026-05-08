# Lexical Analyzer Demo (WinFlexBison)

This demo shows how to implement lexical analysis and automatically generate scanner code with **win_flex**.

## Files

- `lexer.l`: Flex lexical rules
- `build.bat`: One-click generation and build script

## What it recognizes

- Keywords: `if`, `else`, `while`, `for`, `return`, `int`, `float`, `char`, `void`, `break`, `continue`, `struct`
- Identifiers
- Integer constants
- Floating-point constants
- String literals
- Character literals
- Operators and delimiters
- Single-line and block comments

## Build

Run in `lexical_analyzer` directory:

```bat
build.bat
```

The script does:

1. Generate `lexer.yy.c` from `lexer.l`
2. Try to compile `lexer.exe` with `gcc` or `cl`

## Run

If `lexer.exe` is built:

```bat
lexer.exe
```

Then type source code and press `Ctrl+Z` then `Enter` to finish.

Example input:

```c
int main() {
    float x = 12.5;
    if (x > 0) return x;
}
```

Example output:

```text
KW_INT       line 1     int
IDENTIFIER   line 1     main
DELIMITER    line 1     (
DELIMITER    line 1     )
DELIMITER    line 1     {
KW_FLOAT     line 2     float
IDENTIFIER   line 2     x
OPERATOR     line 2     =
FLOAT_CONST  line 2     12.5
DELIMITER    line 2     ;
KW_IF        line 3     if
DELIMITER    line 3     (
IDENTIFIER   line 3     x
OPERATOR     line 3     >
INT_CONST    line 3     0
DELIMITER    line 3     )
KW_RETURN    line 3     return
IDENTIFIER   line 3     x
DELIMITER    line 3     ;
DELIMITER    line 4     }
```
