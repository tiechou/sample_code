/*
 * Compile with:
 * cc -I/usr/local/include -o event-test event-test.c -L/usr/local/lib -levent
 */

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <event.h>

void fifo_read(int fd, short event, void *arg)
{
	char buf[255];
	int len;

    // 申明一个event结构体对象
	struct event *ev = arg;
#ifdef WIN32
	DWORD dwBytesRead;
#endif

	/* Reschedule this event */
	event_add(ev, NULL);

	fprintf(stderr, "fifo_read called with fd: %d, event: %d, arg: %p\n",
		fd, event, arg);
#ifdef WIN32
	len = ReadFile((HANDLE)fd, buf, sizeof(buf) - 1, &dwBytesRead, NULL);

	// Check for end of file. 
	if(len && dwBytesRead == 0) {
		fprintf(stderr, "End Of File");
		event_del(ev);
		return;
	}

	buf[dwBytesRead + 1] = '\0';
#else
	len = read(fd, buf, sizeof(buf) - 1);

	if (len == -1) {
		perror("read");
		return;
	} else if (len == 0) {
		fprintf(stderr, "Connection closed\n");
		return;
	}

	buf[len] = '\0';
#endif
	fprintf(stdout, "Read: %s\n", buf);
}

int main (int argc, char **argv)
{
	struct event evfifo;
#ifdef WIN32
	HANDLE socket;
	// Open a file. 
	socket = CreateFile("test.txt",     // open File 
			GENERIC_READ,                 // open for reading 
			0,                            // do not share 
			NULL,                         // no security 
			OPEN_EXISTING,                // existing file only 
			FILE_ATTRIBUTE_NORMAL,        // normal file 
			NULL);                        // no attr. template 

	if(socket == INVALID_HANDLE_VALUE)
		return 1;

#else
	struct stat st;
	char *fifo = "event.fifo";
	int socket;
 
	if (lstat (fifo, &st) == 0) {
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			errno = EEXIST;
			perror("lstat");
			exit (1);
		}
	}

	unlink (fifo);
	if (mkfifo (fifo, 0600) == -1) {
		perror("mkfifo");
		exit (1);
	}

	/* Linux pipes are broken, we need O_RDWR instead of O_RDONLY */
#ifdef __linux
	socket = open (fifo, O_RDWR | O_NONBLOCK, 0);
#else
	socket = open (fifo, O_RDONLY | O_NONBLOCK, 0);
#endif

	if (socket == -1) {
		perror("open");
		exit (1);
	}

	fprintf(stderr, "Write data to %s\n", fifo);
#endif
	/* Initalize the event library */
    // 初始化全局的event_base
	event_init();

	/* Initalize one event */
#ifdef WIN32
	event_set(&evfifo, (int)socket, EV_READ, fifo_read, &evfifo);
#else
    // 再调用event_set初始化第一步申明的evfifo对象.
    // 这个函数的第一个参数相当于函数返回值的功能了,C语言经常这么用.
    // 把evfifo的地址传进去,然后在函数返回时他就会被初始化成我们想要的类型.
    // 后面都是初始化它的参数.
    // 第二个socket当然是对哪一个socket上的事件进行监测,
    // 第三个是上面说过的事件类型,是一个short类型,
    // 第四个是回调函数名,当fifo可读时就调用这个函数,
    // 最后一个是传给这个函数的第三个参数(为什么是第三个,下面我们看了这个函数的实现就明白了).
	event_set(&evfifo, socket, EV_READ, fifo_read, &evfifo);
#endif

	/* Add it to the active events, without a timeout */
    // 然后调用event_add将初始化完成的evfifo放入事件队列.
    // 后面的NULL参数是一个time_out值.
    // 是libevent作为定时器时的设置,我们这里不需要,所以设置为NULL.
	event_add(&evfifo, NULL);

    // 驱动事件监测器    
	event_dispatch();
#ifdef WIN32
	CloseHandle(socket);
#endif
	return (0);
}

