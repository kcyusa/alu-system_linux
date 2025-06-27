#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Signal handler function for SIGTSTP
void handle_sigstp(int sig) {
    printf("\nCaught SIGTSTP (signal %d). Exiting gracefully...\n", sig);
    // Perform any cleanup here if necessary
    exit(0);  // Exit the program
}

int main() {
    // Register signal handler for SIGTSTP
    signal(SIGTSTP, handle_sigstp);

    printf("Program is running. Press Ctrl+Z to send SIGTSTP and terminate gracefully.\n");

    // Infinite loop to keep the program running
    while (1) {
        printf("Running...\n");
        sleep(2);  // Sleep to avoid flooding the terminal
    }

    return 0;
}
