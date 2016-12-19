/**
 * Copyright (C) 2008 Happy Fish / YuQing
 *
 * FastDFS may be copied only under the terms of the GNU General
 * Public License V3, which may be found in the FastDFS source kit.
 * Please visit the FastDFS Home Page http://www.csource.org/ for more detail.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fdfs_client.h"
#include "logger.h"

int main(int argc, char *argv[])
{
	char *conf_filename;
	//char *local_filename;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer;
	int result;
	int store_path_index;
	ConnectionInfo storageServer;
	char file_id[128];

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

	/*
	   result = storage_upload_by_filename1(pTrackerServer, \
	   &storageServer, store_path_index, \
	   local_filename, NULL, \
	   NULL, 0, group_name, file_id);
	 */	
	char*		file_buff = "nihao woshi ni baba!!!";
	int       file_size =strlen(file_buff);
	result = storage_upload_by_filebuff1(pTrackerServer, &storageServer, \
			store_path_index, file_buff, file_size, NULL, \
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
