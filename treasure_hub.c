#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define SIGUSR3 SIGRTMIN + 1

int pid;
char base_directory[256] = "/home/andrus/OS_project"; // Base directory for the monitor

void handle_signal_list_hunts(int sig) {
    if (sig == SIGUSR3) {
        DIR *dir;
        struct dirent *entry;

        if ((dir = opendir(base_directory)) == NULL) {
            perror("Failed to open directory");
            exit(1);
        }

        int total_treasures = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", base_directory, entry->d_name ? entry->d_name : "");
                DIR *subdir = opendir(path);
                if (subdir) {
                    int count = 0;
                    struct dirent *subentry;
                    while ((subentry = readdir(subdir)) != NULL) {
                        if (subentry->d_type == DT_REG && subentry->d_name[0] != '.') {
                            count++;
                        }
                    }
                    closedir(subdir);
                    printf("Hunt: %s, Treasures: %d\n", entry->d_name, count);
                    total_treasures += count;
                }
            }
        }

        printf("Total treasures: %d\n", total_treasures);
        closedir(dir);
    }
}

void handle_signal_list_treasures(int sig) {
    if (sig == SIGUSR1) {
        char hunt_directory[256];
        printf("Enter the hunt directory to list treasures: ");
        if (scanf("%255s", hunt_directory) != 1) {
            fprintf(stderr, "Error reading hunt directory\n");
            return;
        }

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", base_directory, hunt_directory);

        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("Failed to open specified hunt directory");
            return;
        }
        closedir(dir);

        pid_t child_pid = fork();
        if (child_pid == 0) { // Child process
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

void handle_signal_view_treasure(int sig) {
    if (sig == SIGUSR2) {
        char hunt_directory[256];
        char treasure_id[256];

        printf("Enter the hunt directory: ");
        if (scanf("%255s", hunt_directory) != 1) {
            fprintf(stderr, "Error reading hunt directory\n");
            return;
        }

        printf("Enter the treasure ID: ");
        if (scanf("%255s", treasure_id) != 1) {
            fprintf(stderr, "Error reading treasure ID\n");
            return;
        }

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", base_directory, hunt_directory);

        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("Failed to open specified hunt directory");
            return;
        }
        closedir(dir);

        pid_t child_pid = fork();
        if (child_pid == 0) { // Child process
            execlp("./treasure_manager", "treasure_manager", "view", path, treasure_id, (char *)NULL);
            perror("execlp failed");
            exit(1);
        } else if (child_pid < 0) {
            perror("Failed to fork");
        } else {
            wait(NULL); // Wait for the child process to complete
        }
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

                struct sigaction list_treasures;
                memset(&list_treasures, 0, sizeof(list_treasures));
                list_treasures.sa_handler = handle_signal_list_treasures;
                sigaction(SIGUSR1, &list_treasures, NULL);

                struct sigaction view_treasure;
                memset(&view_treasure, 0, sizeof(view_treasure));
                view_treasure.sa_handler = handle_signal_view_treasure;
                sigaction(SIGUSR2, &view_treasure, NULL);

                struct sigaction list_hunts;
                memset(&list_hunts, 0, sizeof(list_hunts));
                list_hunts.sa_handler = handle_signal_list_hunts;
                sigaction(SIGUSR3, &list_hunts, NULL);




                //signal(SIGUSR1, handle_signal);
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
            printf("Sending signal to monitor to list hunts...\n");
            if( kill(pid, SIGUSR3) != 0) {
                perror("Failed to send signal to monitor");
            }
        } else if (strcmp(command, "list_treasures") == 0) {
            if (!monitor_started) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Sending signal to monitor to list treasures...\n");
            if (kill(pid, SIGUSR1) != 0) {
                perror("Failed to send signal to monitor");
            }
            sleep(1); // Wait for the monitor to process the signal
        } else if (strcmp(command, "view_treasure") == 0) {
            if (!monitor_started) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Viewing treasure...\n");
            if(kill(pid, SIGUSR2) != 0 )
            {
                perror("Failed to send signal to monitor");
            }
            sleep(10); // you get 10 seconds to input the hunt directory in wich the treasure is you are looking for is
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