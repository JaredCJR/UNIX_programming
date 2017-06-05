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

static int width;
static int height;
static int cx = 3;
static int cy = 3;

char localip[] = "127.0.0.1";
//http://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
static int server_connect(char *p)
{
    uint32_t port;
    sscanf(p, "%d", &port);

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
    char sendBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10);
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
    while(1)
    {
        snprintf(sendBuff, sizeof(sendBuff), "%s", "Server\n");
        write(connfd, sendBuff, strlen(sendBuff));
        sleep(1);
    }
    close(connfd);
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
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    memset(recvBuff, '0',sizeof(recvBuff));
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

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    } 
    return 0;
}

int
main(int argc, char* argv[])
{
    char r;
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

    while(1)
    {
        
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

	return 0;
}
