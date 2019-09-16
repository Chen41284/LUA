#include "stdafx.h"
#include "lua.hpp"

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib,"lua.lib")

//���������C����ʵ��
int issort(void* data, int size, int esize, int(*compare)(const void *key1, const void *key2))
{
	char *a = (char*)data;
	void *key;
	int i, j;

	//���ؼ�Ԫ�ط���洢�Ŀռ�
	if ((key = (char*)malloc(esize)) == NULL)
		return -1;

	//���Ѿ������Ԫ�����ظ�����ؼ�Ԫ��
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

//��������
int insertsort(lua_State *L)
{
	//��һ������������һ�ű�
	luaL_checktype(L, 1, LUA_TTABLE);
	int size = luaL_len(L, 1);   //��ȡ��Ĵ�С

	//�ڶ���������һ������
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int i, j;

	//���������Ԫ���в���ؼ�Ԫ��
	//Lua�ı����±��1��ʼ��
	for (j = 2; j <= size; j++)
	{
		i = j - 1;
		//����Ҫ�����Ԫ�ص�λ��
		//ѹ��t[j]��ֵ
		lua_pushinteger(L, j);
		lua_gettable(L, 1);          //��ջ�ĵ�����λ�ã������ֵ
		while (i >= 1)
		{
			//ѹ�뺯��
			lua_pushvalue(L, 2);
			//ѹ��t[i]��ֵ
			lua_pushinteger(L, i); lua_gettable(L, 1);
			//ѹ��t[j]��ֵ
			lua_pushvalue(L, 3); //ѹ��t[j]��ֵ, ԭ������е�t[j]�ڵ�һ�ֱȽϺ��Ѿ�������
			
			//����lua����ıȽϺ���
			lua_call(L, 2, 1); int result = lua_tointeger(L, -1); lua_pop(L, 1);   //����ջ���Ľ��
			if (result > 0)
			{
				//ѹ��t[i]��ֵ����t[i]��ֵ��ֵ��t[i + 1]
				lua_pushinteger(L, i + 1);
				lua_pushinteger(L, i);
				lua_gettable(L, 1);

				lua_settable(L, 1);
				--i;
			}
			else break;	
		}

		lua_pushinteger(L, i + 1);
		lua_pushvalue(L, 3); //ѹ��t[j]��ֵ
		lua_settable(L, 1);  //t[i + 1] = t[j]

		lua_pop(L, 1);    //����t[j]��ֵ
	}

	return 0;
}

//�������������ѡ����λ�����õıȽϺ���
static int compare_int(const void *int1, const void *int2)
{
	if (*(const int *)int1 > *(const int*)int2)
		return 1;
	else if (*(const int*)int1 < *(const int*)int2)
		return -1;
	else
		return 0;
}

//����
static int partition(lua_State *L, int i, int k)
{
	int r[3];

	//ʹ����λ���ķ���ѡ��ָ��ֵ
	r[0] = (rand() % (k - i + 1)) + i;
	r[1] = (rand() % (k - i + 1)) + i;
	r[2] = (rand() % (k - i + 1)) + i;
	issort(r, 3, sizeof(int), compare_int);

	lua_pushinteger(L, r[1]);
	lua_gettable(L, 1);          //��ջ�ĵ�����λ�ã������ֵ

	//ʹ�÷ָ�ֵ�����������ָ������
	i--;
	k++;

	while (1)
	{
		int ret = 0;
		//�ƶ�iֵ��i��ָʾ��ֵ���ڴ���ķ�����
		do {
			k--;

			lua_pushvalue(L, 2);      //ѹ����õĺ���
			lua_pushinteger(L, k);
			lua_gettable(L, 1);       //t[k]��ֵ
			lua_pushvalue(L, 3);      //t[j]��ֵ
			lua_call(L, 2, 1);
			ret = lua_tointeger(L, -1);
			lua_pop(L, 1);
		} while (ret > 0);

		//�ƶ�kֵ��ֱ��k��ָʾ��ֵ���ڴ���ķ�����
		do {
			i++;

			lua_pushvalue(L, 2);      //ѹ����õĺ���
			lua_pushinteger(L, i);
			lua_gettable(L, 1);      //t[i]��ֵ
			lua_pushvalue(L, 3);     //ѹ��t[j]��ֵ
			lua_call(L, 2, 1);
			ret = lua_tointeger(L, -1);
			lua_pop(L, 1);
		} while (ret < 0);

		if (i >= k) break;
		else
		{
			//ѹ��t[k]��ֵ
			lua_pushinteger(L, k); lua_gettable(L, 1);    //��ջ��λ��4
			//ѹ��t[i]��ֵ
			lua_pushinteger(L, i); lua_gettable(L, 1);    //��ջ��λ��5

			//����t[i]��ֵ, t[i] = t[k]
			lua_pushinteger(L, i); lua_pushvalue(L, 4); lua_settable(L, 1);

			//����t[k]��ֵ��t[k] = t[i]
			lua_pushinteger(L, k); lua_pushvalue(L, 5); lua_settable(L, 1);

			//����ջ4��5��ֵ
			lua_pop(L, 2);
		}
	}

	lua_pop(L, 1);   //����ջ��indexΪ3��t[j]

	return k;
}

//��������
//int qksort(void *data, int size, int esize, int i, int k, int(*compare)(const void *key1, const void *key2))
int qksort(lua_State *L, int i, int k)
{
	int j;
	//�ظ��ݹ�����ֱ�������ٷ�����
	while (i < k)
	{
		if ((j = partition(L, i, k)) < 0)
			return -1;
		if (qksort(L, i, j) < 0)
			return -1;

		//�����������ұߵķ���
		i = j + 1;
	}
	return 0;
}


//���������lua�ӿ�, Lua���ݹ����Ĳ�������һ��Ϊ��񣬵ڶ���Ϊ�Ƚϵĺ���
int lua_qksort(lua_State *L)
{
	//��һ������������һ�ű�
	luaL_checktype(L, 1, LUA_TTABLE);
	int size = luaL_len(L, 1);   //��ȡ��Ĵ�С

	//�ڶ���������һ������
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int i = 1, k = size;     //Lua�ı���±��Ǵ�1��ʼ��

	qksort(L, i, k);

	return 0;
}


//�鲢����ĺϲ�����
//static int merge(void *data, int esize, int i, int j, int k, int(*compare)(const void *key1, const void *key2))
static int merge(lua_State *L, int i, int j, int k)
{
	int ipos, jpos, mpos;

	//��ʼ���ںϲ���ʹ�õ��ļ�����
	ipos = i; jpos = j + 1; mpos = 1;
	//���ϲ���Ԫ�ط���洢�ռ�
	
	lua_newtable(L);    //�����µ�Ԫ����ջ��λ��3��

	//�����зָ��Ԫ����Ҫ�ϲ�ʱ�����
	while (ipos <= j || jpos <= k)
	{
		if (ipos > j)
		{
			//��ߵ�Ԫ���Ѿ��ϲ��꣬����ұߵ�Ԫ��
			while (jpos <= k)
			{
				//memcpy(&m[mpos * esize], &a[jpos * esize], esize);
				lua_pushinteger(L, jpos);  lua_gettable(L, 1);  //��ջ��λ��4
				lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //������ʱ���±��ֵ
				lua_pop(L, 1);   //����ջ4�ϵ�ֵ
				jpos++; mpos++;
			}
			continue; //���while
		}
		else if (jpos > k)
		{
			//�ұߵķָ�Ԫ���Ѿ��ϲ��꣬�����ߵ�Ԫ��
			while (ipos <= j)
			{
				//memcpy(&m[mpos * esize], &a[ipos * esize], esize);
				lua_pushinteger(L, ipos);  lua_gettable(L, 1);  //��ջ��λ��4
				lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //������ʱ���±��ֵ
				lua_pop(L, 1);   //����ջ4�ϵ�ֵ
				ipos++; mpos++;
			}
			continue;
		}

		//�ȽϺϲ����������е�Ԫ�أ����һ��Ԫ��
		lua_pushvalue(L, 2);  //ѹ����õĺ���
		lua_pushinteger(L, ipos); lua_gettable(L, 1);
		lua_pushinteger(L, jpos); lua_gettable(L, 1);
		lua_call(L, 2, 1);
		int ret = lua_tointeger(L, -1);  lua_pop(L, 1);   //��ȡ�����õıȽϺ�����ֵ
		if (ret < 0)
		{
			//memcpy(&m[mpos * esize], &a[ipos * esize], esize);
			lua_pushinteger(L, ipos);  lua_gettable(L, 1);  //��ջ��λ��4
			lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //������ʱ���±��ֵ
			lua_pop(L, 1);   //����ջ4�ϵ�ֵ
			ipos++; mpos++;
		}
		else
		{
			//memcpy(&m[mpos * esize], &a[jpos * esize], esize);
			lua_pushinteger(L, jpos);  lua_gettable(L, 1);  //��ջ��λ��4
			lua_pushinteger(L, mpos);  lua_pushvalue(L, 4); lua_settable(L, 3); //������ʱ���±��ֵ
			lua_pop(L, 1);   //����ջ4�ϵ�ֵ
			jpos++; mpos++;
		}
	}

	//׼�����ϲ���Ԫ�ظ�ֵ�����ݽ�����Ԫ�ؼ���
	//memcpy(&a[i * esize], m, esize * ((k - i) + 1));
	int len = k - i + 1;
	for (int j = 1; j <= len; ++j)
	{
		lua_pushinteger(L, j);  lua_gettable(L, 3);    //��ȡ�µ�Ԫ�ص�ֵ
		lua_pushinteger(L, i + j - 1);  lua_pushvalue(L, 4);  lua_settable(L, 1);   //����ԭ������ֵ,��λ��i��ʼ������-1
		lua_pop(L, 1);
	}
	//����ջ����indexΪ3��ֵ
	lua_pop(L, 1);

	return 0;
}

//�鲢����
//int mgsort(void *data, int size, int esize, int i, int k, int(*compare)(const void *key1, const void *key2))
int mgsort(lua_State *L, int i, int k)
{
	int j;

	if (i < k)
	{
		j = (int)(((i + k - 1)) / 2);

		if (mgsort(L, i, j) < 0) return -1;

		if (mgsort(L, j + 1, k) < 0) return -1;

		//�ϲ��������ָ��Ԫ�ص�һ���ܵļ�����
		if (merge(L, i, j, k) < 0) return -1;
	}
	return 0;
}

//�ϲ������lua�ӿ�, Lua���ݹ����Ĳ�������һ��Ϊ��񣬵ڶ���Ϊ�Ƚϵĺ���
int lua_mgsort(lua_State *L)
{
	//��һ������������һ�ű�
	luaL_checktype(L, 1, LUA_TTABLE);
	int size = luaL_len(L, 1);   //��ȡ��Ĵ�С

	//�ڶ���������һ������
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int i = 1, k = size;     //Lua�ı���±��Ǵ�1��ʼ��

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