#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

void clearCache() {
    // Sync to ensure all data is written to disk
    std::cout << "Syncing data to disk..." << std::endl;
    if (system("sync") != 0) {
        std::cerr << "Error: Failed to sync data." << std::endl;
        return;
    }

    // Array of commands to clear caches
    const char* commands[] = {
        "echo 1 > /proc/sys/vm/drop_caches",  // Clear page cache
        "echo 2 > /proc/sys/vm/drop_caches",  // Clear dentries and inodes
        "echo 3 > /proc/sys/vm/drop_caches",  // Clear page cache, dentries, and inodes
        "swapoff -a && swapon -a"              // Clear swap cache
    };

    for (const char* command : commands) {
        std::cout << "Executing: " << command << std::endl;

        pid_t pid = fork();
        if (pid == 0) {
            // Child process to run the command
            execlp("sh", "sh", "-c", command, (char*)nullptr);
            // If execlp fails
            std::cerr << "Error: Failed to execute command: " << command << std::endl;
            exit(1);
        } else if (pid > 0) {
            // Parent process waits for child to finish
            waitpid(pid, nullptr, 0);
        } else {
            // Fork failed
            std::cerr << "Error: Fork failed." << std::endl;
        }
    }

    std::cout << "Cache clearing operations completed." << std::endl;
}

int main() {
    clearCache();
    return 0;
}
// g++ -o clear_all_caches clear_all_caches.cpp