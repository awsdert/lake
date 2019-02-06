DRIVE=$(stat -c %m $0)
ALL=${DRIVE}/common
exists() {
	if [ -d $1 ]; then return 0
	elif [ -L $1 ]; then return 0
	elif [ -f $1 ]; then return 0
	fi
	return 1
}

chkdir() {
	if ! exists $1; then return $(mkdir $1)
	fi
	return 0
}

prep_gcc() {
	local f="-Wall -Wextra -Wno-unused-parameter"
	local p=""
	if [ "$2" = "64" ]; then
		ARCH=amd64
	elif [ "$2" = "32" ]; then
		ARCH=i386
	fi
	if [ "$1" = "mgw" ]; then
		if [ "$3" = "Windows" ]; then
			f="${f} -mconsole -mwindows -mwin32"
		fi
		if [ "$2" = "64" ]; then
			p="x86_64-w64-mingw32-"
			LFLAGS="-m64"
			CFLAGS="-m64"
		elif [ "$2" = "32" ]; then
			p="i386-w64-mingw32-"
			LFLAGS="-m32"
			CFLAGS="-m32"
		fi
	fi
	if [ "$1" = "clang" ]; then
		LD="clang"
		CC="clang"
		CXX="clang++"
		O_DIR="clang"
	else
		LD="${p}gcc"
		CC="${p}gcc"
		CXX="${p}g++"
		O_DIR="${p}gcc"
	fi
	LFLAGS="${f} ${LFLAGS}"
	CFLAGS="-ansi ${f} ${CFLAGS}"
}
prep_gcc "clang" 64
paths() {
	echo CPATH=${CPATH}
	echo LPATH=${LPATH}
	export LIBRARY_PATH=${LPATH}
	export LD_INCLUDE_PATH=${LPATH}
	export LIBRARY_INCLUDE_PATH=${LPATH}
	export C_INCLUDE_PATH=${CPATH}
	export CPP_INCLUDE_PATH=${CPATH}
	export OBJC_INCLUDE_PATH=${CPATH}
}
nothing() {
	return
}
add_path() {
	if [ "$2" = "" ]; then echo "$1"
	else echo "$1;$2"
	fi
}
system() {
	if uname | grep -q "Darwin"; then
		OUT_SFX=.app
	else
		OUT_SFX=
	fi
}
system

PRJ_DIR=.
SRC_DIR="${PRJ_DIR}"
OUT_DIR="${ALL}/lu"
OBJ_DIR="${PRJ_DIR}/objs"
chkdir "${OBJ_DIR}"
OBJ_DIR="${OBJ_DIR}/${O_DIR}"

LUA_DIR="${ALL}/lua-5.3.5"
LUA_INC="${LUA_DIR}/src"
SRC_LUA_DIR="${LUA_DIR}/src"
OBJ_LUA_DIR="${OBJ_DIR}/lua"
CPATH=$(add_path "${LUA_INC}" "${CPATH}")
LPATH=$(add_path "${LUA_DIR}" "${LPATH}")
paths

compile_o() {
	# Check if we should compile the object
	if ! exists $1; then
		return 0
	fi
	# Force skip
	if [ "$3" = "false" ]; then
		return 1
	fi
	# Force Compile
	if [ "$3" = "true" ]; then
		return 0
	fi
	# Check if up to date
	local d=""
	local s=""
	if uname | grep -q "Darwin"; then
		d=$(stat -f %m $1)
		s=$(stat -f %m $2)
	else
		d=$(stat -c %Y $1)
		s=$(stat -c %Y $2)
	fi
	if [ "$d" -ne "$s" ]; then
		return 0
	fi
	return 1
}
chknam(){
	if [ "$1" = "lua.c" ]; then return 1
	elif [ "$1" = "luac.c" ]; then return 1
	fi
	return $(compile_o "$2" "$3" "$4")
}
compile_c() {
	local c="${CC} ${CFLAGS}"
	echo "$1"
	chkdir "$2"
	for i in "$3/"*.c
	do
		n=$(basename "$i")
		o="$2/$n.o"
		if chknam "$n" "$o" "$i" "$4"; then
			# Add object to list
			OBJS="${OBJS} $o"
			# Let Console see the command
			echo $c -o "$o" -c "$i"
			# Actually execute the command
			$c -o "$o" -c "$i"
		fi
	done
}

compile_c "Compiling Lu Objects" "${OBJ_DIR}" "${SRC_DIR}" true
compile_c "Compiling Lua Objects" "${OBJ_LUA_DIR}" "${SRC_LUA_DIR}"

echo Compiling Lu Executable
chkdir "${OUT_DIR}"
${LD} ${LFLAGS} -lm -o "${OUT_DIR}/lu${OUT_SFX}" ${OBJS}
