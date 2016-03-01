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

#define PORT_FOR_GM 5555
#define PORT_FOR_BJ	6666
#define MAX_HANDCARD 10
#define MAX_PILE 4
#define ONLY_OUR 1
#define SPADE "\u2660"
#define HEART "\u2665"
#define DIAMOND "\u2666"
#define CLUB "\u2663"
#define GET_FIVE -3
#define BLACKJACK_SPADE -5
#define BLACKJACK 23
#define TRIBULE -10
#define COLOR -15

int split_num[4] = {2, 2, 2, 2};

void init(int deal_card[], int hand_card[], int All_Card[MAX_PILE][MAX_HANDCARD])
{
	int i, j;
	for(i = 0; i < MAX_HANDCARD; i++)
	{
		deal_card[i] = 0;
		hand_card[i] = 0;
		for(j = 0; j < MAX_PILE; j++)
			All_Card[j][i] = 0;
	}
}

int judge_num(int ary[])
{		
	int i, num = 0;
	for(i = 0; i < MAX_HANDCARD; i++)
		if (ary[i] == 0)
			break;
		else
			num++;
	return num;
}

int compare(const void *data1, const void *data2)
{
	int *ptr1 = (int *)data1;
	int *ptr2 = (int *)data2;
	if(*ptr1 < *ptr2)
		return -1;
	else if(*ptr1 > *ptr2)
		return 1;
	else
		0;
}

void print_card(int all_card[])
{	
	int i, j, k;
	int order;	
	int num = judge_num(all_card);
	qsort(all_card, num, sizeof(int), compare);
	for(i = 0; i < num; i++){
		if((all_card[i] - 1) / 13 == 0)printf(SPADE);
		else if((all_card[i] - 1) / 13 == 1)printf(HEART);
		else if((all_card[i] - 1) / 13 == 2){printf(DIAMOND);printf(" ");}
		else if((all_card[i] - 1) / 13 == 3)printf(CLUB);
		order = all_card[i] % 13;
		if (order > 1 && order < 11)
			printf("%d", order);
		else if(order == 1)
			printf("A");
		else if(order == 11)
			printf("J");
		else if(order == 12)
			printf("Q");
		else if(order == 0)
			printf("K");
		if(i + 1 != num) printf(", ");
		else printf(".\n");
		if((i + 1) % 5 == 0 && (i + 1) / 5 > 0 && (i + 1) != num)
			printf("\n                        ");
	}
}
int count_for_BJ(const int* const y, int pile_num)
{
	int i, j, result = 0, ace = 0;
	int x[52];
	for(j = 0; j < pile_num; ++j)
		x[j] = y[j];
	int color1 = (x[0] - 1) / 13;
	int color2 = (x[1] - 1) / 13;
	int num1 = x[0] % 13;
	int num2 = x[1] % 13;
	for (i = 0; i < pile_num; ++i) {
		x[i] = x[i] % 13;
		if (x[i] == 0) x[i] = 13;
		if (x[i] == 1) {
			++ace;
			result += 11;
		}
		else if (x[i] >= 10)
			result += 10;
		else result += x[i];
	}
	while (ace > 0 && result > 21) {
		--ace;
		result -= 10;
	}

	if(pile_num == 3 && result == 21 && x[0] == x[1] && x[1] == x[2] )
		return TRIBULE;
	else if(pile_num == 2 && ((num1 == 0 && num2 == 1) || (num1 == 1 && num2 == 0) ))
		if(color1 == color2)
			return BLACKJACK_SPADE;
		else
			return BLACKJACK;
	else if(pile_num == 5 && result <= 21)
		return GET_FIVE;
	else if((pile_num == 3) && color1 == color2 &&
		 ((x[2]-1) / 13 == color2) && result == 21 &&
		 ((num1 == 6 && num2 == 7) || (num1 == 6 && num2 == 8) ||
		  (num1 == 7 && num2 == 8) || (num1 == 8 && num2 == 7) ||
		  (num1 == 8 && num2 == 6)) )
		return COLOR;
	else if (result > 21)
		return -1;
	else
		return result;	
}

