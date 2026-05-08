# Syntax Analyzer Demo (WinFlexBison)

This demo shows how to implement syntax analysis with automatic code generation using **win_bison + win_flex**.

## Files

- `parser.y`: Grammar rules for syntax analysis
- `lexer.l`: Token rules for the parser
- `build.bat`: One-click generation and build script

## Supported Grammar

- Declarations with comma-separated declarators
- Assignment expressions
- Compound statements
- `if / else`
- `while`
- `for`
- `return / break / continue`
- Arithmetic, relational, equality, logical, and unary expressions

The grammar is organized in a course-design style with separate expression levels, which keeps the Bison grammar conflict-free in the current build.

## Syntax Tree Output

- Syntax tree output after successful parsing
- Friendlier error messages with line number and nearby token information

## Build

Run in `syntax_analyzer` directory:

```bat
build.bat
```

## Run

If `syntax.exe` is built:

```bat
syntax.exe
```

You can also pass a source file directly:

```bat
syntax.exe input_normal.txt
```

If your editor saves files with a UTF-8 BOM, the scanner already skips it.

Then input a program fragment, for example:

```c
int x = 1;
while (x < 10) {
    x = x + 1;
}
```

If the grammar is valid, the program prints:

```text
Syntax analysis succeeded.
Syntax tree:
Program
  ...
```

If errors are found, the parser reports the line number and nearby token, then attempts to continue parsing where possible.
