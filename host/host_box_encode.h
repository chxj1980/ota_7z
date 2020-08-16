#ifndef __HOST_BOX_ENCODE_H__
#define __HOST_BOX_ENCODE_H__

#include<string.h>
#include<stdlib.h>
#include<unistd.h>

using namespace std;

#define MAX_FILENAME_LEN 64
#define MAX_BUFF_SIZE 1024 * 1024 *1
#define MAX_BOX_HEAD_SIZE 128

#define DEF_OUT_FILE  "def_compress.bin"

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

typedef  unsigned int   uint32_t;
typedef  char  uchar_t;
typedef  char  uint8;

typedef enum
{
	box_type_magic =MKTAG('b','t','m','c'),
	box_type_file_desc,
	box_type_digest,
	box_type_encrypt,
	box_type_compress,
	box_type_pack,
}box_type_t;

typedef enum
{
	file_type_unknow = 0,
	file_type_dsp_firware,
	file_type_mcu_firware,
	file_type_ui_resource,
}file_type_t;

typedef enum
{
	digest_type_none = 0,
	digest_type_md5,
	digest_type_sha1,
	digest_type_crc32,
	digest_type_crc16,
}digest_type_t;


typedef enum
{
	encrypt_type_none = 0,
	encrypt_type_rsa,
}encrypt_type_t;

typedef enum
{
	compress_type_none = 0,
	compress_type_7zip,
	compress_type_gzip,
	compress_type_zip,
	compress_type_rar,
}compress_type_t;


typedef struct
{
	uint32_t box_type = box_type_file_desc;
	uchar_t file_type = file_type_dsp_firware;
	uchar_t reserved[3];
	char file_name[MAX_FILENAME_LEN];
	uint32_t file_len;
	char data[];
} __attribute__((packed)) box_file_desc_t;


typedef struct
{
	uint32_t box_type = box_type_digest;
	uint8 digest_type = digest_type_none;
	uchar_t reserved[3];
	uint32_t tail_len = 0;
	uint32_t file_len = 0;
	char data[];
} __attribute__((packed)) box_digest_head_t;


typedef struct
{
	char data[];
} __attribute__((packed)) box_digest_tail_t;


typedef struct
{
	uint32_t box_type = box_type_encrypt;
	uint8 encrypt_type = encrypt_type_none;
	uchar_t reserved[3];
	uint32_t tail_len = 0;
	uint32_t file_len = 0;
	char data[];
} __attribute__((packed)) box_encrypt_head_t;

typedef struct
{
	char data[];
} __attribute__((packed)) box_encrypt_tail_t;

typedef struct
{
	uint32_t box_type = box_type_compress;
	uint8 compress_type = compress_type_7zip;
	uchar_t pack_fname[64] = {'\0'};
	uchar_t reserved[20] = {'\0'};
	uchar_t version[36];
	
	uint32_t tail_len = 0;
	uint32_t file_len = 0; //compressed already file
	char data[];
} __attribute__((packed)) box_compress_head_t;

typedef struct
{
	char data[];
} __attribute__((packed)) box_compress_tail_t;

typedef struct
{
	uint32_t box_type = box_type_pack;
	uint32_t file_num = 0;
	uint32_t file_offset[];
} __attribute__((packed)) box_pack_t;

class host_box_encode
{
	public:
		host_box_encode(uchar_t *_7z_enc_fname);
		~host_box_encode();

		int init_box_encode( );
		int box_pack_file(unsigned int file_num);

		int _7zenc_pack();
		int box_7zenc_pack(char *compress_algorithm);
		int box_file_desc(char *inname, char* file_type, char *version);
		unsigned long get_file_size(const char *path);
		int filetype_parse(char *type_m, uint32_t lenth, file_type_t *type);
		int readinfile_to_outfile(FILE *in_fd, FILE *out_fd, char *buff, uint32_t size);

		void print_hexP(char *buf, int len, const char *comment);

	private:		
		box_pack_t	*box_pack_m = NULL;
		
		uchar_t box_pack_out_fname[64] = {'\0'};
		FILE *pack_fd_m;
		
		uchar_t tmp_7zenc_in_file[64] ={'\0'};
		uchar_t  _7z_enc_out_file[64] ={'\0'};
 		uint32_t file_name_num = 1;
		uchar_t *box_buff = NULL; 
};

#endif
