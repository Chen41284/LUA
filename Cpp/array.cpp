#include "stdafx.h"
#include "lua.hpp"

#include <limits.h>

#pragma comment(lib,"lua.lib")

#define BITS_PER_WORD (CHAR_BIT * sizeof(unsigned int))   //一个无符号整型数的位数
#define I_WORD(i) ((unsigned int)(i) / BITS_PER_WORD)     //根据指定的索引来计算存放相应比特位的字
#define I_BIT(i) (1 << ((unsigned int)(i) % BITS_PER_WORD))  //用于计算访问这个字中相应比特位要用的掩码

#define checkarray(L) (BitArray *)luaL_checkudata(L, 1, "LuaBook.array")

//使用以下的结构体来表示布尔数组
typedef struct BitArray {
	int size;
	unsigned int values[1];   //可变部分
} BitArray;



//创建布尔数组,注意n不能太大，防止数组越界
static int newarray(lua_State *L) {
	int i;
	size_t nbytes;
	BitArray *a;

	int n = (int)luaL_checkinteger(L, 1);   //比特位数
	luaL_argcheck(L, n >= 1, 1, "invalid size");
	nbytes = sizeof(BitArray) + I_WORD(n - 1) * sizeof(unsigned int);
	a = (BitArray *)lua_newuserdata(L, nbytes);
	a->size = n;
	for (i = 0; i <= I_WORD(n - 1); i++)
		a->values[i] = 0;    //初始化数组

	luaL_getmetatable(L, "LuaBook.array");
	lua_setmetatable(L, -2);

	return 1;    //新的用户数据在调用lua_newuserdata时已压栈
}

static unsigned int *getparams(lua_State *L, unsigned int *mask)
{
	BitArray *a = checkarray(L);
	int index = (int)luaL_checkinteger(L, 2) - 1;
	luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");

	*mask = I_BIT(index);      //访问指定比特位的掩码
	return &a->values[I_WORD(index)];  //字所在的地址
}

//设置布尔数组的值
static int setarray(lua_State *L)
{
	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	//luaL_checkany(L, 3);
	luaL_checktype(L, 3, LUA_TBOOLEAN);   //确保第三个参数位bool类型的值
	if (lua_toboolean(L, 3))
		*entry |= mask;
	else
		*entry &= ~mask;

	return 0;
}

//设置布尔数组
static int getarray(lua_State *L) {
	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	lua_pushboolean(L, *entry & mask);
	return 1;
}

//获取数组的大小
static int getsize(lua_State *L)
{
	BitArray *a = checkarray(L);
	lua_pushinteger(L, a->size);

	return 1;
}


//输出字符数组的内容
int array2string(lua_State *L)
{
	BitArray *a = checkarray(L);
	luaL_Buffer b;
	char str[80];
	sprintf(str, "array size: %d\n", a->size);
	luaL_buffinit(L, &b);
	luaL_addstring(&b, str);
	for (unsigned int i = 0; i < a->size; ++i)
	{
		unsigned int mask = I_BIT(i);  //I_BIT是访问指定字的索引
		unsigned int *tempa = &(a->values[I_WORD(i)]);    //确定索引所在的字节

		unsigned int entryA = (*tempa & mask);

		if (i % 32 == 0 && i)   // i > 0
			luaL_addstring(&b, "\n");

		if (entryA)
			luaL_addchar(&b, '1');   //true
		else
			luaL_addchar(&b, '0');   //false
	}

	luaL_pushresult(&b);

	return 1;
}

//求数组的并集
static int array_union(lua_State *L) {
	BitArray *a = (BitArray *)luaL_checkudata(L, 1, "LuaBook.array");
	BitArray *b = (BitArray *)luaL_checkudata(L, 2, "LuaBook.array");

	if (a->size != b->size)
		luaL_error(L, "array size is not equal %d, %d", a->size, b->size);

	BitArray *c = (BitArray *)lua_newuserdata(L, sizeof(a));  //创建新的数据压栈

	c->size = a->size;

	for (unsigned int i = 0; i <= I_WORD(c->size - 1); i++)
		c->values[i] = 0;    //初始化数组

	for (unsigned int i = 0; i < a->size; ++i)
	{
		unsigned int mask = I_BIT(i);  //I_BIT是访问指定字的索引
		unsigned int *tempa =  &(a->values[I_WORD(i)]);    //确定索引所在的字节
		unsigned int *tempb =  &(b->values[I_WORD(i)]);
		unsigned int *tempc =  &(c->values[I_WORD(i)]);

		unsigned int entryA = (*tempa & mask);
		unsigned int entryB = (*tempb & mask);

		if (entryA || entryB)
			*tempc |= mask;
		else 
			*tempc &= ~mask;
	}

	luaL_getmetatable(L, "LuaBook.array"); //将元表压栈
	lua_setmetatable(L, -2);

	return 1;
}

