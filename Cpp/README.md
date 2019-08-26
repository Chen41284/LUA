# 简介

Lua 与 C语言的交互

## Lua调用C语言编写的函数  
### book
```Lua
book = require "book"  --函数库的名称为book，实现了Programming in Lua(4th)中的一些函数
--通过参数构建表 t = tuple(2, 3, 4)
tuple = book.tuple
--第一个参数为表格，第二个参数为函数，如果参数表中的元素调用函数后返回true，则将该元素加入结果表中以返回
filter = boook.filter
--传入的参数为为表格，该函数将表格的元素逆序后输出，返回一个新的表格  
reverse = book.reverse
--根据传入的字符串和分割符(char)，将字符串分割后插入一个新的表格中返回
split = book.split
--根据传入的字符串和表格，使用标中的映射关系替换字符串（第一个参数），然后返回最终的字符串
transliterate = book.transliterate
```

### array
使用int存储Bool类型的数组

### dir
读取目录下的文件

### lxp
XML文件的读取

### lproc
封装pthread库的一个发送和接受的简单实例