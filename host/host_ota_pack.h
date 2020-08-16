#ifndef __HOST_OTA_PACK_H__
#define __HOST_OTA_PACK_H__

#include<iostream>
#include<list>

using namespace std;


typedef struct  
{
	char name[64] = {'\0'}; //init to be '\0' for open file success.
	char type[44] = {'\0'};	
	char version[64] = {'\0'};
	
}host_ota_file_t;

typedef struct 
{
	char compress_algorithm[32] = {'\0'};
	char out_file_name[64] = {'\0'};
	unsigned int pack_file_num = 1;
}host_ota_pack_t;

class host_ota_pack
{
	public:
		host_ota_pack();
		~host_ota_pack();
		
		int pack_file(host_ota_pack_t *hop, list<host_ota_file_t> *host_ota_file);

	private:

};

#endif