//数组的交集
static int array_intersect(lua_State *L) {
	BitArray *a = (BitArray *)luaL_checkudata(L, 1, "LuaBook.array");
	BitArray *b = (BitArray *)luaL_checkudata(L, 2, "LuaBook.array");	

	if (a->size != b->size)
		luaL_error(L, "array size is not equal %d, %d", a->size, b->size);

	BitArray *c = (BitArray *)lua_newuserdata(L, sizeof(a));

	c->size = a->size;

	for (unsigned int i = 0; i <= I_WORD(c->size - 1); i++)
		c->values[i] = 0;    //初始化数组,否组c中的数据是未定义的，可能出错

	for (unsigned int i = 0; i < a->size; ++i)
	{
		unsigned int mask = I_BIT(i);  //I_BIT是访问指定字的索引
		unsigned int *tempa = &(a->values[I_WORD(i)]);    //确定索引所在的字节
		unsigned int *tempb = &(b->values[I_WORD(i)]);
		unsigned int *tempc = &(c->values[I_WORD(i)]);

		unsigned int entryA = (*tempa & mask);
		unsigned int entryB = (*tempb & mask);

		if (entryA && entryB)
			*tempc |= mask;
		else
			*tempc &= ~mask;
	}

	luaL_getmetatable(L, "LuaBook.array"); //将元表压栈
	lua_setmetatable(L, -2);

	return 1;
}

//传统的函数方法
static const struct luaL_Reg arraylib_f [] = {
	{"new", newarray},
    {"set", setarray},
    {"get", getarray},
    {"size", getsize},
    {"union", array_union},
    {"intersect", array_intersect},
    {NULL, NULL}
};

static const struct luaL_Reg arraylib_m[] = {
	{"__tostring", array2string},
    {"__add", array_union},
    {"__mul", array_intersect},
    {NULL, NULL}
};

extern "C" __declspec(dllexport)
int luaopen_array(lua_State *L)
{
	luaL_newmetatable(L, "LuaBook.array");
	luaL_setfuncs(L, arraylib_m, 0);
	luaL_newlib(L, arraylib_f);
	return 1;
}



//面向对象的方法
/*
static const struct luaL_Reg arraylib_f [] = {
	{"new", newarray},
    {NULL, NULL}
};

static const struct luaL_Reg arraylib_m[] = {
	{"set", setarray},
    {"get", getarray},
    {"size", getsize},
    {"__tostring", array2string},
    {NULL, NULL}
};

extern "C" __declspec(dllexport)
int luaopen_array(lua_State *L) {
	luaL_newmetatable(L, "LuaBook.array");   //创建元表
	lua_pushvalue(L, -1);    //复制元表
	lua_setfield(L, -2, "__index");   //mt.__index = mt
	luaL_setfuncs(L, arraylib_m, 0);   //注册元方法
	luaL_newlib(L, arraylib_f);   //创建库
	return 1;
}*/


//数组访问的形式
/*
static const struct luaL_Reg arraylib_f[] = {
	{"new", newarray},
	{NULL, NULL}
};

static const struct luaL_Reg arraylib_m[] = {
	{"__newindex", setarray},
	{"__index", getarray},
	{"__len", getsize},
	{"__tostring", array2string},
	{NULL, NULL}
};

extern "C" __declspec(dllexport)
int luaopen_array(lua_State *L) {
	luaL_newmetatable(L, "LuaBook.array");   //创建元表
	luaL_setfuncs(L, arraylib_m, 0);   //注册元方法
	luaL_newlib(L, arraylib_f);   //创建库
	return 1;
}*/

