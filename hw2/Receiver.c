#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>

#define DROP_PACKET_RATE 0 // 10%
#define SENDER_PORT 5555
#define AGENT_PORT 6666
#define RECEIVER_PORT 7777
#define HEADER_LEN 50
#define DATALEN 65
#define BUFSIZE 60000
#define PACK_LEN 1000


void duplicate(char* src, char* des, int n);

void agent(int sockfd, FILE *fp, struct sockaddr *addr_age);

void inti_buf_len(int buf_len[]);

int full_check(int buf_len[]);

int num_of_buf(int buf_len[]);

void write_into_buf(FILE *fp,  char write_buf[][1050], int buf_len[],int num);

int main(int argc, char const *argv[])
{	
    FILE *fp;
	int sockfd;
	struct sockaddr_in agent_addr, rec_addr;
    struct in_addr **addr_age;

    //fprintf(stderr, "%s %s \n", argv[1], argv[2]);

    fp = fopen(argv[1], "wb+");
	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1)fprintf(stderr, "socket error\n");
	
    //fprintf(stderr, "set socket\n");

    bzero(&(agent_addr), sizeof(agent_addr));
    agent_addr.sin_family = AF_INET; // address format is host and port number                                              
    agent_addr.sin_addr.s_addr = inet_addr(argv[2]);
    agent_addr.sin_port = htons(AGENT_PORT);

    //fprintf(stderr, "agent\n");

    bzero(&rec_addr , sizeof(rec_addr));
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_addr.s_addr = INADDR_ANY;
    rec_addr.sin_port = htons(RECEIVER_PORT);
    if (bind(sockfd, (struct sockaddr *) &rec_addr, sizeof(rec_addr))==-1)fprintf(stderr, "rece bind error\n");

    //fprintf(stderr, "bind\n");
    //memcpy(&(agent_addr.sin_addr.s_addr), *addr_age, sizeof(struct in_addr));
    //fprintf(stderr, "before agent\n");

    agent(sockfd, fp, (struct sockaddr *)&agent_addr);

    close(sockfd);
	return 0;
}

void agent(int sockfd, FILE *fp, struct sockaddr *addr_age)
{
	char tmp[2000], write_buf[32][1050], ack[1050];
	int pack_num, seek_ptr,  buf_len[32]={0},end = 0, n = 0, count=0;
    int i, last_seek, group=0, now_group=0, section=0;
	struct sockaddr_in addr;
    int rece_seq[100000] = {0};
	
	socklen_t len=sizeof(struct sockaddr_in);

	//printf("Receiver start receiving\n");
    srand(time(NULL));
	while(!end)
	{
		 /*************** RECEIVE MESSAGE ***************/
        // if error in receiving
        if ((n = recvfrom(sockfd, &tmp, 1050, 0, (struct sockaddr *)&addr, &len)) == -1)
        {
            printf("Error when receiving\n");
            //exit(1);
        }
        else if (n == 0)
        {
            printf("Nothing received\n");
        }
        else
        {	  
            pack_num=atoi(&tmp[3]);
            group=(pack_num-1)/32;
            section=(pack_num-1)%32;
            count++;
            if (tmp[0]=='s')
            {   
                //printf("into s\n");
                if (group==now_group)
                {
                    if (buf_len[section] == 0)
                    {   
                        //printf("now_group=%d\n", now_group);
                        printf("recv      data   #%d\n", pack_num);
                        
                        buf_len[section] = n - HEADER_LEN;

                        duplicate(tmp + HEADER_LEN, write_buf[section], buf_len[section]);

                        sprintf(ack,"ack%d", pack_num);
                        if ((n = sendto(sockfd, &ack, 1050, 0, addr_age, len)) == -1)
                            printf("ACK send error!\n");

                        printf("send      ack    #%d\n", pack_num);
                        rece_seq[pack_num]++;                        
                    }
                    else 
                    {
                        printf("ignr      data   #%d\n", pack_num);
                    }
                    sprintf(ack,"ack%d", pack_num);
                        if ((n = sendto(sockfd, &ack, 1050, 0, addr_age, len)) == -1)
                            printf("ACK send error!\n");
                    printf("send      ack    #%d\n", pack_num);
                }
                else if (rece_seq[pack_num]>=1)
                {   
                    //printf("wannnnnnnnna\n");
                    if ((rand() % 100) > DROP_PACKET_RATE)
                    {
                        sprintf(ack,"ack%d", pack_num);
                        if ((n = sendto(sockfd, &ack, 1050, 0, addr_age, len)) == -1)
                            printf("ACK send error!\n");
                    }
                    else
                    {
                        printf("drop      data   #%d\n", pack_num);
                        if (full_check(buf_len)) {
                            //printf("buf full\n");
                            now_group++;
                            write_into_buf(fp,write_buf, buf_len, 32);
                            printf("flush\n");
                        }
                    }
                }
                else
                {
                     printf("drop      data   #%d\n", pack_num);
                     if (full_check(buf_len)) {
                        //printf("buf full\n");
                        now_group++;
                        write_into_buf(fp,write_buf, buf_len, 32);
                        printf("flush\n");
                    }
                }
            }
            else if (tmp[0]=='e')
            {
                printf("recv      fin\n");
                printf("send      finack\n");
                sprintf(ack,"finack");
                sendto(sockfd, &ack, 1050, 0, addr_age, len);
                write_into_buf(fp,write_buf, buf_len, num_of_buf(buf_len));
                printf("fflush\n");
                end = 1;
            }
        }
	}
    fclose(fp);
    //printf("A file has been successfully received!\n");

}

void inti_buf_len(int buf_len[])
{
    for (int i = 0; i < 32; ++i)
    {
        buf_len[i]=0;
    }
    return;
}

int num_of_buf(int buf_len[])
{   
    int count=0;
    for (int i = 0; i < 32; ++i)
    {
        if (buf_len[i]!=0)
            count++;
    }
    return count;
}

int full_check(int buf_len[])
{   
    int flag=1;
    for (int i = 0; i < 32; ++i)
    {
        if (buf_len[i]==0)
            flag=0;
    }
    return flag;
}

void write_into_buf(FILE *fp, char write_into_buf[][1050], int buf_len[], int num)
{
    for (int i = 0; i < num; ++i)
    {
        fwrite(write_into_buf[i], sizeof(char), buf_len[i], fp);
        fflush(fp);
    }
    inti_buf_len(buf_len);
    return;
}

void duplicate(char* src, char* des, int n)
{
    int i;
    for (i = 0; i < n; i++)
        des[i] = src[i];
    return;
}