#杀死之前启动的cgi程序
kill -9 `ps aux | grep "upload" |grep -v grep | awk '{print $2}'`
kill -9 `ps aux | grep "cloud_file" |grep -v grep | awk '{print $2}'`

#启动cgi测试程序
spawn-fcgi -a 127.0.0.1 -p 8013 -f ./upload
spawn-fcgi -a 127.0.0.1 -p 8014 -f ./cloud_file


#启动nginx
sudo /usr/local/nginx/sbin/nginx -s reload

#启动redis
#redis-server ./conf/redis.conf

#启动fastDFS
sudo fdfs_trackerd ./conf/tracker.conf
sudo fdfs_storaged ./conf/storage.conf
