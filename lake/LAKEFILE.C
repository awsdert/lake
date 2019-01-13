#include "lake.h"
// LakeFile functions
static int LakeFile_gc( lua_State *L ) {
	LakeFile_t *f = (LakeFile_t*)luaL_checkudata(L,1,LAKEFILE_META);
	if ( f ) {
		if ( f->file ) fclose( f->file );
		if ( f->path ) free( f->path );
		memset( f, 0, sizeof(LakeFile_t) );
	}
	return 0;
}
static int LakeFile_tostring( lua_State *L ) {
	LakeFile_t *f = (LakeFile_t*)luaL_checkudata(L,1,LAKEFILE_META);
	char addr[ADDR_BIT] = {0};
	if ( !f ) {
		lua_pushstring(L,"LakeFile Object");
		return 1;
	}
	sprintf(addr,"%p",f);
	if ( f->path )
		lua_pushfstring( L, "LakeFile: 0x%s; \"%s\"", addr, f->path );
	else lua_pushfstring( L, "LakeFile: 0x%s", addr );
	return 1;
}

static int LakeFile_readout( lua_State *L ) {
	LakeFile_t *f = (LakeFile_t*)luaL_checkudata(L,1,LAKEFILE_META);
	size_t bytes = lake_getinteger( L, 2 );
	if ( !f ) return 0;
	bytes = (bytes > BUFSIZ) ? BUFSIZ : bytes;
	memset( f->buff, 0, BUFSIZ );
	if ( f->file )
		bytes = fread(f->buff,BUFSIZ,bytes,f->file);
	lua_pushinteger(L,bytes);
	return 1;
}
static int LakeFile_writein( lua_State *L ) {
	LakeFile_t *f = (LakeFile_t*)luaL_checkudata(L,1,LAKEFILE_META);
	size_t bytes = lake_getinteger( L, 2 );
	if ( !f ) return 0;
	bytes = (bytes > BUFSIZ) ? BUFSIZ : bytes;
	memset( f->buff, 0, BUFSIZ );
	if ( f->file )
		bytes = fwrite(f->buff,BUFSIZ,bytes,f->file);
	lua_pushinteger(L,bytes);
	return 1;
}
static const luaL_Reg LakeFileIndex[] = {
	{ "readout", LakeFile_readout },
	{ "writein", LakeFile_writein },
	{ "shut", LakeFile_gc },
{ NULL } };
static int LakeFile_index( lua_State *L ) {
	LakeFile_t *f = (LakeFile_t*)luaL_checkudata(L,1,LAKEFILE_META);
	int type = lua_type(L,2);
	const char *strkey = NULL;
	int i = 0;
	if ( !f ) return lake_returnnil(L,1,"WTF!? How did we get an empty object on indexing!?");
	if ( type == LUA_TSTRING ) {
		strkey = lua_tostring(L,2);
		while ( LakeFileIndex[i].func ) {
			if ( strcmp( strkey, LakeFileIndex[i].name ) == 0 ) {
				lua_pushcfunction( L, LakeFileIndex[i].func );
				return 1;
			} ++i;
		}
		if ( strcmp( strkey, "path" ) == 0 ) lua_pushstring( L, f->path ? f->path : "" );
		else {
			printf( "LakeFile_index(LAKEFILE[%s]) Invalid key!", strkey );
			lua_pushnil(L);
		}
	} else {
		switch ( type ) {
#if 0
		case LUA_TINTEGER:
			printf("LAKEFILE[%lld] Invalid key!", (long long int)lua_tointeger(L,2) );
			break;
#endif
		case LUA_TNUMBER:
			printf("LAKEFILE[%lg] Invalid key!", lua_tonumber(L,2) );
			break;
		default:
			puts("LakeFile_index(LAKEFILE) Invalid key!");
		}
		lua_pushnil( L );
		lua_error( L );
	}
	return 1;
}
static int LakeFile( lua_State *L ) {
	size_t size = 0;
	const char *path = luaL_checklstring( L, 1, &size );
	const char *mode = luaL_checkstring( L, 2 );
	LakeFile_t *f = (LakeFile_t*)lua_newuserdata(L, sizeof(LakeFile_t) );
	if ( !f ) {
		lua_pushnil( L );
		return 1;
	}
	memset( f, 0, sizeof(LakeFile_t) );
	f->path = (char*)calloc( size, 1 );
	if ( !f->path ) {
		lua_pushnil( L );
		return 1;
	}
	f->file = fopen( path, mode );
	if ( !f->file ) {
		free( f->path );
		f->path = NULL;
		lua_pushnil( L );
		return 1;
	}
	luaL_getmetatable( L, LAKEFILE_META );
    lua_setmetatable( L, -2 );
	lua_pushstring( L, "shut" );
	lua_pushcfunction( L, LakeFile_gc );
	lua_settable( L, -3 );
	lua_pushstring( L, "readout" );
	lua_pushcfunction( L, LakeFile_readout );
	lua_settable( L, -3 );
	lua_pushstring( L, "writein" );
	lua_pushcfunction( L, LakeFile_writein );
	lua_settable( L, -3 );
    return 1;
}
static const luaL_Reg LakeFileReg[] = {
	{ "__gc", LakeFile_gc },
	{ "__tostring", LakeFile_tostring },
	{ "__index", LakeFile_index },
	{ NULL, NULL }
};
int LakeFile_register( lua_State *L ) {
    int result = LakeRegisterClass( L, LAKEFILE_META, LakeFileReg );
	if ( result != 0 ) return result;
	/* register the 'LakeFile' function */
	lua_pushcfunction( L, LakeFile );
	lua_setglobal( L, "LakeFile" );
	return 0;
}
