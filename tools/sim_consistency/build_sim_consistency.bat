@echo off
setlocal

set OUT=sim_consistency.exe
set INCLUDES=-I. -I..\..\apps\watch\pet_shared\include -I..\..\apps\watch\pet2d_scene
set SOURCES=sim_consistency_main.c sim_consistency_replay.c sim_consistency_golden.c ..\..\apps\watch\pet2d_scene\pet2d_mvp_a_renderer_contract.c

if not "%CC%"=="" goto use_cc
where clang >nul 2>nul
if %ERRORLEVEL%==0 (
    set CC=clang
    goto use_cc
)
where gcc >nul 2>nul
if %ERRORLEVEL%==0 (
    set CC=gcc
    goto use_cc
)
where cl >nul 2>nul
if %ERRORLEVEL%==0 (
    set CC=cl
    goto use_cl
)

echo No host C compiler found. Install clang, gcc or MSVC cl, or set CC.
exit /b 2

:use_cc
%CC% -std=c99 -Wall -Wextra %INCLUDES% %SOURCES% -o %OUT%
exit /b %ERRORLEVEL%

:use_cl
cl /nologo /W3 %INCLUDES% %SOURCES% /Fe:%OUT%
exit /b %ERRORLEVEL%
