@echo off
setlocal

set ROOT=%~dp0

echo [1/1] Building semantic analyzer...
where gcc >nul 2>nul
if not errorlevel 1 (
    gcc -Wall -Wextra -o "%ROOT%semantic.exe" "%ROOT%semantic_analyzer.c" "%ROOT%main.c"
    if errorlevel 1 exit /b 1
    echo Build succeeded: semantic.exe
    echo.
    echo Run with:
    echo   semantic.exe 1  - Test case 1 (simple declaration and assignment)
    echo   semantic.exe 2  - Test case 2 (nested scopes)
    echo   semantic.exe 3  - Test case 3 (undeclared identifier)
    echo   semantic.exe 4  - Test case 4 (type mixing)
    exit /b 0
)

where cl >nul 2>nul
if not errorlevel 1 (
    cl /nologo /W3 /Fe:"%ROOT%semantic.exe" "%ROOT%semantic_analyzer.c" "%ROOT%main.c"
    if errorlevel 1 exit /b 1
    echo Build succeeded: semantic.exe
    echo.
    echo Run with:
    echo   semantic.exe 1  - Test case 1 (simple declaration and assignment)
    echo   semantic.exe 2  - Test case 2 (nested scopes)
    echo   semantic.exe 3  - Test case 3 (undeclared identifier)
    echo   semantic.exe 4  - Test case 4 (type mixing)
    exit /b 0
)

echo Code generation completed, but no C compiler found.
exit /b 1
