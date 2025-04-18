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
#define LOG_FILE "logged_hunt"
#define PATH_MAX 4096

//To do: Add a log where we save the commands used
//To do: Add some better way of comunicating the commands the user can use (like a help command)

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

/**
 * Helper: Combines the directory and fixed file name to build the full path.
 */
void build_file_path(char *dest, const char *dir) {
    snprintf(dest, PATH_MAX, "%s/%s", dir, TREASURE_FILE);
}

void log_command(const char *command, const char *dir)
{
    char log_path[PATH_MAX];
    snprintf(log_path, PATH_MAX, "%s/%s", dir, LOG_FILE);

    int fl = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fl == -1) {
        perror("Error opening log file");
        exit(1);
    }

    write(fl, command, strlen(command));
    write(fl, "\n", 1);
    close(fl);
}

void add_treasure(const char *dir_name) {

    // Create the directory if it does not exist
    struct stat st;
    if (stat(dir_name, &st) == -1) {
        if (mkdir(dir_name, 0755) == -1) {
            perror("Error creating directory");
            exit(1);
        }
        printf("Directory '%s' created successfully.\n", dir_name);
    }

    char path[PATH_MAX];
    build_file_path(path, dir_name);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    treasure_t treasure;

    printf("Enter treasure ID: ");
    scanf("%9s", treasure.ID);
    getchar(); // consume newline left by scanf

    printf("Enter user name: ");
    fgets(treasure.User, sizeof(treasure.User), stdin);
    treasure.User[strcspn(treasure.User, "\n")] = 0; // remove newline

    printf("Enter latitude: ");
    scanf("%f", &treasure.coords.latitude);

    printf("Enter longitude: ");
    scanf("%f", &treasure.coords.longitude);
    getchar(); // consume newline

    printf("Enter clue text: ");
    fgets(treasure.Clue, sizeof(treasure.Clue), stdin);
    treasure.Clue[strcspn(treasure.Clue, "\n")] = 0;

    printf("Enter treasure value: ");
    scanf("%d", &treasure.value);

    if (write(fd, &treasure, sizeof(treasure)) != sizeof(treasure)) {
        perror("Error writing to file");
        close(fd);
        exit(1);
    }

    printf("Treasure written to %s successfully.\n", path);
    close(fd);
}

void read_treasure(const char *dir_name) {
    char path[PATH_MAX];
    build_file_path(path, dir_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    treasure_t treasure;
    while (read(fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
        printf("Treasure Details:\n");
        printf("ID: %s\n", treasure.ID);
        printf("User: %s\n", treasure.User);
        printf("Coordinates: Latitude %.4f, Longitude %.4f\n", treasure.coords.latitude, treasure.coords.longitude);
        printf("Clue: %s\n", treasure.Clue);
        printf("Value: %d\n\n", treasure.value);
    }

    close(fd);
}

void list_all_IDs(const char *dir_name) {
    char path[PATH_MAX];
    build_file_path(path, dir_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    treasure_t treasure;
    while (read(fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
        printf("Treasure ID: %s\n", treasure.ID);
    }
    close(fd);

}

void list_treasure(const char *dir_name) {

    printf("Hunt: %s\n", dir_name);
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("Error opening directory");
        exit(1);
    }

    struct dirent *entry;
    struct stat file_stat;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            char file_path[PATH_MAX];
            snprintf(file_path, PATH_MAX, "%s/%s", dir_name, entry->d_name);

            if (stat(file_path, &file_stat) == 0) {
                char timebuf[80];
                struct tm *timeinfo = localtime(&file_stat.st_atime);
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo);

                printf("File: %s, Size: %ld bytes, Last access time: %s\n", entry->d_name, file_stat.st_size, timebuf);
            } else {
                perror("Error getting file stats");
            }
        }
    }
    printf("\n");
    list_all_IDs(dir_name); // Read treasures after listing files

    closedir(dir);

}

