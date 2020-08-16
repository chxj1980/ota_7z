
#include"ota_unpack.h"

ota_unpack::ota_unpack()
{
	if(tmpbuff == NULL)
	{
		tmpbuff = (char *)malloc(MAX_BUFF_SIZE);
	}
	
	memset(tmpbuff,  0, MAX_BUFF_SIZE);
	
	fnums = 0;
}

ota_unpack::~ota_unpack()
{
	if(tmpbuff != NULL)
	{
		free(tmpbuff);
		tmpbuff = NULL;
	}

	if(!ota_file.empty())
	{
		ota_file.clear();
	}
}

int ota_unpack::parse_box_type(FILE* fd, char *data, int size, box_type_t * box_type, int *offsize)
{	
	int ret = 0;
	int i = 1;
	char *tmpdata =NULL;
	box_type_t tmp_box_type = box_type_magic;
	
	ret = fread(data, 1, size, fd);
	if(ret != size)
	{
		//perror("read file header failed, error:");
		return -1;
	}
	
	tmpdata = data;
	//box type:
	while(size--)
	{
		for(; (tmp_box_type + i) < box_type_max; i++ )
		{
			if(MKTAG(tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]) == tmp_box_type + i) 
			{
				*box_type = (box_type_t)(tmp_box_type + i);// Calculate the offsize
				*offsize = tmpdata - data; //
				printf("^^^^^^^^^^^^^^^^77777777777777\n");
				return 0; 
			}
		}

		i = 1;
		tmp_box_type = box_type_magic;

		tmpdata++;
	}
	
	return 0;
}

int ota_unpack::set_decomp_path( char *filepath, int len)
{
	if(filepath == NULL)
	{
		ERROR("decompress file path is not empty !\n");
		return -1;
	}

	if((unsigned int)len > sizeof(decompresspath) / sizeof(char))
	{
		ERROR("The file path  len is too long, more then:[%d]!\n", sizeof(decompresspath) / sizeof(char));
		return -1;
	}

	memset(decompresspath, '\0', sizeof(decompresspath));
	memcpy(decompresspath, filepath, len);
	
   return 0;
}

