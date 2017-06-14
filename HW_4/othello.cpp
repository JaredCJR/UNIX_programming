#include "othello_lib.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <fcntl.h>
#include <vector>


pthread_t tid[1];
pthread_mutex_t start_lock;
pthread_mutex_t conn_lock;
pthread_mutex_t play_lock;
#define MAGIC    "9487"
#define RESTART_MAGIC  "94_RESTART"
#define QUIT_MAGIC   "94_QUIT"

typedef struct {
    int is_start;
    int lost_con;
    int player;
    int pos_x;
    int pos_y;
    int need_update;
    int need_restart;
    int need_quit;
}game_comm;

game_comm comm = { 0 };
int targetFD;
int I_AM;


static int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}

static void log_game()
{
    char buff[256];
    FILE *log_fd = NULL;
    if(I_AM == PLAYER2)
    {
        log_fd = fopen("client_log.txt","a+");
    }else
    {
        log_fd = fopen("server_log.txt","a+");
    }

    if(log_fd == NULL)
    {
        return;
    }
    strcpy(buff,"\n------------------------------\n");
    fwrite(buff, sizeof(char), 32, log_fd);
    for(int i = 0;i < BOARDSZ; i++)
    {
        for(int j = 0;j < BOARDSZ; j++)
        {
            if(board[i][j] == PLAYER1)
            {
                strcpy(buff," 1 ");
                fwrite(buff, sizeof(char), 3, log_fd);
            }else if(board[i][j] == PLAYER2)
            {
                strcpy(buff," 2 ");
                fwrite(buff, sizeof(char), 3, log_fd);
            }else
            {
                strcpy(buff," _ ");
                fwrite(buff, sizeof(char), 3, log_fd);
            }
        }
        strcpy(buff,"\n");
        fwrite(buff, sizeof(char), 1, log_fd);
    }
    strcpy(buff,"\n------------------------------\n");
    fwrite(buff, sizeof(char), 32, log_fd);
    fclose(log_fd);
}

static void sock_write(int fd,char *sendBuff)
{
    int count = write(fd, sendBuff, strlen(sendBuff));
    if(count < 0 && errno == EAGAIN) {
        // If this condition passes, the data is writing
    }
    else if(count >= 0) {
        // Otherwise, write "count" bytes.
    }
    else {
        // Some other error occurred during read.
        pthread_mutex_lock(&conn_lock);
        comm.lost_con = 1;
        pthread_mutex_unlock(&conn_lock);
        perror("write error:");
    }
}

static void update(char *input)
{
    char deli[] = ",";
    char *magic, *player, *x, *y;
    if(comm.need_quit == 1)
    {
        return;
    }
    magic = strtok (input,deli);
    if(strcmp(magic, RESTART_MAGIC) == 0)
    {
        pthread_mutex_lock(&play_lock);
        comm.need_restart = 1;
        pthread_mutex_unlock(&play_lock);
    }else if(strcmp(magic, QUIT_MAGIC) == 0)
    {
        pthread_mutex_lock(&play_lock);
        comm.need_quit = 1;
        pthread_mutex_unlock(&play_lock);
    }else if(strcmp(magic,MAGIC) == 0)
    {
        player = strtok (NULL, deli);
        x = strtok (NULL, deli);
        y = strtok (NULL, deli);

        pthread_mutex_lock(&play_lock);
        sscanf(player, "%d", &comm.player);
        sscanf(x, "%d", &comm.pos_x);
        sscanf(y, "%d", &comm.pos_y);
        comm.need_update = 1;
        pthread_mutex_unlock(&play_lock);
    }else
    {
        fprintf(stderr,"UNKNOWN cmd\n");
    }
}

