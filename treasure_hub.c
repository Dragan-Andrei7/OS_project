#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main() {
    printf("Program started, use \'help\' if you need instructions on how to use it...\n");

    char command[100];

    while (1)
    {
        bool monitor_stated = false;
        if(scanf("%s", command) != 1) {
            fprintf(stderr, "Error reading command\n");
            return 1;
        }

        if(strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("start_monitor: Starts a separate background process that monitors the hunts and prints to the standard output information about them when asked to\n"); 
            printf("list_hunts: Asks the monitor to list the hunts and the total number of treasures in each\n");
            printf("list_treasures: Tells the monitor to show the information about all treasures in a hunt, the same way as the command line at the previous stage did\n");
            printf("view_treasure: Tells the monitor to show the information about a treasure in hunt, the same way as the command line at the previous stage did\n");
            printf("stop_monitor: Asks the monitor to end then returns to the prompt. Prints monitors termination state when it ends.\n");
            printf("exit: If the monitor still runs, prints an error message, otherwise ends the program\n");
        }
        else if(strcmp(command, "start_monitor") == 0) {
            monitor_stated = true;
            printf("Starting monitor...\n");
            // Here you would typically fork a process and start the monitor
        }
        else if(strcmp(command, "list_hunts") == 0) {
            if(monitor_stated == false) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Listing hunts...\n");
            // Here you would typically send a signal to the monitor to list hunts
        }
        else if(strcmp(command, "list_treasures") == 0) {
            if(monitor_stated == false) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Listing treasures...\n");
            // Here you would typically send a signal to the monitor to list treasures
        }
        else if(strcmp(command, "view_treasure") == 0) {
            if(monitor_stated == false) {
                printf("Monitor not started. Please start the monitor first.\n");
                continue;
            }
            printf("Viewing treasure...\n");
            // Here you would typically send a signal to the monitor to view a specific treasure
        }
        else if(strcmp(command, "stop_monitor") == 0) {
            if(monitor_stated == false) {
                printf("Monitor is not even started. How would YOU close it?\n");
                continue;
            }
            monitor_stated = false;
            printf("Stopping monitor...\n");
            // Here you would typically send a signal to the monitor to stop it
        }
        else if(strcmp(command, "exit") == 0) {
            printf("Exiting program...\n");
            break;
        }
        else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}