void view_treasure(const char *dir_name, const char *ID){
    char path[PATH_MAX];
    build_file_path(path, dir_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    treasure_t treasure;
    int found = 0;
    while (read(fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
        if (strcmp(treasure.ID, ID) == 0) {
            printf("Treasure Details:\n");
            printf("ID: %s\n", treasure.ID);
            printf("User: %s\n", treasure.User);
            printf("Coordinates: Latitude %.4f, Longitude %.4f\n", treasure.coords.latitude, treasure.coords.longitude);
            printf("Clue: %s\n", treasure.Clue);
            printf("Value: %d\n\n", treasure.value);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Treasure with ID '%s' not found.\n", ID);
    }

    close(fd);
}

void delete(const char *dir_name, const char *ID) {
    char path[PATH_MAX], temp_path[PATH_MAX];
    build_file_path(path, dir_name);
    snprintf(temp_path, PATH_MAX, "%s/temp_%s", dir_name, TREASURE_FILE);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    int temp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        perror("Error creating temporary file");
        close(fd);
        exit(1);
    }

    treasure_t treasure;
    int found = 0;
    while (read(fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
        if (strcmp(treasure.ID, ID) == 0) {
            found = 1; // Mark as found and skip writing this treasure
            continue;
        }
        if (write(temp_fd, &treasure, sizeof(treasure)) != sizeof(treasure)) {
            perror("Error writing to temporary file");
            close(fd);
            close(temp_fd);
            unlink(temp_path); // Cleanup temporary file
            exit(1);
        }
    }

    close(fd);
    close(temp_fd);

    if (!found) {
        printf("Treasure with ID '%s' not found.\n", ID);
        unlink(temp_path); // Cleanup temporary file
        return;
    }

    if (rename(temp_path, path) == -1) {
        perror("Error replacing original file");
        unlink(temp_path); // Cleanup temporary file
        exit(1);
    }

    printf("Treasure with ID '%s' deleted successfully.\n", ID);
}

void remove_hunt(const char *dir_name) {
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("Error opening directory");
        exit(1);
    }

    struct dirent *entry;
    char file_path[PATH_MAX];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip current and parent directory entries
        }

        snprintf(file_path, PATH_MAX, "%s/%s", dir_name, entry->d_name);
        if (entry->d_type == DT_DIR) {
            remove_hunt(file_path); // Recursively remove subdirectories
        } else {
            if (unlink(file_path) == -1) {
                perror("Error deleting file");
                closedir(dir);
                exit(1);
            }
        }
    }

    closedir(dir);

    // Remove the symbolic link in the root directory
    char link_name[PATH_MAX];
    snprintf(link_name, PATH_MAX, "logged_hunt_%s", dir_name);
    if (unlink(link_name) == -1) {
        perror("Error removing symbolic link");
    }

    if (rmdir(dir_name) == -1) {
        perror("Error removing directory");
        exit(1);
    }

    printf("Hunt directory '%s' and its symbolic link removed successfully.\n", dir_name);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <command> <hunt_directory>\n", argv[0]);
        return 1;
    }

    const char *command = argv[1];
    const char *directory = argv[2];

    struct stat st;
    if (strcmp(command, "add") != 0) {
        if (stat(directory, &st) == -1 || !S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: '%s' is not a valid directory.\n", directory);
            return 1;
        }
    }

    if (strcmp(command, "add") == 0) {
        add_treasure(directory);
        char logged_command[100];
        strcpy(logged_command,"add ");
        strcat(logged_command, directory);
        log_command(logged_command, directory);

        // Create symbolic link to the logged_hunt file
        char link_name[PATH_MAX];
        snprintf(link_name, PATH_MAX, "logged_hunt_%s", directory);
        char log_path[PATH_MAX];
        snprintf(log_path, PATH_MAX, "%s/%s", directory, LOG_FILE);

        if (symlink(log_path, link_name) == -1) {
            perror("Error creating symbolic link");
        } else {
            printf("Symbolic link '%s' created successfully.\n", link_name);
        }
    } else if (strcmp(command, "read") == 0) {
        read_treasure(directory);
        char logged_command[100];
        strcpy(logged_command,"read ");
        strcat(logged_command, directory);
        log_command(logged_command,directory);
    } else if (strcmp(command, "list") == 0) {
        list_treasure(directory);
        char logged_command[100];
        strcpy(logged_command,"list ");
        strcat(logged_command, directory);
        log_command(logged_command, directory);
    } else if (strcmp(command, "view") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s view <hunt_directory> <treasure_ID>\n", argv[0]);
            return 1;
        }
        view_treasure(directory, argv[3]);
        char logged_command[100];
        strcpy(logged_command,"view ");
        strcat(logged_command, directory);
        log_command(logged_command, directory);
    } else if(strcmp(command, "remove_treasure") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s remove_treasure <hunt_directory> <treasure_ID>\n", argv[0]);
            return 1;
        }
        delete(directory, argv[3]);
        char logged_command[100];
        strcpy(logged_command,"remove_treasure ");
        strcat(logged_command, directory);
        strcat(logged_command, argv[3]);
        log_command(logged_command, directory);
    } else if(strcmp(command, "remove_hunt") == 0) {
        remove_hunt(directory);
    } else {
        // Invalid command
        fprintf(stderr, "Invalid command. Use 'add' or 'read'.\n");
        return 1;
    }

    return 0;
}
