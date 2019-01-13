#include "lake.h"
void luat_setnumberfield(
	lua_State *L, char const * const key, lua_Number val ) {
	lua_pushstring( L, key );
	lua_pushnumber( L, val );
	lua_settable( L, -3 );
}
void luat_setcfunctionfield(
	lua_State *L, char const * const key, lua_CFunction val ) {
	lua_pushstring( L, key );
	lua_pushcfunction( L, val );
	lua_settable( L, -3 );
}
void luat_setintegerfield(
	lua_State *L, char const * const key, lua_Integer val ) {
	lua_pushstring( L, key );
	lua_pushinteger( L, val );
	lua_settable( L, -3 );
}
void luat_setstringfield(
	lua_State *L, char const * const key, char const * const val ) {
	lua_pushstring( L, key );
	lua_pushstring( L, val );
	lua_settable( L, -3 );
}
int LakeRegisterClass( lua_State *L, const char *name, const luaL_Reg *methods ) {
#if 0
	char text[BUFSIZ] = {0};
#endif
	int i = 0;
	if ( !L || !name || !methods ) return -1;
	/* Ensure our objects exist where we expect them too */
	lua_settop(L, 0);
	/* local tmp = {{}} */
	if ( strcmp( name, "Lake" ) == 0 ) lua_newtable( L );
	else luaL_newmetatable( L, name );
	/* for i,v in pairs(methods) do */
	for ( ; methods[i].name; ++i ) {
		/* tmp[1][v.name] = v.func */
		lua_pushstring( L, methods[i].name );
		lua_pushcfunction( L, methods[i].func );
		lua_settable( L, -3 );
	}
	/* _G[name] = tmp[1] */
	lua_setglobal( L, name );
	return 0;
}
int lake_returnnil( lua_State *L, int returncount, const char *msg ) {
	if ( msg ) puts( msg );
	lua_pushnil( L );
	return returncount;
}
int lake_returnstring( lua_State *L, int returncount, const char *msg, const char *val ) {
	if ( msg ) puts( msg );
	lua_pushstring( L, val );
	return returncount;
}
int lake_returnnumber( lua_State *L, int returncount, const char *msg, lua_Number val ) {
	if ( msg ) puts( msg );
	lua_pushnumber( L, val );
	return returncount;
}
int lake_returninteger( lua_State *L, int returncount, const char *msg, lua_Integer val ) {
	if ( msg ) puts( msg );
	lua_pushinteger( L, val );
	return returncount;
}
