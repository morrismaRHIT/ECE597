TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes
#
# Programs
#
all:	button_led etch_buttons

button_led:  button_led.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^
	
etch_buttons:	etch_buttons.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

#
# Objects
#

%.o: %.c
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

clean:
	rm button_led.o button_led gpio-utils.o etch_buttons.o etch_buttons
