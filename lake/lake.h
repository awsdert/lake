#ifndef LAKE_H
#define LAKE_H
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#endif
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
extern const char LakeScript[];

int Lake_register( lua_State *L );
int LakeDir_register( lua_State *L );
int LakeFile_register( lua_State *L );
int LakeRegisterClass( lua_State *L, const char *name, const luaL_Reg *methods );

int bail_bootup( int code, bool useExit, char *msg );
int bail_script( lua_State *L, int code, bool useExit, char *msg );

char const * const lake_getstring( lua_State *L, int pos );
char const * const lake_getlstring( lua_State *L, int pos, size_t *len );
lua_Number lake_getnumber( lua_State *L, int pos );
lua_Integer lake_getinteger( lua_State *L, int pos );

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

int lake_returnnil( lua_State *L, int returncount, const char *msg );
int lake_returnstring( lua_State *L, int returncount, const char *msg, const char *val );
int lake_returnnumber( lua_State *L, int returncount, const char *msg, lua_Number val );
int lake_returninteger( lua_State *L, int returncount, const char *msg, lua_Integer val );

typedef struct _LakeFile {
	FILE *file;
	char *path;
	char buff[BUFSIZ];
} LakeFile_t;
#define LAKEFILE_META "Lake.FILE"
typedef struct _LakeDir {
	DIR *dir;
	struct dirent *ent;
	char *path;
} LakeDir_t;
#define LAKEDIR_META "Lake.meta_DIR"
#ifdef __cplusplus
}
#endif
#endif
