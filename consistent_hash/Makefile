all:
	g++ -fPIC -g -c chash.cpp -I. -I/usr/local/include/lsystem/lserver/ -I/usr/local/include/
	g++ -fPIC -g -shared -o libchash.so chash.o

test:
	g++ -fPIC -g -DTESTCASE chash.cpp -o test_chash -lprotobuf -L. -lchash -lm -lpthread -L/usr/local/lib/lsystem/ -llbase -I. -I/usr/local/include/lsystem/lserver/ -I/usr/local/include/

clean:
	rm -rf *.o *.so test_*
	install:
	cp libchash.so /usr/local/lib

