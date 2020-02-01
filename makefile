
all : server.bin client1.bin test.bin

server.bin : server.o
	cc -Wall -o $@ $<
server.o : server.c
	cc -c $<

client1.bin : client1.o
	cc -Wall -o $@ $<
client1.o : client1.c
	cc -c $<

test.bin : test.o
	cc -Wall -o $@ $< -lpthread
test.o : test.c
	cc -c $<

.PHONY : clean
clean : 
	rm *.o *.bin
