#include "lu.h"
/* LuDir functions */
static int LuDir_next( lua_State *L ) {
	LuDir_t *d = (LuDir_t*)luaL_checkudata(L,1,LUDIR_META);
	if ( !d ) lu_returnnil( L, 1, "WTF!? We have no object!" );
	else if ( (d->ent = readdir(d->dir)) == NULL )
		return lu_returnnil( L, 1, NULL );
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
static int LuDir_gc( lua_State *L ) {
	LuDir_t *d = (LuDir_t*)luaL_checkudata(L,1,LUDIR_META);
	if ( d ) {
		if ( d->dir ) closedir( d->dir );
		if ( d->path ) free( d->path );
		memset( d, 0, sizeof(LuDir_t) );
	}
	return 0;
}
static int LuDir_tostring( lua_State *L ) {
	char addr[ADDR_BIT] = {0};
	LuDir_t *d = (LuDir_t*)luaL_checkudata(L,1,LUDIR_META);
	if ( !d ) return lu_returnstring( L, 1, NULL, "LuDir Object");
	sprintf(addr,"%p",d);
	if ( d->dir )
		lua_pushfstring( L, "LuDir: 0x%s; \"%s\"", addr, d->path );
	else lua_pushfstring( L, "LuDir: 0x%s", addr );
	return 1;
}
static const luaL_Reg LuDirIndex[] = {
	{ "next", LuDir_next },
	{ "shut", LuDir_gc },
{ NULL } };
static int LuDir_index( lua_State *L ) {
	LuDir_t *d = (LuDir_t*)luaL_checkudata(L,1,LUDIR_META);
	int type = lua_type(L,2);
	const char *strkey = NULL;
	int i = 0;
	if ( !d ) return lu_returnnil(L,1,"WTF!? How did we get an empty object on indexing!?");
	if ( type == LUA_TSTRING ) {
		strkey = lua_tostring(L,2);
		while ( LuDirIndex[i].func ) {
			if ( strcmp( strkey, LuDirIndex[i].name ) == 0 ) {
				lua_pushcfunction( L, LuDirIndex[i].func );
				return 1;
			} ++i;
		}
		if ( strcmp( strkey, "path" ) == 0 ) lua_pushstring( L, d->path ? d->path : "" );
		else {
			printf( "LuDir_index(LUDIR[%s]) Invalid key!", strkey );
			lua_pushnil(L);
		}
	} else {
		switch ( type ) {
#if 0
		case LUA_TINTEGER:
			printf("LUDIR[%d] Invalid key!", lua_tointeger(L,2) );
			break;
#endif
		case LUA_TNUMBER:
			printf("LUDIR[%lg] Invalid key!", lua_tonumber(L,2) );
			break;
		default:
			puts("LuDir_index(LUDIR) Invalid key!");
		}
		lua_pushnil( L );
		lua_error( L );
	}
	return 1;
}
static int LuDir( lua_State *L ) {
	size_t size = 0;
	const char *path = luaL_checklstring( L, 1, &size );
	LuDir_t *d;
	lua_settop(L, 0);
	d = (LuDir_t*)lua_newuserdata(L, sizeof(LuDir_t) );
	if ( !d ) {
		lua_pushnil(L);
		return 1;
	}
	memset( d, 0, sizeof(LuDir_t) );
	luaL_getmetatable( L, LUDIR_META );
	lua_setmetatable( L, 1 );
	d->path = (char*)calloc( size, 1 );
	if ( !d->path ) {
		lua_pop(L,1);
		return lu_returnnil( L, 1, NULL );
	}
	d->dir = opendir( path );
	if ( !d->dir ) {
		free( d->path );
		return lu_returnnil( L, 1, NULL );
	}
    return 1;
}
static const luaL_Reg LuDirReg[] = {
	{ "__gc", LuDir_gc },
	{ "__tostring", LuDir_tostring },
	{ "__index", LuDir_index },
{ NULL } };
int LuDir_register( lua_State *L ) {
	int result = LuRegisterClass( L, LUDIR_META, LuDirReg );
	if ( result != 0 ) return result;
	/* register the 'LuDir' function */
	lua_pushcfunction( L, LuDir );
	lua_setglobal( L, "LuDir" );
	return 0;
}
