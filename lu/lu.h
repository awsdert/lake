#ifndef LU_H
#define LU_H
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#ifndef LINE_MAX
#define LINE_MAX 8191
#endif

#ifndef PATH_MAX
#define PATH_MAX LINE_MAX
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern const char LuScript[];

int Lu_register( lua_State *L );
int LuDir_register( lua_State *L );
int LuFile_register( lua_State *L );
int LuRegisterClass( lua_State *L, const char *name, const luaL_Reg *methods );

int bail_bootup( int code, bool useExit, char *msg );
int bail_script( lua_State *L, int code, bool useExit, char *msg );

char const * lu_getstring( lua_State *L, int pos );
char const * lu_getlstring( lua_State *L, int pos, size_t *len );
lua_Number lu_getnumber( lua_State *L, int pos );
lua_Integer lu_getinteger( lua_State *L, int pos );
bool lu_getboolean( lua_State *L, int pos );

void luat_setcfunctionfield(
	lua_State *L, char const * const key, lua_CFunction val );
void luat_setintegerfield(
	lua_State *L, char const * const key, lua_Integer val );
void luat_setnumberfield(
	lua_State *L, char const * const key, lua_Number val );
void luat_setstringfield(
	lua_State *L, char const * const key, char const * const val );

#define ADDR_BIT (sizeof(void*) * CHAR_BIT)

#ifdef _WIN32
#define setenv(name,value,replace) ((replace==0) ? \
	((getenv(name)) ? SetEnvironmentVariable(name,value) : 1) \
	: SetEnvironmentVariable(name,value))
#define unsetenv(name) SetEnvironmentVariable(name,NULL)
#endif

int lu_returnnil( lua_State *L, int returncount, const char *msg );
int lu_returnstring( lua_State *L, int returncount, const char *msg, const char *val );
int lu_returnnumber( lua_State *L, int returncount, const char *msg, lua_Number val );
int lu_returninteger( lua_State *L, int returncount, const char *msg, lua_Integer val );

typedef struct _LuFile {
	FILE *file;
	char *path;
	char buff[BUFSIZ];
} LuFile_t;
#define LUFILE_META "Lu.FILE"
typedef struct _LuDir {
	DIR *dir;
	struct dirent *ent;
	char *path;
} LuDir_t;
#define LUDIR_META "Lu.meta_DIR"
#ifdef __cplusplus
}
#endif
#endif
