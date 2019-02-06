DRIVE=$(stat -c %m $0)
ALL=${DRIVE}/common
exists() {
	if [ -d $1 ]; then return 1
	elif [ -L $1 ]; then return 1
	elif [ -f $1 ]; then return 1
	fi
	return 0
}
missing() {
	if ! exists $1; then return 1
	fi
	return 0
}

chkdir() {
	if missing $1; then return $(mkdir $1)
	fi
	return 0
}

prep_gcc() {
	local f="-Wall -Wextra -Werror"
	local p=""
	if [ "$1" = "64" ]; then
		ARCH=amd64
	elif [ "$1" = "32" ]; then
		ARCH=i386
	fi
	if [ "$2" = "mgw" ]; then
		if [ "$3" = "Windows" ]; then
			f="${f} -mconsole -mwindows -mwin32"
		fi
		if [ "$1" = "64" ]; then
			p="x86_64-w64-mingw32-"
			LFLAGS="-m64"
			CFLAGS="-m64"
		elif [ "$1" = "32" ]; then
			p="i386-w64-mingw32-"
			LFLAGS="-m32"
			CFLAGS="-m32"
		fi
	fi
	LD="${p}gcc"
	CC="${p}gcc"
	CXX="${p}g++"
	LFLAGS="${f} ${LFLAGS}"
	CFLAGS="${f} ${CFLAGS}"
	O_DIR="${p}gcc"
}
prep_gcc 64
compile() {
	local c="${CC} ${CFLAGS} -o \"$2\" -c \"$3\""
	echo "$1: $c"
	$c
}
compile_obj() {
	
	# Add object to list
	OBJS="${OBJS} $1"
	# Check if we should compile the object
	if [ $(missing $1) ]; then
		compile "Missing object" $1 $2
		return 0
	fi
	# Force skip
	if [ "$3" = "false" ]; then return 0
	fi
	# Force Compile
	if [ "$3" = "true" ]; then
		compile "Forced" $1 $2
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
	echo Object: $d vs Source: $s
	if [ "$d" -ne "$s" ]; then
		compile "Modified" $1 $2
		return 0
	fi
	return 0
}
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

#PRJ_DIR=$(pwd)/
SRC_DIR=${PRJ_DIR}
OUT_DIR="${ALL}/lu"
OBJ_DIR=${PRJ_DIR}objs
chkdir "${OBJ_DIR}"
OBJ_DIR="${OBJ_DIR}/${O_DIR}"

LUA_DIR="${ALL}/lua-5.3.5"
LUA_INC="${LUA_DIR}/src"
OBJ_LUA_DIR="${OBJ_DIR}/lua"
CPATH=$(add_path "${LUA_INC}" "${CPATH}")
LPATH=$(add_path "${LUA_DIR}" "${LPATH}")
paths

echo Compiling Lu Objects
chkdir "${OBJ_DIR}"
chknam(){
	n=$(basename $1)
	compile_obj "${OBJ_DIR}/$n.o" $1 true
}
for i in "${SRC_DIR}"*.c
do
	chknam "$i"
done
unset chknam

echo Compiling Lua Objects
chkdir "${OBJ_LUA_DIR}"
chknam(){
	n=$(basename $1)
	if [ "$n" = "lua.c" ]; then return
	elif [ "$n" = "luac.c" ]; then return
	else compile_obj "${OBJ_LUA_DIR}/$n.o" $1 false
	fi
}
for i in "${LUA_DIR}/src/"*.c
do
	chknam "$i"
done
unset chknam

echo Compiling Lu Executable
chkdir "${OUT_DIR}"
${LD} ${LFLAGS} -o "${OUT_DIR}/lu${OUT_SFX}" ${OBJS}
