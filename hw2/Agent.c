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

#define SENDER_PORT 5555
#define AGENT_PORT 6666
#define RECEIVER_PORT 7777
#define DATALEN 65
#define BUFSIZE 60000
#define PACK_LEN 1000
#define DROP_PACKET_RATE 1 // 10%
#define TRUE 1
#define FALSE 0
#define TIMEOUT 1


void agent(int sockfd,struct sockaddr *addrs_re, struct sockaddr *addrs_send);

int main(int argc, char  **argv)
{
	int sockfd;
	struct sockaddr_in my_addr, rec_addr, sen_addr;
	struct in_addr **addrs_re, **addrs_send;
	socklen_t len_recvfrom=sizeof(struct sockaddr_in);

    //fprintf(stderr, "%s %s\n", argv[1], argv[2]);

    memset(&my_addr, 0 ,sizeof(my_addr));

	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1)fprintf(stderr, "socket error\n");

    bzero( &my_addr , sizeof(my_addr));
	my_addr.sin_family = AF_INET; //Address family must be AF_INET
	my_addr.sin_port = htons(AGENT_PORT); //Internet Protocol (IP) port.
	my_addr.sin_addr.s_addr = INADDR_ANY;//IP address in network byte order. INADDR_ANY is 0.0.0.0 meaning "all the addr"
    if(bind(sockfd, (struct sockaddr *)&my_addr , sizeof(my_addr))<0)fprintf(stderr, "bind error\n");
    // places nbyte null bytes in the string s
    // this function will be used to set all the socket structures with null values
    //fprintf(stderr, "after bind\n");

    bzero(&sen_addr,sizeof(sen_addr));
    sen_addr.sin_family = AF_INET;
    sen_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sen_addr.sin_port = htons(SENDER_PORT);

    //fprintf(stderr, "sen set\n");

    bzero(&rec_addr, sizeof(rec_addr));
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_addr.s_addr = inet_addr(argv[2]);
    rec_addr.sin_port = htons(RECEIVER_PORT);

    agent(sockfd,  (struct sockaddr *)&rec_addr, (struct sockaddr *)&sen_addr);

    close(sockfd);
	return 0;
}

void agent(int sockfd ,struct sockaddr *rec_addr, struct sockaddr *sen_addr)
{
	FILE *fp;
    char buf[BUFSIZE], header[30], ack[100];
    int end = 0, n = 0;
    float lost_num=0, total=0;
    long lseek = 0;
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);

    //printf("Agent Start receiving...\n");

    srand(time(NULL)); // seed for random number

    while(!end)
    {
        /*************** RECEIVE MESSAGE ***************/
        // if error in receiving
        if ((n = recvfrom(sockfd, &buf, 1050, 0, (struct sockaddr *)&addr, &len)) == -1)
        {
            fprintf(stderr, "Error when receiving\n");
            exit(1);
        }       
        else if (n == 0) // if nothing received
        {
            fprintf(stderr, "Agent Nothing received\n");
        }        
        else // if something received
        {
            //strncpy(header,buf,30);
            //fprintf(stderr, "receive %d byte\n", n);
            if (buf[0]=='s')
            {   
                //fprintf(stderr, "header be s\n");
                if ((rand() % 100) > DROP_PACKET_RATE)
                {
                // tell sender what to expect next
                    total++;
                    printf("get       data   #%d\n",  atoi(&buf[3]));
                    // random number 0-99
                    /*************** SEND ACK ***************/
                    if ((sendto(sockfd, buf, n, 0, rec_addr, len)) == -1)
                    {
                        printf("send to receiver error!\n");
                        exit(1);
                    }
                        printf("fwd       data   #%d   loss rate = %lf\n",atoi(&buf[3]),lost_num/total);                    
                }
                // does not send packet
                else
                {
                    lost_num++;
                    total++;
                    printf("drop      data   #%d   loss rate = %lf\n",atoi(&buf[3]),lost_num/total);
                }
            }
            else if(buf[0]=='a')
            {
                printf("get       ack    #%d\n",atoi(&buf[3]));
                sendto(sockfd, &buf , 1050, 0 , sen_addr, len );
                printf("fwd       ack    #%d\n",atoi(&buf[3]));
            }
            else if (buf[0]=='e')
            {
                sendto(sockfd,"end\n", 5 , 0 , rec_addr, len);
                printf("get       fin\n");
                printf("fwd       fin\n");
            }
            else if (buf[0]=='f')
            {
            	printf("get       finack\n");
                printf("fwd       finack\n");
                sendto(sockfd, &buf , n, 0 , sen_addr, len );
                break;
            }
        }
    }
    //printf("A file has been successfully transfered by Agent!\n");
}

