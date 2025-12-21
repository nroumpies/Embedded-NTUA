#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_IN 64

/*
    Put the serial port into raw mode and flush both input and output queues.
*/
static int set_raw_and_flush(int fd) {
    struct termios t;

    if (tcgetattr(fd, &t) < 0) {
        return -1;
    }

    cfmakeraw(&t);

    if (tcsetattr(fd, TCSANOW, &t) < 0) {
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) < 0) {
        return -1;
    }

    return 0;
}

/*
    Read exactly nbytes, handling short reads and EINTR.
    Returns:
        >0  on success (always == nbytes)
         0  on EOF
        -1  on error (errno set)
*/
static ssize_t read_exact(int fd, void *buf, size_t nbytes) {
    size_t got = 0;
    unsigned char *p = (unsigned char *)buf;

    while (got < nbytes) {
        ssize_t r = read(fd, p + got, nbytes - got);

        if (r == 0) {
            return 0;
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        got += (size_t)r;
    }

    return (ssize_t)got;
}

/*
    If the user typed more than our input buffer, consume until newline
    so the next prompt starts cleanly.
*/
static void flush_stdin_until_newline(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        ;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/pts/N\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (set_raw_and_flush(fd) < 0) {
        perror("termios");
        close(fd);
        return 1;
    }

    for (;;) {
        char line[256];
        char out[MAX_IN + 1];

        /*
            New binary response protocol from guest:
                resp[0] = best character
                resp[1] = count (0..64)
        */
        unsigned char resp[2];

        printf("Please give a string to send to host:\n");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t len = strcspn(line, "\n");
        if (line[len] != '\n') {
            flush_stdin_until_newline();
        }

        if (len > MAX_IN) {
            len = MAX_IN;
        }

        memcpy(out, line, len);
        out[len] = '\n';

        if (write(fd, out, len + 1) < 0) {
            perror("write");
            break;
        }

        /*
            Receive exactly 2 bytes: character + count.
            This is more efficient than parsing an ASCII line.
        */
        if (read_exact(fd, resp, sizeof(resp)) <= 0) {
            perror("read");
            break;
        }

        char best = (char)resp[0];
        int count = (int)resp[1];

        printf("The most frequent character is\n");
        printf("\"%c\"\n", (unsigned char)best);
        printf("and it appeared %d times.\n", count);
    }

    close(fd);
    return 0;
}
