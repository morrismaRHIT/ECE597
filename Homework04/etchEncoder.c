//echo bone_eqep2b > /sys/devices/bone_capemgr.*/slots

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

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define POLL_TIMEOUT (100) /* 100 milliseconds */
#define MAX_BUF 64

#define BICOLOR		// undef if using a single color display

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

#define ROWS 8		//number of rows in etch-a-sketch matrix
#define COLS 8		//number of columns...

#define UP 1
#define DOWN 2
#define RIGHT 3
#define LEFT 4

#define LED_ON 1
#define LED_OFF 0
#define LED_ON_TIME 100000  //time in us to turn led on

//var eQEP0 = "/sys/devices/ocp.3/48300000.epwmss/48300180.eqep/",
//    eQEP1 = "/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/",
//    eQEP2 = "/sys/devices/ocp.3/48304000.epwmss/48304180.eqep/",
//    eQEP = eQEP2;
#define eQEP0 "/sys/devices/ocp.3/48300000.epwmss/48300180.eqep/"
#define eQEP1 "/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/"
#define eQEP2 "/sys/devices/ocp.3/48304000.epwmss/48304180.eqep/"
#define period 100	//period of encoder in ms

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed
char matrix[ROWS][COLS];
int xPos = 0, yPos = 0;

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

void draw_matrix(int file);
void move(int dir);
void clear_matrix(void);
void setupEncoder(int, int);
int readEncoder(int);

static int check_funcs(int file) {
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
		fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
		return -1;
	}
	return 0;
}

// Writes block of data to the display
static int write_block(int file, __u16 *data) {
	int res;
#ifdef BICOLOR
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)data);
	return res;
#else
/*
 * For some reason the single color display is rotated one column, 
 * so pre-unrotate the data.
 */
	int i;
	__u16 block[I2C_SMBUS_BLOCK_MAX];
//	printf("rotating\n");
	for(i=0; i<8; i++) {
		block[i] = (data[i]&0xfe) >> 1 | 
			   (data[i]&0x01) << 7;
	}
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)block);
	return res;
#endif
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, led1_fd, button2_fd, led2_fd, timeout, rc;
	int button3_fd, button4_fd, button5_fd;
	int led3_fd, led4_fd, led5_fd;
	char buf[MAX_BUF];
	int len;
	int res, i2cbus, matrix_addr, file;
	char filename[20];
	int force = 0;
	int lastPos2 = 0, lastPos1 = 0, pos2 = 0, pos1 = 0;
	int periodFile2, enabledFile2, positionFile2;
	int periodFile1, enabledFile1, positionFile1;
	
	
	i2cbus = lookup_i2c_bus("1");
	printf("i2cbus = %d\n", i2cbus);
	if (i2cbus < 0)
		printf("i2cbus problem\n");

	matrix_addr = parse_i2c_address("0x70");
	printf("address = 0x%2x\n", matrix_addr);
	if (matrix_addr < 0)
		printf("address problem\n");
		
	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	if (file < 0 || check_funcs(file) || set_slave_addr(file, matrix_addr, force))
		exit(1);
		
	printf("opening eQEP2 period file\n");
	periodFile2 = open(eQEP2 "period", O_WRONLY);
	if(periodFile2 < 0)
	{
		printf("failed to open eQEP2 period file.\n");
		exit(1);
	}
	printf("opening eQEP2 enabled file\n");
	enabledFile2 = open(eQEP2 "enabled", O_WRONLY);
	if(enabledFile2 < 0)
	{
		printf("failed to open eQEP2 enabled file.\n");
		exit(1);
	}
	
	printf("opening eQEP1 period file\n");
	periodFile1 = open(eQEP1 "period", O_WRONLY);
	if(periodFile1 < 0)
	{
		printf("failed to open eQEP1 period file.\n");
		exit(1);
	}
	printf("opening eQEP1 enabled file\n");
	enabledFile1 = open(eQEP1 "enabled", O_WRONLY);
	if(enabledFile1 < 0)
	{
		printf("failed to open eQEP1 enabled file.\n");
		exit(1);
	}
	
	
	/*periodFile2 = fopen(strcat(eqep2,"period"), "w");
	if(periodFile2 == NULL)
	{
		printf("could not open eQEP2 period file\n");
		exit(1);
	}
	printf("opening enabled file");
	enabledFile2 = fopen(strcat(eqep2, "enabled"), "w");
	if(enabledFile2 == NULL)
	{
		printf("could not open eQEP2 enabled file\n");
		exit(1);
	}
	printf("opening position file");
	positionFile2 = fopen(strcat(eqep2, "position"), "r");
	if(positionFile2 == NULL)
	{
		printf("could not open eQEP2 position file\n");
		exit(1);
	}*/

	setupEncoder(periodFile2, enabledFile2);
	setupEncoder(periodFile1, enabledFile1);
	
	// Check the return value on these if there is trouble
	i2c_smbus_write_byte(file, 0x21); // Start oscillator (p10)
	i2c_smbus_write_byte(file, 0x81); // Disp on, blink off (p11)
	i2c_smbus_write_byte(file, 0xe7); // Full brightness (page 15)

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	//for buttons
	gpio_export(button5);
	gpio_set_dir(button5, "in");
	gpio_set_edge(button5, "rising");  // Can be rising, falling or both
	button5_fd = gpio_fd_open(button5, O_RDONLY);

	timeout = POLL_TIMEOUT;
	
	clear_matrix();
	draw_matrix(file);
	
	positionFile2 = open(eQEP2 "position", O_RDONLY);
	if(!(positionFile2 < 0))
	{
		lastPos2 = pos2;
		pos2 = readEncoder(positionFile2);
	}
	close(positionFile2);
	
	positionFile1 = open(eQEP1 "position", O_RDONLY);
	if(!(positionFile1 < 0))
	{
		lastPos1 = pos1;
		pos1 = readEncoder(positionFile1);
	}
	close(positionFile1);
 
	while (keepgoing) {
		//printf("start loop\n");
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
		
		fdset[1].fd = button5_fd;
		fdset[1].events = POLLPRI;

		//printf("poll\n");

		rc = poll(fdset, nfds, timeout);    
		
		//printf("open files\n");
		
		positionFile2 = open(eQEP2 "position", O_RDONLY);
		if(!(positionFile2 < 0))
		{
			lastPos2 = pos2;
			pos2 = readEncoder(positionFile2);
		}
		close(positionFile2);
		
		positionFile1 = open(eQEP1 "position", O_RDONLY);
		if(!(positionFile1 < 0))
		{
			lastPos1 = pos1;
			pos1 = readEncoder(positionFile1);
		}
		close(positionFile1);
		
		//printf("files closed\n");
            
		if (pos1 < lastPos1) {   
			move(LEFT);
		}
		if (pos2 < lastPos2) {   //button 2 pressed
			move(DOWN);
		}
		if (pos1 > lastPos1) {   
			move(RIGHT);
		}
		if (pos2 > lastPos2) {   //button 4 pressed
			move(UP);
		}
		if (fdset[1].revents & POLLPRI) {   //button 5 pressed
		    lseek(fdset[1].fd,0,SEEK_SET);
		    len = read(fdset[1].fd, buf, MAX_BUF);
			clear_matrix();
		}
		
		//printf("moved\n");

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}
		
		
		draw_matrix(file);
		
		//printf("drawn.\n");

		fflush(stdout);
		
		//printf("end loop\n");
	}

	gpio_fd_close(button5_fd);
	close(periodFile2);
	close(positionFile2);
	close(enabledFile2);
	
	close(periodFile1);
	close(positionFile1);
	close(enabledFile1);
	
	return 0;
}

