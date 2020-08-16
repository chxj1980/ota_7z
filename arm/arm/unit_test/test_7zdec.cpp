#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include"../ota_unpack.h"

#define OTA_FILE_PATH	"a.bin"
//#define OTA_FILE_PATH	"au_1.bin"

int main()
{
	int ret = 0;
	char cmd[52] = {'\0'};
	ota_unpack unpack;
	ota_file_t ota;
	const char *path = ".";

	//set deccompress file save path
	unpack.set_decomp_path((char *)path, strlen(path));

	PRINTF("#########---3--->\n");
	list<ota_file_t> *ota_f;
	//deccompress file
	ret = unpack.parse_ota_pack((char *)OTA_FILE_PATH);
	if(ret < 0)
	{
		printf("parse ota file erroe exit!\n");
		exit(-1);
	}

	//get the ota file list 
	ota_f = unpack.getfilelist();
	if(!ota_f->empty())
	{
		int i = 0;
	    list<ota_file_t>::iterator it;
		for(it = ota_f->begin(); it != ota_f->end(); it++ )
		{
			ota = *it;
			printf("path:%s, file[%d]: %s\n", ota.path, i, ota.file);
			i++;
			memset(&ota, 0, sizeof(ota_file_t));
		}
	}

	printf("parse ota file success !\n");

	sprintf(cmd, "ls -l %s", path);
	system(cmd);

	return 0;
}
