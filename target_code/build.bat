@echo off
setlocal

set ROOT=%~dp0

gcc -std=c99 -Wall -Wextra -DSYNTAX_NO_MAIN -I"%ROOT%..\semantic_analyzer" -I"%ROOT%..\syntax_analyzer" ^
    "%ROOT%main.c" "%ROOT%target_code.c" "%ROOT%..\syntax_analyzer\parser.tab.c" "%ROOT%..\syntax_analyzer\lexer.yy.c" ^
    -o "%ROOT%target_code.exe"
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build succeeded.