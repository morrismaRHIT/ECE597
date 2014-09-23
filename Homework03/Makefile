TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes
#
# Programs
#
all:	tmp101 matrixLEDi2c etchLEDs

tmp101:  tmp101.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^
	
matrixLEDi2c:  matrixLEDi2c.o i2cbusses.o
	$(CC) $(LDFLAGS) -o $@ $^

etchLEDs:	etchLEDs.o i2cbusses.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

#
# Objects
#

%.o: %.c
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

clean:
	rm tmp101.o tmp101 gpio-utils.o etchLEDs.o etchLEDs i2cbusses.o matrixLEDi2c.o matrixLEDi2c
