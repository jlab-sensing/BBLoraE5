/**
 * @file teroslogger.c
 * @author Alvin Tan
 * @author John Madden (jtmadden@ucsc.edu)
 * @brief Serial logger of TEROS-12 sensor to a .csv file
 * @date 2022-06-30
 *
 * This program is intended to be used in conjunction with \p SerialLogger
 * running on a Arduino connected to a serial port. Usage information can be
 * found by running the program without any arguments.
 *
 * The output format of the csv file is as follows
 * \verbatim
 * timestamp,raw_soil_moisture,temp,electrical_conductivity
 * \verbatim
 *
 * It is recommended to use the output parameter rather than redirecting the
 * stream. The output is buffered until a full line is read resulting in fewer
 * errors in the output file.
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <error.h>
#include <errno.h>

#include "ipc.h"

/** Baud rate of serial port */
#define BAUD_RATE B1200

/**
 * Prints help prompt
 *
 * @param name Name of program
 */
void print_help(const char * name) {
    printf("Run '%s -h' for usage.\n", name);
}

int main(int argc, char** argv){
    /* -------------- */
    /* Parse CLI args */
    /* -------------- */

    /* Display help prompt */
    int help = 0;
    /* Verbose logging */
    int verbose = 0;
    /* Do not print any output */
    int quiet = 0;
    /* Path to tty */
    char *tty_path = NULL;
    /* Output file name */
    char * output_file = NULL;
    /* Socket filepath */
    char * socket_file = NULL;

    opterr = 0;

    int c;
    while ((c = getopt (argc, argv, "hvqo:s:")) != -1) {
        switch (c) {
            case 'h':
                help = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'q':
                quiet = 1;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 's':
                socket_file = optarg;
                break;
            case '?':
                if (optopt == 'o') {
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                    print_help(argv[0]);
                }
                else if (isprint(optopt)) {
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                    print_help(argv[0]);
                }
                else {
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                    print_help(argv[0]);
                }
                return 1;
            default:
                abort();
        }
    }
    
    if (help) {
        printf("Usage: %s [-hvq] [-o file] [-s socket] [tty]\n", argv[0]);
        printf("Log TEROS-12 Soil Moisture sensor data read from <tty> to csv file or to a Unix Domain Socket. The -q option allows suppression of the data stream being outputted to stdout. Debug messaged enabled with the -v option will still be enabled.\n\n");
        printf("Options:\n");
        printf("  -h\t\tPrints this message\n");
        printf("  -v\t\tVerbose debug statements\n");
        printf("  -q\t\tRun quietly (no stdout)\n");
        printf("  -o <file>\tpath to output log file\n");
        printf("  -s <socket>\tpath to socket\n");
        return 0;
    }
    
    if (verbose && quiet) {
        printf("Cannot use -q and -v together!\n");
        print_help(argv[0]);
        return 1;
    }

    // Remaining options
    int optsrem = argc - optind;
    if (optsrem > 1) {
        printf("Too many arguments!\n");
        print_help(argv[0]);
        return 1;
    }
    else if (optsrem < 1) {
        printf("Too few arguments!\n");
        print_help(argv[0]);
        return 1;
    }
    else {
        tty_path = argv[optind];
    }

    if (verbose && output_file) {
        printf("Logging output to %s\n", output_file);
    }
    
    if (verbose && socket_file) {
        printf("Streaming to socket %s\n", socket_file);
    }

    /* ---------------- */
    /* Open serial port */
    /* ---------------- */

    // Open USB Port in readonly
    int serial_port = open(tty_path, O_RDONLY);
    // Check for errors
    if (serial_port < 0) {
        printf("%s\n", strerror(errno));
        return 1;
    }

    if (verbose) {
        printf("Opened %s\n", tty_path);
    }

    /* --------------------- */
    /* Configure serial port */
    /* --------------------- */

    struct termios tty;

    // Read existing settings
    if (tcgetattr(serial_port, &tty) != 0) {
        printf("%s\n", strerror(errno));
        return 1;
    }


    // 8-N-1
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    // No flow control
    tty.c_cflag &= ~CRTSCTS;

    // Turn on READ and ignore ctrl lines
    tty.c_cflag |= CREAD | CLOCAL;

    // Non-canonical mode (process as characters come in)
    tty.c_lflag &= ~ICANON;

    // Disable eching
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;

    // Disable signal characters
    tty.c_lflag &= ~ISIG;

    // Disable software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable special handling of bytes (allows only raw data)
    tty.c_iflag &= ~(IGNBRK | BRKINT |PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // TODO: Allow infinite wait time for character to account for any amount
    // of delays in reading from the serial port
    // 10 sec read timeout
    tty.c_cc[VTIME] = 100;
    tty.c_cc[VMIN] = 0;

    /* Set Baud Rate */
    cfsetospeed (&tty, BAUD_RATE);
    cfsetispeed (&tty, BAUD_RATE);

    // Apply settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("%s\n", strerror(errno));
        return 1;
    }

    if (verbose) {
        printf("Applied settings to %s\n", tty_path);
    }

    /* ---------------- */
    /* Log data to file */
    /* ---------------- */

    // Open output file
    FILE * outfile;
    if (output_file) {
        outfile = fopen(output_file, "w");
    }

    // Open socket
	printf("Opening teros socket\n");
    int socket;
    if (socket_file) {
        do { 
            socket = ipc_client(socket_file);
            if (verbose && (socket < 0)) {
                error(0, errno, "Could not open client socket");
            }
        }
        while (socket < 0);
    }

    if (verbose) {
        printf("Reading serial...\n");
    }

    /* Output Buffer */
    char outbuf[256];

    /* Input buffer */
    char * inbuf = outbuf;

    // Discard output until newline
    do {
        read(serial_port, inbuf, 1);
    }
    while (*inbuf != '\r');

    while (1) {
        // Store time
        int num_char = sprintf(inbuf, "%lu,", (unsigned long) time(NULL));
        inbuf += num_char;

        while(1) {
            // Read single byte of data
            num_char = read(serial_port, inbuf, sizeof(char)); 

            // Check for errors on read
            if (num_char < 0) {
                printf("%s\n", strerror(errno));
            }

            // Convert + to , for csv formatting
            if (*inbuf == '+') {
                *inbuf = ',';
            }

            // Stop reading on line ending
            if (*inbuf == '\r') {
                ++inbuf;
                break;
            }

            ++inbuf;
        }



        // Write to output file
        if (output_file) {
            fwrite(outbuf, sizeof(char), inbuf - outbuf, outfile);
            fflush(outfile);
        }

        // Write to socket
        if (socket_file) {
            int num_write;
            num_write = ipc_write(socket, outbuf, inbuf - outbuf);
            if (verbose && (num_write < 0)) {
                error(0, errno, "Could not write to socket");
            }
        }

        // Print to terminal
        if (!quiet) {
            *inbuf = '\0';
            printf("%s", outbuf);
        }

        // Reset pointer position 
        inbuf = outbuf;
    }

    if (output_file) {
        fclose(outfile);
    }

    if (socket_file) {
        ipc_close(socket);
    }

    return 0;
}
