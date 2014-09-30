#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio-utils.h"
#include "i2c-dev.h"
#include "i2cbusses.h"

#define eQEP2 "/sys/devices/ocp.3/48304000.epwmss/48304180.eqep/"
#define testPath "/root/ECE597/Homework04/"

int main(int argc, char *argv[])
{
    int file, len, success;
    char buf[50];
    char filename[100];
    sprintf(filename, eQEP2);
    strcat(filename, "position");
    printf("opening %speriod\n", eQEP2);
    file = open(eQEP2 "position", O_RDWR);
    if (file < 0) {
        printf("open failed\n");
		return 1;
	}
	len = snprintf(buf,sizeof(buf), "%d", 100000000);
	printf("%d\n", len);
	printf("writing %speriod\n", eQEP2);
	//success = write(file, buf, len);
	printf("reading %speriod\n", eQEP2);
	success = read(file, buf, sizeof(buf));
	printf("%s\n", buf);
	printf("%d\n", atoi(buf));
	printf("closing %speriod\n", eQEP2);
	close(file);
	printf("closed.\n");
	return 0;
}