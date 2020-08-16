
#include"box_decode.h"
#include "am_dec7z_if.h"


box_decode::box_decode()
{

}

box_decode::~box_decode()
{

}

int box_decode::readinfile_to_outfile(FILE *in_fd, FILE *out_fd,  char *buff, uint32_t size)
{
	uint32_t readlen = 0;
	
	//file data.
	while(!feof(in_fd))
	{
		INFO("xx ");
		readlen = fread(buff, 1, size, in_fd);
		if(readlen < 0)
		{
			perror("read faile:");
			return -1;
		}
		
		readlen = fwrite(buff, 1, readlen, out_fd);
		if(readlen < 0)
		{
			perror("write faile:");
			return -1;
		}
		
		memset(buff, 0, size);
	}
	PRINTF("\n");

	fflush(out_fd);
	return 0;
}

int box_decode::read_file_desc(FILE *in_fd, FILE *out_fd, char *buff, uint32_t size,  uint32_t file_len)
{
	uint32_t readlen = 0;
	uint32_t i = 0;
	int time = 20;
	
	//file data.
	while(time--)
	{
		PRINTF("x ");
		//printf("have read:[%d], need read:[%d],  size: %d\n", i, file_len, size);
		if((file_len - i) <= size)
		{
			size = file_len - i;
		}
			
		readlen = fread(buff, 1, size, in_fd);
		if(readlen < 0)
		{
			perror("read faile:");
			return -1;
		}
		
		readlen = fwrite(buff, 1, size, out_fd);
		if(readlen < 0)
		{
			perror("write faile:");
			return -1;
		}

		i+=readlen;
		if(i >= file_len)
		{
			INFO("have read:[%d], need read:[%d] \n", i, file_len);
			break;
		}
	}	
	PRINTF("\n");
	
	fflush(out_fd);
	
	if(i == file_len)
	{
		ERROR("read file description success !\n");
		return 0;
	}
	
	return -1;
}

int box_decode::parse_file_desc(box_file_desc_t      *file_desc,  char *data,  unsigned int size)
{
	if(file_desc == NULL)
	{
		ERROR(" This file_desc is not empty !\n");
		return -1;
	}

	box_file_desc_t	 box_file_desc;
		
	if( (data[0] & 0x000000ff )== box_type_file_desc)
	{	
		memcpy(file_desc, data, sizeof(box_file_desc_t));
		
		INFO("\nname      :%s\n", file_desc->file_name);
		INFO("file_len  :%d\n",   file_desc->file_len);
		INFO("file_type :%02x\n", file_desc->file_type);
		INFO("box_type  :%02x\n", file_desc->box_type);
		
		return 0;	
	}
	else
	{
		ERROR("Please check file_desc box flag form the data!\n");
		return  -1;
	}	
}

int box_decode::parse_compress(box_compress_head_t      *compress_head,  char *data,  unsigned int size)
{
	if(compress_head == NULL)
	{
		ERROR(" This file_desc is not empty !\n");
		return -1;
	}
		
	if( (data[0] & 0x000000ff ) == box_type_compress)
	{	
		memcpy(compress_head, data, sizeof(box_compress_head_t));
		
		INFO("\nfile len      :%d\n", compress_head->file_len);
		INFO("version       :%s\n", compress_head->version);
		INFO("compress_type :%02x\n", compress_head->compress_type);
		INFO("box_type      :%02x\n", compress_head->box_type);
		INFO("reserved      :%s\n", compress_head->reserved);
		
		return 0;	
	}
	else
	{
		ERROR("Please check process box flag form the data!\n");
		return  -1;
	}	
}

int box_decode::uncompress_file(char *file_name, uint32_t *size)
{	
	#if 0
	if( file_name == NULL )
	{
		printf(" unprocess file do not find !\n");
		return -1;
	}

	char cmd[100] = {'\0'};
	uint32_t len = 0;
	
	const char *_7z_enc_cmd ="./7za x ";
	
	sprintf(cmd, "%s %s -o%s", _7z_enc_cmd, file_name,  dirname(file_name));//
	len = strlen(_7z_enc_cmd);
	
	system(cmd);
	#else
		decode(file_name);
	#endif
	return 0;
}

int box_decode::decode(char *file)
{
	unsigned long long time_ms = 0L;
	if(file == NULL)
	{
		ERROR("Please input dec file!\n");
		return -1;
	}

	//string filename = file;
	PRINTF("dec file name:%s\n", file);
	
	AMIDec7zPtr dec = AMIDec7z::create(file);
	if (dec)
	{
		dec->dec7z("/sdcard");
		PRINTF("dec finished !\n");
	}
	PRINTF("destroyed!, USE TIME:[%lld]\n",  time_ms);

	return 0;
}



int box_decode::get_packet_file(char *tar_file, char *tmpdir)
{
	if(tmpdir == NULL)
	{
		ERROR("dir is not empty !\n");
		return -1;
	}
	
	DIR *dp ;
	struct dirent *dirp ;
	
	if( (dp = opendir( tmpdir )) == NULL ) //打开指定目录
	{
		perror("opendir failed, error:");
	}
	
	while( ( dirp = readdir( dp ) ) != NULL) //开始遍历目录
	{
		if(strcmp(dirp->d_name,".")==0  || strcmp(dirp->d_name,"..")==0) //跳过'.'和'..'两个目录
			continue;
 
		int size = strlen(dirp->d_name);	
		if(strncmp(dirp->d_name + (size - 4), ".tar", 4) != 0) //只存取.tar扩展名的文件名
			continue;	
		
		memcpy(tar_file, dirp->d_name, strlen(dirp->d_name));
	}
 	
	closedir(dp);
	PRINTF("find upgrate file:%s\n", tar_file);
	return 0;
}

int box_decode::parse_pack( unsigned int *filenums,  char *data, int *headersize)
{		
	if( (data[0] & 0x000000ff ) == box_type_pack)
	{	
		if( (data[4] & 0x000000ff) == 0)
		{
			ERROR("Don't cheack the file num !\n");
		}

		box_pack_t  *pack = (box_pack_t *)malloc(sizeof(box_pack_t) + sizeof(uint32_t) * data[4]);
		*headersize = sizeof(box_pack_t) + sizeof(uint32_t) * data[4];
		memcpy(pack, data, sizeof(box_pack_t) +  sizeof(uint32_t) * data[4]);

		PRINTF("\nbox_type         :%02x\n", pack->box_type);
		PRINTF("file_num         :%d\n", pack->file_num);
		*filenums = pack->file_num;
		for(unsigned int i = 0; i < pack->file_num; i++)
		{
			INFO("file_size[%d]    :%d\n", i, pack->file_offset[i]);
		}
		
		return 0;	
	}
	else
	{
		ERROR("Please check process box flag form the data!\n");
		return  -1;
	}	

	return 0;
}

int box_decode::untar_pack(char *tarfile, char *tarpath)
{
	if( tarfile == NULL  ||  tarpath == NULL )
	{
		ERROR(" tar file do not find !\n");
		return -1;
	}

	char cmd[100] = {'\0'};
	uint32_t len = 0;
	
	const char *_7z_enc_cmd ="tar -xvf ";
	
	sprintf(cmd, "%s %s -C %s", _7z_enc_cmd, tarfile, tarpath);//
	len = strlen(_7z_enc_cmd);

	PRINTF("cmd[%d]:%s\n", len, cmd);
	
	system(cmd);
	
	return 0;
}
