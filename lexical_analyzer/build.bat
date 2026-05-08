@echo off
setlocal

set ROOT=%~dp0
set WFB=%ROOT%..\WinFlexBison

if not exist "%WFB%\win_flex.exe" (
    echo Cannot find win_flex.exe at: %WFB%
    exit /b 1
)

echo [1/2] Generating lexer source with win_flex...
"%WFB%\win_flex.exe" -o "%ROOT%lexer.yy.c" "%ROOT%lexer.l"
if errorlevel 1 exit /b 1

echo [2/2] Trying to build executable...
where gcc >nul 2>nul
if not errorlevel 1 (
    gcc -Wall -Wextra -o "%ROOT%lexer.exe" "%ROOT%lexer.yy.c"
    if errorlevel 1 exit /b 1
    echo Build succeeded: lexer.exe
    exit /b 0
)

where cl >nul 2>nul
if not errorlevel 1 (
    cl /nologo /W3 /Fe:"%ROOT%lexer.exe" "%ROOT%lexer.yy.c"
    if errorlevel 1 exit /b 1
    echo Build succeeded: lexer.exe
    exit /b 0
)

echo Code generation completed, but no C compiler found.
echo Generated file: lexer.yy.c
exit /b 0
