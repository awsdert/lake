#include "lu.h"
/* Single line for quick copy into Geany command parameter
O:/Common/lu32/lu32
O:/Common/lu64/lu64
O:/Common/MinGW\bin/gcc" -Wall -mconsole -mwindows -m32 -I "O:/Common/lua64-53/include" "O:/Common/lua64-53/lua53.dll" lu.c -o "O:/Common/lu32/lu32.exe"
O:/Common/MinGW\bin/gcc" -Wall -mconsole -mwindows -m64 -I "O:/Common/lua64-53/include" "O:/Common/lua64-53/lua53.dll" lu.c -o "O:/Common/lu64/lu64.exe"
*/

int bail_bootup( int code, bool useExit, char *msg ) {
	if ( code == 0 ) return 0;
	printf( "%s %d\n", msg, code );
	if ( useExit ) exit( code );
	return code;
}

int bail_script( lua_State *L, int code, bool useExit, char *msg ) {
	if ( code == 0 ) return 0;
	printf( "%s %d\n", msg, code );
	puts( lua_tostring( L, -1 ) );
	luaL_traceback(L, L, NULL, 1);
	puts( lua_tostring( L, -1 ) );
	lua_close(L);
	if ( useExit ) exit(code);
	return code;
}

int l_argc = 0;
char **l_argv = NULL;

/* Was gonna use the argparse project but that didn't
 * support on each instance as far I could tell */
typedef int (*option_cb)( lua_State *L, char *opt, char *val );
typedef struct _option {
	char easy;
	char *hard;
	option_cb func;
	char *help;
} option_t;
static char *l_luafile = "lu.c";
#define exists( path ) (access( path, F_OK ) == 0)
#define mayreadout( path ) (access( path, R_OK ) == 0)
#define maywritein( path ) (access( path, W_OK ) == 0)
#define mayexecute( path ) (access( path, X_OK ) == 0)
static int option_file( lua_State *L, char *opt, char *val ) {
	if ( !exists( val ) || !mayreadout( val ) )
		return bail_bootup( errno, 0, "File Error, errno =");
	l_luafile = val;
	return 0;
}
static int option_execute( lua_State *L, char *opt, char *val ) {
	return bail_script( L, luaL_dostring( L, val ), 0, "Lua Error =" );
}
#define ENV_NAME_MAX 255
#define ENV_TEXT_MAX (ENV_NAME_MAX+1+LINE_MAX)
static int option_define( lua_State *L, char *opt, char *val ) {
	char key[ENV_NAME_MAX] = "";
	char one[5] = "'1'";
	char *dec = key;
	char *def = strchr( val, '=' );
	size_t len = 0;
	if ( !def ) {
		dec = val;
		def = one;
	}
	else {
		len = ((uintptr_t)def) - ((uintptr_t)val);
		if ( len > ENV_NAME_MAX ) return ENAMETOOLONG;
		memcpy( key, val, len );
		++def;
	}
	/* Also set via Lua itself */
	lua_pushstring( L, def );
	lua_setglobal( L, dec );
	return 0;
}
static int option_undefine( lua_State *L, char *opt, char *val ) {
	/* Also unset in Lua */
	lua_pushnil( L );
	lua_setglobal( L, val );
	return bail_bootup( errno, 0, "Name Error, errno =" );
}
static option_t options[] = {
	{ 'f', "file", option_file },
	{ 'e', "execute", option_execute },
	{ 'D', "define", option_define },
	{ 'U', "undefined", option_undefine },
{0} };
int parseoptions( lua_State *L, int argc, char *argv[] ) {
	int a = 0, p;
	char buff[BUFSIZ] = {0};
	char *arg, *val, buf[] = "-?";
	size_t len;
	option_t option;
	for ( a = 1; a < argc; ++a ) {
		arg = argv[a];
		if ( *arg == 0 ) break;
		val = strchr( arg, '=' );
		len = val ? (size_t)((uintptr_t)val - (uintptr_t)arg) : strlen(arg);
		if ( len >= BUFSIZ ) {
			puts("Option name too long!");
			exit(1);
			return 1;
		}
		memcpy( buff, arg, len );
		buff[len] = 0;
		val = (!val && (a+1)<argc) ? argv[++a] : val;
		for ( p = 0; options[p].hard; ++p ) {
			option = options[1];
			buf[1] = option.easy;
			if ( strcmp( buff, buf ) == 0 ||
				strcmp( buff, option.hard ) == 0
			) {
				if ( !val ) {
					puts( option.help ? option.help : "Invalid Option!" );
					puts( buff );
					exit(1);
					return 1;
				}
				if ( !option.func ) {
					puts( "Option not supported!" );
					puts( buff );
					exit(1);
					return 1;
				}
				option.func( L, buff, val );
			}
		}
	}
	return 0;
}

