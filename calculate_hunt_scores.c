/*
Will take a hunt as argument and print to stdout the score of all users in the hunt
The score is the sum of values of a ll treasures assigned to said user
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define TREASURE_FILE "treasures.dat"
#define PATH_MAX 4096

int user_count = 0;

struct User_score {
    char *username;
    int score;
};
typedef struct User_score user_t;
user_t *users = NULL;

struct coordinates {
    float latitude, longitude;
};
typedef struct coordinates coords_t;

struct treasure {
    char ID[10];
    char User[30];
    coords_t coords;
    char Clue[100];
    int value;
};
typedef struct treasure treasure_t;

void build_file_path(char *dest, const char *dir) {
    snprintf(dest, PATH_MAX, "%s/%s", dir, TREASURE_FILE);
}

/**
 * Returns the index of the user if it exists, -1 otherwise
 */
int user_exists(char *username) {
    for(int i = 0; i < user_count ; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

void add_user_score(treasure_t *treasure) {
    int index = user_exists(treasure->User);
    if (index != -1) {
        users[index].score += treasure->value;
    } else {
        users = realloc(users, (user_count + 1) * sizeof(user_t));
        users[user_count].username = strdup(treasure->User);
        users[user_count].score = treasure->value;
        user_count++;
    }
}

void print_scores() {
    printf("User Scores:\n");
    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", users[i].username, users[i].score);
        free(users[i].username); // Free the allocated memory for username
    }
    free(users); // Free the users array
    printf("\n");
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_directory>\n", argv[0]);
        return 1;
    }

    char *hunt_directory = argv[1];

    char path[PATH_MAX];
    build_file_path(path, hunt_directory);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    } 

    users = malloc(sizeof(user_t));
    if (users == NULL) {
        perror("Error allocating memory");
        close(fd);
        exit(1);
    }

    treasure_t treasure;
    while(read(fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
        add_user_score(&treasure);
    }
    printf("User count: %d\n", user_count);

    close(fd);

    print_scores();
    
}