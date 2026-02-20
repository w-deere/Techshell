#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024

int main() {
    char input[MAX_LINE];
    char *args[64];

    while (1) {
        // 1. Display Prompt
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);
        fflush(stdout);

        // 2. Read Input
        if (fgets(input, MAX_LINE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0; // Remove newline

        // 3. Parse Input 
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL) {
            args[++i] = strtok(NULL, " ");
        }

        if (args[0] == NULL) continue; // Empty input

        // 4. Handle Built-ins
        if (strcmp(args[0], "exit") == 0) break;
        if (strcmp(args[0], "cd") == 0) {
            if (chdir(args[1]) != 0) perror("cd failed");
            continue;
        }

        // 5. Fork and Exec
        pid_t pid = fork();
        if (pid == 0) {
            // Child: Handle redirection here with dup2() then:
            execvp(args[0], args);
            perror("Execution failed"); // Only prints if execvp fails
            exit(1);
        } else {
            // Parent
            wait(NULL);
        }
    }
    return 0;
}