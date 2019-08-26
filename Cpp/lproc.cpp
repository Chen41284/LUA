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

//辅助函数getself可以从指定的状态中获取相关联的Proc结构体
static Proc *getself(lua_State *L) {
	Proc *p;
	lua_getfield(L, LUA_REGISTRYINDEX, "_SELF");
	p = (Proc *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return p;
}

//将值从发送进程移动到接受进程
static void movevalues(lua_State *send, lua_State *rec)
{
	int n = lua_gettop(send);
	int i;
	luaL_checkstack(rec, n, "too many results");
	for (i = 2; i <= n; i++)   //将值传给接受进程
		lua_pushstring(rec, lua_tostring(send, i));
}

//用于寻找等待通道的进程函数
static Proc *searchmatch(const char *channel, Proc **list)
{
	Proc *node;
	//遍历列表
	for (node = *list; node != NULL; node = node->next) {
		if (strcmp(channel, node->channel) == 0) {  //匹配
			//将节点从列表移除
			if (*list == node)  //结点是否为第一个元素
				*list = (node->next == node) ? NULL : node->next;
			node->previous->next = node->next;
			node->next->previous = node->previous;
			return node;
		}
	}
	return NULL;
}

//用于在等待列表中新增一个进程的函数
static void waitonlist(lua_State *L, const char *channel, Proc **list)
{
	Proc *p = getself(L);

	//将其自身放到链表的末尾
	if (*list == NULL) {  //链表为空
		*list = p;
		p->previous = p->next = p;
	}
	else {
		p->previous = (*list)->previous;
		p->next = *list;
		p->previous->next = p->next->previous = p;
	}

	p->channel = channel;   //等待的通道

	do {  //等待的通道
		pthread_cond_wait(&p->cond, &kernel_access);
	} while (p->channel);
}

//发送消息的函数
static int ll_send(lua_State *L) {
	Proc *p;
	const char *channel = luaL_checkstring(L, 1);

	int ret = pthread_mutex_lock(&kernel_access);

	p = searchmatch(channel, &waitreceive);

	if (p) {  //找到匹配的接受线程?
		movevalues(L, p->L);   //将值传递给接受线程
		p->channel = NULL;     //标记接受线程无需等待
		pthread_cond_signal(&p->cond);   //唤醒接收线程
	}
	else waitonlist(L, channel, &waitsend);

	pthread_mutex_unlock(&kernel_access);
	
	return 0;
}

//接受消息的函数
static int ll_receive(lua_State *L) {
	Proc *p;
	const char *channel = luaL_checkstring(L, 1);
	lua_settop(L, 1);

	pthread_mutex_lock(&kernel_access);

	p = searchmatch(channel, &waitsend);

	if (p) {  //找到匹配的发送线程
		movevalues(p->L, L);  //从发送线程获得值
		p->channel = NULL;   //标记发送线程无需再等待
		pthread_cond_signal(&p->cond);
	}
	else waitonlist(L, channel, &waitreceive);

	pthread_mutex_unlock(&kernel_access);

	//返回除通道外的栈中的值
	return lua_gettop(L) - 1;
}


extern "C" __declspec(dllexport)
int luaopen_lproc(lua_State *L);

static void *ll_thread(void *arg) {
	lua_State *L = (lua_State *)arg;
	Proc *self;   //进程自身的控制块

	luaL_openlibs(L);
	luaL_requiref(L, "lproc", luaopen_lproc, 1);
	lua_pop(L, 1);   //移除之前调用的结果

	self = (Proc *)lua_newuserdata(L, sizeof(Proc));
	lua_setfield(L, LUA_REGISTRYINDEX, "_SELF");
	self->L = L;
	self->thread = pthread_self();
	self->channel = NULL;
	pthread_cond_init(&self->cond, NULL);

	if (lua_pcall(L, 0, 0, 0) != 0)  //调用主代码段
		fprintf(stderr, "thread error:%s", lua_tostring(L, -1));

	pthread_cond_destroy(&getself(L)->cond);
	lua_close(L);
	return NULL;
}

//用于创建进程的函数
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

	pthread_detach(thread);   //不需要线程的任何运行结果
	return 0;
}

//模块lproc的其他函数
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

