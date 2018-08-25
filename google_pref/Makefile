CFLAG=-c -g -Wall
LDLIBS=-L/usr/local/lib/
GCC=g++
.SUFFIXES: .o .cpp .c

srcs=profiler_test.cpp
objs=$(srcs:.cpp=.o)

profiler_test:${objs}
	${GCC} -o $@ $^ ${LDLIBS} -lprofiler
	./profiler_test
	pprof --dot profiler_test CPUProfile >>log.dot

tcmalloc_test:tcmalloc_test.o
	${GCC} -o $@ tcmalloc_test.cpp ${LDLIBS} -ltcmalloc

malloc_test:malloc_test.o
	${GCC} -o $@ -pg -lc_p malloc_test.cpp -lpthread
.cpp.o:
	$(GCC) $(CFLAG) -o $@ $<


clean:
	rm *.o profiler_test 
