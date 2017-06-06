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


pthread_t tid[1];
pthread_mutex_t start_lock;
pthread_mutex_t conn_lock;
pthread_mutex_t play_lock;
#define MAGIC    "9487"
#define RESTART_MAGIC  "9487_RESTART"

typedef struct {
    int is_start;
    int lost_con;
    int player;
    int pos_x;
    int pos_y;
    int need_update;
    int need_restart;
}game_comm;

game_comm comm = { 0 };
int targetFD;
int I_AM;

char localip[] = "127.0.0.1";

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
    magic = strtok (input,deli);
    if(strcmp(magic,MAGIC) == 0)
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
    }
    if(strcmp(magic, RESTART_MAGIC) == 0)
    {
        pthread_mutex_lock(&play_lock);
        comm.need_restart = 1;
        pthread_mutex_unlock(&play_lock);
    }
}

static int play_game(game_comm *com)
{
    int width;
    int height;
    int cx = 3;
    int cy = 3;

    printf("Waiting...\n");
    while(1)
    {
        pthread_mutex_lock(&start_lock);
        if(com->is_start == 1)
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
    com->player = PLAYER1;
    com->need_restart = 0;
	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();

	attron(A_BOLD);
	move(height-1, 0);	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);

	while(true) {			// main loop
        pthread_mutex_lock(&conn_lock);
        if(com->lost_con == 1)
        {
            pthread_mutex_unlock(&conn_lock);
            goto quit;
        }
        pthread_mutex_unlock(&conn_lock);
        //update from sender
        pthread_mutex_lock(&play_lock);
        if(com->need_update == 1)
        {
			board[com->pos_y][com->pos_x] = com->player;
			draw_cursor(com->pos_x, com->pos_y, 1);
			draw_score();
            com->need_update = 0;
            com->player = I_AM;
        }
        pthread_mutex_unlock(&play_lock);
        pthread_mutex_lock(&play_lock);
        if(com->need_restart == 1)
        {
            pthread_mutex_unlock(&play_lock);
            goto restart;
        }
        pthread_mutex_unlock(&play_lock);
		int ch = getch();
		int moved = 0;

		switch(ch) {
		case ' ':
            pthread_mutex_lock(&play_lock);
            if((I_AM == PLAYER1) && (com->player == PLAYER1))
            {
			    board[cy][cx] = PLAYER1;
			    draw_cursor(cx, cy, 1);
			    draw_score();

                com->player = PLAYER2;//next round is P2
                snprintf(sendBuff, sizeof(sendBuff), "%s,%d,%d,%d\n", MAGIC, PLAYER1, cx, cy);
                sock_write(targetFD,sendBuff);
            }
            pthread_mutex_unlock(&play_lock);
			break;
		case 0x0d:
		case 0x0a:
		case KEY_ENTER:
            pthread_mutex_lock(&play_lock);
            if((I_AM == PLAYER2) && (com->player == PLAYER2))
            {
			    board[cy][cx] = PLAYER2;
			    draw_cursor(cx, cy, 1);
			    draw_score();

                com->player = PLAYER1;//next round is P1
                snprintf(sendBuff, sizeof(sendBuff), "%s,%d,%d,%d\n", MAGIC, PLAYER2, cx, cy);
                sock_write(targetFD,sendBuff);
            }
            pthread_mutex_unlock(&play_lock);
			break;
		case 'q':
		case 'Q':
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
	endwin();			// end curses mode

    pthread_mutex_lock(&conn_lock);
    comm.lost_con = 1;
    pthread_mutex_unlock(&conn_lock);

	return 0;
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
    int err = pthread_create(&tid[0], NULL, (void*)&play_game, &comm);
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

        //write
        /*
        snprintf(sendBuff, sizeof(sendBuff), "%s", "ack from server\n");
        sock_write(connfd,sendBuff);
        */

        //read
        count = read(connfd, recvBuff, sizeof(recvBuff)-1);
        if(count < 0 && errno == EAGAIN) {
            // If this condition passes, there is no data to be read
        }
        else if(count >= 0) {
            // Otherwise, you're good to go and buffer should contain "count" bytes.
            recvBuff[count] = 0;
            update(recvBuff);
            fwrite(recvBuff, sizeof(char), count, log);
        }
        else {
            // Some other error occurred during read.
            pthread_mutex_lock(&conn_lock);
            comm.lost_con = 1;
            pthread_mutex_unlock(&conn_lock);
            perror("Read error:");
        }
    }
    close(connfd);
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
    if(strcmp(ip,"localhost") == 0)
    {
        ip = localip;
    }
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
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    int err = pthread_create(&tid[0], NULL, (void*)&play_game, &comm);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       perror("Connect Failed ");
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

        //write
        /*
        snprintf(sendBuff, sizeof(sendBuff), "%s", "ack from client\n");
        sock_write(sockfd,sendBuff);
        */

        //read
        count = read(sockfd, recvBuff, sizeof(recvBuff)-1);
        if(count < 0 && errno == EAGAIN) {
            // If this condition passes, there is no data to be read
        }
        else if(count >= 0) {
            // Otherwise, you're good to go and buffer should contain "count" bytes.
            recvBuff[count] = 0;
            update(recvBuff);
            fwrite(recvBuff, sizeof(char), count+1, log);
        }
        else {
            // Some other error occurred during read.
            pthread_mutex_lock(&conn_lock);
            comm.lost_con = 1;
            pthread_mutex_unlock(&conn_lock);
            perror("Read error:");
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
