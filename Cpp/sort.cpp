#include "stdafx.h"
#include "lua.hpp"

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib,"lua.lib")

//插入排序的C语言实现
int issort(void* data, int size, int esize, int(*compare)(const void *key1, const void *key2))
{
	char *a = (char*)data;
	void *key;
	int i, j;

	//给关键元素分配存储的空间
	if ((key = (char*)malloc(esize)) == NULL)
		return -1;

	//在已经排序的元素中重复插入关键元素
	for (j = 1; j < size; j++)
	{
		memcpy(key, &a[j * esize], esize);
		i = j - 1;

		while (i >= 0 && compare(&a[i * esize], key) > 0)
		{
			memcpy(&a[(i + 1) * esize], &a[i * esize], esize);
			i--;
		}
		memcpy(&a[(i + 1) * esize], key, esize);
	}

	free(key);

	return 0;
}

//插入排序
int insertsort(lua_State *L)
{
	//第一个参数必须是一张表
	luaL_checktype(L, 1, LUA_TTABLE);
	int size = luaL_len(L, 1);   //获取表的大小

	//第二个参数是一个函数
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int i, j;

	//在已排序的元素中插入关键元素
	//Lua的表格的下标从1开始的
	for (j = 2; j <= size; j++)
	{
		i = j - 1;
		//决定要插入的元素的位置
		//压入t[j]的值
		lua_pushinteger(L, j);
		lua_gettable(L, 1);          //在栈的第三个位置，保存该值
		while (i >= 1)
		{
			//压入函数
			lua_pushvalue(L, 2);
			//压入t[i]的值
			lua_pushinteger(L, i); lua_gettable(L, 1);
			//压入t[j]的值
			lua_pushvalue(L, 3); //压入t[j]的值, 原来表格中的t[j]在第一轮比较后已经被覆盖
			
			//调用lua传入的比较函数
			lua_call(L, 2, 1); int result = lua_tointeger(L, -1); lua_pop(L, 1);   //弹出栈顶的结果
			if (result > 0)
			{
				//压入t[i]的值，将t[i]的值赋值给t[i + 1]
				lua_pushinteger(L, i + 1);
				lua_pushinteger(L, i);
				lua_gettable(L, 1);

				lua_settable(L, 1);
				--i;
			}
			else break;	
		}

		lua_pushinteger(L, i + 1);
		lua_pushvalue(L, 3); //压入t[j]的值
		lua_settable(L, 1);  //t[i + 1] = t[j]

		lua_pop(L, 1);    //弹出t[j]的值
	}

	return 0;
}

//在三个随机数中选择中位数所用的比较函数
static int compare_int(const void *int1, const void *int2)
{
	if (*(const int *)int1 > *(const int*)int2)
		return 1;
	else if (*(const int*)int1 < *(const int*)int2)
		return -1;
	else
		return 0;
}

//分区
static int partition(lua_State *L, int i, int k)
{
	int r[3];

	//使用中位数的方法选择分割的值
	r[0] = (rand() % (k - i + 1)) + i;
	r[1] = (rand() % (k - i + 1)) + i;
	r[2] = (rand() % (k - i + 1)) + i;
	issort(r, 3, sizeof(int), compare_int);

	lua_pushinteger(L, r[1]);
	lua_gettable(L, 1);          //在栈的第三个位置，保存该值

	//使用分割值，创建两个分割的区域
	i--;
	k++;

	while (1)
	{
		int ret = 0;
		//移动i值，i所指示的值处在错误的分区中
		do {
			k--;

			lua_pushvalue(L, 2);      //压入调用的函数
			lua_pushinteger(L, k);
			lua_gettable(L, 1);       //t[k]的值
			lua_pushvalue(L, 3);      //t[j]的值
			lua_call(L, 2, 1);
			ret = lua_tointeger(L, -1);
			lua_pop(L, 1);
		} while (ret > 0);

		//移动k值，直到k所指示的值处在错误的分区中
		do {
			i++;

			lua_pushvalue(L, 2);      //压入调用的函数
			lua_pushinteger(L, i);
			lua_gettable(L, 1);      //t[i]的值
			lua_pushvalue(L, 3);     //压入t[j]的值
			lua_call(L, 2, 1);
			ret = lua_tointeger(L, -1);
			lua_pop(L, 1);
		} while (ret < 0);

		if (i >= k) break;
		else
		{
			//压入t[k]的值
			lua_pushinteger(L, k); lua_gettable(L, 1);    //在栈的位置4
			//压入t[i]的值
			lua_pushinteger(L, i); lua_gettable(L, 1);    //在栈的位置5

			//设置t[i]的值, t[i] = t[k]
			lua_pushinteger(L, i); lua_pushvalue(L, 4); lua_settable(L, 1);

			//设置t[k]的值，t[k] = t[i]
			lua_pushinteger(L, k); lua_pushvalue(L, 5); lua_settable(L, 1);

			//弹出栈4和5的值
			lua_pop(L, 2);
		}
	}

	lua_pop(L, 1);   //弹出栈上index为3的t[j]

	return k;
}

