#include "stdafx.h"
#include "lua.hpp"

#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>


#pragma comment(lib,"lua.lib")

typedef struct Proc {
	lua_State *L;
	pthread_t thread;
	pthread_cond_t cond;
	const char *channel;
	struct Proc *previous, *next;
} Proc;

static Proc *waitsend = NULL;
static Proc *waitreceive = NULL;

static pthread_mutex_t kernel_access = PTHREAD_MUTEX_INITIALIZER;

//��������getself���Դ�ָ����״̬�л�ȡ�������Proc�ṹ��
static Proc *getself(lua_State *L) {
	Proc *p;
	lua_getfield(L, LUA_REGISTRYINDEX, "_SELF");
	p = (Proc *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return p;
}

//��ֵ�ӷ��ͽ����ƶ������ܽ���
static void movevalues(lua_State *send, lua_State *rec)
{
	int n = lua_gettop(send);
	int i;
	luaL_checkstack(rec, n, "too many results");
	for (i = 2; i <= n; i++)   //��ֵ�������ܽ���
		lua_pushstring(rec, lua_tostring(send, i));
}

//����Ѱ�ҵȴ�ͨ���Ľ��̺���
static Proc *searchmatch(const char *channel, Proc **list)
{
	Proc *node;
	//�����б�
	for (node = *list; node != NULL; node = node->next) {
		if (strcmp(channel, node->channel) == 0) {  //ƥ��
			//���ڵ���б��Ƴ�
			if (*list == node)  //����Ƿ�Ϊ��һ��Ԫ��
				*list = (node->next == node) ? NULL : node->next;
			node->previous->next = node->next;
			node->next->previous = node->previous;
			return node;
		}
	}
	return NULL;
}

//�����ڵȴ��б�������һ�����̵ĺ���
static void waitonlist(lua_State *L, const char *channel, Proc **list)
{
	Proc *p = getself(L);

	//��������ŵ������ĩβ
	if (*list == NULL) {  //����Ϊ��
		*list = p;
		p->previous = p->next = p;
	}
	else {
		p->previous = (*list)->previous;
		p->next = *list;
		p->previous->next = p->next->previous = p;
	}

	p->channel = channel;   //�ȴ���ͨ��

	do {  //�ȴ���ͨ��
		pthread_cond_wait(&p->cond, &kernel_access);
	} while (p->channel);
}

//������Ϣ�ĺ���
static int ll_send(lua_State *L) {
	Proc *p;
	const char *channel = luaL_checkstring(L, 1);

	int ret = pthread_mutex_lock(&kernel_access);

	p = searchmatch(channel, &waitreceive);

	if (p) {  //�ҵ�ƥ��Ľ����߳�?
		movevalues(L, p->L);   //��ֵ���ݸ������߳�
		p->channel = NULL;     //��ǽ����߳�����ȴ�
		pthread_cond_signal(&p->cond);   //���ѽ����߳�
	}
	else waitonlist(L, channel, &waitsend);

	pthread_mutex_unlock(&kernel_access);
	
	return 0;
}

//������Ϣ�ĺ���
static int ll_receive(lua_State *L) {
	Proc *p;
	const char *channel = luaL_checkstring(L, 1);
	lua_settop(L, 1);

	pthread_mutex_lock(&kernel_access);

	p = searchmatch(channel, &waitsend);

	if (p) {  //�ҵ�ƥ��ķ����߳�
		movevalues(p->L, L);  //�ӷ����̻߳��ֵ
		p->channel = NULL;   //��Ƿ����߳������ٵȴ�
		pthread_cond_signal(&p->cond);
	}
	else waitonlist(L, channel, &waitreceive);

	pthread_mutex_unlock(&kernel_access);

	//���س�ͨ�����ջ�е�ֵ
	return lua_gettop(L) - 1;
}


extern "C" __declspec(dllexport)
int luaopen_lproc(lua_State *L);

static void *ll_thread(void *arg) {
	lua_State *L = (lua_State *)arg;
	Proc *self;   //��������Ŀ��ƿ�

	luaL_openlibs(L);
	luaL_requiref(L, "lproc", luaopen_lproc, 1);
	lua_pop(L, 1);   //�Ƴ�֮ǰ���õĽ��

	self = (Proc *)lua_newuserdata(L, sizeof(Proc));
	lua_setfield(L, LUA_REGISTRYINDEX, "_SELF");
	self->L = L;
	self->thread = pthread_self();
	self->channel = NULL;
	pthread_cond_init(&self->cond, NULL);

	if (lua_pcall(L, 0, 0, 0) != 0)  //�����������
		fprintf(stderr, "thread error:%s", lua_tostring(L, -1));

	pthread_cond_destroy(&getself(L)->cond);
	lua_close(L);
	return NULL;
}

//���ڴ������̵ĺ���
static int ll_start(lua_State *L) {
	pthread_t thread;
	const char *chunk = luaL_checkstring(L, 1);
	lua_State *L1 = luaL_newstate();

	if (L1 == NULL)
		luaL_error(L, "unable to create new state");

	if (luaL_loadstring(L1, chunk) != 0)
		luaL_error(L, "error in thread body:%s", lua_tostring(L1, -1));

	if (pthread_create(&thread, NULL, ll_thread, L1) != 0)
		luaL_error(L, "unable to create new thread");

	pthread_detach(thread);   //����Ҫ�̵߳��κ����н��
	return 0;
}

//ģ��lproc����������
static int ll_exit(lua_State *L) {
	pthread_exit(NULL);
	return 0;
}

static const struct luaL_Reg ll_funcs[] = {
	{"start", ll_start},
    {"send", ll_send},
    {"receive", ll_receive},
    {"exit", ll_exit},
    {NULL,NULL}
};

int luaopen_lproc(lua_State *L) {
	luaL_newlib(L, ll_funcs);   //open library
	return 1;
}

