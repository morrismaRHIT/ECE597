// From : http://stackoverflow.com/questions/13124271/driving-beaglebone-gpio-through-dev-mem
//
// Read one gpio pin and write it out to another using mmap.
// Be sure to set -O3 when compiling.
// Modified by Mark A. Yoder  26-Sept-2013
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h>    // Defines signal-handling functions (i.e. trap Ctrl-C)
#include "beaglebone_gpio.h"


#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;    // Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
    printf( "\nCtrl-C pressed, cleaning up and exiting...\n" );
	keepgoing = 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, const char* dir)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	write(fd, dir, sizeof(dir)+1);

	close(fd);
	return 0;
}

int main(int argc, char *argv[]) {
    volatile void *gpio_addr;
    volatile unsigned int *gpio_oe_addr;
    volatile unsigned int *gpio_datain;
    volatile unsigned int *gpio_setdataout_addr;
    volatile unsigned int *gpio_cleardataout_addr;
    volatile void *gpio3_addr;
    volatile unsigned int *gpio3_oe_addr;
    volatile unsigned int *gpio3_datain;
    volatile unsigned int *gpio3_setdataout_addr;
    volatile unsigned int *gpio3_cleardataout_addr;
    unsigned int reg;

    // Set the signal callback for Ctrl-C
    signal(SIGINT, signal_handler);

    int fd = open("/dev/mem", O_RDWR);

    printf("Mapping %X - %X (size: %X)\n", GPIO0_START_ADDR, GPIO0_END_ADDR, 
                                           GPIO0_SIZE);

    gpio_addr = mmap(0, GPIO0_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 
                        GPIO0_START_ADDR);
    gpio3_addr = mmap(0, GPIO3_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 
                        GPIO3_START_ADDR);

    gpio_oe_addr           = gpio_addr + GPIO_OE;
    gpio_datain            = gpio_addr + GPIO_DATAIN;
    gpio_setdataout_addr   = gpio_addr + GPIO_SETDATAOUT;
    gpio_cleardataout_addr = gpio_addr + GPIO_CLEARDATAOUT;
    
    gpio3_oe_addr           = gpio3_addr + GPIO_OE;
    gpio3_datain            = gpio3_addr + GPIO_DATAIN;
    gpio3_setdataout_addr   = gpio3_addr + GPIO_SETDATAOUT;
    gpio3_cleardataout_addr = gpio3_addr + GPIO_CLEARDATAOUT;
    
    gpio_set_dir(4, "out");

    if(gpio_addr == MAP_FAILED) {
        printf("Unable to map GPIO\n");
        exit(1);
    }
    if(gpio3_addr == MAP_FAILED) {
        printf("unable to map GPIO3\n");
        exit(1);
    }
    printf("GPIO mapped to %p\n", gpio_addr);
    printf("GPIO OE mapped to %p\n", gpio_oe_addr);
    printf("GPIO SETDATAOUTADDR mapped to %p\n", gpio_setdataout_addr);
    printf("GPIO CLEARDATAOUT mapped to %p\n", gpio_cleardataout_addr);

    printf("Start copying GPIO_07 to GPIO_03\n");
    printf("Start copying GPIO_115 to GPIO_30 and GPIO_31\n");
    
    while(keepgoing) {
    	if(*gpio_datain & GPIO_07) {
            *gpio_setdataout_addr= GPIO_03;
    	} else {
            *gpio_cleardataout_addr = GPIO_03;
    	}
    	if(*gpio3_datain & GPIO_19) {
    	    *gpio_setdataout_addr= GPIO_30;
    	    *gpio_setdataout_addr= GPIO_31;
    	} else {
    	    *gpio_cleardataout_addr = GPIO_30;
    	    *gpio_cleardataout_addr = GPIO_31;
    	}
        usleep(0);
    }

    munmap((void *)gpio_addr, GPIO0_SIZE);
    munmap((void *)gpio3_addr, GPIO3_SIZE);
    close(fd);
    return 0;
}
