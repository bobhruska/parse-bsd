#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

char alldata[512];

int wait_get = 1;
int capturing = 0;

int print_values(char *name, char *val) {
	int retval = 0;

//	printf("'%s'  '%s'\n", name, val);

	if ((!strcmp(name, "action")) || (!strcmp(name, "baromin"))
			|| (!strcmp(name, "battery")) || (!strcmp(name, "dailyrainin"))
			|| (!strcmp(name, "dateutc")) || (!strcmp(name, "dewptf"))
			|| (!strcmp(name, "humidity")) || (!strcmp(name, "id"))
			|| (!strcmp(name, "mt")) || (!strcmp(name, "rainin"))
			|| (!strcmp(name, "realtime")) || (!strcmp(name, "rssi"))
			|| (!strcmp(name, "sensor")) || (!strcmp(name, "tempf"))
			|| (!strcmp(name, "winddir")) || (!strcmp(name, "windspeedmph"))) {
		retval = 1;
	}

	return retval;

}

// This is looking for an entry like: somefield=somevalue
// It sanitizes it first, then if it is a valid named field
// it adds it to the buffer to be sent to weewx
void done_capture(char *s, int len) {
	char *left;
	char *right;
	char *eq;
	char copy[256];

	left = s;
	right = s;
	eq = s;
	if (len <= 4) {
		return;
	}

	s[len] = '\0';
//        printf("str: '%s'\n", s);

    // Find the equal sign, the left and right fields
	eq = strstr(s, "=");
	if (eq) {
		strcpy(copy, s);
		eq[0] = '\0';
		left = s;
		right = eq + 1;

        // If the field name is valid, add it to output
		if (print_values(left, right)) {
			if (alldata[0] != '\0') {
				strcat(alldata, "&");
			}
			strcat(alldata, copy);
		}

	}
    
    // The end of the request will always be the field "rssi"
    // so when we see this, print out the data for weewx
    // and start looking for another GET request
	if (strlen(left) >= 4) {
		if (!strncmp(left, "rssi", 4)) {
//printf("rssi trigger\n");
			printf("%s\n", alldata);
			alldata[0] = '\0';
			fflush(NULL);
			capturing = 0;
			wait_get = 1;
		}
	}
}

// Used to keep a running buffer window of the input stream
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
        // Keep a running buffer window of 21 characters and look for
        // it to contain the update weatherstation GET request
		if (wait_get) {
			shiftleft(get, 21);
			get[21] = ch;
			get[22] = '\0';
			if (strncmp(get, "updateweatherstation?", 21)) {
				continue;
			}
            // Now we got it.  Start collecting data
			wait_get = 0;
			get[0] = '\0';
			i = 0;
			alldata[0] = '\0';
			capture = 1;
			capturing = 1;
		}

        // Capture characters up to the first '&' sign
        // This signifies a field is complete
		if (capturing) {
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
                // Keep growing the string
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
