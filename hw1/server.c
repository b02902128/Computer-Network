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
#define MAX_PLAYER 10
#define MIN_PLAYER 1
#define MAX_HANDCARD 10
#define SPADE "\u2660"
#define HEART "\u2665"
#define DIAMOND "\u2666"
#define CLUB "\u2663"


int Stake_count(int id, int bet[], int round)
{	
	int i, sum = 0;
	int price;
	price = bet[id] * (12 - round);
	return price;
}

void Overset_init(int x[], int player_num)
{	
	int i;
	for (i = 0; i < player_num; ++i)
		x[i] = 0;
}

void calculate(int *a, int *b, int num[], int ans[])
{	
	int i, j;
	for (i = 0; i < 4; ++i)
	{
		if(num[i] == ans[i])
			(*a)++;
		else
			for (j = 0; j < 4; ++j)
				if (i != j)
					if(num[i] == ans[j])
						(*b)++;
	}
}
void shuffle(int ary[], int amount)
{
	int i, tmp, num;
	srand(time(NULL));
	for(i = 0; i < amount; i++)
	{
		num = rand() % (i + 1);
		tmp = ary[i];
		ary[i] = ary[num];
		ary[num] = tmp;
	}
}
void gen_num(int ans[4])
{
	int i;
	int ary[10];
	for(i = 0; i < 10; i++)
		ary [i] = i;
	shuffle(ary, 10);
	for (i = 0; i < 4; ++i)
		ans[i] = ary[i];
}
void shuffle_card(int card[])
{	
	int i;
	for(i = 0; i < 52; i++)
		card[i] = i + 1;
	shuffle(card, 52);
}
void init(int k[], int player_num)
{
	int i;
	for (i = 0; i < 52; ++i)		
		k[i] = 0;
}
int judge_num(int ary[])
{		
	int i, num = 0;
	for(i = 0; i < 52; i++)
		if (ary[i] == 0)
			break;
		else
			num++;
	return num;
}
int count_for_BJ(const int* const y, int num) {
	int i, j, result = 0, ace = 0;
	int x[52];
	for(j = 0; j < num; j++)
		x[j] = y[j];
	for (i = 0; i < num; ++i) {
		x[i] = x[i] % 13;
		if(x[i] == 0) x[i] = 13;	
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
	if (result > 21)
		return -1;
	else
		return result;
}
void print_card(int all_card[])
{	
	int i;
	int num = judge_num(all_card);
	for(i = 0; i < num; i++){
		if((all_card[i] - 1) / 13 == 0)printf(SPADE);
		else if((all_card[i] - 1) / 13 == 1)printf(HEART);
		else if((all_card[i] - 1) / 13 == 2){printf(DIAMOND);printf(" ");}
		else if((all_card[i] - 1) / 13 == 3)printf(CLUB);
		int order = all_card[i] % 13;
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
			printf("\n ");
	}
}
int main(int argc, char const *argv[])
{	
	setvbuf(stdout, (char *)NULL, _IONBF, 0);
	while(1)
	{	
		char command;
		printf("=============================\n");
		printf("       [SERVER CREATER]      \n");
		printf("         Blackjack (b)       \n");
		printf("       Gusee Number (g)      \n");
		printf("           Quit (q)          \n");
		printf("=============================\n");
		printf("Enter your choice: ");
		scanf(" %c", &command);
		printf("=============================\n");
		printf("Loading...\n");
		if(command != 'b' && command != 'g')
			break;
		else
		{	
			int player_num;
			while(1){
				printf("=============================\n");
				printf("Enter the capacity: ");
				scanf(" %d", &player_num);
				printf("=============================\n");
				if (player_num > MAX_PLAYER || player_num < MIN_PLAYER)
					printf("Enter 1 to 10.\n");
				else
					break;
			}
			int sock_fd;
			int conn_fd[player_num];
			char game_type_uppercase, game_type_lowercase;
			char not_fit = 'D';	//don't fit the game type
			char msg[100], msg0[100] ,msg1[100], msg2[100], msg3[100], msg4[100], msg5[100], msg6[300];
		    struct sockaddr_in ser_addr, cli_addr;
			socklen_t cli_len = sizeof(cli_addr);
			
			if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				fprintf(stderr, "socket error\n");
			int timeout = 1;
			setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&timeout, sizeof(int));
			bzero(&ser_addr , sizeof(ser_addr));
		    ser_addr.sin_family = AF_INET;
		    ser_addr.sin_addr.s_addr = INADDR_ANY;
		    if(command == 'G' || command == 'g')
				ser_addr.sin_port = htons(PORT_FOR_GM);
			else
				ser_addr.sin_port = htons(PORT_FOR_BJ);
			bind(sock_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
			listen(sock_fd, 1);
			
			if (command == 'G' || command =='B'){
				game_type_uppercase = command;
				game_type_lowercase = command + 32; //transfrom ascii to lowercase
				if(command == 'G')
					printf("Server is going to hold Guess Number.\n");
				else
					printf("Server is going to hold Blackjack.\n");
			}
			else if (command == 'g' || command == 'b')
			{
				game_type_lowercase = command;
				game_type_uppercase = command - 32; //transfrom ascii to uppercase
				if(command == 'g')
					printf("Server is going to hold Guess Number.\n");
				else
					printf("Server is going to hold Blackjack.\n");
			}	
			else
				printf("Game type error, please try again.\n");
			
			int i;
			int player_state[player_num];
			for (i = 0; i < player_num; ++i)
			{	
				char tmp_type;	
				conn_fd[i] = accept(sock_fd , (struct sockaddr *)&cli_addr, &cli_len);
				read(conn_fd[i], &tmp_type, sizeof(char));
				if (tmp_type == game_type_uppercase ||
					tmp_type == game_type_lowercase){
					write(conn_fd[i], &game_type_uppercase, sizeof(char));
					sprintf(msg, "You are the %d player.\nWait for other %d player(s) join.\n"
						, i+1, player_num - (i+1));
					write(conn_fd[i], &msg, sizeof(msg));
					read(conn_fd[i], &player_state[i], sizeof(int));
				}
				else{
					write(conn_fd[i], &not_fit, sizeof(char));
					i--;
				}
			}
			int bet[10];
			for (i = 0; i < player_num; ++i)	//ask every player to enter bet.
			{	
				sprintf(msg0, "Enter your bet: ");
				write(conn_fd[i], msg0, sizeof(msg0));
				read(conn_fd[i], &bet[i], sizeof(int));
				player_state[i] -= bet[i];
			}
			if (game_type_uppercase == 'G')
			{	
				int ans[4];
				int is_Over[player_num];
				int end = 0;
				
				gen_num(ans);
				Overset_init(is_Over, player_num);
				printf("=============================\n");
				printf("          GAME START         \n");
				printf("=============================\n");
				printf("The answer is : %d %d %d %d\n", ans[0], ans[1], ans[2], ans[3]);
				sprintf(msg1, "          GAME START         \n");
				for(i = 0; i < 10; ++i){
					if (end == 1) break;
					int j, k, l;
					for(j = 0; j < player_num; j++){
						int A = 0, B = 0;
						int tmp_num[4];
						if (i == 0)
							write(conn_fd[j], msg1, sizeof(msg));
						sprintf(msg2, "Round %d.\n", i + 1);
						write(conn_fd[j], msg2, sizeof(msg));
						for(k = 0; k < 4; k++ )	
							read(conn_fd[j], &tmp_num[k], sizeof(int));
						printf("=============================\n");
						printf("Round %d.\nThe %d player entered %d %d %d %d.\n",
						 i + 1, j + 1, tmp_num[0], tmp_num[1], tmp_num[2], tmp_num[3]);
						calculate(&A, &B, tmp_num, ans);
						if (A == 4){
							is_Over[j] = 1;
							end = 1;
						}
						write(conn_fd[j], &A, sizeof(int));
						write(conn_fd[j], &B, sizeof(int));
					}
					if (end == 1){					
						for(l = 0; l < player_num; l++)
							if(is_Over[l] == 1){	
								int money = Stake_count(l , bet, i + 1);
								sprintf(msg3, "Correct answer!\nYou get $%d.\n", money);
								write(conn_fd[l], &msg3, sizeof(msg));
								player_state[l] += money;
								//fprintf(stderr, "player_state = %d\n", player_state[l]);
							}
							else{
								sprintf(msg4, "Game Over.\nThe correct answer is %d%d%d%d.\nOthers wins, you lose %d dollars.\n", 
								ans[0], ans[1], ans[2], ans[3], bet[l]);
								write(conn_fd[l], &msg4, sizeof(msg));
								//fprintf(stderr, "player_state = %d\n", player_state[l]);
							}	
					}
					else
						for(l = 0; l < player_num; l++)
							if(i != 9){
								sprintf(msg5, "No one is correct. Continue...\n");
								write(conn_fd[l], &msg5, sizeof(msg));
							}
							else{
								sprintf(msg6, "=============================\n          GAME OVER          \n=============================\nCorrect answer is %d%d%d%d.\nNo one wins, you lose %d dollars.\n",
								ans[0], ans[1], ans[2], ans[3], bet[l]);
								write(conn_fd[l], &msg6, sizeof(msg));
								//fprintf(stderr, "player_state = %d\n", player_state[l]);
							}
				}
				int m;
				for (m = 0; m < player_num; ++m)
					write(conn_fd[m], &player_state[m], sizeof(int));
				printf("=============================\n");
				printf("           GAME OVER         \n");
				printf("=============================\n");
				for (m = 0; m < player_num; ++m)
					close(conn_fd[m]);
				close(sock_fd);
			}
			else if(game_type_uppercase == 'B'){
				int i, j, k;
				int card[52];
				int deal_card_num = 0;	
				int current = 0, visible_cur = 0;
				int visible_card[52] = {0};//visible card
				int deal_card[10] = {0};
				int player_card_num = 0;
				char option = 'I'; // init
				shuffle_card(card);
				printf("          GAME START         \n");
				sprintf(msg1, "          GAME START         \n");
				printf("Card order: \n");
				print_card(card);
				printf("\n");
				printf("=============================\n");
				deal_card[deal_card_num++] = card[current++];
				visible_card[visible_cur++] = card[current];
				deal_card[deal_card_num++] = card[current++];
				for (i = 0; i < player_num; ++i)
				{
					visible_card[visible_cur] = card[2 * visible_cur + 1];
					visible_cur++;
				}
				for (i = 0; i < player_num; ++i)
				{
					write(conn_fd[i], &msg1, sizeof(msg));//1
					write(conn_fd[i], &deal_card[1], sizeof(int));	//2	//send deal_card to player
					write(conn_fd[i], &card[current++], sizeof(int));//3	//send player card to player
					//visible_card[visible_cur++] = card[current];
					write(conn_fd[i], &card[current++], sizeof(int));//4
					//write(conn_fd[i], &visible_cur, sizeof(int));//5
					//for (j = 0; j < visible_cur; ++j)
						//write(conn_fd[i], &visible_card[j], sizeof(int ));//6 7
				}
				printf("You get: ");
				print_card(deal_card);
				printf("=============================\n");
				int test[2] = {0};
				for(i = 0; i < player_num; ++i)
				{	
					write(conn_fd[i], &visible_cur, sizeof(int));
					for (j = 0; j < visible_cur; ++j)
						write(conn_fd[i], &visible_card[j], sizeof(int));
					while(option != 'S'){
						read(conn_fd[i], &option, sizeof(char));
						if(option == 'H'){//hit
							visible_card[visible_cur++] = card[current];
							test[0] = card[current];
							printf("send to client: ");
							print_card(test);
							printf("=============================\n");
							write(conn_fd[i], & card[current++], sizeof(int));
						}
						else if(option == 'S')//stand
							break;
						else if(option == 'P'){//split
							visible_card[visible_cur++] = card[current];
							write(conn_fd[i], & card[current++], sizeof(int));
							visible_card[visible_cur++] = card[current];
							write(conn_fd[i], & card[current++], sizeof(int));
						}
						else{
							perror("Error input.\n");
						}
					}
					option = 'I';
				}
				char buffer;
				int Dealer;
				while (1) {
					Dealer = (count_for_BJ(deal_card, deal_card_num));					
					if (Dealer == -1) {
						printf("        You BUST!!!\n");
						break;
					}
					printf("Now %d points\n", Dealer);
					printf("You have: ");
					print_card(deal_card);
					printf("\n");
					printf("           HIT (h)           \n");
					printf("          STAND (s)          \n");
					printf("=============================\n");
					printf("Enter your choice: ");
					scanf(" %c", &buffer);
					printf("=============================\n");
					if (buffer == 's')
						if ( Dealer < 17)
							printf("Need to get over 17.\n");
						else 
							break;
					else if (buffer == 'h') {
						test[0] = card[current];
						printf("=============================\n");
						printf("Send card to server: ");
						print_card(test);						
						printf("=============================\n");
						deal_card[deal_card_num++] = card[current++];
					}
				}
				printf("=============================\n");
				printf("           GAME OVER         \n");
				printf("=============================\n");
				for(i = 0; i < player_num; i++){
					write(conn_fd[i], &Dealer, sizeof(int));
					close(conn_fd[i]);
				}
				close(sock_fd);
			}
		}
	}
	return 0;
}
