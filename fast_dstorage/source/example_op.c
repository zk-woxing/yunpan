#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hiredis.h>
#include "redis_op.h"

int main(int argc, char **argv) {
    int 											ret = 0;
    redisContext 							*conn = NULL;
    
   	conn = rop_connectdb_nopwd("127.0.0.1", "6379");
  	ret =  rop_zset_set_string(conn, "my_test_list", "你好世界!");
   
    return ret;
}
