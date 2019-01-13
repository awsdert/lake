#include "lake.h"
/* LakeDir functions */
static int LakeDir_next( lua_State *L ) {
	LakeDir_t *d = (LakeDir_t*)luaL_checkudata(L,1,LAKEDIR_META);
	if ( !d ) lake_returnnil( L, 1, "WTF!? We have no object!" );
	else if ( (d->ent = readdir(d->dir)) == NULL )
		return lake_returnnil( L, 1, NULL );
	lua_newtable( L );
	luat_setintegerfield( L, "d_ino", d->ent->d_ino );
#ifndef _WIN32
	luat_setintegerfield( L, "d_off", d->ent->d_off );
	luat_setintegerfield( L, "d_reclen", d->ent->d_reclen );
	luat_setintegerfield( L, "d_type", d->ent->d_type );
#endif
	luat_setstringfield( L, "d_name", d->ent->d_name );
	return 1;
}
static int LakeDir_gc( lua_State *L ) {
	LakeDir_t *d = (LakeDir_t*)luaL_checkudata(L,1,LAKEDIR_META);
	if ( d ) {
		if ( d->dir ) closedir( d->dir );
		if ( d->path ) free( d->path );
		memset( d, 0, sizeof(LakeDir_t) );
	}
	return 0;
}
static int LakeDir_tostring( lua_State *L ) {
	char addr[ADDR_BIT] = {0};
	LakeDir_t *d = (LakeDir_t*)luaL_checkudata(L,1,LAKEDIR_META);
	if ( !d ) return lake_returnstring( L, 1, NULL, "LakeDir Object");
	snprintf(addr, ADDR_BIT,"%p",d);
	if ( d->dir )
		lua_pushfstring( L, "LakeDir: 0x%s; \"%s\"", addr, d->path );
	else lua_pushfstring( L, "LakeDir: 0x%s", addr );
	return 1;
}
static const luaL_Reg LakeDirIndex[] = {
	{ "next", LakeDir_next },
	{ "shut", LakeDir_gc },
{ NULL } };
static int LakeDir_index( lua_State *L ) {
	LakeDir_t *d = (LakeDir_t*)luaL_checkudata(L,1,LAKEDIR_META);
	int type = lua_type(L,2);
	const char *strkey = NULL;
	int i = 0;
	if ( !d ) return lake_returnnil(L,1,"WTF!? How did we get an empty object on indexing!?");
	if ( type == LUA_TSTRING ) {
		strkey = lua_tostring(L,2);
		while ( LakeDirIndex[i].func ) {
			if ( strcmp( strkey, LakeDirIndex[i].name ) == 0 ) {
				lua_pushcfunction( L, LakeDirIndex[i].func );
				return 1;
			} ++i;
		}
		if ( strcmp( strkey, "path" ) == 0 ) lua_pushstring( L, d->path ? d->path : "" );
		else {
			printf( "LakeDir_index(LAKEDIR[%s]) Invalid key!", strkey );
			lua_pushnil(L);
		}
	} else {
		switch ( type ) {
#if 0
		case LUA_TINTEGER:
			printf("LAKEDIR[%d] Invalid key!", lua_tointeger(L,2) );
			break;
#endif
		case LUA_TNUMBER:
			printf("LAKEDIR[%lg] Invalid key!", lua_tonumber(L,2) );
			break;
		default:
			puts("LakeDir_index(LAKEDIR) Invalid key!");
		}
		lua_pushnil( L );
		lua_error( L );
	}
	return 1;
}
static int LakeDir( lua_State *L ) {
	size_t size = 0;
	const char *path = luaL_checklstring( L, 1, &size );
	LakeDir_t *d;
	lua_settop(L, 0);
	d = (LakeDir_t*)lua_newuserdata(L, sizeof(LakeDir_t) );
	if ( !d ) {
		lua_pushnil(L);
		return 1;
	}
	memset( d, 0, sizeof(LakeDir_t) );
	luaL_getmetatable( L, LAKEDIR_META );
	lua_setmetatable( L, 1 );
	d->path = (char*)calloc( size, 1 );
	if ( !d->path ) {
		lua_pop(L,1);
		return lake_returnnil( L, 1, NULL );
	}
	d->dir = opendir( path );
	if ( !d->dir ) {
		free( d->path );
		return lake_returnnil( L, 1, NULL );
	}
    return 1;
}
static const luaL_Reg LakeDirReg[] = {
	{ "__gc", LakeDir_gc },
	{ "__tostring", LakeDir_tostring },
	{ "__index", LakeDir_index },
{ NULL } };
int LakeDir_register( lua_State *L ) {
	int result = LakeRegisterClass( L, LAKEDIR_META, LakeDirReg );
	if ( result != 0 ) return result;
	/* register the 'LakeDir' function */
	lua_pushcfunction( L, LakeDir );
	lua_setglobal( L, "LakeDir" );
	return 0;
}
