CC = g++
.SUFFIXES: .cpp
BIN_OBJ = tcp_server client ocot_server select_server
all:${BIN_OBJ}

tcp_server:tcp_server.o
	${CC} $< -o $@

ocot_server:ocot_server.o
	${CC} $< -pthread -o $@

select_server:select_server.o
	${CC} $< -o $@

client:client.o
	${CC} $< -o $@

.cpp.o:
	$(CC) -c $<

clean:
	rm -f ${BIN_OBJ}
	rm -f *.o
