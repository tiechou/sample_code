#-fPIC：表示编译为位置独立的代码，不用此选项的话编译后的代码是位置相关的所以动态载入时是通过代码拷贝的方式来满足不同进程的需要，而不能达到真正代码段共享的目的。

#-L.：表示要连接的库在当前目录中

#-l${obj}：编译器查找动态连接库时有隐含的命名规则，即在给出的名字前面加上lib，后面加上.so来确定库的名称

CC = g++
.SUFFIXES: .cpp

obj = hello

all:${obj}

${obj}:${obj}.cpp
	${CC} $< -fPIC -shared -o lib$@.so
	${CC}  -L. -l${obj} -o ${obj}

clean:
	rm lib${obj}.so ${obj}