int ota_unpack::parse_ota_pack(char *fpath)
{
	
	FILE *ota_fd;
	FILE *tmp_fd;
	FILE *tmp_sd_fd;
	
	int times = 20;
	
	int box_offsize = 0;
	char file_Name[52] = {'\0'};
	char file_path[42] = {'\0'};
	char tmp_file[64] = {'\0'};
	char tmp_tarfile[64] = {'\0'};

	char tmp_f[64] = {0};
	char cmd[32] = {'\0'};
	char decompressfile[64] = {'\0'};
	char *tmp = NULL;
	
	long  int len = 0;
	long  int file_offsize = 0;
	unsigned int uncompress_size = 0; //uncompress file size.

	ota_file_t otafile;
	box_decode decode;
	box_file_desc_t  file_desc;
	box_compress_head_t compress_head;

	int headsize = 0;
	memset(&compress_head, 0, sizeof(box_compress_head_t));
	memset(&file_desc, 0, sizeof(box_file_desc_t));
	
	box_type_t box_type = box_type_magic;
	
	char headdata[RD_HEAD_SIZE] = {'\0'};
	
	if(fpath == NULL)
	{
		ERROR("Plese input oat file path !\n"); 
		return -1;
	}
	
	if(access(fpath, 0) < 0)
	{
		ERROR("OTA file can not be find .\n");
		return -1;
	}

	ota_fd = fopen(fpath, "r");
	if(ota_fd < 0)
	{
		perror("Open ota file failed!,error:");
		return -1;
	}

	tmp_fd = ota_fd;

	//set decompress  		file 	 path.
	if(decompresspath[0] != '\0')
	{
		memcpy(file_path, decompresspath, strlen(decompresspath));
	}
	else
	{
		sprintf(file_path, "%s", DEF_OTA_PATH_SD);//tmp file save path:[sd path]
	}
	
	//parse and decompress  		file.
	do{
		memset(headdata, 0, RD_HEAD_SIZE);
		memset(tmp_file, 0, sizeof(tmp_file));

		file_offsize = ftell(tmp_fd);
		parse_box_type(tmp_fd, headdata, RD_HEAD_SIZE, &box_type, &box_offsize);
		switch(box_type)
		{
			case box_type_file_desc:
				decode.parse_file_desc(&file_desc, headdata + box_offsize, RD_HEAD_SIZE - box_offsize);
				box_type = box_type_magic;
			
				if((file_desc.file_type <= file_type_unknow) || (file_desc.file_type > file_type_ui_resource))
				{
					ERROR("Don't know the file type!\n");
					break;
				}
				
				if((file_desc.file_len <= 0) || (file_desc.file_len >= MAX_FILE_SIZE) )
				{
					ERROR("The file size is too big, more then [%d] !\n", MAX_FILE_SIZE);
					break;
				}
				
				memset(file_Name, 0, sizeof(file_Name));
				if(file_desc.file_name != NULL)
				{
					sprintf(file_Name, "%s/", file_path);
					len = strlen(file_path) + 1;
					
					tmp = strstr(file_desc.file_name, "_1.");//filter the tmp file flag."-1."
					if(tmp == NULL)
					{
 						ERROR("Can not find the tmp file flag , exit !\n");
						break;
					}
					
					strncpy(file_Name + len , file_desc.file_name, tmp - file_desc.file_name);	
					len += tmp - file_desc.file_name;
					sprintf(file_Name + len, ".%s", tmp + 3); //fill the file name extension.
					PRINTF("Generator the file:%s\n", file_Name);
					tmp_sd_fd = fopen(file_Name, "w+");
					if(tmp_sd_fd < 0)
					{
						perror("Open create file failed, error:\n");
						break;
					}
					
					//read from tar file
					fseek(tmp_fd, file_offsize + box_offsize + sizeof(box_file_desc_t), SEEK_SET);//locate to file descrption header.
					decode.read_file_desc(tmp_fd, tmp_sd_fd, tmpbuff, MAX_BUFF_SIZE, file_desc.file_len); 

					len = 0;
					memset(&otafile, 0,  sizeof(ota_file_t));
					
					len =strlen(basename(file_Name));
					memcpy(otafile.file, basename(file_Name),  len );
					memcpy(otafile.path, file_path,  strlen(file_path) );	
					ota_file.push_back(otafile);
						
					fclose(tmp_sd_fd);
					len = 0;
				}
				
				break;
				
			case box_type_digest:
				break;

			case box_type_encrypt:
				break;
				
			case box_type_compress: 
				decode.parse_compress(&compress_head, headdata + box_offsize, RD_HEAD_SIZE - box_offsize);
				box_type = box_type_magic;
				
				memset(file_Name, 0, sizeof(file_Name));
				sprintf(file_Name, "%s/uncompress.%s", file_path, compress_head.reserved);//reserved saved compress arithmetic string.
				tmp_sd_fd = fopen(file_Name, "w+");
				if(tmp_sd_fd < 0)
				{
					perror("Open create file failed, error:\n");
					break;
				}

				memcpy(tmp_f, file_Name, strlen(file_Name));
				PRINTF("lenth:[%d]\n", sizeof(box_compress_head_t));
			
				fseek(tmp_fd, sizeof(box_compress_head_t) , SEEK_SET);
				if (decode.readinfile_to_outfile(tmp_fd, tmp_sd_fd, tmpbuff, MAX_BUFF_SIZE) < 0)
				{
					ERROR("read write file error !\n");
					break;
				}

				decode.uncompress_file(file_Name, &uncompress_size);
				memcpy(decompressfile, file_Name, strlen(file_Name));

				fclose(tmp_sd_fd);
				fclose(tmp_fd);

				//get the box tar file.
				sprintf(tmp_tarfile, "%s/%s", file_path, compress_head.pack_fname);
				PRINTF("tar file:[%s]\n", tmp_tarfile);
				
				tmp_fd = fopen(tmp_tarfile, "r");
				if(tmp_fd < 0)
				{
					perror("Open create file failed, error:\n");
					break;
				}
				
				break;

			case box_type_pack:
				decode.parse_pack(&fnums, headdata + box_offsize, &headsize);
				fseek(tmp_fd, );
				box_type = box_type_magic;
			
				break;

			default:
				break;
		}
	}while(times--);

	free(tmpbuff);
	tmpbuff = NULL;

	//deleat tmp file.
	if(access(tmp_tarfile, 0) != -1)
	{
		fclose(tmp_fd);
		sprintf(cmd, "rm -rf %s", tmp_tarfile);
		system(cmd);

		if(access(tmp_f, 0) != -1)
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "rm -rf %s", tmp_f);
			system(cmd);
		}
	}
	
	return 0;	
}

int ota_unpack::getfilenums(unsigned int *filenums)
{
	if(fnums <= 0 )
	{
		ERROR("error, file nums is empty, exit !\n");
		return -1;
	}
	
	*filenums = fnums;
	return 0;
}

list<ota_file_t> * ota_unpack::getfilelist( )
{

	if(!ota_file.empty())
	{
		return &ota_file;
	}
	
	return	NULL;
}

