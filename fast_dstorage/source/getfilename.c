#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "make_log.h"
#include <pthread.h>


int main(int argc,char* argv[]){
	if(argc < 2){
		perror("argc < 2:");
		exit(1);
	}
	int 					pipefd[2] = { 0 };
	pid_t 				pid = 0;
	char					buf[512] = { 0 };
	int						buflen = 0;
	
	if(pipe(pipefd) ==-1){
			perror("pipe is error:\n");		
			exit(1);
	}
	pid = fork();
	if(pid > 0){
		close(pipefd[1]);
		wait(NULL);
		buflen = read(pipefd[0],buf,sizeof(buf));
		LOG("filename","列表","FileName=%s",buf);
		printf("%s",buf);
	}
	else if(pid == 0){
		close(pipefd[0]);
		dup2(pipefd[1],STDOUT_FILENO);
		execlp("fdfs_upload_file", "fdfs_upload_file", 
		"./conf/client.conf", argv[1], NULL);
	}
	return 0;	
}
