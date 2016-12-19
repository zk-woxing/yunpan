VPATH=./source
CC=gcc
CPPFLAGS= -I./include   -I/usr/local/include/hiredis/
CFLAGS=-Wall 
LIBS= -lhiredis -lfcgi -lpthread -lfastcommon -lfdfsclient

#找到当前目录下所有的.c文件
src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))




BIN1=client
BIN2=example_redis
BIN3=example_op
BIN4=upload
BIN5=cloud_file


all:$(BIN5) 




#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

#clent.o make_log.o

$(BIN1):$(VPATH)/fdfs_upload_file.o
	$(CC) $^ -o $@ $(LIBS)

test_log:testlog.o make_log.o
	$(CC) $(LIBS) $^ -o $@

$(BIN2):$(VPATH)/example.o
	$(CC) $^ -o $@ $(LIBS)

$(BIN3):$(VPATH)/example_op.o redis_op.o
	$(CC) $^ -o $@ $(LIBS)
$(BIN4):$(VPATH)/upload_cgi.o redis_op.o make_log.o
	$(CC) $^ -o $@ $(LIBS) 
$(BIN5):$(VPATH)/cloud_file.o redis_op.o make_log.o util_cgi.o
	$(CC) $^ -o $@ $(LIBS) 

.PHONY:clean


clean:
	rm  $(VPATH)/*.o
	rm  ./*.o
