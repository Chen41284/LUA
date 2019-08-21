// pra.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "tuple.h"
#include <stdio.h>
#include <string.h>
#include <memory>

#pragma comment(lib,"lua.lib")


int t_tuple(lua_State *L) {
	lua_Integer op = luaL_optinteger(L, 1, 0);
	if (op == 0) {  //没有参数
		int i;
		//将每一个有效的上值压栈
		for (i = 1; !lua_isnone(L, lua_upvalueindex(i)); i++)
			lua_pushvalue(L, lua_upvalueindex(i));
		return i - 1;   //值的个数
	}
	else {  //获取字段'op'
		luaL_argcheck(L, 0 < op && op <= 256, 1, "index out of range");   //检查参数的个数
		if (lua_isnone(L, lua_upvalueindex(op)))
			return 0;
		lua_pushvalue(L, lua_upvalueindex(op));
		return 1;
	}
}

//根据传入的参数构建表（使用函数的形式调用）
int t_new(lua_State *L)
{
	int top = lua_gettop(L);
	luaL_argcheck(L, top < 256, top, "too many fields");
	lua_pushcclosure(L, t_tuple, top);
	return 1;
}

//根据传入的函数（第二个参数）过滤表中的数据（第一个参数）
int filter(lua_State *L)
{
	int i, n;

	//第一个参数必须是一张表
	luaL_checktype(L, 1, LUA_TTABLE);

	//第二个参数必须是一个函数f
	luaL_checktype(L, 2, LUA_TFUNCTION);

	n = luaL_len(L, 1);   //获取表的大小

	int j = 1;

	lua_newtable(L);   //结果表

	for (i = 1; i <= n; i++)
	{
		lua_pushvalue(L, 2);   //压入f
		lua_geti(L, 1, i);  //压入t[i]
		lua_call(L, 1, 1);  //调用f(t[i])
		int flag = lua_toboolean(L, -1);
		if (flag)
		{
			lua_pop(L, 1);
			lua_geti(L, 1, i);
			lua_seti(L, 3, j++);  //t[i] = result;
		}
		else
			lua_pop(L, 1);   //弹出栈顶的元素
		
	}

	return 1;  //返回结果表, 在栈顶
}

//将参数逆序输出
int reverse(lua_State *L)
{
	int i, n;

	//第一个参数必须是一张表
	luaL_checktype(L, 1, LUA_TTABLE);
	n = luaL_len(L, 1);   //获取表的大小
	int j = 1;
	lua_newtable(L);   //结果表

	for (i = n; i >= 1; --i)
	{
		lua_geti(L, 1, i);  //压入t[i]
		lua_seti(L, 2, j++);  //t[i] = result;
	}

	return 1;  //返回结果表, 在栈顶
}

//分割字符串
static int l_split(lua_State *L)
{
	size_t len = 0;
	const char *s = luaL_checklstring(L, 1, &len);   //目标字符串
	const char *sep = luaL_checkstring(L, 2);        //分割符
	const char *e;
	int i = 1;

	lua_newtable(L);    //结果表

	//依次处理每个分隔符
	while ((e = (char*)memchr(s, *sep, len)) != NULL) {
		lua_pushlstring(L, s, e - s);   //压入字串
		lua_rawseti(L, -2, i++);        //向表中插入
		len = len - (e - s);            //更新长度
		s = e + 1;                      
	}

	//插入最后一个字串
	lua_pushstring(L, s);
	lua_rawseti(L, -2, i);

	return 1;   //将结果表返回
}

//字符串替换
char * replace(char *str, const char *src, const char *dst, size_t *len)
{
	const char* pos = str;
	int count = 0;
	while ((pos = strstr(pos, src))) {
		count++;
		pos += strlen(src);
	}

	size_t result_len = strlen(str) + (strlen(dst) - strlen(src)) * count + 1;
	char *result = (char *)malloc(result_len);
	memset(result, 0, result_len);
	*len = result_len;

	const char *left = str;
	const char *right = NULL;

	while ((right = strstr(left, src))) {
		strncat(result, left, right - left);
		strcat(result, dst);
		right += strlen(src);
		left = right;
	}
	free(str);   //str不能是const
	strcat(result, left);
	return result;
}

//根据第二个参数中的键值映射关系，替换第一个参数中相关的字符串
//Lua中的表的形式为{["AAA" = "aaa"}，键值都需要为字符串的形式，否则转换的时候可能出错
static int transliterate(lua_State *L)
{
	size_t len = 0;
	const char* src = lua_tolstring(L, 1, &len);
	char *result = (char *)malloc(len);
	strncpy(result, src, len);
	/* 表放在索引2处 */
	lua_pushnil(L);  /* 第一个键 */
	while (lua_next(L, 2) != 0) {
		/* 使用 '键' （在索引 -2 处） 和 '值' （在索引 -1 处）*/
		const char *key = lua_tostring(L, -2);
		const char *value = lua_tostring(L, -1);
		result = replace(result, key, value, &len);
		/* 移除 '值' ；保留 '键' 做下一次迭代 */
		lua_pop(L, 1);
	}

	luaL_Buffer b;
	luaL_buffinit(L, &b);
	luaL_addlstring(&b, result, len);

	free(result);

	luaL_pushresult(&b);

	return 1;
}

static const struct luaL_Reg booklib[] = {
	{"tuple", t_new},
	{"filter", filter},
    {"reverse", reverse},
    {"split", l_split},
    {"transliterate", transliterate},
	{NULL, NULL}
};


extern "C" __declspec(dllexport)
int luaopen_book(lua_State *L)
{
	luaL_newlib(L, booklib);
	return 1;
}