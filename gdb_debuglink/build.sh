g++ -g -fpic -shared -o libmyfunction.so myfunction.cpp
strip --only-keep-debug libmyfunction.so -o func.gdb

strip libmyfunction.so
g++ -o test -g test.cpp -L./ -lmyfunction

objcopy --add-gnu-debuglink=func.gdb libmyfunction.so 
gdb test
