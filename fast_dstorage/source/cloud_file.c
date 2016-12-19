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
#include "util_cgi.h"






#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"

  

int InsetDateToRedis(char* filename,char* file_id){

	int					ret = 0;
	redisContext*		conn = NULL;

	conn =rop_connectdb_nopwd("127.0.0.1","6379");
	ret =  rop_list_push(conn, "My_list", file_id);

	ret = rop_insert_hash(conn, "FileId_FileName", file_id,filename);
	time_t nowtime;
	time(&nowtime);

	ret = rop_insert_hash(conn, "FileId_Time", file_id,ctime(&nowtime));
	

	return ret;
}



int client_upload(char* Mybuf,int lenbuf,char* file_id){

	char *conf_filename;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer;
	int result;
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
/*
static void PrintEnv(char *label, char **envp)
{
	printf("%s:<br>\n<pre>\n", label);
	for ( ; *envp != NULL; envp++) {
		printf("%s\n", *envp);
	}
	printf("</pre><p>\n");
}
*/
int main ()
{

	char 					fromId[128] = { 0 };
	int						fromIdLen = 0;
	char					count[128] = { 0 };
	int						countLen = 0;
	int 					ret = 0;
	char					values[10][1024] = {{ 0 }};
	int 					get_num = 0;
	int						i = 0;
	char					data[2048] = { 0 };
	char					outbufName[256] = { 0 };
	char					outbuftime[256] = { 0 }; 
	char					SumBuf[20480] = { 0 }; 
	redisContext*	conn = NULL;
	char					suffixname[128] = { 0 }; 	
	char					pvnum[12] = { 0 };
	char					urldata[1024] = "http://192.168.2.108/";
	
	
	while (FCGI_Accept() >= 0) {
		printf("Content-type: text/html\r\n"
                "\r\n");
		char *contentLength = getenv("QUERY_STRING");
		//http://192.168.23.244/data?cmd=newFile&fromId=0&count=8&user=
		ret = query_parse_key_value(contentLength,"fromId", fromId,  &fromIdLen);
		if(ret == -1){
			LOG("1111","cloud_file","query_parse_key_value=%d",ret);
			goto End;
		}
		ret = query_parse_key_value(contentLength,"count", count,  &countLen);
		if(ret == -1){
			LOG("1111","cloud_file","query_parse_key_value=%d",ret);
			goto End;
		}
		
		
	//读取数据库
		conn =rop_connectdb_nopwd("127.0.0.1","6379");
		ret = rop_range_list(conn, "My_list", atoi(fromId), atoi(count), values, &get_num);
		sprintf(SumBuf,"{\
    \"games\":   [");
		for(i = 0; i < get_num; i++){
			memset(outbufName,0,sizeof(outbufName));
			memset(outbuftime,0,sizeof(outbuftime));
			memset(pvnum,0,sizeof(pvnum));
			memset(urldata,0,sizeof(urldata));
			strcpy(urldata,"http://192.168.23.244/");
			
			ret = rop_GetHash_one_field(conn, "FileId_FileName", values[i],outbufName );
			ret = rop_GetHash_one_field(conn, "FileId_Time", values[i], outbuftime);
			ret = rop_GetHash_one_field(conn, "FileId_download", values[i],pvnum);
			strcat(urldata,values[i]);
			
			ret = get_file_suffix(outbufName, suffixname);
			char* tempbuf = strtok(outbuftime,"\n");
			sprintf(data,"{\"id\":\"%s\",\
													\"kind\":2,     \
													\"title_m\":\"%s\",\
													\"title_s\":\"文件title_s\",\
													\"descrip\": \"%s\",\
													\"picurl_m\":\"http://192.168.23.244/static/file_png/%s.png\",\
													\"url\": \"%s\",\
													\"pv\": %d,\
													\"hot\":0\
			}",values[i],outbufName,tempbuf,suffixname,urldata,atoi(pvnum));
			
			strcat(SumBuf,data);
			if(i != get_num-1){
				strcat(SumBuf,",");
			}
			memset(data,0,sizeof(data));
		}

		strcat(SumBuf,"]}");
		LOG("JsonSum","Sum","json=%s",SumBuf);
		printf("%s\n",SumBuf);		

	/*	PrintEnv("Request environment", environ);
		PrintEnv("Initial environment", initialEnv);
	*/
	} /* while */

End:
	return 0;
}
