
all : server.bin client.bin

server.bin : server.o
	cc -Wall -o $@ $<
server.o : server.c
	cc -c $<

client.bin : client.o
	cc -Wall -o $@ $<
client.o : client.c
	cc -c $<

.PHONY : clean
clean : 
	rm *.o *.bin
