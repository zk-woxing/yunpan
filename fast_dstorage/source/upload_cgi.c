/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include "fcgi_config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


#include <stdio.h>
#include <errno.h>
#include "fdfs_client.h"
#include "logger.h"
#include "redis_op.h"
#include "make_log.h"







#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"

char* memstr(char* full_data, int full_data_len, char* substr)  
{  
	if (full_data == NULL || full_data_len <= 0 || substr == NULL) {  
		return NULL;  
	}  

	if (*substr == '\0') {  
		return NULL;  
	}  

	int sublen = strlen(substr);  

	int i;  
	char* cur = full_data;  
	int last_possible = full_data_len - sublen + 1;  
	for (i = 0; i < last_possible; i++) {  
		if (*cur == *substr) {  
			//assert(full_data_len - i >= sublen);  
			if (memcmp(cur, substr, sublen) == 0) {  
				//found  
				return cur;  
			}  
		}  
		cur++;  
	}  

	return NULL;  
}  


int InsetDateToRedis(char* filename,char* file_id){

	int					ret = 0;
	redisContext*		conn = NULL;

	conn =rop_connectdb_nopwd("127.0.0.1","6379");
	ret =  rop_list_push(conn, "My_list", file_id);

	ret = rop_insert_hash(conn, "FileId_FileName", file_id,filename);
	time_t nowtime;
	time(&nowtime);

	ret = rop_insert_hash(conn, "FileId_Time", file_id,ctime(&nowtime));
	
	ret = rop_insert_hash(conn, "FileId_download", file_id,(char*)0);
		
	return ret;
}



int client_upload(char* Mybuf,int lenbuf,char* file_id){

	char *conf_filename =NULL;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer = NULL;
	int result = 0;
	int store_path_index;
	ConnectionInfo storageServer;

	log_init();
	g_log_context.log_level = LOG_ERR;
	ignore_signal_pipe();


	//初始化客户端
	conf_filename ="/home/yunpan/mygit/fast_dstorage/conf/client.conf";
	if ((result=fdfs_client_init(conf_filename)) != 0)
	{
		return result;
	}

	//获取tracker连接
	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		return errno != 0 ? errno : ECONNREFUSED;
	}



	if ((result=tracker_query_storage_store(pTrackerServer, \
					&storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		fprintf(stderr, "tracker_query_storage fail, " \
				"error no: %d, error info: %s\n", \
				result, STRERROR(result));
		return result;
	}

	result = storage_upload_by_filebuff1(pTrackerServer, &storageServer, \
			store_path_index, Mybuf, lenbuf, NULL, \
			NULL, 0, group_name, file_id);


			 if (result == 0)
			 {
				 printf("%s\n", file_id);
			 }
			 else
			 {
				 fprintf(stderr, "upload file fail, " \
						 "error no: %d, error info: %s\n", \
						 result, STRERROR(result));
			 }

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();
	return result;
}

static void PrintEnv(char *label, char **envp)
{
	printf("%s:<br>\n<pre>\n", label);
	for ( ; *envp != NULL; envp++) {
		printf("%s\n", *envp);
	}
	printf("</pre><p>\n");
}

int main ()
{
	char **initialEnv = environ;
	int count = 0;

	while (FCGI_Accept() >= 0) {
		char *contentLength = getenv("CONTENT_LENGTH");
		int len;

		printf("Content-type: text/html\r\n"
				"\r\n"
				"<title>FastCGI echo</title>"
				"<h1>FastCGI echo</h1>\n"
				"Request number %d,  Process ID: %d<p>\n", ++count, getpid());

		if (contentLength != NULL) {
			len = strtol(contentLength, NULL, 10);
		}
		else {
			len = 0;
		}

		if (len <= 0) {
			printf("No data from standard input.<p>\n");
		}
		else {
			int i, ch;
			char* Mybuf = (char*)malloc(len*sizeof(char));
			memset(Mybuf,0,len);
			char* p = Mybuf;

			printf("Standard input:<br>\n<pre>\n");
			for (i = 0; i < len; i++) {
				if ((ch = getchar()) < 0) {
					printf("Error: Not enough bytes received on standard input<p>\n");
					break;
				}
				// putchar(ch);

				//用来实现二进制流的保存
				*p = ch;
				p++;
			}
			char* temp = NULL;
			char namebuf[512] = { 0 };
			char boundray[256] = { 0 };
			int  lentemp = 0;
			p = Mybuf;
			temp = memstr(p, len, "\r\n");
			lentemp = temp - p;
			len -=lentemp;
			memcpy(boundray,p,lentemp);
			temp +=2;
			len-=2;
			//int bounlen = lentemp;	
			p = temp;

			temp = memstr(p,len ,"filename=\"");
			//printf("filename001:%s\n",temp);
			len -= (temp - p);

			p = memstr(temp, len, "\"");
			len --;
			p++;
			temp = memstr(p, len,"\"");
			len --;

			lentemp = temp - p;
			len -=lentemp;


			memcpy(namebuf,p,lentemp);

			//printf("filename001:%s\n",temp);
			temp += lentemp;
			temp +=4;
			len -= 4;
			//	printf("filename:%s\n",namebuf);


			p = memstr(temp, len, "\r\n");
			len -= (p-temp+2);
			p += 2;

			temp = memstr(p, len, "\r\n");

			len -= (temp-p+2);

			temp+=2;

			char* p2 = memstr(temp, len, boundray);
			p2-=2;
			lentemp  = p2 - temp;

			memcpy(Mybuf,temp,lentemp);

			Mybuf[lentemp] = '\0';

			char file_id[128] ={ 0 };


			client_upload(Mybuf,lentemp,file_id);

			InsetDateToRedis(namebuf,file_id);


			printf("%s\n", file_id);
			free(Mybuf);
			Mybuf = NULL;
			printf("\n</pre><p>\n");
		}

		PrintEnv("Request environment", environ);
		PrintEnv("Initial environment", initialEnv);
	} /* while */

	return 0;
}