void draw_matrix(int file)
{
	int i, j;
	__u16 matrix_bmp[8] = {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};
	system("clear");
	for(i = 0; i < ROWS; i++)
	{
		for(j = 0; j < COLS; j++)
		{
			if(matrix[i][j] == 1)
			{
				matrix_bmp[j] += 1<<(7-i);
				printf("x");
			}
			else if(matrix[i][j] == 2)
			{
				matrix_bmp[j] += 1<<(15-i);
				printf("x");
			}
			else if(matrix[i][j] == 3)
			{
				matrix_bmp[j] += 1<<(7-i);
				matrix_bmp[j] += 1<<(15-i);
				printf("x");
			}
			else
			{
				printf(" ");
			}
		}
		printf("\n");
	}
	write_block(file,matrix_bmp);
}

void clear_matrix()
{
	int i, j;
	for(i = 0; i < ROWS; i++)
	{
		for(j = 0; j < COLS; j++)
		{
			matrix[i][j] = 0;
		}
	}
}

void move(int dir)
{
	if(dir==UP && yPos > 0)
	{
	    yPos = yPos - 1;
	}
	else if(dir == DOWN && yPos < ROWS-1)
	{
	    yPos++;
	}
	else if(dir == RIGHT && xPos < COLS-1)
	{
	    xPos++;
	}
	else if(dir == LEFT && xPos > 0)
	{
	    xPos--;
	}
	matrix[yPos][xPos]++;
	if(matrix[yPos][xPos] > 3)
	{
		matrix[yPos][xPos] = 0;
	}
}

void setupEncoder(int periodFile, int enabledFile)
{
	char buf[MAX_BUF];
	int len;
	len = snprintf(buf, sizeof(buf), "%d", period*1000000);
	write(periodFile, buf, len);
	len = snprintf(buf, sizeof(buf), "%d", 1);
	write(enabledFile, buf, len);
}

int readEncoder(int positionFile)
{
	char buf[MAX_BUF];
	int pos;
	read(positionFile, buf, sizeof(buf));
	pos = atoi(buf);
	return pos;
}