int main( int argc, char *argv[] ) {
	int result = 0;
	/* Provide interface via an object */
	lua_State *L = luaL_newstate();
	if ( !L ) return bail_bootup( 1, false, "Could not boot Lua!" );
	l_argc = argc;
	l_argv = argv;
	if ( (result = bail_bootup(
		parseoptions( L, argc, argv ), false,
		"Couldn't parse all options!" )) != 0 )
		return 1;
	luaL_openlibs(L);
#if defined( _DEBUG ) || defined( DEBUG )
	lua_opendebug(L);
#endif
	result = bail_script(L, Lu_register(L), true, "Failed to create Lu object");
	if ( result != 0 ) goto main_fail;
	result = bail_script(L, LuDir_register(L), true, "Failed to create LuDir object");
	if ( result != 0 ) goto main_fail;
	result = bail_script(L, LuFile_register(L), true, "Failed to create LuFile object");
	if ( result != 0 ) goto main_fail;
	/* Basic stuff makefile should not need to do */
	if ( (result = luaL_dostring( L, LuScript )) != 0 )
		return bail_script( L, result, false, "Failed to prepare wildcard and co" );
	/* Get on with makefile now */
	if ( (result = luaL_loadfile(L,"lu.lua")) != 0 )
		return bail_script( L, result, true, "Failed to load makefile.lua" );
	if ( (result = lua_pcall(L,0,0,0)) != 0 )
		return bail_script( L, result, true, "Failed to run makefile.lua" );
	main_fail:
	lua_close(L);
	return result;
}

char const * const lu_getstring( lua_State *L, int pos ) {
	if ( lua_isstring(L,pos) )
		return lua_tostring(L,pos);
	return NULL;
}
char const * const lu_getlstring( lua_State *L, int pos, size_t *len ) {
	if ( lua_isstring(L,pos) )
		return luaL_tolstring(L,pos,len);
	if ( len ) *len = 0;
	return NULL;
}
lua_Number lu_getnumber( lua_State *L, int pos ) {
	if ( lua_isnumber(L,pos) )
		return lua_tonumber(L,pos);
	return 0;
}
lua_Integer lu_getinteger( lua_State *L, int pos ) {
	if ( lua_isinteger(L,pos) )
		return lua_tointeger(L,pos);
	return 0;
}
bool lu_getboolean( lua_State *L, int pos ) {
	if ( lua_isboolean(L,pos) )
		return lua_toboolean(L,pos);
	return false;
}
int Lu_tostring( lua_State *L ) {
	lua_pushstring( L, "Lu{}" );
	return 1;
}
typedef struct _LU_T_SIZE {
	size_t size;
	char const * const type;
} LU_T_SIZE_t;

LU_T_SIZE_t lutypes[] = {
	{ sizeof(void*), "*" },
	{ 1, "char" },
	{ sizeof(short), "short" },
	{ sizeof(int), "int" },
	{ sizeof(long), "long" },
	{ sizeof(size_t), "size_t" },
	{ 0, NULL }
};

int LuLaunchArg( lua_State *L ) {
	int a = lu_getinteger(L,1);
	if ( a >= l_argc ) return 0;
	lua_pushstring(L,l_argv[a]);
	return 1;
}
int LuLaunchPath( lua_State *L ) {
#ifdef _WIN32
	HMODULE hModule = GetModuleHandleW(NULL);
	CHAR path[BUFSIZ];
	GetModuleFileNameA(hModule, path, BUFSIZ);
	lua_pushstring(L, path );
#else
	lua_pushnil(L);
#endif
	return 1;
}
int LuStrCmp( lua_State *L ) {
	const char *str1 = luaL_checkstring( L, 1 );
	const char *str2 = luaL_checkstring( L, 2 );
	if ( !str1 ) lua_pushinteger( L, str2 ? -1 : 0 );
	else if ( !str2 ) lua_pushinteger( L, 1 );
	else lua_pushinteger( L, strcmp( str1, str2 ) );
	return 1;
}

