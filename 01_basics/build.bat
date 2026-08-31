@echo off
setlocal
cd /D "%~dp0"

if not exist "..\mverse.exe" (
  echo ..\mverse.exe was not found.
  exit /b 1
)

where clang-cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
  echo clang-cl was not found in PATH.
  exit /b 1
)

if exist build rd /s /q build

echo [mverse] expanding main.c
..\mverse.exe --expand --target basics.exe main.c
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo [clang-cl] compiling and linking the generated C
clang-cl /nologo /DDEBUG /Zi /Od /FC build\main.c /Fobuild\main.obj /Fe:basics.exe /Fd:basics.pdb /link /DEBUG:FULL /INCREMENTAL:NO /PDB:basics.pdb > build\clang.log 2>&1
if %ERRORLEVEL% neq 0 (
  ..\mverse.exe --remap-diagnostics --target basics.exe build\clang.log
  exit /b 1
)

basics.exe
