#include "stdafx.h"
#include "lua.hpp"
#include "expat.h"

#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>


#pragma comment(lib,"lua.lib")

typedef struct lxp_userdata {
	XML_Parser parser;       //关联的Expat解析器
	lua_State *L;
} lxp_userdata;

//创建XML解析器对象的函数
static void f_StartElement(void *ud,
	const char *name,
	const char **atts);
static void f_CharData(void *ud, const char *s, int len);
static void f_EndElement(void *ud, const char *name);

static int lxp_make_parser(lua_State *L) {
	XML_Parser p;

	//（1）创建解析器对象
	lxp_userdata *xpu = (lxp_userdata *)lua_newuserdata(L, sizeof(lxp_userdata));

	//预先初始化以防止错误发生
	xpu->parser = NULL;

	//设置元表
	luaL_getmetatable(L, "Expat");
	lua_setmetatable(L, -2);

	//（2）创建Expat解析器
	p = xpu->parser = XML_ParserCreate(NULL);
	if (!p)
		luaL_error(L, "XML_ParserCreate failed");

	//（3）检查并保存回调函数表
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_pushvalue(L, 1);   //回调函数表入栈
	lua_setuservalue(L, -2);   //将回调函数表设为用户数据的值

	//（4）设置Expat解析器
	XML_SetUserData(p, xpu);
	XML_SetElementHandler(p, f_StartElement, f_EndElement);
	XML_SetCharacterDataHandler(p, f_CharData);
	return 1;
}

// 解析XML片段的函数
static int lxp_parse(lua_State *L)
{
	int status;
	size_t len;
	const char *s;
	lxp_userdata *xpu;

	//获取并检查第一个参数（应该是一个解析器）
	xpu = (lxp_userdata *)luaL_checkudata(L, 1, "Expat");

	//检查解析器是否已经被关闭
	luaL_argcheck(L, xpu->parser != NULL, 1, "parser is closed");

	//获取第二个参数（一个字符串）
	s = luaL_optlstring(L, 2, NULL, &len);

	//将回调函数表放在栈索引为3的位置
	lua_settop(L, 2);
	lua_getuservalue(L, 1); // 将和用户数据相关联的回调函数压栈

	xpu->L = L;    //设置Lua状态

	//调用Expat解析器字符串,封装带有属性的节点容易出错,原因未知
	status = XML_Parse(xpu->parser, s, (int)len, s == NULL);

	//返回错误的代码
	lua_pushboolean(L, status);
	return 1;
}

// 字符数据事件的处理函数
static void f_CharData(void *ud, const char *s, int len)
{
	lxp_userdata *xpu = (lxp_userdata *)ud;
	lua_State *L = xpu->L;

	//从回调函数表中获得处理函数
	lua_getfield(L, 3, "CharacterData");
	if (lua_isnil(L, -1)) {   //没有处理函数
		lua_pop(L, 1);
		return;
	}

	lua_pushvalue(L, 1);   //解析器压栈
	lua_pushlstring(L, s, len);  //压入字符数据
	lua_call(L, 2, 0);    //调用处理函数
}

//结束元素事件的处理函数
static void f_EndElement(void *ud, const char *name)
{
	lxp_userdata *xpu = (lxp_userdata *)ud;
	lua_State *L = xpu->L;

	lua_getfield(L, 3, "EndElement");
	if (lua_isnil(L, -1)) {  //没有处理函数
		lua_pop(L, 1);
		return;
	}

	lua_pushvalue(L, 1);  //解析器压栈
	lua_pushstring(L, name);  //压入标签名
	lua_call(L, 2, 0);    //调用处理函数
}

//开始元素事件的处理函数
static void f_StartElement(void *ud,
	const char *name,
	const char **atts) {
	lxp_userdata *xpu = (lxp_userdata *)ud;
	lua_State *L = xpu->L;

	lua_getfield(L, 3, "StartElement");
	if (lua_isnil(L, -1)) {  //没有处理函数
		lua_pop(L, 1);
		return;
	}

	lua_pushvalue(L, 1);   //解析器压栈('self')
	lua_pushstring(L, name);  //压入标签名

	//创建并填充属性表
	lua_newtable(L);
	for (; *atts; atts += 2) {
		lua_pushstring(L, *(atts + 1));
		lua_setfield(L, -2, *atts);  //table[*atts] = *(atts + 1)
	}

	lua_call(L, 3, 0);   //调用处理函数
}

//关闭XML解析器的方法
static int lxp_close(lua_State *L) {
	lxp_userdata *xpu = (lxp_userdata *)luaL_checkudata(L, 1, "Expat");

	//释放Expat解析器（如果没有）
	if (xpu->parser)
		XML_ParserFree(xpu->parser);
	xpu->parser = NULL;
	return 0;
}

//lxp库的初始化代码
static const struct luaL_Reg lxp_meths[] = {
	{"parse", lxp_parse},
    {"close", lxp_close},
    {"__gc", lxp_close},
    {NULL, NULL}
};

static const struct luaL_Reg lxp_funcs[] = {
	{"new", lxp_make_parser},
    {NULL, NULL}
};

extern "C" __declspec(dllexport)
int luaopen_lxp(lua_State *L) {
	//创建元表
	luaL_newmetatable(L, "Expat");

	//metatable.__index = metatable
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	//注册方法
	luaL_setfuncs(L, lxp_meths, 0);

	//注册
	luaL_newlib(L, lxp_funcs);
	return 1;
}
