#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

int pid;
char base_directory[256] = "/home/andrus/OS_project"; // Base directory for the monitor

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        DIR *dir;
        struct dirent *entry;

        if ((dir = opendir(base_directory)) == NULL) {
            perror("Failed to open directory");
            exit(1);
        }

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
            pid_t child_pid = fork();
            if (child_pid == 0) { // Child process
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", base_directory, entry->d_name ? entry->d_name : "");
                execlp("./treasure_manager", "treasure_manager", "list", path, (char *)NULL);
                perror("execlp failed");
                exit(1);
            } else if (child_pid < 0) {
                perror("Failed to fork");
            } else {
                wait(NULL); // Wait for the child process to complete
            }
            }
        }

        closedir(dir);
    }
}

int main() {
    printf("Program started, use \'help\' if you need instructions on how to use it...\n");

    char command[100];
    bool monitor_started = false;

    while (1) {
        if (scanf("%s", command) != 1) {
            fprintf(stderr, "Error reading command\n");
            return 1;
        }

        if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("start_monitor: Starts a separate background process that monitors the hunts and prints to the standard output information about them when asked to\n");
            printf("list_hunts: Asks the monitor to list the hunts and the total number of treasures in each\n");
            printf("list_treasures: Tells the monitor to show the information about all treasures in a hunt, the same way as the command line at the previous stage did\n");
            printf("view_treasure: Tells the monitor to show the information about a treasure in hunt, the same way as the command line at the previous stage did\n");
            printf("stop_monitor: Asks the monitor to end then returns to the prompt. Prints monitors termination state when it ends.\n");
            printf("exit: If the monitor still runs, prints an error message, otherwise ends the program\n");
        } else if (strcmp(command, "start_monitor") == 0) {
            if (monitor_started) {
                printf("Monitor is already running.\n");
                continue;
            }
            pid = fork();
            if (pid < 0) {
                perror("Error creating monitor process");
                exit(1);
            }
            if (pid == 0) {
                signal(SIGUSR1, handle_signal);
                printf("Monitor process started. Waiting for signals...\n");
                while (1) {
                    pause(); // Wait for signals
                }
            } else {
                monitor_started = true;
                printf("Starting monitor...\n");
            }
        } else if (strcmp(command, "list_hunts") == 0) {
            if (!monitor_started) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Listing hunts...\n");
        } else if (strcmp(command, "list_treasures") == 0) {
            if (!monitor_started) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Sending signal to monitor to list treasures...\n");
            if (kill(pid, SIGUSR1) != 0) {
                perror("Failed to send signal to monitor");
            }
        } else if (strcmp(command, "view_treasure") == 0) {
            if (!monitor_started) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Viewing treasure...\n");
        } else if (strcmp(command, "stop_monitor") == 0) {
            if (!monitor_started) {
                printf("Monitor is not even started. How would YOU close it?\n");
                continue;
            }
            if (kill(pid, SIGTERM) == 0) {
                printf("Monitor process terminated successfully.\n");
                wait(NULL); // Wait for the child process to terminate
            } else {
                perror("Failed to terminate monitor process");
            }
            monitor_started = false;
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_started) {
                printf("Monitor is still running. Please stop it before exiting.\n");
                continue;
            }
            printf("Exiting program...\n");
            break;
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}