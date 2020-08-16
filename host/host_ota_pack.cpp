#include"host_ota_pack.h"
#include"host_box_encode.h"

using namespace std;

host_ota_pack::host_ota_pack()
{

}

host_ota_pack::~host_ota_pack()
{

}

int host_ota_pack::pack_file(host_ota_pack_t *hop, list<host_ota_file_t> *host_ota_file)
{
	if(hop == NULL)
	{
		return -1;
	}

	int ret = 0;
	host_box_encode hbe(hop->out_file_name);

	list<host_ota_file_t>::iterator iter;
	host_ota_file_t file;

	//init 
	hbe.init_box_encode( );
	
	printf("\nbox file descrption:\n");
	for(iter = host_ota_file->begin(); iter != host_ota_file->end(); iter++)
	{
		memset(&file, 0, sizeof(host_ota_file_t));
		file = *iter;
		ret = hbe.box_file_desc(file.name, file.type, file.version);//input file: name, type,version.
		if(ret < 0)
		{
			return -1;
		}
	}

	printf("\nbox pack file:\n");
	ret = hbe.box_pack_file(hop->pack_file_num); 
	if(ret < 0)
	{
		printf("tar pack file failed !\n");
	}
	
	printf("\nbox 7z enc file:\n");
	ret = hbe.box_7zenc_pack(hop->compress_algorithm);
	if(ret < 0)
	{
		printf("7z pack compress file failed !\n");
	}

	return 0;
}