static bool is_valid_loc(int player, int target_x, int target_y)
{
    int target;
    std::vector<std::pair<int,int> > valid_loc;
    if(player == PLAYER1)
    {
        target = PLAYER2;
    }else
    {
        target = PLAYER1;
    }
    for(int pos_x = 0; pos_x < BOARDSZ; pos_x++)
    {
        for(int pos_y = 0; pos_y < BOARDSZ; pos_y++)
        {
            if(board[pos_y][pos_x] == player)
            {
                for(int x_step = -1; x_step <= 1; x_step++)
                {
                    for(int y_step = -1; y_step <= 1; y_step++)
                    {
                        if((x_step == 0) && (y_step == 0))
                        {
                            continue;
                        }
                        bool is_candidate = false;
                        int new_x = pos_x + x_step;
                        int new_y = pos_y + y_step;
                        while((new_y >= 0) && (new_y < BOARDSZ) &&
                              (new_x >= 0) && (new_x < BOARDSZ))
                        {
                            if(board[new_y][new_x] == target)
                            {
                                new_x = new_x + x_step;
                                new_y = new_y + y_step;
                                is_candidate = true;
                                continue;
                            }
                            if((board[new_y][new_x] == 0) && is_candidate)
                            {
                                valid_loc.push_back(std::make_pair(new_x, new_y));
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    //log start
    char buff[256];
    FILE *log_fd = NULL;
    if(I_AM == PLAYER2)
    {
        log_fd = fopen("client_log.txt","a+");
    }else
    {
        log_fd = fopen("server_log.txt","a+");
    }

    if(log_fd == NULL)
    {
        return false;
    }
    strcpy(buff,"\n------------------------------\n");
    fwrite(buff, sizeof(char), 32, log_fd);
    for(uint32_t i = 0;i < valid_loc.size();i++)
    {
        memset(buff, 0, sizeof(buff));
        snprintf(buff,sizeof(buff),"valid : (%d,%d)\n",valid_loc[i].first,valid_loc[i].second);
        fwrite(buff, sizeof(char), 32, log_fd);
    }
    strcpy(buff,"\n------------------------------\n");
    fwrite(buff, sizeof(char), 32, log_fd);
    fclose(log_fd);
    //end log
    for(uint32_t i = 0;i < sizeof(valid_loc);i++)
    {
        if((valid_loc[i].first == target_x) && (valid_loc[i].second == target_y))
        {
            return true;
        }
    }
    return false;
}

static void adjust_game(int player, int target_x, int target_y)
{
    int target;
    std::vector<std::pair<int,int> > valid_loc;
    if(player == PLAYER1)
    {
        target = PLAYER2;
    }else
    {
        target = PLAYER1;
    }
    for(int x_step = -1; x_step <= 1; x_step++)
    {
        for(int y_step = -1; y_step <= 1; y_step++)
        {
            if((x_step == 0) && (y_step == 0))
            {
                continue;
            }
            int new_x = target_x + x_step;
            int new_y = target_y + y_step;
            while((new_y >= 0) && (new_y < BOARDSZ) &&
                  (new_x >= 0) && (new_x < BOARDSZ))
            {
                if(board[new_y][new_x] == target)
                {
                    new_x = new_x + x_step;
                    new_y = new_y + y_step;
                    continue;
                }else if(board[new_y][new_x] == player)
                {
                    for(;(new_x != target_x) || (new_y != target_y);new_x -= x_step, new_y -= y_step)//update based on the rule
                    {
                        board[new_y][new_x] = player;
			            draw_cursor(new_x, new_y, 0);
			            draw_score();
                    }
                }
                break;
            }
        }
    }
    refresh();
}

static void *play_game(void *com)
{
    game_comm *com_obj = (game_comm *)com;
    int width;
    int height;
    int cx = 3;
    int cy = 3;

    printf("Waiting...\n");
    while(1)
    {
        pthread_mutex_lock(&start_lock);
        if(com_obj->is_start == 1)
        {
            pthread_mutex_unlock(&start_lock);
            break;
        }
        pthread_mutex_unlock(&start_lock);
    }
    char sendBuff[1024];
    memset(sendBuff, '0', sizeof(sendBuff)); 

	initscr();			// start curses mode 
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
					// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);			// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

	init_colors();

restart:
    com_obj->player = PLAYER1;
    com_obj->need_restart = 0;
	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();

	attron(A_BOLD);
    if(I_AM == PLAYER1)
    {
	    move(0, 0);
        printw("Player #1 ");   
    }else
    {
	    move(0, 0);
        printw("Player #2 ");
    }
	move(height-1, 0);	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);

	while(true) {			// main loop
        pthread_mutex_lock(&conn_lock);
        if(com_obj->lost_con == 1)
        {
            pthread_mutex_unlock(&conn_lock);
            goto quit;
        }
        pthread_mutex_unlock(&conn_lock);
        //update from sender
        pthread_mutex_lock(&play_lock);
        if(com_obj->need_update == 1)
        {
			board[com_obj->pos_y][com_obj->pos_x] = com_obj->player;
			draw_cursor(com_obj->pos_x, com_obj->pos_y, 1);
			draw_score();
            //based on the rules
            adjust_game(com_obj->player, com_obj->pos_x, com_obj->pos_y);
            com_obj->need_update = 0;
            com_obj->player = I_AM;
        }
        pthread_mutex_unlock(&play_lock);
        pthread_mutex_lock(&play_lock);
        if(com_obj->need_restart == 1)
        {
            pthread_mutex_unlock(&play_lock);
            goto restart;
        }
        if(com_obj->need_quit == 1)
        {
            pthread_mutex_unlock(&play_lock);
            goto quit;
        }
        pthread_mutex_unlock(&play_lock);
		int ch = getch();
		int moved = 0;
        pthread_mutex_lock(&play_lock);
        if(com_obj->need_quit == 1)
        {
            pthread_mutex_unlock(&play_lock);
            goto quit;
        }
        pthread_mutex_unlock(&play_lock);

		switch(ch) {
		case ' ':
            pthread_mutex_lock(&play_lock);
            if((I_AM == PLAYER1) && (com_obj->player == PLAYER1))
            {
                if(is_valid_loc(PLAYER1, cx, cy))
                {
			        board[cy][cx] = PLAYER1;
			        draw_cursor(cx, cy, 1);
			        draw_score();
                    //based on the rules
                    adjust_game(PLAYER1, cx, cy);

                    com_obj->player = PLAYER2;//next round is P2
                    snprintf(sendBuff, sizeof(sendBuff), "%s,%d,%d,%d\n", MAGIC, PLAYER1, cx, cy);
                    sock_write(targetFD,sendBuff);
                    log_game();
                }
            }
            pthread_mutex_unlock(&play_lock);
			break;
		case 0x0d:
		case 0x0a:
		case KEY_ENTER:
            pthread_mutex_lock(&play_lock);
            if((I_AM == PLAYER2) && (com_obj->player == PLAYER2))
            {
                if(is_valid_loc(PLAYER2, cx, cy))
                {
			        board[cy][cx] = PLAYER2;
			        draw_cursor(cx, cy, 1);
			        draw_score();
                    //based on the rules
                    adjust_game(PLAYER2, cx, cy);

                    com_obj->player = PLAYER1;//next round is P1
                    snprintf(sendBuff, sizeof(sendBuff), "%s,%d,%d,%d\n", MAGIC, PLAYER2, cx, cy);
                    sock_write(targetFD,sendBuff);
                    log_game();
                }
            }
            pthread_mutex_unlock(&play_lock);
			break;
		case 'q':
		case 'Q':
            pthread_mutex_lock(&play_lock);
            snprintf(sendBuff, sizeof(sendBuff), "%s,%d,%d,%d\n", QUIT_MAGIC, 0, 0, 0);
            sock_write(targetFD,sendBuff);
            pthread_mutex_unlock(&play_lock);
			goto quit;
			break;
		case 'r':
		case 'R':
            pthread_mutex_lock(&play_lock);
            snprintf(sendBuff, sizeof(sendBuff), "%s,%d,%d,%d\n", RESTART_MAGIC, 0, 0, 0);
            sock_write(targetFD,sendBuff);
            pthread_mutex_unlock(&play_lock);
			goto restart;
			break;
		case 'k':
		case KEY_UP:
			draw_cursor(cx, cy, 0);
			cy = (cy-1+BOARDSZ) % BOARDSZ;
			draw_cursor(cx, cy, 1);
			moved++;
			break;
		case 'j':
		case KEY_DOWN:
			draw_cursor(cx, cy, 0);
			cy = (cy+1) % BOARDSZ;
			draw_cursor(cx, cy, 1);
			moved++;
			break;
		case 'h':
		case KEY_LEFT:
			draw_cursor(cx, cy, 0);
			cx = (cx-1+BOARDSZ) % BOARDSZ;
			draw_cursor(cx, cy, 1);
			moved++;
			break;
		case 'l':
		case KEY_RIGHT:
			draw_cursor(cx, cy, 0);
			cx = (cx+1) % BOARDSZ;
			draw_cursor(cx, cy, 1);
			moved++;
			break;
		}

		if(moved) {
			refresh();
			moved = 0;
		}

		napms(1);		// sleep for 1ms
	}

quit:
    com_obj->lost_con = 1;
	endwin();			// end curses mode
	return NULL;
}


//http://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
static int server_connect(char *p)
{
    I_AM = PLAYER1;
    uint32_t port;
    sscanf(p, "%d", &port);

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10);
    printf("Waiting for a client on port %d ...\n",port);
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    comm.player = PLAYER1;
    int err = pthread_create(&tid[0], NULL, &play_game, &comm);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
    }
    pthread_mutex_lock(&start_lock);
    comm.is_start = 1;
    pthread_mutex_unlock(&start_lock);
    FILE *log = fopen("server_log.txt","w");
	
    char recvBuff[1024];
    char sendBuff[1024];
    memset(sendBuff, '0', sizeof(sendBuff)); 
    memset(recvBuff, '0',sizeof(recvBuff));
    int flags = fcntl(connfd, F_GETFL, 0);
	if(fcntl(connfd, F_SETFL, flags | O_NONBLOCK))
    {
    	perror("fcntl failed:");
    }
    int count;
    targetFD = connfd;
    while(1)
    {
        if(comm.lost_con == 1)
        {
            break;
        }

        //read
        count = read(connfd, recvBuff, sizeof(recvBuff)-1);
        if(count < 0 && errno == EAGAIN) {
            // If this condition passes, there is no data to be read
        }
        else if(count >= 0) {
            // Otherwise, you're good to go and buffer should contain "count" bytes.
            recvBuff[count] = 0;
            fwrite(recvBuff, sizeof(char), count, log);
            update(recvBuff);
            fflush(stdout);
            fflush(log);
        }
        else {
            // Some other error occurred during read.
            pthread_mutex_lock(&conn_lock);
            comm.lost_con = 1;
            pthread_mutex_unlock(&conn_lock);
            perror("Read error:");
            break;
        }
    }
    close(connfd);
    close(listenfd);
    fclose(log);
    return 0;
}

static int client_connect(char *d)
{
    I_AM = PLAYER2;
    /*parse ip and port*/
    char *ip, *port_ch;
    uint32_t port;
    ip = strtok (d,":");
    port_ch = strtok (NULL, ":");
    sscanf(port_ch, "%d", &port);

    /*setup socket*/
    int sockfd = 0;
    struct sockaddr_in serv_addr; 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        char converted_ip[32];
        if(hostname_to_ip(ip, converted_ip))
        {
            printf("\n inet_pton error occured\n");
        }
        printf("ip=%s\n", converted_ip);
        if(inet_pton(AF_INET, converted_ip, &serv_addr.sin_addr)<=0)
        {
            printf("\n inet_pton error occured\n");
            return 1;
        }
    } 

    int err = pthread_create(&tid[0], NULL, &play_game, &comm);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       perror("Connect Failed, please use a new port ");
       fprintf(stderr,"exit the othello automatically\n");
       exit(EXIT_FAILURE);
       return 1;
    }

    FILE *log = fopen("client_log.txt","w");
    targetFD = sockfd;

    pthread_mutex_lock(&start_lock);
    comm.is_start = 1;
    pthread_mutex_unlock(&start_lock);
    char recvBuff[1024];
    char sendBuff[1024];
    memset(sendBuff, '0', sizeof(sendBuff)); 
    memset(recvBuff, '0',sizeof(recvBuff));
    int flags = fcntl(sockfd, F_GETFL, 0);
	if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK))
    {
    	perror("fcntl failed:");
    }
    int count;
    while(1)
    {
        if(comm.lost_con == 1)
        {
            break;
        }

        //read
        count = read(sockfd, recvBuff, sizeof(recvBuff)-1);
        if(count < 0 && errno == EAGAIN) {
            // If this condition passes, there is no data to be read
        }
        else if(count >= 0) {
            // Otherwise, you're good to go and buffer should contain "count" bytes.
            recvBuff[count] = 0;
            fwrite(recvBuff, sizeof(char), count+1, log);
            update(recvBuff);
            fflush(stdout);
            fflush(log);
        }
        else {
            // Some other error occurred during read.
            pthread_mutex_lock(&conn_lock);
            comm.lost_con = 1;
            pthread_mutex_unlock(&conn_lock);
            perror("Read error:");
            break;
        }
    }
    close(sockfd);
    fclose(log);
    return 0;
}


int main(int argc, char* argv[])
{
    char r;
    pthread_mutex_init(&start_lock,NULL);  
    pthread_mutex_init(&conn_lock,NULL);  
    pthread_mutex_init(&play_lock,NULL);  
    if((r = getopt(argc, argv, "s:c:")) != -1)
    {
        switch(r)
        {
            case 's':
                server_connect(optarg);
                break;
            case 'c':
                client_connect(optarg);
                break;
            default:
                break;
        }
    }
    pthread_join(tid[0], NULL);
    pthread_mutex_destroy(&start_lock);  
    pthread_mutex_destroy(&conn_lock);  
    pthread_mutex_destroy(&play_lock);  
    return 0;
}
