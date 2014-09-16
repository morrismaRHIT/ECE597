#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

#define button1 115
#define button2 7
#define button3 20
#define button4 110
#define button5 111

#define led1 30
#define led2 31
#define led3 48
#define led4 50
#define led5 51

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[6];
	int nfds = 6;
	int gpio_fd, led1_fd, button2_fd, led2_fd, timeout, rc;
	int button3_fd, button4_fd, button5_fd;
	int led3_fd, led4_fd, led5_fd;
	char buf[MAX_BUF];
	int len;
	int toggle1 = 1, toggle2 = 1, toggle3 = 1, toggle4 = 1, toggle5 = 1;

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	//for buttons
	gpio_export(button1);
	gpio_set_dir(button1, "in");
	gpio_set_edge(button1, "rising");  // Can be rising, falling or both
	gpio_fd = gpio_fd_open(button1, O_RDONLY);
	
	gpio_export(button2);
	gpio_set_dir(button2, "in");
	gpio_set_edge(button2, "rising");  // Can be rising, falling or both
	button2_fd = gpio_fd_open(button2, O_RDONLY);
	
	gpio_export(button3);
	gpio_set_dir(button3, "in");
	gpio_set_edge(button3, "rising");  // Can be rising, falling or both
	button3_fd = gpio_fd_open(button3, O_RDONLY);
	
	gpio_export(button4);
	gpio_set_dir(button4, "in");
	gpio_set_edge(button4, "rising");  // Can be rising, falling or both
	button4_fd = gpio_fd_open(button4, O_RDONLY);
	
	gpio_export(button5);
	gpio_set_dir(button5, "in");
	gpio_set_edge(button5, "rising");  // Can be rising, falling or both
	button5_fd = gpio_fd_open(button5, O_RDONLY);
	
	//for led
	gpio_export(led1);
	gpio_set_dir(led1,"out");
	led1_fd = gpio_fd_open(led1, O_RDONLY);
	
	gpio_export(led2);
	gpio_set_dir(led2,"out");
	led2_fd = gpio_fd_open(led2, O_RDONLY);
	
	gpio_export(led3);
	gpio_set_dir(led3,"out");
	led3_fd = gpio_fd_open(led3, O_RDONLY);
	
	gpio_export(led4);
	gpio_set_dir(led4,"out");
	led4_fd = gpio_fd_open(led4, O_RDONLY);
	
	gpio_export(led5);
	gpio_set_dir(led5,"out");
	led5_fd = gpio_fd_open(led5, O_RDONLY);

	timeout = POLL_TIMEOUT;
 
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;
		
		fdset[2].fd = button2_fd;
		fdset[2].events = POLLPRI;
		
		fdset[3].fd = button3_fd;
		fdset[3].events = POLLPRI;
		
		fdset[4].fd = button4_fd;
		fdset[4].events = POLLPRI;
		
		fdset[5].fd = button5_fd;
		fdset[5].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
            
		if (fdset[1].revents & POLLPRI) {   //button 1 pressed
		 lseek(fdset[1].fd, 0, SEEK_SET);
		 len = read(fdset[1].fd,buf,MAX_BUF);
			toggle1 = !toggle1;
			gpio_set_value(led1, toggle1);
		}
		if (fdset[2].revents & POLLPRI) {   //button 2 pressed
		 lseek(fdset[2].fd,0,SEEK_SET);
		 len = read(fdset[2].fd, buf, MAX_BUF);
			toggle2 = !toggle2;
			gpio_set_value(led2, toggle2);
		}
		if (fdset[3].revents & POLLPRI) {   //button 3 pressed
		 lseek(fdset[3].fd,0,SEEK_SET);
		 len = read(fdset[3].fd, buf, MAX_BUF);
			toggle3 = !toggle3;
			gpio_set_value(led3, toggle3);
		}
		if (fdset[4].revents & POLLPRI) {   //button 4 pressed
		 lseek(fdset[4].fd,0,SEEK_SET);
		 len = read(fdset[4].fd, buf, MAX_BUF);
			toggle4 = !toggle4;
			gpio_set_value(led4, toggle4);
		}
		if (fdset[5].revents & POLLPRI) {   //button 5 pressed
		 lseek(fdset[5].fd,0,SEEK_SET);
		 len = read(fdset[5].fd, buf, MAX_BUF);
			toggle5 = !toggle5;
			gpio_set_value(led5, toggle5);
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}
		
		//etch-a-sketch game goes here

		fflush(stdout);
	}

	gpio_fd_close(led1_fd);
	gpio_fd_close(gpio_fd);
	gpio_fd_close(led2_fd);
	gpio_fd_close(button2_fd);
	gpio_fd_close(led3_fd);
	gpio_fd_close(button3_fd);
	gpio_fd_close(led4_fd);
	gpio_fd_close(button4_fd);
	gpio_fd_close(led5_fd);
	gpio_fd_close(button5_fd);
	
	return 0;
}

