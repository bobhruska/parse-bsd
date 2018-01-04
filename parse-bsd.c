#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

//#define FILENAME "/webserver/wxdata"
//#define LOGFILE "/webserver/wxdata-logfile"
#define FILENAME "test-wxdata"
#define LOGFILE "test-logfile"

char alldata[512];

char temp[80];
char humi[80];
char baro[80];
char wspd[80];
char wdir[80];
char rain[80];
char tmp2[80];
char tmp3[80];
char tmp4[80];
char hum2[80];
char hum3[80];
char hum4[80];

float last_rain = 0;
int wait_http=0;
int wait_get=1;
int capturing=0;

#define got_temp 1
#define got_humi 2
#define got_baro 4
#define got_wspd 8
#define got_wdir 16
#define got_rain 32

int got_all = 0;

int print_values(char *name, char *val) {
	int retval=0;

//	printf("'%s'  '%s'\n", name, val);

	if ((!strcmp(name, "action")) || \
            (!strcmp(name, "baromin"))  || \
            (!strcmp(name, "battery"))  || \
            (!strcmp(name, "dailyrainin"))  || \
            (!strcmp(name, "dateutc"))  || \
            (!strcmp(name, "dewptf"))  || \
            (!strcmp(name, "humidity"))  || \
            (!strcmp(name, "id"))  || \
            (!strcmp(name, "mt"))  || \
            (!strcmp(name, "rainin"))  || \
            (!strcmp(name, "realtime"))  || \
            (!strcmp(name, "rssi"))  || \
            (!strcmp(name, "sensor"))  || \
            (!strcmp(name, "tempf"))  || \
            (!strcmp(name, "winddir"))  || \
            (!strcmp(name, "windspeedmph"))) {
		retval = 1;
	}

	return retval;


}

void done_capture(char *s, int len) {
	char *left;
	char *right;
	char *eq;
	char copy[256];

	left=s;
	right=s;
	eq=s;
        if(len <= 4) {
            return;
        }

        s[len] = '\0';
//        printf("str: '%s'\n", s);

	eq = strstr(s, "=");
	if (eq) {
		strcpy(copy, s);
		eq[0] = '\0';
		left = s;
		right = eq + 1;

		if( print_values(left, right)) {
		        if (alldata[0] != '\0') {
               	 		strcat(alldata, "&");
        		}
        		strcat(alldata, copy);
		}

	}
        if(strlen(left) >=4) {
	if(!strncmp(left, "rssi", 4)) {
//printf("rssi trigger\n");
		printf("%s\n", alldata);
		alldata[0] = '\0';
		fflush(NULL);
		capturing=0;
		wait_get=1;
	}
        }
}

void shiftleft(char *s, int len) {

	int i;

	for (i = 0; i < len; i++) {
		s[i] = s[i + 1];
	}

}

int wait_for_gdb = 0;
void main() {

	char ch;
	char str[100];
	int i;
	int capture = 0;
	char get[40];

	while (wait_for_gdb) {
		printf("waiting\n");
	}
	while (read(STDIN_FILENO, &ch, 1) > 0) {
//printf("%c", ch);
		if(wait_http) {
			shiftleft(get, 6);
			get[6] = ch;
			get[7] = '\0';
			if (!strncmp(get, "!http]", 6)) {
				wait_http=0;
				wait_get=1;
                		get[0] = '\0';
			}
		} else if(wait_get) {
                        shiftleft(get, 21);
                        get[21] = ch;
                        get[22] = '\0';
                        if (strncmp(get, "updateweatherstation?", 21)) {
                                continue;
                        }
                        //wait_http=1;
			wait_get=0;
                	get[0] = '\0';
			i = 0;
			alldata[0] = '\0';
			capture = 1;
			capturing = 1;
		}

		if(capturing) {
		if (ch == '&') {
			if (capture) {
				done_capture(str, i);
				i = 0;
			} else {
				capture = 1;
			}
		}
		if (ch < 0x2d && ch != '&') {
			//printf("special char 0x%x\n", ch);
			if (capture) {
				str[i] = '\0';
				done_capture(str, i);
				capture = 0;
				i = 0;
			}
		}
		if (capture && ch != '&' && ch != '\n' && ch != '\r') {
			str[i++] = ch;
//printf("string: '%s'\n", str);
			if (i == 100) {
				printf("too big!\n");
				// String has gotten too big, it is bogus, truncate it
				i = 0;
			}
		}
		}
	}

}
