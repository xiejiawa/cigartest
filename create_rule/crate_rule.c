#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <getopt.h>
#include "../include/errlog.h"

#define FILE_SIZE 1024
#define SLOT	  6   /*槽位号*/
static int rule_type  = 0; 
static int rule_count = 0; 
char file_name[128];
char logfile[128];

int isnum(char s[])
{
	int i;  
	for (i=0;i<(int)strlen(s);i++) {
		if ( s[i] < '0' || s[i] > '9' ) {
        return 0;
		}       
	}
	return 1;
}

void rule_create_print_usage()
{
    printf("usage: rule_crate\n");
    printf("  [ -r ] #rule_type :\n");
    printf("          [  1 ] #ipv4掩码五元组：ipv4_mask\n");
    printf("          [  2 ] #ipv4动态五元组：ipv4_flex\n");
    printf("          [  3 ] #ipv6掩码五元组：ipv6_mask\n");
    printf("          [  4 ] #ipv6动态五元组：ipv6_flex\n");
    printf("          [  5 ] #浮动位置关键词：dpi_float\n");
    printf("          [  6 ] #固定位置关键词：dpi_fix\n");
    printf("          [  7 ] #L2规则        ：L2\n");
    printf("  [ -n ] # rule count\n");
    printf("  eg   ./rule_crate -r 2 -n 10w\n");
}

int rule_create_parse_param(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 3) {
        rule_create_print_usage();
        printf("param too short!\n");
        return -1;
    }

    while ((ret = getopt(argc, argv, "r:n:h")) != -1) {
        switch (ret) {
			case 'n':
				if (isnum( optarg )) {
					rule_count = atol(optarg);
				} else if (strstr(optarg,"W") || strstr(optarg,"w")) {
					rule_count = atol(optarg) * 10000;
				} else {
					printf("Only suport W(w) or pure number!\n");
					rule_create_print_usage();
					return -1;
				}
				break;
			case 'r':
				rule_type = atoi(optarg);
				break;
			case 'h':
			default:
				rule_create_print_usage();
				return -1;
        }
    }

    return 0;
}

