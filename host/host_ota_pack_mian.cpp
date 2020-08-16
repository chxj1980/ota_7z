#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>

#include"host_ota_pack.h"

using namespace std;

host_ota_pack_t *host_ota_pack_m = NULL;
list<host_ota_file_t> host_ota_file;

int do_name, do_gf_name;  
char *l_opt_arg;  

//host_ota_pack -i cvf1.bin -t mcu -v 1.0.0 -i fskg.bin -t dsp -v 1.0.1 -p 2  -c 7zip -o a588_fw_20190627.bin 
struct option longopts[] = {  
	 { "input_file _name",	       required_argument,     NULL,		'i'	},  
	 { "file_type", 	       required_argument,     NULL,		't'	},  
	 { "file_version",	       required_argument,     NULL,		'v'	},  
	 { "compress_file num",        required_argument,     NULL,		'p'	},  
	 { "file_compress_algorithm",  required_argument,     NULL,		'c'	},  
	 { "out_file_name",	       required_argument,     NULL,		'o'	},  
	 { "command_help",	       no_argument	,     NULL,		'h'	}, 
	 {	0,     0,     0,     0},  
};  

int get_argv(int argc, char *argv[])
{
	int c;
	bool is_push_flag = false;
	host_ota_file_t  host_ota;
	memset(&host_ota, 0, sizeof(host_ota_file_t));
	memset(host_ota_pack_m, 0,  sizeof(host_ota_pack_t));
	
	while((c = getopt_long(argc, argv, "i:t:v:p:c:o:h", longopts, NULL)) != -1)
	{  
		 switch (c)
		 {  
			case 'i':
				memcpy(host_ota.name, optarg, strlen(optarg));
				printf("input file:		[%s]\n", host_ota.name);
			 	break;  
			 
			case 't':  
				memcpy(host_ota.type, optarg, strlen(optarg));
				printf("file type:		[%s]\n", host_ota.type); 
			 	break;  
			 
			case 'v':
				memcpy(host_ota.version, optarg, strlen(optarg));
				if(host_ota.version[0] < '0' || host_ota.version[0] > '9' )
				{
					printf("Please input right version, eg: [ -v 1.2.3 ]!\n");
					 return -1;
				}
				
				printf("file version:		[%s]\n", host_ota.version);
				host_ota_file.push_back(host_ota);
				printf("*****************************\n");
				
				memset(&host_ota, 0, sizeof(host_ota_file_t)); 
				is_push_flag = true;
				break;
			 
			case 'p':  
				l_opt_arg = optarg; 
				host_ota_pack_m->pack_file_num = atoi(l_opt_arg);
				if(host_ota_pack_m->pack_file_num == 0)
				{
					printf("Please input right file nums, eg: [ -p 3 ]!\n");
					return -1;	
				}
				printf("compress file num:	[%d]\n", host_ota_pack_m->pack_file_num);
				break;  
			 
			case 'c':  
				memcpy(host_ota_pack_m->compress_algorithm, optarg, strlen(optarg));
				printf("file compress algorithm:[%s]\n", host_ota_pack_m->compress_algorithm);  
				break;  
			 
			case 'o':  
				memcpy(host_ota_pack_m->out_file_name, optarg, strlen(optarg));
				printf("out file name: 		[%s]\n", host_ota_pack_m->out_file_name);  
				break;  
			 
			case 'h':
				printf("=====>>:help message, pleae reference to command: host_ota_pack -i cvf1.bin -t mcu -v 1.0.0 -i fskg.bin -t dsp -v 1.0.1 -p 2  -c 7zip -o a588_fw_20190627.bin \n");  
				return -1;    

			default:
				printf("unknown option found:\n");
				break;
		 }  
	 } 

	if((is_push_flag != true) || (host_ota_pack_m->pack_file_num  != host_ota_file.size() ))
	{
		printf("please input right comdand ! reference to run the command: ./host_ota_pack  -h  \n");
		printf("when input one file, please reference  to run command, eg: host_ota_pack -i file1.bin -t mcu -v 1.0.0  -p 1  -c 7zip -o a588_fw_20190627.bin\n");
		return -1;
	}
	
		
	 return 0;  
}
	
int argv_parse(int argc, char *argv[])
{
	if(argc < 1)
	{
		printf("Faile:pleae input such as host_ota_pack -i cvf1.bin -t mcu -v 1.0.0 -i fskg.bin -t dsp -v 1.0.1 -p 2  -c 7zip -o a588_fw_20190627.bin ...\n");
		return -1;
	}

	host_ota_pack_m = new(host_ota_pack_t);
	if( get_argv(argc, argv) < 0)
	{
		return -1;
	}

	return 0;
}

int file_proc()
{
	host_ota_pack hop;
	hop.pack_file(host_ota_pack_m, &host_ota_file);

	free(host_ota_pack_m);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argv_parse(argc, argv) < 0)
	{
		return -1;
	}

	file_proc();

	if(!host_ota_file.empty())
	{
		host_ota_file.clear();
	}
	return 0;
}
