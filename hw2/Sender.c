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
#include <stdlib.h>


#define PACK_LEN 1000 //change PACKLEN send_fd
#define SENDER_PORT 5555
#define AGENT_PORT 6666
#define RECEIVER_PORT 7777
#define DATALEN 65
#define HEADER_LEN 20
#define NO_ACK_RATE 0 // 10%
#define WRONG_ACK_RATE 0 // 10%
#define BUFSIZE 60000
#define TRUE 1
#define FALSE 0
#define TIMEOUT 0.001

float sending(FILE *send_fp, int sockfd, long *len, struct sockaddr *addr, int addrlen, socklen_t *len_recvfrom);
//calculate the time interval between out and in
void tv_sub(struct  timeval *out, struct timeval *in);

int get_all_ack(int rece_list[], int round_should_be_send);

int max(int x, int y);

int main(int argc, char  **argv)
{
	int sockfd;
	float time_interval, rate_time;
	long len;
    struct sockaddr_in agent_addr, send_addr;
	struct in_addr **addrs;
	FILE *send_fp;
	socklen_t len_recvfrom;
	
    //fprintf(stderr ,"%s %s \n", argv[1], argv[2] );

    if((send_fp = fopen (argv[1],"rb+")) == NULL)fprintf(stderr,"File doesn't exit\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) fprintf(stderr, "socket error"); // create the socket
    
    //fprintf(stderr, "after socket\n");
    
    bzero(&send_addr , sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = INADDR_ANY;
    send_addr.sin_port = htons(SENDER_PORT);
    if(bind(sockfd, (struct sockaddr *)&send_addr , sizeof(send_addr))<0)fprintf(stderr, "sender bind error\n");

    bzero(&(agent_addr), sizeof(agent_addr));
    agent_addr.sin_family = AF_INET; // address format is host and port number                                              
    agent_addr.sin_addr.s_addr = inet_addr(argv[2]);
    agent_addr.sin_port = htons(AGENT_PORT);

    //memcpy(&(agent_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    // perform the transmission and receiving
    //fprintf(stderr, "go to sending\n");
    
    time_interval = sending(send_fp, sockfd, &len, (struct sockaddr *)&agent_addr, sizeof(struct sockaddr_in), &len_recvfrom);

    rate_time = ((len-1) / (float)time_interval); // caculate the average transmission rate
    //printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", time_interval, (int)len-1, rate_time);

    close(sockfd);
    fclose(send_fp);
	return 0;
}

float sending(FILE *send_fp, int sockfd, long *len, struct sockaddr *addr, int addrlen, socklen_t *len_recvfrom)
{   
    char buf[BUFSIZE], ack[1050];
    long lsize;
    int seq_num=1, winsize=1, threshold=16,next_packet_num,count=0;
    int sendbyte, last_seek, end=0,n , rece_list[1000000]={0}, rsend_list[1000000]={0};
    int timeout, round_should_be_send, check=0;
    float time_inv = 0.0;
    struct timeval sendt, recvt;
    struct timeval sendTime, curTime;
    //fprintf(stderr, "into sending\n");
    
    fseek(send_fp, 0, SEEK_END);
    lsize = ftell (send_fp);//to find the sizeof fp
    if (lsize%1000!=0)round_should_be_send=(lsize/1000)+1;//count how many times should be sent
    else round_should_be_send=lsize/1000;
    rewind(send_fp);
    //printf("The file length is %d bytes\n", (int)lsize);
    //printf("the round should be send %d\n", round_should_be_send);
    //  getchar();
    gettimeofday(&sendt, NULL); // get the current time
    while(1)
    {   
        timeout=0;   
        for(int i=0; i < winsize; i++)
        {   
            if ((seq_num*1000)>lsize)
            {   
            	if (seq_num>round_should_be_send){
            		break;
            	}
                last_seek= lsize-((seq_num-1)*1000);
                sprintf(buf,"seq%d", seq_num);            
                fseek(send_fp,(seq_num-1)*1000,SEEK_SET);
                sendbyte = fread(&buf[50],1,1000,send_fp);
            }
            else{    
                // form the packet to transmit
                sprintf(buf,"seq%d",seq_num);//            
                fseek(send_fp,(seq_num-1)*1000,SEEK_SET);
                sendbyte = fread(&buf[50],1,1000,send_fp);
            }
            rsend_list[seq_num-1]=1;
                //printf("sendbyte  %d\n", sendbyte);
            /*************** SEND MESSAGE ***************/
            gettimeofday(&sendTime, NULL);
            if((n = sendto(sockfd, &buf, sendbyte+50, 0, addr, addrlen)) == -1)
                fprintf(stderr,"Send error!\n"); // send the data
            else
                printf("send      data   #%d     winsize = %d\n", seq_num, winsize);

            if (seq_num<=round_should_be_send)
            {
                seq_num++;
            }            
        }
        for(int i=0; i < winsize; i++){
        /*************** RECEIVE ACK ***************/
        // MSG_DONTWAIT flag, non-blocking
        // receives nothing
            if ( recvfrom(sockfd, &ack, 1050, MSG_DONTWAIT, addr, len_recvfrom) < 0)
            {   // monitors how long nothing is received
                gettimeofday(&curTime, NULL);
                
                if (curTime.tv_sec - sendTime.tv_sec > TIMEOUT)
                {	
                	//printf("curTime.tv_sec = %d  sendTime.tv_sec = %d\n", curTime.tv_sec ,sendTime.tv_sec);
                    timeout=1;
                    threshold=max((winsize/2),1);
                    printf("time      out            threshold = %d\n", threshold);
                    //getchar();
                }
            }
            // An ACK is received
            else
            {
                printf("received  ack    #%d\n", atoi(&ack[3]));
                
                rece_list[atoi(&ack[3])-1]=1;
                //printf("rece_list %d = 1\n", atoi(&ack[3])-1);
            }
        }
        if(!timeout)
	    {
	        if (winsize<threshold)
	            winsize *=2;
	        else
	            winsize++;

	    }
	    else{
		    seq_num=get_all_ack(rece_list, round_should_be_send)+1;
		    if (seq_num<=0)
		    	break;
		    else{
		    	winsize=1;
		    	printf("resnd     data   #%d     winsize = %d\n", seq_num, winsize);
		    }
		}
    }
    sprintf(buf,"end\n");
    sendto(sockfd, &buf, 1000, 0, addr, addrlen);
    printf("send      fin\n");
    recvfrom(sockfd, &ack, 1050, 18, addr, len_recvfrom);
    sleep(1);
    printf("recv      finack\n");


    gettimeofday(&recvt, NULL);
    fseek(send_fp, 0, SEEK_END);
    lsize = ftell (send_fp);
    *len= lsize; // get current time
    tv_sub(&recvt, &sendt); // get the whole trans time
    time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;

    return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) <0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }

    out->tv_sec -= in->tv_sec;
}

int get_all_ack(int rece_list[], int round_should_be_send)
{	
	int count=0;
	int i;
	for (i = 0; i < round_should_be_send; ++i)
	{	
		if (rece_list[i]==1)
			count++;
		else
			break;
	}
	
	if (count==round_should_be_send){
		return -1;
    }
	else{
		return i;
    }
}

int max(int x, int y)
{
    if(x>y)
        return x;
    else 
        return y;
}