int parse_file_name(int rule_type)
{
    char tmp_count[256];
    if (rule_count >= 10000) {
        sprintf(tmp_count,"%dw",rule_count/10000 );
    } else {
        sprintf(tmp_count,"%d",rule_count );
        
    }
	switch(rule_type) {
		case 1:
            DBUG("cpme into create ipv4-mask\n");
			//sprintf(file_name,"./rule/ipv4_mask_%dw.txt",rule_count/10000);
			sprintf(file_name,"./rule/ipv4_mask_%s.txt",tmp_count);
			break;
		case 2:
			sprintf(file_name,"./rule/ipv4_flex_%dw.txt",rule_count/10000);
			break;
		case 3:
			sprintf(file_name,"./rule/ipv6_mask_%dw.txt",rule_count/10000);
			break;
		case 4:
			sprintf(file_name,"./rule/ipv6_flex_%dw.txt",rule_count/10000);
			break;
		case 5:
			sprintf(file_name,"./rule/dpi_float_%dw.txt",rule_count/10000);
			break;
		case 6:
			sprintf(file_name,"./rule/dpi_fix_%dw.txt",rule_count/10000);
			break;
		case 7:
			sprintf(file_name,"./rule/L2_%d.txt",rule_count);
			break;
		default:
            rule_create_print_usage();
            return -1;
	}
    return 0;
}
int main(int argc, char *argv[])
{
	//int IP_1 = 1;
	int IP_2 = 0;
	int IP_3 = 0;
	int IP_4 = 0;
	int rule_id   = 1;
	int ret = 0;
	
	FILE *fp;
	char rule_str[FILE_SIZE];
	memset(rule_str, 0x00, FILE_SIZE);
	char tmp_rule_str[FILE_SIZE];
	memset(tmp_rule_str, 0x00, FILE_SIZE);
    sprintf(logfile, "create_rule");
    DBUG("this is the first log file\n");
	ret = rule_create_parse_param(argc, argv);
	if ( ret < 0) {
		return -1;
	}

	ret = parse_file_name(rule_type);
	if ( ret < 0) {
		return -1;
	}

	fp = fopen(file_name, "w+" );
	if( fp == NULL ) {
		printf( "open file error!!\n" );
		return -1;
	}
	setbuf( fp, (char*)NULL );
	if ( rule_type == 7 ) {
		/* L2规则:通过脚本方式添加，并且单用户生成500条规则*/
		sprintf(rule_str, "configure terminal\nacl\n");
		fprintf(fp, "%s", rule_str);
	}


	for(IP_2=1; IP_2 < 256; IP_2++)
	    for(IP_3=0; IP_3 < 256; IP_3++)
			for(IP_4=0; IP_4 < 256; IP_4++)
			{
				if ( rule_type == 1 ) {
					/* ipv4_mask	(sip dip sport dport proto sip-mask dip-mask sport-mask sport-mask proto-mask model forward-action aging-time)  */	
					/* 生成规则		(sip+dip model forward-action)  */	
					sprintf(rule_str, "10.%d.%d.%d 192.0.0.1 0 0 0 255.255.255.255 255.255.255.255 0 0 0 1 1 0\n",IP_2, IP_3, IP_4);
				}
				
				if ( rule_type == 2 ) {
					/* ipv4_flex  	(sip dip sport dport proto model forward-action aging-time) */	
					/* 生成规则  	(sip+sport+proto model forward-action) */	
					//sprintf(rule_str, "10.%d.%d.%d 0.0.0.0 1024 0 6 1 1 0\n",IP_1, IP_3, IP_4);
					sprintf(rule_str, "10.%d.%d.%d 10.10.10.10 0 0 0 1 1 0\n",IP_2, IP_3, IP_4);
				}
				
				if ( rule_type == 3 ) {
					/* ipv6_mask	(sip dip sport dport proto sip-mask dip-mask sport-mask sport-mask proto-mask model forward-action aging-time) */
					sprintf(rule_str, "100::%x 200::1 0 0 0 ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff 0 0 0 1 14 0\n", rule_id);
				}

				if ( rule_type == 4 ) {
					/* ipv6-flex	(sip dip sport dport proto model forward-action aging-time) */
					sprintf(rule_str, "100::%x 200::1 0 0 0 ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff 0 0 0 1 14 0\n", rule_id);
				}
				
				if ( rule_type == 5 ) {
					/* dpi-float	(sip dip sport dport proto sip-mask dip-mask sport-mask dport-mask proto-mask model forward-action aging-time key offset keymask )*/	
					/* dpi-float	(sip+sport+proto sip-mask  sport-mask proto-mask model forward-action  key offset)*/	
					//sprintf(rule_str, "10.%d.%d.%d 0.0.0.0 1024 0 6 255.255.255.255 0.0.0.0 0xffff 0 0xff 1 1 0 0x6a_%02x_%02x_%02x 65535\n", IP_2, IP_3, IP_4, IP_2, IP_3, IP_4);
					sprintf(tmp_rule_str, "\\x%02x\\x%02x\\xff\\xff\\xff\\xff\\xff\\xff", IP_3, IP_4);
					if (rule_id > 100){
					sprintf(rule_str, "rule_id=%d sg_id=0 pg_id=0 task_id=0 content=%s type=regex\n", rule_id-100, tmp_rule_str);
						}
				}
				if ( rule_type == 6 ) {
					/* dpi-float	(sip dip sport dport proto sip-mask dip-mask sport-mask sport-mask proto-mask model forward-action aging-time key offset keymask )*/
					/* dpi-float	(sip+sport+proto sip-mask  sport-mask proto-mask model forward-action  key offset)*/
					sprintf(rule_str, "10.%d.%d.%d 0.0.0.0 1024 0 6 255.255.255.255 0.0.0.0 0xffff 0 0xff 1 1 0 0x6a_%02x_%02x_%02x 4\n", IP_2, IP_3, IP_4, IP_2, IP_3, IP_4);
		    	}
				if ( rule_type == 7 ) {
					/* L2规则 (一般只支持512个)*/
					sprintf(rule_str, "l2-polic add id %d vlanid1 %d lineno-ctr 1 forward 1 out_dmac 00:00:01:00:00:01 slot %d\n", rule_id, rule_id, SLOT);
				}
				
				fprintf(fp, "%s", rule_str);
                if(rule_id >= rule_count) {
                    goto stop_all_loop;
                }

				rule_id ++;
			}
stop_all_loop:
	if( fp != NULL )
		fclose(fp);
	return 0;
	
}


