#ifndef __OTA_UNPACK_H__
#define __OTA_UNPACK_H__

#include<iostream>
#include<list>
#include"box_decode.h"
#include <unistd.h>

#define RD_HEAD_SIZE 512
#define DEF_OTA_PATH_SD  "sdcard"
#define MAX_BUFF_SIZE 1024*1024*1
#define MAX_FILE_SIZE 1024*1024*80


using namespace std;

typedef struct OTA_FILE
{
	char path[64] = {'\0'};
	char file[64] = {'\0'};
}ota_file_t;


class 	ota_unpack
{
	public:
		ota_unpack();
		~ota_unpack();

		//set decompress file path.
		int set_decomp_path( char *filepath, int len);
		//decompress
		int parse_ota_pack( char *fpath);
		//get file list
		list<ota_file_t> * getfilelist();
		//get file nums
		int getfilenums(unsigned int *filenums);
	
	private:
		char *tmpbuff = nullptr;
		unsigned int fnums = 0;
		char decompresspath[60] = {'\0'};
		list<ota_file_t> ota_file;
		
		int parse_box_type(FILE* fd, char *data,   int size, box_type_t * box_type, int *offsize);
		void print_hexP(char *buf, int len, const char *comment);
};

#endif
