#include<iostream>
#include<sys/stat.h>
#include<list>

#include"host_box_encode.h"

host_box_encode::host_box_encode( uchar_t *_7z_enc_fname)
{
	if(_7z_enc_fname != NULL)
	{
		memcpy(_7z_enc_out_file, _7z_enc_fname, strlen(_7z_enc_fname));
	}
	else
	{	
		 memcpy(_7z_enc_out_file, DEF_OUT_FILE, strlen(DEF_OUT_FILE));// if do not appointed the out file name, will use default.
	}

	box_buff = (uchar_t *)malloc(MAX_BUFF_SIZE);
	if(box_buff == NULL)
	{
		perror("malloc faile:");
	}
	
	memset(box_buff, 0, MAX_BUFF_SIZE);
}

host_box_encode::~host_box_encode()
{
	if(box_buff != NULL)
	{
		free(box_buff);
		box_buff = NULL;
	}

	if(box_pack_m != NULL)
	{
		free(box_pack_m);
		box_pack_m = NULL;
	}
}

int host_box_encode::filetype_parse(char *type_m, uint32_t lenth, file_type_t *type)
{
	if(type_m == NULL)
	{
		return -1;
	}

	char *buff = type_m;
	if(strncmp(buff, "dsp", 3) == 0)
	{
		*type =  file_type_dsp_firware;
	}
	else if(strncmp(buff, "mcu", 3)  == 0)
	{
		*type =  file_type_mcu_firware;
	}
	else if(strncmp(buff, "ui", 2)  == 0)
	{
		*type =  file_type_ui_resource;
	}
	else
	{
		printf("Cant not judge the file type, error exit!\n");
		return -1;
	}
	
	return 0;
}

