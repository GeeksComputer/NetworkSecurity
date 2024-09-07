#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_command(const char* command) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process to run the command
        execlp("sh", "sh", "-c", command, (char*)NULL);
        // If execlp fails
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process waits for child to finish
        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid failed");
        }
    } else {
        // Fork failed
        perror("fork failed");
    }
}

void clear_cache() {
    // Sync to ensure all data is written to disk
    printf("Syncing data to disk...\n");
    if (system("sync") != 0) {
        fprintf(stderr, "Error: Failed to sync data.\n");
        return;
    }

    // Commands to clear caches
    const char* commands[] = {
        "echo 1 > /proc/sys/vm/drop_caches",  // Clear page cache
        "echo 2 > /proc/sys/vm/drop_caches",  // Clear dentries and inodes
        "echo 3 > /proc/sys/vm/drop_caches",  // Clear both page cache, dentries, and inodes
        "swapoff -a && swapon -a"              // Clear swap cache
    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        printf("Executing: %s\n", commands[i]);
        execute_command(commands[i]);
    }

    printf("Cache clearing operations completed.\n");
}

int main() {
    clear_cache();
    return 0;
}
// gcc -o clear_all_caches clear_all_caches.c