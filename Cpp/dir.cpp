#include "stdafx.h"
#include "lua.hpp"

#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#pragma comment(lib,"lua.lib")

//����������ǰ������
static int dir_iter(lua_State *L);

static int l_dir(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	//����һ������DIR�ṹ����û�����
	//������û����ݱ������һ��'ָ��DIR���ͽṹ���ָ��'

	DIR **d = (DIR**)lua_newuserdata(L, sizeof(DIR*));

	//Ԥ�ȳ�ʼ��
	*d = NULL;

	//����Ԫ��
	luaL_getmetatable(L, "LuaBook.dir");
	lua_setmetatable(L, -2);

	*d = opendir(path);
	if (*d == NULL)
		luaL_error(L, "cannot open %s: %s", path, strerror(errno));

	//���������ص����������ú���Ψһ����ֵ��������Ŀ¼���û����ݱ������ջ��
	lua_pushcclosure(L, dir_iter, 1);

	return 1;
}

//Ŀ¼�ĵ�������
static int dir_iter(lua_State *L)
{
	DIR *d = *(DIR **)lua_touserdata(L, lua_upvalueindex(1));
	struct dirent *entry = readdir(d);
	if (entry != NULL) {
		lua_pushstring(L, entry->d_name);
		return 1;
	}
	else return 0;   //�������  //closedir(d)
}

static int dir_gc(lua_State *L)
{
	DIR *d = *(DIR **)lua_touserdata(L, 1);
	if (d) closedir(d);
	return 0;
}

static const struct luaL_Reg dirlib[] = {
	{"open", l_dir},
    {NULL, NULL}
};

extern "C" __declspec(dllexport)
int luaopen_dir(lua_State *L)
{
	luaL_newmetatable(L, "LuaBook,dir");

	//����__gc�ֶ�
	lua_pushcfunction(L, dir_gc);
	lua_setfield(L, -2, "__gc");

	//������
	luaL_newlib(L, dirlib);

	return 1;
}