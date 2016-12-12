#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>

#include"make_log.h"
#include<pthread.h>


int main(int argc, char* argv[]){
	
	LOG("1111","2222","33333","4444","55555");
	
	return 0;
}