int host_box_encode::readinfile_to_outfile(FILE *in_fd, FILE *out_fd,  char *buff, uint32_t size)
{
	uint32_t readlen = 0;
	uint32_t i = 0;

	fseek(in_fd, 0L, SEEK_SET);
	
	//file data.
	while(!feof(in_fd))
	{
		printf("==> ");
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
	printf("\n");

	fflush(out_fd);
	return 0;
}

unsigned long host_box_encode::get_file_size(const char *path)
{
	unsigned long filesize = -1;	
	struct stat statbuff_t;
	if(stat(path, &statbuff_t) < 0)
	{
		return filesize;
	}else{
		filesize = statbuff_t.st_size;
	}

	return filesize;
}

int host_box_encode::init_box_encode( )
{
	char tmpname[64] = {'\0'};
	int ret = 0;
	memset(box_pack_out_fname, 0, sizeof(box_pack_out_fname));

	strncpy(tmpname, _7z_enc_out_file,  strlen(_7z_enc_out_file) -4 );
	sprintf(box_pack_out_fname, "%s_pack.bin", tmpname);
	
	//128 Byte for box header, 8 Byte(type+file_num) + 120 Byte(file_len: the len of the 30 files)
	box_pack_m = (box_pack_t *)malloc(MAX_BOX_HEAD_SIZE);
	memset(box_pack_m, 0, MAX_BOX_HEAD_SIZE);

	pack_fd_m = fopen(box_pack_out_fname,  "w+");
	if(pack_fd_m < 0)
	{
		perror("failed open, error:\n");
		return -1;
	}

	ret = fwrite(box_pack_m, 1, MAX_BOX_HEAD_SIZE, pack_fd_m);
	if(ret !=  MAX_BOX_HEAD_SIZE)
	{
		perror("failed write, error:\n");
		return -1;
	}


	return 0;
}

int host_box_encode::box_file_desc( char *inname, char* file_type, char *version)
{
	if(inname == NULL || file_type == NULL || version == NULL)
	{
		return -1;
	}
	
	printf("----------------------------------------------------------------\n");

	int ret = 0;
	int len_th = 0;
	int file_offsize = 0;
	char *buff = NULL;

	char *p = NULL;
	FILE *infd = NULL;
	char tmpname[64] = {'\0'};
	uint32_t perlen = MAX_BUFF_SIZE;
	static uint32_t files = 0;
	box_file_desc_t box_file_desc;
	file_type_t filetype  = file_type_dsp_firware;

	if(box_buff != NULL)
	{
		buff = box_buff;
	}
	
	memset(&box_file_desc, 0, sizeof(box_file_desc));

	if((p = strstr(inname, ".bin"))!= NULL)
	{
		infd = fopen(inname, "r");
		if(infd <= 0)
		{
			printf("==>%s\n", inname);
			perror("faile:open file, error:");
			return -1;
		}

		file_offsize = ftell(pack_fd_m);
		box_pack_m->file_offset[files++] = file_offsize; //Record the location of the input file 
		printf("offsize:%d\n", file_offsize);
		//add file descrption.
		filetype_parse(file_type, strlen(inname), &filetype);
		box_file_desc.box_type = box_type_file_desc;
		box_file_desc.file_type = filetype;
		box_file_desc.file_len = get_file_size(inname)  + sizeof(box_file_desc_t); 
		memcpy(box_file_desc.file_name, inname, strlen(inname));
		memcpy(box_file_desc.reserved, version, strlen(version));
	
		ret = fwrite(&box_file_desc, 1, sizeof(box_file_desc_t), pack_fd_m); //add the file description head to pack, after 128Byte.
		if(ret != sizeof(box_file_desc_t))
		{
			perror("read faile:");
			goto ERROR;
		}

		print_hexP((char *)&box_file_desc, 36, "head:");
		readinfile_to_outfile(infd, pack_fd_m, buff, perlen); //add the file data to pack,

		printf("==: int file:[%s] size:[%ld]==\n", inname, get_file_size(inname));
		
		fclose(infd);

		printf("----------------------------------------------------------------\n");
		
		return 0;
	}	
	else
	{
		printf("[%s] file is not standard .bin file, exit!\n", inname);
		return -1;
	}

	ERROR:
		fclose(infd);
		memset(box_buff, 0, MAX_BUFF_SIZE);
	return -1;	
}


void host_box_encode::print_hexP(char *buf, int len, const char *comment)
{
	int i = 0;
	printf("\r\n%s start buf:%p len:%d \r\n", comment, buf, len);
	for(i = 0; i < len; i++)
	{
	if(i%16 == 0)
		printf("\r\n");
	 printf("%02x ", buf[i]);
	}
}

int host_box_encode::box_pack_file(unsigned int file_num)
{

	int ret = 0;
	box_pack_m->box_type = box_type_pack;
	box_pack_m->file_num = file_num; //when input the file nums error from command line. use the 

	fseek(pack_fd_m, 0L, SEEK_SET);

	ret = fwrite(box_pack_m, 1, MAX_BOX_HEAD_SIZE, pack_fd_m); //add the file description head to pack, after 128Byte.
	if(ret != MAX_BOX_HEAD_SIZE)
	{
		perror("write faile:");
		return -1;
	}

	print_hexP((char *)box_pack_m, 36, "head:");

	fclose(pack_fd_m);

	free(box_pack_m);
	box_pack_m = NULL;
	
	return 0;
}

int host_box_encode::box_7zenc_pack(char *compress_algorithm_m)
{
	char compress_algorithm[20] = {'\0'};
	if(compress_algorithm == NULL)
	{
		printf("######################################\n");
		memcpy(compress_algorithm, "7zip", 4);
	}
	else
	{
		memcpy(compress_algorithm, compress_algorithm_m, strlen(compress_algorithm_m));
	}
	
	int ret = 0;
	printf("****************************************************************\n");
	if(_7zenc_pack() == 0)
	{
		time_t rawtime;
		struct tm *info;
		char time_t[70];

		FILE *_7zenc_in_fd;
		FILE *_7zenc_out_fd;
		uchar_t _7zenc_out_name[64] ={'\0'};
		uint32_t _7zenc_out_name_len = 0;
		uint8 *box_7zenc_buff = NULL;
		uint32_t persize = MAX_BUFF_SIZE;
		
		box_compress_head_t box_compress_head;
		
		box_compress_head.box_type = box_type_compress;
		box_compress_head.compress_type = compress_type_7zip;
		memcpy(box_compress_head.pack_fname, box_pack_out_fname,  strlen(box_pack_out_fname)); //for ease of find pack file.
		box_compress_head.tail_len = 0;
		box_compress_head.file_len = get_file_size(tmp_7zenc_in_file);
		memcpy(box_compress_head.reserved, compress_algorithm, strlen(compress_algorithm));

		time( &rawtime );
		info = localtime( &rawtime );
		strftime(time_t, 50, "%y-%m-%d-%H:%M", info);
		memcpy(box_compress_head.version, time_t,  strlen(time_t));
		
		sprintf(_7zenc_out_name, "%s", _7z_enc_out_file);		
		printf("Generate a new 7z  compress file, file name: %s\n", _7zenc_out_name);

		_7zenc_out_fd = fopen(_7zenc_out_name, "w+");
		if(_7zenc_out_fd <= 0)
		{
			perror("faile:open out 7z enc file, error:");
			return -1;
		}
		
		

		ret = fwrite(&box_compress_head, 1, sizeof(box_compress_head_t), _7zenc_out_fd);
		if(ret < 0)
		{
			perror("Writ box 7z compress header failed, exit! error:\n");
			return -1;
		}

		memset(box_buff, 0, MAX_BUFF_SIZE);
		box_7zenc_buff = box_buff;

		_7zenc_in_fd = fopen(tmp_7zenc_in_file, "r");
		if(_7zenc_in_fd <= 0)
		{
			perror("faile:open in 7z compress file, error:");
			return -1;
		}

		ret = readinfile_to_outfile(_7zenc_in_fd, _7zenc_out_fd, box_7zenc_buff, persize);
		if(ret < 0)
		{
			printf("read pack input file data to box pack out file failed, exit!\n");
		}

		printf("==[tar]: out file:[%s] size:[%ld]==\n", _7zenc_out_name, get_file_size(_7zenc_out_name));
		printf("****************************************************************\n");
		
		fclose(_7zenc_in_fd);
		fclose(_7zenc_out_fd);
		free(box_buff);
		
		box_buff = NULL;
		printf("Box finish !\n");
		
		return 0;
	}
	
	return -1;
}

int host_box_encode::_7zenc_pack()
{
	if((box_pack_out_fname[0] == '\0') || (_7z_enc_out_file[0] == '\0'))
	{
		printf("packet or 7z enc file do not find !\n");
		return -1;
	}

	char cmd[100] = {'\0'};
	uint32_t len = 0;
	
	sprintf(tmp_7zenc_in_file, "_%s", _7z_enc_out_file);
	const char *_7z_enc_cmd ="./7za a ";
	sprintf(cmd, "%s", _7z_enc_cmd);
	len = strlen(_7z_enc_cmd);

	sprintf(cmd + len, " %s ", tmp_7zenc_in_file);//xx.bin [after 7z command compress.]
	len += strlen(tmp_7zenc_in_file) + 2;

	sprintf(cmd + len, " %s", box_pack_out_fname);//xx_pack.bin
	len = strlen(box_pack_out_fname) + 1;
	
	system(cmd);
	printf("process file:%s\n", tmp_7zenc_in_file);

	if(access(tmp_7zenc_in_file, 0) < 0)
	{
		printf(" 7z compress file generate failed , file is not, exisit !\n");  
		return -1;
	}
	
	printf("==[7z]: in file:[%s] size:[%ld]==\n", tmp_7zenc_in_file, get_file_size(tmp_7zenc_in_file));
	return 0;
}

