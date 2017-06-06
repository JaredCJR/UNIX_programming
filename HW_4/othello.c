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

typedef struct {
    int is_start;
    int lost_con;
}game_comm;

game_comm comm = { 0 };

char localip[] = "127.0.0.1";

static int play_game(game_comm *com)
{
    int width;
    int height;
    int cx = 3;
    int cy = 3;

    while(1)
    {
        printf("Waiting...\n");
        pthread_mutex_lock(&start_lock);
        if(com->is_start == 1)
        {
            pthread_mutex_unlock(&start_lock);
            break;
        }
        pthread_mutex_unlock(&start_lock);
    }

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
		int ch = getch();
		int moved = 0;

		switch(ch) {
		case ' ':
			board[cy][cx] = PLAYER1;
			draw_cursor(cx, cy, 1);
			draw_score();
			break;
		case 0x0d:
		case 0x0a:
		case KEY_ENTER:
			board[cy][cx] = PLAYER2;
			draw_cursor(cx, cy, 1);
			draw_score();
			break;
		case 'q':
		case 'Q':
			goto quit;
			break;
		case 'r':
		case 'R':
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

//http://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
static int server_connect(char *p)
{
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
    while(1)
    {
        if(comm.lost_con == 1)
        {
            break;
        }

        //write
        snprintf(sendBuff, sizeof(sendBuff), "%s", "ack from server\n");
        sock_write(connfd,sendBuff);

        //read
        count = read(connfd, recvBuff, sizeof(recvBuff)-1);
        if(count < 0 && errno == EAGAIN) {
            // If this condition passes, there is no data to be read
        }
        else if(count >= 0) {
            // Otherwise, you're good to go and buffer should contain "count" bytes.
            recvBuff[count] = 0;

            fwrite(recvBuff, sizeof(char), count, log);
        }
        else {
            // Some other error occurred during read.
            pthread_mutex_lock(&conn_lock);
            comm.lost_con = 1;
            pthread_mutex_unlock(&conn_lock);
            perror("Read error:");
        }
        sleep(1);
    }
    close(connfd);
    fclose(log);
    return 0;
}

static int client_connect(char *d)
{
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
        snprintf(sendBuff, sizeof(sendBuff), "%s", "ack from client\n");
        sock_write(sockfd,sendBuff);

        //read
        count = read(sockfd, recvBuff, sizeof(recvBuff)-1);
        if(count < 0 && errno == EAGAIN) {
            // If this condition passes, there is no data to be read
        }
        else if(count >= 0) {
            // Otherwise, you're good to go and buffer should contain "count" bytes.
            recvBuff[count] = 0;
            fwrite(recvBuff, sizeof(char), count+1, log);
        }
        else {
            // Some other error occurred during read.
            pthread_mutex_lock(&conn_lock);
            comm.lost_con = 1;
            pthread_mutex_unlock(&conn_lock);
            perror("Read error:");
        }
        sleep(1);
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
    return 0;
}