int main(int argc, char const *argv[])
{	
	setvbuf(stdout, (char *)NULL, _IONBF, 0);

	int dollar = 2000;
	while(1)
	{
		char command;
		printf("=============================\n");
		printf("           [GAME]            \n");  
		printf("	   Play (p)              \n");
		printf("         Options (o)         \n");
		printf("         About us(a)         \n");
		printf("           Story(s)          \n");
		printf("      other key to leave...  \n");	
		printf("=============================\n");
		printf("Enter your choice: ");
		scanf(" %c", &command);
		if (command == 'p')
		{
			int sock_fd;
			char buf[100], game_type, option;
			struct sockaddr_in ser_addr;
			socklen_t *len_recvfrom;
			printf("=============================\n");
			printf("      [Gambling system]      \n");
			printf("        Black Jack (b)       \n");
			printf("       Guess number (g)      \n");
			printf("           leave (l)         \n");
			printf("     you have %d dollars.    \n", dollar);			
			printf("=============================\n");
			printf("Enter your choice: ");
			scanf(" %c", &option);			
			printf("=============================\n");
			if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				fprintf(stderr, "socket error\n");
			bzero(&ser_addr , sizeof(ser_addr));
		    ser_addr.sin_family = AF_INET;
		    ser_addr.sin_addr.s_addr = inet_addr(argc[1]);
			if(option == 'g'){
				ser_addr.sin_port = htons(PORT_FOR_GM);
				printf("Connect to Server for GN...\n");
			}
			else if(option == 'b'){
				ser_addr.sin_port = htons(PORT_FOR_BJ);
				printf("Connect to Server for BJ...\n");
			}
			connect(sock_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
			write(sock_fd, &option, sizeof(char));
			read(sock_fd, &game_type, sizeof(char));	//check the game type
			int bet;
			printf("=============================\n");
			read(sock_fd, &buf, sizeof(buf));   //You are the %d player.
			printf("%s", buf);
			write(sock_fd, &dollar, sizeof(int));
			read(sock_fd, &buf, sizeof(buf));	//Pleasa enter bet				
			printf("=============================\n");
			while(1){
				printf("%s", buf);
				scanf(" %d", &bet);
				printf("=============================\n");
				if (bet > dollar)
					printf("Not enough money.        \n");
				else if(bet < 100)
					printf("Lowest bet is $100.      \n");
				else
					break;
			}			
			write(sock_fd, &bet, sizeof(int));
			if (game_type == 'G'){
				int i, j;
				for (i = 0; i < 10; ++i)
				{	
					int A, B;
					int num[4];
					if(i == 0){
						read(sock_fd, &buf, sizeof(buf)); //GAME START
						printf("%s", buf);
					}
					printf("=============================\n");			
					read(sock_fd, &buf, sizeof(buf));
					printf("%s", buf);
					printf("Enter 4 numbers: ");
					for (j = 0; j < 4; ++j){
						scanf("%d", &num[j]);
						write(sock_fd, &num[j], sizeof(int));
					}
					read(sock_fd, &A, sizeof(int));
					read(sock_fd, &B, sizeof(int));
					printf("You get: %d A %d B.\n", A, B);
					read(sock_fd, &buf, sizeof(buf));
					printf("%s", buf);
					printf("=============================\n");
					if (buf[0] != 'N')				
						break;
				}
				read(sock_fd, &dollar, sizeof(int));
				printf("Now you have %d dollars.\n", dollar);
			}
			else if (game_type == 'B'){
				int i, j, k;
				int deal_card[MAX_HANDCARD];
				int hand_card[MAX_HANDCARD];
				int visible_card[52] = {0}, visible_num = 0;
				int All_Card[MAX_PILE][MAX_HANDCARD];
				int stack = 0, split = 1;
				char buffer;
				dollar -= bet;
				for(i = 0; i < 4; i++)
					split_num[i] = 2;
				init(deal_card, hand_card, All_Card);
				read(sock_fd, &buf, sizeof(buf));
				printf("%s", buf);	//GAME START
				printf("=============================\n");
				read(sock_fd, &deal_card[0], sizeof(int));//get the deal card				
				for (i = 0; i < 2; ++i){		//get player card
					read(sock_fd, &hand_card[i], sizeof(int));
				}
				/////////////////////////////////////////////////////////////////////////
				//hand_card[0] = 1;
				//hand_card[1] = 52;
				////////////////////////////////////////////////////////////////////////
				read(sock_fd, &visible_num, sizeof(int));
				for(i = 0; i < visible_num; i++){
					read(sock_fd, &visible_card[i], sizeof(int));
				}
				printf("Visible card:  ");
				print_card(visible_card);
				printf("Dealer card:   ");
				print_card(deal_card);
				printf("Split #1 card: ");
				print_card(hand_card);
				printf("=============================\n");
				int Point = count_for_BJ(hand_card, split_num[0]); // 1 means the first pile				
				All_Card[0][0] = hand_card[0];
				All_Card[0][1] = hand_card[1];
				if(Point == BLACKJACK){//BLACKJACK
					split_num[0] = 2;
					stack = 1.5 * bet;
				}
				else if (Point == BLACKJACK_SPADE){ //BLACKJACK_SPADE
					split_num[0] = 2;
					stack = bet * (-BLACKJACK_SPADE);
				}
				else{
					printf("Double your bet? (y/n): "); 	// ask for double bet
					scanf(" %c", &buffer);
					printf("=============================\n");
					if(buffer == 'y')
						if (bet > dollar){
							printf("Not enough money.\n");
							printf("=============================\n");
						}else{
							dollar -= bet;
							bet *= 2;
						}
					if((hand_card[0] % 13) == (hand_card[1] % 13 ) || ((((hand_card[0] - 1) % 13) >= 9) && (((hand_card[1] - 1) % 13) >= 9) ))
					{// first split
						printf("Want to SPLIT? (y/n): ");
						scanf(" %c", &buffer);
						printf("=============================\n");						
						if (buffer == 'y')//first time split
						{	
							if( bet > dollar){
								printf("%d %d\n", bet, dollar);
								printf("Not enough money.\n");
								printf("=============================\n");
							}
							else
							{	
								dollar -= bet;
								split++;
								write(sock_fd, "P", sizeof(char));
								All_Card[1][0] = All_Card[0][1]; //shift pile1 to pile2
								read(sock_fd, &All_Card[0][1], sizeof(int));
								read(sock_fd, &All_Card[1][1], sizeof(int));
								for(k = 0; k < split; k++ ){
									hand_card[0] = All_Card[k][0];
									hand_card[1] = All_Card[k][1];
									printf("Split #%d has:  ", k + 1);
									print_card(hand_card);
								}
								////////////////////////////////////////////////////////////
								//All_Card[0][0] = 1;
								//All_Card[0][1] = 27;
								////////////////////////////////////////////////////////////
								if(( All_Card[0][0] % 13) == (All_Card[0][1] % 13)|| 
									((((All_Card[0][0] - 1) % 13) >= 9) && (((All_Card[0][1] - 1) % 13) >= 9 )))//second times split
								{								
									printf("Want to SPLIT? (y/n): ");
									scanf(" %c", &buffer);
									printf("=============================\n");
									if (buffer == 'y')
									{	
										if(bet > dollar){
											printf("Not enough money.\n");
											printf("=============================\n");
										}
										else{
											dollar -= bet;
											split++;
											write(sock_fd, "P", sizeof(char));
											All_Card[2][0] = All_Card[0][1]; //shift pile1 to pile3
											read(sock_fd, &All_Card[0][1], sizeof(int));
											read(sock_fd, &All_Card[2][1], sizeof(int));
											for(k = 0; k < split; k++ )
											{
												hand_card[0] = All_Card[k][0];
												hand_card[1] = All_Card[k][1];
												printf("Split #%d has:  ", k + 1);
												print_card(hand_card);
											}
										}
									}
								}
								/////////////////////////////////////////////////////////////
								//All_Card[1][0] = 1;
								//All_Card[1][1] = 40;
								/////////////////////////////////////////////////////////////
								if ( (All_Card[1][0] % 13) == (All_Card[1][1] % 13 ) || 
									((((All_Card[1][0] - 1) % 13) >= 9) && (((All_Card[1][1] - 1) % 13) >= 9 ))) //third split
								{
									printf("Want to SPLIT? (y/n): ");
									scanf(" %c", &buffer);
									printf("=============================\n");
									if (buffer == 'y')
									{	
										if(bet > dollar){
											printf("Not enough money.\n");
											printf("=============================\n");
										}
										else{
											dollar -= bet;
											split++;
											write(sock_fd, "P", sizeof(char));
											All_Card[3][0] = All_Card[1][1]; //shift pile1 to pile3
											read(sock_fd, &All_Card[1][1], sizeof(int));
											read(sock_fd, &All_Card[3][1], sizeof(int));
											for(k = 0; k < split; k++ )
											{
												hand_card[0] = All_Card[k][0];
												hand_card[1] = All_Card[k][1];
												printf("Split #%d has:  ", k + 1);
												print_card(hand_card);
											}
										}
									}
								}
							}
						}
					}
					for(i = 0; i < split; ++i)
					{	
						for(j = 0; j < split_num[i]; j++)
							hand_card[j] = All_Card[i][j];
						printf("Split #%d:                   \n", i + 1);
						int getCard;
						while(1) 
						{							
							printf(" %d points                   \n", count_for_BJ(hand_card, split_num[i]));
							printf("\n");
							printf("          HIT (h)            \n");
							printf("         STAND (s)           \n");
							printf("=============================\n");
							printf("Enter your choice: ");
							scanf(" %c", &buffer);
							printf("=============================\n");
							if (buffer == 'h') {
								write(sock_fd, "H", sizeof(char));
								read(sock_fd, &All_Card[i][split_num[i]++], sizeof(int));
								printf("Split #%d card: ", i + 1);
								for(j = 0; j < split_num[i]; j++)
									hand_card[j] = All_Card[i][j];
								print_card(hand_card);
								if (count_for_BJ(hand_card, split_num[i]) == -1){
									printf("=============================\n");
									break;
								}
							}
							else if (buffer == 's')
								break;
						}
					}
				}
				int dealer;
				int pay_back = split;
				write(sock_fd, "S", sizeof(char));
				read(sock_fd, &dealer, sizeof(int));
				if(dealer != -1)
					printf("Dealer gets %d points.\n", dealer);
				else if(Point != -1)
					printf("Dealer BUST!!!\n");
				printf("=============================\n");
				for(i = 0; i < split; i++)
				{	
					for(j = 0; j < split_num[i]; j++)
						hand_card[j] = All_Card[i][j];

					if ((Point = count_for_BJ(hand_card, split_num[i])) == -1 &&  dealer != -1)//self BUST
					{	
						printf("Split #%d BUST!!!.\n", i + 1);
						printf("Split #%d lose %d dollars.\n", i + 1, bet);
						stack -= bet;
						pay_back--;
					}
					else if (Point == GET_FIVE)
					{
						printf("Split #%d get Charlie.\n", i + 1);
						printf("Split #%d win %d dollars\n", i + 1, (-GET_FIVE) * bet);
						stack += bet;

					}
					else if (Point == TRIBULE)
					{
						printf("Split #%d get Three of a kind.\n", i + 1);
						printf("Split #%d win %d dollars\n", i + 1, (-TRIBULE) * bet);
						stack += bet;
					}
					else if (Point == COLOR)
					{
						printf("Split #%d get Flush.\n", i + 1);
						printf("Split #%d win %d dollars\n", i + 1, (-COLOR) * bet);
						stack += bet;
					}
					else if (Point ==BLACKJACK_SPADE)
					{
						printf("Split #%d get Blackjack of spade.\n", i + 1);
						printf("Split #%d win %d dollars\n", i + 1, bet);
					}
					else if (Point == BLACKJACK)
					{
						printf("Split #%d get Blackjack.\n", i + 1);
						printf("Split #%d win %d dollars\n", i + 1, bet);
					}
					else if ( (dealer == -1 || dealer < Point ) && (Point != -1))
					{	
						printf("Split #%d get %d points.\n", i + 1, Point);
						printf("Split #%d win %d dollars\n", i + 1, bet);
						stack += bet;
					}
					else if (dealer == Point)
					{
						printf("Split #%d get %d points.\n", i + 1, Point);
						printf("Split #%d draw, bet taken back.\n", i + 1);

					}					
					else{
						printf("Split #%d get %d points.\n", i + 1, Point);
						printf("Split# %d lose %d dollars.\n", i + 1, bet);
						stack -= bet;
						pay_back--;
					}					
				}
				printf("=============================\n");
				if(stack >= 0){
					stack += bet * pay_back;
					dollar += stack;
					printf("You get %d dollars.\n", stack);
					printf("Now you have %d dollars.\n", dollar );
				}
				else{
					printf("Overall:            \n");
					printf("    You lose %d dollars.\n", (-stack));
					printf("    Now you have %d dollars.\n", dollar);
				}
				printf("=============================\n");
				printf("          GAME START         \n");
				printf("=============================\n");				
			}
			close(sock_fd);
		}
		else if(command == 'o'){
			char option;
			int gain;			
			printf("=============================\n");
			printf("Want  more stack? (y/n)\n");
			printf("Enter your choice: ");
			scanf(" %c", &option);
			printf("=============================\n");
			if (option == 'y'){
				printf("How much do you want? \n");
				scanf("%d", &gain);
				dollar += gain;
				printf("Now you have %d dollars\n", dollar);
				printf("=============================\n");
			}
		}
		else if (command == 'a'){
			printf("=============================\n");
			printf("We are some poor students, who\nare trying to make progress on CS.\n");
			printf("Burning our passion to pass the CN.\n");			
			printf("=============================\n");
		}
		else if (command == 's'){	
			char option;
			printf("=============================\n");
			printf("Want to know more stories? (y/n)\n");
			printf("Enter your choice: ");
			scanf(" %c", &option);
			printf("=============================\n");
			if (option == 'y')
				printf("click: http://ppt.cc/PEySr\n");
				printf("=============================\n");
		}
	}
	return 0;
}
