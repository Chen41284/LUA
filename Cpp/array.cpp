#include "stdafx.h"
#include "lua.hpp"

#include <limits.h>

#pragma comment(lib,"lua.lib")

#define BITS_PER_WORD (CHAR_BIT * sizeof(unsigned int))   //һ���޷�����������λ��
#define I_WORD(i) ((unsigned int)(i) / BITS_PER_WORD)     //����ָ������������������Ӧ����λ����
#define I_BIT(i) (1 << ((unsigned int)(i) % BITS_PER_WORD))  //���ڼ���������������Ӧ����λҪ�õ�����

#define checkarray(L) (BitArray *)luaL_checkudata(L, 1, "LuaBook.array")

//ʹ�����µĽṹ������ʾ��������
typedef struct BitArray {
	int size;
	unsigned int values[1];   //�ɱ䲿��
} BitArray;



//������������,ע��n����̫�󣬷�ֹ����Խ��
static int newarray(lua_State *L) {
	int i;
	size_t nbytes;
	BitArray *a;

	int n = (int)luaL_checkinteger(L, 1);   //����λ��
	luaL_argcheck(L, n >= 1, 1, "invalid size");
	nbytes = sizeof(BitArray) + I_WORD(n - 1) * sizeof(unsigned int);
	a = (BitArray *)lua_newuserdata(L, nbytes);
	a->size = n;
	for (i = 0; i <= I_WORD(n - 1); i++)
		a->values[i] = 0;    //��ʼ������

	luaL_getmetatable(L, "LuaBook.array");
	lua_setmetatable(L, -2);

	return 1;    //�µ��û������ڵ���lua_newuserdataʱ��ѹջ
}

static unsigned int *getparams(lua_State *L, unsigned int *mask)
{
	BitArray *a = checkarray(L);
	int index = (int)luaL_checkinteger(L, 2) - 1;
	luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");

	*mask = I_BIT(index);      //����ָ������λ������
	return &a->values[I_WORD(index)];  //�����ڵĵ�ַ
}

//���ò��������ֵ
static int setarray(lua_State *L)
{
	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	//luaL_checkany(L, 3);
	luaL_checktype(L, 3, LUA_TBOOLEAN);   //ȷ������������λbool���͵�ֵ
	if (lua_toboolean(L, 3))
		*entry |= mask;
	else
		*entry &= ~mask;

	return 0;
}

//���ò�������
static int getarray(lua_State *L) {
	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	lua_pushboolean(L, *entry & mask);
	return 1;
}

//��ȡ����Ĵ�С
static int getsize(lua_State *L)
{
	BitArray *a = checkarray(L);
	lua_pushinteger(L, a->size);

	return 1;
}


//����ַ����������
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
		unsigned int mask = I_BIT(i);  //I_BIT�Ƿ���ָ���ֵ�����
		unsigned int *tempa = &(a->values[I_WORD(i)]);    //ȷ���������ڵ��ֽ�

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

//������Ĳ���
static int array_union(lua_State *L) {
	BitArray *a = (BitArray *)luaL_checkudata(L, 1, "LuaBook.array");
	BitArray *b = (BitArray *)luaL_checkudata(L, 2, "LuaBook.array");

	if (a->size != b->size)
		luaL_error(L, "array size is not equal %d, %d", a->size, b->size);

	BitArray *c = (BitArray *)lua_newuserdata(L, sizeof(a));  //�����µ�����ѹջ

	c->size = a->size;

	for (unsigned int i = 0; i <= I_WORD(c->size - 1); i++)
		c->values[i] = 0;    //��ʼ������

	for (unsigned int i = 0; i < a->size; ++i)
	{
		unsigned int mask = I_BIT(i);  //I_BIT�Ƿ���ָ���ֵ�����
		unsigned int *tempa =  &(a->values[I_WORD(i)]);    //ȷ���������ڵ��ֽ�
		unsigned int *tempb =  &(b->values[I_WORD(i)]);
		unsigned int *tempc =  &(c->values[I_WORD(i)]);

		unsigned int entryA = (*tempa & mask);
		unsigned int entryB = (*tempb & mask);

		if (entryA || entryB)
			*tempc |= mask;
		else 
			*tempc &= ~mask;
	}

	luaL_getmetatable(L, "LuaBook.array"); //��Ԫ��ѹջ
	lua_setmetatable(L, -2);

	return 1;
}

//����Ľ���
static int array_intersect(lua_State *L) {
	BitArray *a = (BitArray *)luaL_checkudata(L, 1, "LuaBook.array");
	BitArray *b = (BitArray *)luaL_checkudata(L, 2, "LuaBook.array");	

	if (a->size != b->size)
		luaL_error(L, "array size is not equal %d, %d", a->size, b->size);

	BitArray *c = (BitArray *)lua_newuserdata(L, sizeof(a));

	c->size = a->size;

	for (unsigned int i = 0; i <= I_WORD(c->size - 1); i++)
		c->values[i] = 0;    //��ʼ������,����c�е�������δ����ģ����ܳ���

	for (unsigned int i = 0; i < a->size; ++i)
	{
		unsigned int mask = I_BIT(i);  //I_BIT�Ƿ���ָ���ֵ�����
		unsigned int *tempa = &(a->values[I_WORD(i)]);    //ȷ���������ڵ��ֽ�
		unsigned int *tempb = &(b->values[I_WORD(i)]);
		unsigned int *tempc = &(c->values[I_WORD(i)]);

		unsigned int entryA = (*tempa & mask);
		unsigned int entryB = (*tempb & mask);

		if (entryA && entryB)
			*tempc |= mask;
		else
			*tempc &= ~mask;
	}

	luaL_getmetatable(L, "LuaBook.array"); //��Ԫ��ѹջ
	lua_setmetatable(L, -2);

	return 1;
}

//��ͳ�ĺ�������
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



//�������ķ���
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
	luaL_newmetatable(L, "LuaBook.array");   //����Ԫ��
	lua_pushvalue(L, -1);    //����Ԫ��
	lua_setfield(L, -2, "__index");   //mt.__index = mt
	luaL_setfuncs(L, arraylib_m, 0);   //ע��Ԫ����
	luaL_newlib(L, arraylib_f);   //������
	return 1;
}*/


//������ʵ���ʽ
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
	luaL_newmetatable(L, "LuaBook.array");   //����Ԫ��
	luaL_setfuncs(L, arraylib_m, 0);   //ע��Ԫ����
	luaL_newlib(L, arraylib_f);   //������
	return 1;
}*/

