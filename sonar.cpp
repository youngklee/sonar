#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <error.h>
#include <sys/ioctl.h>
#include <iostream>

enum sonar_state {
    sonar_waiting,
    sonar_reading
};

static uint8_t alt[4];

int main(int argc, char *argv[])
{
    printf("Reading sonar...\n");
    struct termios origtio;

    int sonar_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

    if (sonar_fd < 0) {
        perror("open");
        exit(-1);
    }

    tcgetattr(sonar_fd, &origtio);

    cfsetispeed(&origtio, B9600);
    cfsetospeed(&origtio, B9600);

    origtio.c_cflag |= (CLOCAL | CREAD);

    tcsetattr(sonar_fd, TCSANOW, &origtio);

    fcntl(sonar_fd, F_SETFL, FNDELAY);

    int ret = 0;
    static enum sonar_state state = sonar_waiting;
    
    uint8_t c;
    char alt_char[4];
    int alt;  // altitude in mm
    int bytes;
    int to_read;
    int to_read_inner;
    unsigned int digit = 0;

    while (1) {
        ioctl(sonar_fd, FIONREAD, &to_read);
        for (int i = 0; i < to_read; i++) {
            read(sonar_fd, &c, 1);
            switch (state) {
                case sonar_waiting:
                    if (c == 82) {
                        state = sonar_reading;
                    }
                    break;
                case sonar_reading:
                    alt_char[digit] = c;
                    digit++;
                    if (digit > 3) {
                        digit = 0;
                        state = sonar_waiting;
                    }
                    break;
            }
        }
        alt = atoi(alt_char);
        printf("%i\n", alt);
        usleep(100);
    }
    return ret;
}
