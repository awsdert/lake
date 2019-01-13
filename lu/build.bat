@echo off
call :tmp_env
set ALL=%~d0\Common
call :chk_system
set LUA_DIR=O:\Common\lua-5.3.5
set CPATH=%CPATH%;%LUA_DIR%\src
set LFLAGS=-ansi %LFLAGS%
set CFLAGS=-ansi %CFLAGS%
call :paths
set OBJS=
set OBJ_LUA_DIR=%OBJ_DIR%\lua
set OUT_DIR=%ALL%\lu
if %ABITS% == 64 set OUT_DIR=%OUT_DIR%\lu64
set OUT_EXE=%OUT_DIR%\lu%SBITS%.exe

@echo on
@echo Compiling Lu Objects
@echo off
call :chk_dir "%OBJ_DIR%"
for %%i in (.\*.c) do call :compile_obj "%OBJ_DIR%\%%~nxi.o" "%%i" true

@echo on
@echo Compiling Lua Objects
@echo off
call :chk_dir "%OBJ_LUA_DIR%"
for %%i in (%LUA_DIR%\src\*.c) do (
if %%~nxi == lua.c ( call :nothing
) else (
if %%~nxi == luac.c ( call :nothing
) else call :compile_obj "%OBJ_LUA_DIR%\%%~nxi.o" "%%i" true
)
)

@echo on
@echo Compiling Lu Executable
@echo off
call :chk_dir "%OUT_DIR%"
@echo on
%LD% %LFLAGS% -o "%OUT_DIR%\lu%SBITS%.exe" %OBJS%
@echo PATH=%PATH%
@echo CPATH=%CPATH%
@echo LIBRARY_PATH=%LIBRARY_PATH%
@echo off
call :restore_env
exit

:chk_dir
if exist %1 goto :eof
@echo on
mkdir %1
@echo off
goto :eof

:compile_obj
@echo off
:: Add object to list
set OBJS=%OBJS% %1
:: Check if we should compile the object
if not exist %1 goto :compile_obj_sub
:: Force skip
if "%3" == "false" goto :eof
:: Force Compile
if "%3" == "true" goto :compile_obj_sub
:: Check if up to date
if "%~t1" == "%~t2" goto :eof
:compile_obj_sub
@echo on
::rem @echo Object: %~t1 vs Source: %~t2
%CC% %CFLAGS% -o %1 -c %2
@echo off
goto :eof

:system32
set SBITS=32
set ABITS=32
set OBJ_DIR=.\obj32
set SYSTEM32=%WINDIR%\System32
call :mgw32
goto :eof

:system6432
set SBITS=64
set ABITS=32
set OBJ_DIR=.\obj32
set SYSTEM32=%WINDIR%\SysWOW64
call :mgw32
goto :eof

:system64
set SBITS=64
set OBJ_DIR=.\obj64
set SYSTEM32=%WINDIR%\System32
call :mgw64
goto :eof

:paths
set LIBRARY_PATH=%LPATH%
set LD_INCLUDE_PATH=%LPATH%
set LIBRARY_INCLUDE_PATH=%LPATH%
set C_INCLUDE_PATH=%CPATH%
set CPP_INCLUDE_PATH=%CPATH%
set OBJC_INCLUDE_PATH=%CPATH%
goto :eof

:mgw
set X_DISTRO=nuwen
set LD=gcc
set CC=gcc
set CC_DIR=%ALL%\MinGW
set PATH=%SYSTEM32%\Wbem;%WINDIR%;%SYSTEM32%
if exist "%CC_DIR%\git" set PATH=%CC_DIR%\git\bin;%PATH%
set PATH=%CC_DIR%\bin;%PATH%
set CPATH=%CC_DIR%\include;%CC_DIR%\include\freetype
set LPATH=%CC_DIR%\lib;%PATH%
set _FLAGS=-Wall -Werror -mconsole -mwindows -mwin32
set LFLAGS=%_FLAGS% %LFLAGS%
set CFLAGS=%_FLAGS% %CFLAGS%
call :paths
goto :eof

:mgw32
set ARCH=amd64
set LFLAGS=-m32
set CFLAGS=-m32
call :mgw
goto :eof

:mgw64
set ARCH=i386
set LFLAGS=-m64
set CFLAGS=-m64
call :mgw
goto :eof

:lcc
set CC=lcc
set LD=lcclnk
set CPATH=%LCC_DIR%\include
set LPATH=%LCC_DIR%\lib
set CFLAGS=-g2 -A
set LFLAGS=-subsystem console
call :paths
goto :eof

:lcc32
set LCC_DIR=%ALL%\LCC32
call :lcc
goto :eof

:lcc64
set LCC_DIR=%ALL%\LCC64
call :lcc
goto :eof

:tmp_env
set TTMP=%TMP%
set TTEMP=%TEMP%
set TPATH=%PATH%
set TMP=%~d0\Temp
set TEMP=%TMP%
set PATH=
call :chk_dir %TMP%
goto :eof

:restore_env
set TMP=%TTMP%
set TEMP=%TTEMP%
set PATH=%TPATH%
goto :eof

:chk_system
if "%WINDIR%" == "" set WINDIR=%SYSTEMROOT%
if "%PROCESSOR_ARCHITECTURE%" == "x86" (
	if not defined PROCESSOR_ARCHITEW6432 (
		call :system32
	) else (
		if "%PROCESSOR_ARCHITEW6432%" == "AMD64" (
			call :system6432
		) else (
			call :system32
		)
	)
) else call :system64
goto :eof

:nothing
:eof
