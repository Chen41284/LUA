#include "stdafx.h"
#include "lua.hpp"
#include "expat.h"

#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>


#pragma comment(lib,"lua.lib")

typedef struct lxp_userdata {
	XML_Parser parser;       //������Expat������
	lua_State *L;
} lxp_userdata;

//����XML����������ĺ���
static void f_StartElement(void *ud,
	const char *name,
	const char **atts);
static void f_CharData(void *ud, const char *s, int len);
static void f_EndElement(void *ud, const char *name);

static int lxp_make_parser(lua_State *L) {
	XML_Parser p;

	//��1����������������
	lxp_userdata *xpu = (lxp_userdata *)lua_newuserdata(L, sizeof(lxp_userdata));

	//Ԥ�ȳ�ʼ���Է�ֹ������
	xpu->parser = NULL;

	//����Ԫ��
	luaL_getmetatable(L, "Expat");
	lua_setmetatable(L, -2);

	//��2������Expat������
	p = xpu->parser = XML_ParserCreate(NULL);
	if (!p)
		luaL_error(L, "XML_ParserCreate failed");

	//��3����鲢����ص�������
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_pushvalue(L, 1);   //�ص���������ջ
	lua_setuservalue(L, -2);   //���ص���������Ϊ�û����ݵ�ֵ

	//��4������Expat������
	XML_SetUserData(p, xpu);
	XML_SetElementHandler(p, f_StartElement, f_EndElement);
	XML_SetCharacterDataHandler(p, f_CharData);
	return 1;
}

// ����XMLƬ�εĺ���
static int lxp_parse(lua_State *L)
{
	int status;
	size_t len;
	const char *s;
	lxp_userdata *xpu;

	//��ȡ������һ��������Ӧ����һ����������
	xpu = (lxp_userdata *)luaL_checkudata(L, 1, "Expat");

	//���������Ƿ��Ѿ����ر�
	luaL_argcheck(L, xpu->parser != NULL, 1, "parser is closed");

	//��ȡ�ڶ���������һ���ַ�����
	s = luaL_optlstring(L, 2, NULL, &len);

	//���ص����������ջ����Ϊ3��λ��
	lua_settop(L, 2);
	lua_getuservalue(L, 1); // �����û�����������Ļص�����ѹջ

	xpu->L = L;    //����Lua״̬

	//����Expat�������ַ���,��װ�������ԵĽڵ����׳���,ԭ��δ֪
	status = XML_Parse(xpu->parser, s, (int)len, s == NULL);

	//���ش���Ĵ���
	lua_pushboolean(L, status);
	return 1;
}

// �ַ������¼��Ĵ�����
static void f_CharData(void *ud, const char *s, int len)
{
	lxp_userdata *xpu = (lxp_userdata *)ud;
	lua_State *L = xpu->L;

	//�ӻص��������л�ô�����
	lua_getfield(L, 3, "CharacterData");
	if (lua_isnil(L, -1)) {   //û�д�����
		lua_pop(L, 1);
		return;
	}

	lua_pushvalue(L, 1);   //������ѹջ
	lua_pushlstring(L, s, len);  //ѹ���ַ�����
	lua_call(L, 2, 0);    //���ô�����
}

//����Ԫ���¼��Ĵ�����
static void f_EndElement(void *ud, const char *name)
{
	lxp_userdata *xpu = (lxp_userdata *)ud;
	lua_State *L = xpu->L;

	lua_getfield(L, 3, "EndElement");
	if (lua_isnil(L, -1)) {  //û�д�����
		lua_pop(L, 1);
		return;
	}

	lua_pushvalue(L, 1);  //������ѹջ
	lua_pushstring(L, name);  //ѹ���ǩ��
	lua_call(L, 2, 0);    //���ô�����
}

//��ʼԪ���¼��Ĵ�����
static void f_StartElement(void *ud,
	const char *name,
	const char **atts) {
	lxp_userdata *xpu = (lxp_userdata *)ud;
	lua_State *L = xpu->L;

	lua_getfield(L, 3, "StartElement");
	if (lua_isnil(L, -1)) {  //û�д�����
		lua_pop(L, 1);
		return;
	}

	lua_pushvalue(L, 1);   //������ѹջ('self')
	lua_pushstring(L, name);  //ѹ���ǩ��

	//������������Ա�
	lua_newtable(L);
	for (; *atts; atts += 2) {
		lua_pushstring(L, *(atts + 1));
		lua_setfield(L, -2, *atts);  //table[*atts] = *(atts + 1)
	}

	lua_call(L, 3, 0);   //���ô�����
}

//�ر�XML�������ķ���
static int lxp_close(lua_State *L) {
	lxp_userdata *xpu = (lxp_userdata *)luaL_checkudata(L, 1, "Expat");

	//�ͷ�Expat�����������û�У�
	if (xpu->parser)
		XML_ParserFree(xpu->parser);
	xpu->parser = NULL;
	return 0;
}

//lxp��ĳ�ʼ������
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
	//����Ԫ��
	luaL_newmetatable(L, "Expat");

	//metatable.__index = metatable
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	//ע�᷽��
	luaL_setfuncs(L, lxp_meths, 0);

	//ע��
	luaL_newlib(L, lxp_funcs);
	return 1;
}