//快速排序
//int qksort(void *data, int size, int esize, int i, int k, int(*compare)(const void *key1, const void *key2))
int qksort(lua_State *L, int i, int k)
{
	int j;
	//重复递归排序直到不能再分区了
	while (i < k)
	{
		if ((j = partition(L, i, k)) < 0)
			return -1;
		if (qksort(L, i, j) < 0)
			return -1;

		//迭代和排序右边的分区
		i = j + 1;
	}
	return 0;
}


//快速排序的lua接口, Lua传递过来的参数，第一个为表格，第二个为比较的函数
int lua_qksort(lua_State *L)
{
	//第一个参数必须是一张表
	luaL_checktype(L, 1, LUA_TTABLE);
	int size = luaL_len(L, 1);   //获取表的大小

	//第二个参数是一个函数
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int i = 1, k = size;     //Lua的表格下标是从1开始的

	qksort(L, i, k);

	return 0;
}


//归并排序的合并部分
//static int merge(void *data, int esize, int i, int j, int k, int(*compare)(const void *key1, const void *key2))
static int merge(lua_State *L, int i, int j, int k)
{
	int ipos, jpos, mpos;

	//初始化在合并中使用到的计数器
	ipos = i; jpos = j + 1; mpos = 1;
	//给合并的元素分配存储空间
	
	lua_newtable(L);    //创建新的元表，在栈的位置3上

	//当还有分割的元素需要合并时则继续
	while (ipos <= j || jpos <= k)
	{
		if (ipos > j)
		{
			//左边的元素已经合并完，添加右边的元素
			while (jpos <= k)
			{
				//memcpy(&m[mpos * esize], &a[jpos * esize], esize);
				lua_pushinteger(L, jpos);  lua_gettable(L, 1);  //在栈的位置4
				lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //设置临时的新表的值
				lua_pop(L, 1);   //弹出栈4上的值
				jpos++; mpos++;
			}
			continue; //外层while
		}
		else if (jpos > k)
		{
			//右边的分割元素已经合并完，添加左边的元素
			while (ipos <= j)
			{
				//memcpy(&m[mpos * esize], &a[ipos * esize], esize);
				lua_pushinteger(L, ipos);  lua_gettable(L, 1);  //在栈的位置4
				lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //设置临时的新表的值
				lua_pop(L, 1);   //弹出栈4上的值
				ipos++; mpos++;
			}
			continue;
		}

		//比较合并的两个队列的元素，添加一个元素
		lua_pushvalue(L, 2);  //压入调用的函数
		lua_pushinteger(L, ipos); lua_gettable(L, 1);
		lua_pushinteger(L, jpos); lua_gettable(L, 1);
		lua_call(L, 2, 1);
		int ret = lua_tointeger(L, -1);  lua_pop(L, 1);   //获取被调用的比较函数的值
		if (ret < 0)
		{
			//memcpy(&m[mpos * esize], &a[ipos * esize], esize);
			lua_pushinteger(L, ipos);  lua_gettable(L, 1);  //在栈的位置4
			lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //设置临时的新表的值
			lua_pop(L, 1);   //弹出栈4上的值
			ipos++; mpos++;
		}
		else
		{
			//memcpy(&m[mpos * esize], &a[jpos * esize], esize);
			lua_pushinteger(L, jpos);  lua_gettable(L, 1);  //在栈的位置4
			lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //设置临时的新表的值
			lua_pop(L, 1);   //弹出栈4上的值
			jpos++; mpos++;
		}
	}

	//准备将合并的元素赋值给传递进来的元素集合
	//memcpy(&a[i * esize], m, esize * ((k - i) + 1));
	int len = k - i + 1;
	for (int j = 1; j <= len; ++j)
	{
		lua_pushinteger(L, j);  lua_gettable(L, 3);    //获取新的元素的值
		lua_pushinteger(L, i + j - 1);  lua_pushvalue(L, 4);  lua_settable(L, 1);   //更新原来表格的值,从位置i开始，所以-1
		lua_pop(L, 1);
	}
	//弹出栈顶，index为3的值
	lua_pop(L, 1);

	return 0;
}

//归并排序
//int mgsort(void *data, int size, int esize, int i, int k, int(*compare)(const void *key1, const void *key2))
int mgsort(lua_State *L, int i, int k)
{
	int j;

	if (i < k)
	{
		j = (int)(((i + k - 1)) / 2);

		if (mgsort(L, i, j) < 0) return -1;

		if (mgsort(L, j + 1, k) < 0) return -1;

		//合并两个被分割的元素到一个总的集合中
		if (merge(L, i, j, k) < 0) return -1;
	}
	return 0;
}

//合并排序的lua接口, Lua传递过来的参数，第一个为表格，第二个为比较的函数
int lua_mgsort(lua_State *L)
{
	//第一个参数必须是一张表
	luaL_checktype(L, 1, LUA_TTABLE);
	int size = luaL_len(L, 1);   //获取表的大小

	//第二个参数是一个函数
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int i = 1, k = size;     //Lua的表格下标是从1开始的

	mgsort(L, i, k);

	return 0;
}


static const struct luaL_Reg sortlib[] = {
	{"insertsort", insertsort},
    {"qksort", lua_qksort},
    {"mgsort", lua_mgsort},
	{NULL, NULL}
};


extern "C" __declspec(dllexport)
int luaopen_sort(lua_State *L)
{
	luaL_newlib(L, sortlib);
	return 1;
}