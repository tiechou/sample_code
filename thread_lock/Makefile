CFLAG = -c -g -O2 -Wall

GCC = g++

# 用伪目标suffixes 来让make知道特定的后缀
.SUFFIXES: .o .cpp

srcs=t_spinlock_thread.cpp

objs=$(srcs:.cpp=.o)

	
t_spinlock_thread: ${objs}
	$(GCC) -lpthread -o $@ $^

.cpp.o:
	${GCC} ${CFLAG} -I./ -DUSE_SPINLOCK -o $@ $<

clean:
	rm -f *.o *~
	rm -f t_spinlock_thread 

#t_spin_thread :t_spinlock_thread.cpp
#
#t_spin_thread :t_spinlock_thread.cpp
#	g++ -g -O2 -Wall -I./ -DUSE_SPINLOCK -lpthread t_spinlock_thread.cpp -o t_spin_thread
