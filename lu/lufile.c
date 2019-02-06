#include "lu.h"
/* LuFile functions */
static int LuFile_gc( lua_State *L ) {
	LuFile_t *f = (LuFile_t*)luaL_checkudata(L,1,LUFILE_META);
	if ( f ) {
		if ( f->file ) fclose( f->file );
		if ( f->path ) free( f->path );
		memset( f, 0, sizeof(LuFile_t) );
	}
	return 0;
}
static int LuFile_tostring( lua_State *L ) {
	LuFile_t *f = (LuFile_t*)luaL_checkudata(L,1,LUFILE_META);
	char addr[ADDR_BIT] = {0};
	if ( !f ) {
		lua_pushstring(L,"LuFile Object");
		return 1;
	}
	sprintf(addr,"%p",f);
	if ( f->path )
		lua_pushfstring( L, "LuFile: 0x%s; \"%s\"", addr, f->path );
	else lua_pushfstring( L, "LuFile: 0x%s", addr );
	return 1;
}

static int LuFile_readout( lua_State *L ) {
	LuFile_t *f = (LuFile_t*)luaL_checkudata(L,1,LUFILE_META);
	size_t bytes = lu_getinteger( L, 2 );
	if ( !f ) return 0;
	bytes = (bytes > BUFSIZ) ? BUFSIZ : bytes;
	memset( f->buff, 0, BUFSIZ );
	if ( f->file )
		bytes = fread(f->buff,BUFSIZ,bytes,f->file);
	lua_pushinteger(L,bytes);
	return 1;
}
static int LuFile_writein( lua_State *L ) {
	LuFile_t *f = (LuFile_t*)luaL_checkudata(L,1,LUFILE_META);
	size_t bytes = lu_getinteger( L, 2 );
	if ( !f ) return 0;
	bytes = (bytes > BUFSIZ) ? BUFSIZ : bytes;
	memset( f->buff, 0, BUFSIZ );
	if ( f->file )
		bytes = fwrite(f->buff,BUFSIZ,bytes,f->file);
	lua_pushinteger(L,bytes);
	return 1;
}
static const luaL_Reg LuFileIndex[] = {
	{ "readout", LuFile_readout },
	{ "writein", LuFile_writein },
	{ "shut", LuFile_gc },
{ NULL, NULL } };
static int LuFile_index( lua_State *L ) {
	LuFile_t *f = (LuFile_t*)luaL_checkudata(L,1,LUFILE_META);
	int type = lua_type(L,2);
	const char *strkey = NULL;
	int i = 0;
	if ( !f ) return lu_returnnil(L,1,"WTF!? How did we get an empty object on indexing!?");
	if ( type == LUA_TSTRING ) {
		strkey = lua_tostring(L,2);
		while ( LuFileIndex[i].func ) {
			if ( strcmp( strkey, LuFileIndex[i].name ) == 0 ) {
				lua_pushcfunction( L, LuFileIndex[i].func );
				return 1;
			} ++i;
		}
		if ( strcmp( strkey, "path" ) == 0 ) lua_pushstring( L, f->path ? f->path : "" );
		else {
			printf( "LuFile_index(LUFILE[%s]) Invalid key!", strkey );
			lua_pushnil(L);
		}
	} else {
		switch ( type ) {
#if 0
		case LUA_TINTEGER:
			printf("LUFILE[%lld] Invalid key!", (long long int)lua_tointeger(L,2) );
			break;
#endif
		case LUA_TNUMBER:
			printf("LUFILE[%lg] Invalid key!", lua_tonumber(L,2) );
			break;
		default:
			puts("LuFile_index(LUFILE) Invalid key!");
		}
		lua_pushnil( L );
		lua_error( L );
	}
	return 1;
}
static int LuFile( lua_State *L ) {
	size_t size = 0;
	const char *path = luaL_checklstring( L, 1, &size );
	const char *mode = luaL_checkstring( L, 2 );
	LuFile_t *f = (LuFile_t*)lua_newuserdata(L, sizeof(LuFile_t) );
	if ( !f ) {
		lua_pushnil( L );
		return 1;
	}
	memset( f, 0, sizeof(LuFile_t) );
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
	luaL_getmetatable( L, LUFILE_META );
    lua_setmetatable( L, -2 );
	lua_pushstring( L, "shut" );
	lua_pushcfunction( L, LuFile_gc );
	lua_settable( L, -3 );
	lua_pushstring( L, "readout" );
	lua_pushcfunction( L, LuFile_readout );
	lua_settable( L, -3 );
	lua_pushstring( L, "writein" );
	lua_pushcfunction( L, LuFile_writein );
	lua_settable( L, -3 );
    return 1;
}
static const luaL_Reg LuFileReg[] = {
	{ "__gc", LuFile_gc },
	{ "__tostring", LuFile_tostring },
	{ "__index", LuFile_index },
	{ NULL, NULL }
};
int LuFile_register( lua_State *L ) {
    int result = LuRegisterClass( L, LUFILE_META, LuFileReg );
	if ( result != 0 ) return result;
	/* register the 'LuFile' function */
	lua_pushcfunction( L, LuFile );
	lua_setglobal( L, "LuFile" );
	return 0;
}
