// 一个通过的C语言调用Lua的函数
// 示例：call_va(L, "f", "dd>d", x, y, &z);
void call_va(lua_State *L, const char *func, const char *sig, ...) {
	va_list vl;
	int narg, nres;    // 参数和结果的个数

	va_start(vl, sig);
	lua_getglobal(L, func);   //函数压栈

	//为通用调用函数压入参数
	for (narg = 0; *sig; narg++) {  //对于每一个参数循环
		//检查栈空间
		luaL_checkstack(L, 1, "too many arguments");

		switch (*sig++) {
		case 'd':   //double 类型的参数
			lua_pushnumber(L, va_arg(vl, double));
			break;
		case 'i':  //int 类型的参数
			lua_pushinteger(L, va_arg(vl, int));
			break;
		case 'b':  //bool 类型的参数
			lua_pushboolean(L, va_arg(vl, bool));
			break;
		case 's':  // string类型的参数
			lua_pushstring(L, va_arg(vl, char*));
			break;
		case '>':  //参数部分结束
			goto endargs;   //从循环中跳出

		default:
			error(L, "invalid option (%c)", *(sig - 1));
		}
	}

	endargs:   //Goto的跳转标签

	nres = strlen(sig);   // 期望的结果数

	if (lua_pcall(L, narg, nres, 0) != 0)  //进行调用
		error(L, "error calling '%s'", func, lua_tostring(L, -1));

	//为通用调用函数检索结果
	nres = -nres;  //第一个结果的栈索引
	while (*sig) {  //对于每一个结果循环
		switch (*sig++) {
		case 'd': {  //double 类型的结果
			int isnum;
			double n = lua_tonumberx(L, nres, &isnum);
			if (!isnum)
				error(L, "wrong result type");
			*va_arg(vl, double *) = n;  //返回值是指针类型
			break;
		}
		
		case 'i': {  // int 类型的结果
			int isnum;
			int n = lua_tointegerx(L, nres, &isnum);
			if (!isnum)
				error(L, "wrong result type");
			*va_arg(vl, int*) = n;
			break;
		}

		case 'b': { //bool 类型的结果
			bool n = lua_toboolean(L, nres);  //任意类型的值均可
			*va_arg(vl, bool*) = n;
			break;
		}

		case 's': {  // 类型的结果
			const char *s = lua_tostring(L, nres);
			if (s == NULL)
				error(L, "wrong result type");
			*va_arg(vl, const char **) = s;
			break;
		}

		default:
			error(L, "invalid option (%c)", *(sig - 1));
		}
		nres++;
	}

	va_end(vl);
}

// 错误处理
void error(lua_State *L, const char * fmt, ...)
{
	va_list  argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	lua_close(L);
	exit(EXIT_FAILURE);
}

//测试
int main()
{
	lua_State *L = luaL_newstate();
	
	luaL_openlibs(L);   //需要使用到的标准库
	luaL_loadfile(L, filename);
	lua_pcall(L, 0, 0, 0);
	
	double z = 0.0;
	
	call_va(L, "f", "dd>d", 100.0, 3.14, &z);
	
	return 0;
}