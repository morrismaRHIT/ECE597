#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"

#define gpio 112
#define MAX_BUF 64
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */

int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, timeout, rc;
	char buf[MAX_BUF];
	int len;

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	//for button
	gpio_export(gpio);
	gpio_set_dir(gpio, "in");
	gpio_set_edge(gpio, "both");  // Can be rising, falling or both
	gpio_fd = gpio_fd_open(gpio, O_RDONLY);
	
	system("./setTemp.sh 1 0x48 0x19 0x1b");
 
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;
		
		timeout = POLL_TIMEOUT;
		rc = poll(fdset, nfds, timeout);   
      
		if (rc == 0) {
			system("./temp.sh 1 0x48");
		}
            
		if (fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[1].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
				 gpio, buf[0], len);
		    //print temp value
		    system("./temp.sh 1 0x48");
		    system("./setTemp.sh 1 0x48 0x19 0x1b");
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
	}

	gpio_fd_close(gpio_fd);
	return 0;
}