VPATH=./source
src=$(wildcard *.c ./source)
obj=$(patsubst %.c,%.o,$(src))
INC=-g -I/home/yunpan/fast_dstorage/include/
CC=gcc

BIN=client


all:$(BIN)

#clent.o make_log.o

client:getfilename.o make_log.o
	$(CC)  $(INC) $^ -o $@

test_log:testlog.o make_log.o
	$(CC)  $(INC) $^ -o $@

%.o:%.c
	$(CC)  $(INC) -c $^ -o $@


.PHONY:clean


clean:
	rm  *.o
	rm	$(BIN)