int LuSizeof( lua_State *L ) {
	char const * const type = lu_getstring( L, 1 );
	size_t size = 0;
	int i = 0;
	for ( ; lutypes[i].size; ++i ) {
		if ( strcmp( type, lutypes[i].type ) == 0 ) {
			size = lutypes[i].size;
			break;
		}
	}
	lua_pushinteger( L, size );
	return 1;
}
int LuBitsof( lua_State *L ) {
	char const * const type = lu_getstring( L, 1 );
	size_t size = 0;
	int i = 0;
	for ( ; lutypes[i].size; ++i ) {
		if ( strcmp( type, lutypes[i].type ) == 0 ) {
			size = lutypes[i].size * CHAR_BIT;
			break;
		}
	}
	lua_pushinteger( L, size );
	return 1;
}
int LuBaseName( lua_State *L ) {
	size_t len = 0;
	char const * const path = lu_getlstring( L, 1, &len );
	char *next = calloc( 1, len );
	char *pos = NULL;
	if ( !next ) return 0;
#ifdef _WIN32
	_splitpath( path, NULL, NULL, next, NULL );
#else
	memcpy( next, path, len );
	basename( next );
#endif
	if ( (pos = strrchr(next, '.')) )
		*pos = '\0';
	lua_pushstring(L,next);
	free( next );
	return 1;
}
int LuDirName( lua_State *L ) {
	size_t len = 0;
	char const * const path = lu_getlstring( L, 1, &len );
	char *next = calloc( 1, len );
	if ( !next ) return 0;
	memcpy( next, path, len );
#ifdef _WIN32
	_splitpath( path, NULL, next, NULL, NULL );
#else
	dirname( next );
#endif
	lua_pushstring(L,next);
	free( next );
	return 1;
}
int LuMountRoot( lua_State *L ) {
	/* Longest possible mount root */
	char root[LINE_MAX] = {0};
	getcwd( root, LINE_MAX );
#ifdef _WIN32
	char D = (strchr( root, ':' );
	*(++D) = 0;
	lua_pushstring(L,root);
#else
	struct stat m = {0};
	do {
		if ( stat( root, &m ) == 0 ) {
			break;
		}
		if ( !(S_ISDIR(m.st_mode)) ) {
			lua_pushstring( L, root );
			return 1;
		}
	} while ( dirname( root ) );
	lua_pushnil(L);
#endif
	return 1;
}
int LuAccess( lua_State *L ) {
	char const * const path = lu_getstring( L, 1 );
	int perm = (int)lu_getnumber( L, 2 );
	lua_pushinteger( L, access( path, perm ) );
	return 1;
}
int LuSystem( lua_State *L ) {
	char const * const command = lu_getstring( L, 1 );
	lua_pushinteger( L, system( command ) );
	return 1;
}
int Lu_Stat( lua_State *L, struct stat data, int code ) {
	/* First parameter is what people are
	 * interested in so give table there */
	if ( code == 0 ) {
		lua_createtable(L,2,0);
		luat_setintegerfield( L, "st_dev", data.st_dev );
		luat_setintegerfield( L, "st_ino", data.st_ino );
		luat_setintegerfield( L, "st_mode", data.st_mode );
		luat_setintegerfield( L, "st_nlink", data.st_nlink );
		luat_setintegerfield( L, "st_uid", data.st_uid );
		luat_setintegerfield( L, "st_gid", data.st_gid );
		luat_setintegerfield( L, "st_rdev", data.st_rdev );
		luat_setintegerfield( L, "st_size", data.st_size );
		luat_setintegerfield( L, "st_atime", data.st_atime );
		luat_setintegerfield( L, "st_mtime", data.st_mtime );
		luat_setintegerfield( L, "st_ctime", data.st_ctime );
	}
	else lua_pushnil(L);
	lua_pushinteger(L,code);
	return 2;
}
int LuStat( lua_State *L ) {
	char const * const path = lu_getstring(L,1);
	struct stat data;
	int code = path ? stat( path, &data ) : EINVAL;
	return Lu_Stat( L, data, code );
}
#ifndef _WIN32
/*  modified from https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c */
int cp(const char *src, const char *dst, bool overwrite)
{
	int fd_src, fd_dst;
	char buf[BUFSIZ] = {0};
	ssize_t bytes;
	int code = 0;
	fd_src = open( src, O_RDONLY );
	if ( fd_src < 0 ) return errno;
	fd_dst = creat( dst, 0777 );
	if ( fd_dst < 0 ) {
		close( fd_src );
		return errno;
	}
	while ( (bytes = read( fd_src, buf, BUFSIZ )) > 0 )
	{
		if ( write( fd_dst, buf, bytes ) != bytes )
			goto copy_fail;
	}
	if (bytes == 0)
	{
		code = (close(fd_dst) < 0) ? errno : 0;
		close(fd_src);
		/* Success! */
		return code;
	}
	copy_fail:
	close( fd_src );
	close( fd_dst );
	return errno;
}
#endif
int LuCopy( lua_State *L ) {
	char const * const src = lua_tostring(L,1);
	char const * const dst = lua_tostring(L,2);
	bool overwrite = lu_getboolean(L,3);
	int code = 0;
#ifdef _WIN32
	if ( CopyFileA( src, dst, overwrite ? TRUE : FALSE ) == TRUE )
		goto LuCopy_done;
	switch ( GetLastError() ) {
	case ERROR_PATH_NOT_FOUND:
	case ERROR_FILE_NOT_FOUND: code = ENOENT; break;
	default: code = 1;
	}
	LuCopy_done:
#else
	code = cp( src, dst, overwrite );
#endif
	lua_pushinteger( L, code );
	return 1;
}
int LuMkDir( lua_State *L ) {
	char const * const path = lu_getstring(L,1);
	int code = 1;
	if ( path ) code = mkdir(path,0777);
	lua_pushinteger(L,code);
	return 1;
}
#ifdef _WIN32
int execve( const char *cmd, const char *argv[], const char *envp[] ) {
	int result = 0;
	char *cmdl = NULL;
	size_t cmdl_len = 0;
	size_t i = 1;
	size_t pos = 0;
	size_t len;
	while ( argv[i] && argv[i][0] ) {
		cmdl_len += strlen( argv[i++] ) + 1;
	}
	if ( cmdl_len ) {
		cmdl = calloc( cmdl_len + 1, 1 );
		if ( !cmdl ) return ENOMEM;
		for ( i = 1; argv[i] && argv[i][0]; ++i ) {
			memcpy( &(cmdl[pos]), argv[i], (len = strlen(argv[i]) + 1));
			pos += len;
		}
	}
	result = CreateProcessA( cmd, cmdl, NULL, NULL, 0, 0, (void*)envp, NULL, NULL, NULL );
	if ( cmdl ) free( cmdl );
	return result;
}
#endif
typedef struct STRVEC {
	size_t count;
	size_t node_size;
	size_t list_size;
	size_t buff_size;
	size_t full_size;
	char *buff;
	char **list;
} STRVEC_t;
int LuExecve( lua_State *L ) {
	size_t i = 0;
	STRVEC_t arg = {0}, env = {0};
	const char *path = NULL;
	lua_settop(L, 1);
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield( L, 1, "exec" ); /* 3 */
	lua_getfield( L, 1, "argv" ); /* 2 */
	lua_getfield( L, 1, "envp" ); /* 1 */
	path = luaL_checkstring( L, 3 );
	arg.count = lua_rawlen( L, 2 ) + 1;
	arg.node_size = PATH_MAX + 1;
	arg.list_size = (arg.count+1) * sizeof(char*);
	arg.buff_size = (arg.node_size * arg.count);
	arg.full_size = arg.buff_size + arg.list_size;
	env.count = lua_rawlen( L, 1 ) + 1;
	env.node_size = (PATH_MAX*2) + 2;
	env.list_size = (env.count+1) * sizeof(char*);
	env.buff_size = (env.node_size * env.count);
	env.full_size = (env.buff_size + env.list_size);
	/* Wanna make sure there is no segv fault */
	size_t size = arg.full_size + env.full_size + 2;
	char *buff = calloc( size, 1 );
	if ( !buff ) {
		lua_pushnil( L );
		return 1;
	}
	env.buff = &(buff[arg.full_size]);
	env.list = (char**)(&(env.buff[env.buff_size]));
	lua_gettable( L, 3 );
	for ( i = 0; i < env.count; ++i ) {
		strncpy( env.list[i], luaL_checkstring( L, -2 - i ), env.node_size );
		puts( env.list[i] );
	}
	arg.buff = buff;
	arg.list = (char**)(&(buff[arg.buff_size]));
	arg.list[0] = buff;
	strncpy( arg.list[0], path, arg.node_size );
	lua_gettable( L, 2 );
	for ( i = 1; i < arg.count; ++i ) {
		strncpy( arg.list[i], luaL_checkstring( L, -1 - i ), arg.node_size );
		puts( arg.list[i] );
	}
	puts( "execvp() is about to be tried" );
	lua_pushinteger( L, execve( path, arg.list, env.list ) );
	free(buff);
	return 1;
}

int LuGetCwd( lua_State *L ) {
	char root[BUFSIZ] = {0};
	getcwd(root,BUFSIZ);
	lua_pushstring( L, root );
	return 1;
}
static const luaL_Reg LuReg[] = {
	{ "__tostring", Lu_tostring },
	{ "sizeof", LuSizeof },
	{ "bitsof", LuBitsof },
	{ "launcharg", LuLaunchArg },
	{ "launchpath", LuLaunchPath },
	{ "mountroot", LuMountRoot },
	{ "access", LuAccess },
	{ "system", LuSystem },
	{ "execve", LuExecve },
	{ "dirname", LuDirName },
	{ "basename", LuBaseName },
	{ "getcwd", LuGetCwd },
	{ "mkdir", LuMkDir },
	{ "strcmp", LuStrCmp },
	{ "stat", LuStat },
	{ "copy", LuCopy },
	{ NULL }
};

int Lu_register( lua_State *L ) {
	int r = LuRegisterClass( L, "Lu", LuReg );
	if ( r != 0 ) return r;
#ifdef _WIN32
	(void)luaL_dostring( L, "Lu.dirsep = '\\\\'" );
#else
	(void)luaL_dostring( L, "Lu.dirsep = '/'" );
#endif
    return 0;
}
