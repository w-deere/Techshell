// Name: William Deere
// Description: A simple Bash-like shell in C. It supports standard external 
//              commands, < and > redirection, and the built-in cd
//              and exit commands.

// first our headers
#include <stdio.h>      // for printf, fgets, perror
#include <stdlib.h>     // for malloc, calloc, free, exit
#include <string.h>     // for strtok, strcmp, strdup, strcspn
#include <sys/wait.h>   // for wait()
#include <unistd.h>     // for fork, execvp, getcwd, chdir, dup2
#include <fcntl.h>      // for open(), O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC
#include <errno.h>      // for errno and strerror()     

// hint 13 says to declare these so here they are 
#define MAX_ARGS 64
#define MAX_INPUT 1024

// looked up how to use a struct in C and created it
struct ShellCommand {
    char* args[MAX_ARGS]; // array to hold the command and its arguments
    char* inputFile;      // pointer to the filename for '<' redirection
    char* outputFile;     // pointer to the filename for '>' redirection
};

// function prototypes 
void displayPrompt();
char* getInput();
struct ShellCommand parseInput(char* input);
void executeCommand(struct ShellCommand command);

// main, using template + extra added reccomendations
int main(){
    char* input;
    struct ShellCommand command;
    
    for (;;) {
        displayPrompt();
        input = getInput();
        command = parseInput(input);
        executeCommand(command);

        // free memory so we don't leak
        if (input != NULL){
			free(input);
		}
        if (command.inputFile != NULL){
			free(command.inputFile);
		}
        if (command.outputFile != NULL){
			free(command.outputFile);
		}
        for (int i = 0; command.args[i] != NULL; i++) {
            free(command.args[i]);
        }
    }
    
    exit(0);
}

void displayPrompt() {
    char cwd[MAX_INPUT]; // buffer to hold path
    
    // getcwd fills the array with the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // print the directory in example format
        printf("%s$ ", cwd); 
    } else {
        // if fails for some reason, just a safety check, prints error msg
        perror("techshell error"); 
    }
    // force prompt to display immediately
    fflush(stdout); 
}

char* getInput() {
    // allocate memory for the input string
    char* buffer = (char*)malloc(MAX_INPUT * sizeof(char));
    
    // safety check
    if (buffer == NULL) {
        perror("techshell error: malloc failed");
        exit(1);
    }

    // read user input
    if (fgets(buffer, MAX_INPUT, stdin) == NULL) {
        // handle ctrl+d (eof)
        printf("\n");   
        free(buffer);   
        exit(0);        
    }

    return buffer;
}


/* * AI ASSISTANCE CITATION:
 * Prompt used (gemini): "I am writing a shell in C. My current parse function uses strtok, 
 * but it fails when a file path contains spaces inside quotes, like 'cd "My Documents"'. 
 * Can you show me how to manually parse a string in C using a loop to handle 
 * double quotes and escaped characters without using strtok?"
 *
 * The AI suggested a state-machine approach using flags 
 * (in_quotes and escaped) to track whether a space should be treated as a 
 * delimiter or a literal character. And so i did it, integrating with my 
 * existing ShellCommand struct.
 */
struct ShellCommand parseInput(char* input) {
    struct ShellCommand cmd;
    
    // initialize the struct pointers
    cmd.inputFile = NULL;
    cmd.outputFile = NULL;
    int argIndex = 0;

    char token[MAX_INPUT];
    int t_idx = 0;
    int in_quotes = 0;
    int escaped = 0;
    int i = 0;

    int redirect_in_next = 0;
    int redirect_out_next = 0;

    // remove trailing newline from fgets
    input[strcspn(input, "\n")] = 0;

    // loop through chars to handle quotes and escaped spaces manually
    while (input[i] != '\0') {
        if (escaped) {
            token[t_idx++] = input[i];
            escaped = 0; 
        } else if (input[i] == '\\') {
            escaped = 1; 
        } else if (input[i] == '"') {
            in_quotes = !in_quotes; 
        } else if (input[i] == ' ' && !in_quotes) {
            // finished a word
            if (t_idx > 0) {
                token[t_idx] = '\0';
                
                if (redirect_in_next) {
                    cmd.inputFile = strdup(token); 
                    redirect_in_next = 0;
                } else if (redirect_out_next) {
                    cmd.outputFile = strdup(token);
                    redirect_out_next = 0;
                } else if (strcmp(token, "<") == 0) {
                    redirect_in_next = 1;
                } else if (strcmp(token, ">") == 0) {
                    redirect_out_next = 1;
                } else {
                    // normal arg
                    cmd.args[argIndex] = strdup(token);
                    argIndex++;
                }
                t_idx = 0; 
            }
        } else {
            // normal char
            token[t_idx++] = input[i];
        }
        i++;
    }
    
    // grab the last token when loop ends
    if (t_idx > 0) {
        token[t_idx] = '\0';
        if (redirect_in_next) {
            cmd.inputFile = strdup(token);
        } else if (redirect_out_next) {
            cmd.outputFile = strdup(token);
        } else if (strcmp(token, "<") != 0 && strcmp(token, ">") != 0) {
            cmd.args[argIndex] = strdup(token);
            argIndex++;
        }
    }

    // execvp needs a null pointer at the very end
    cmd.args[argIndex] = NULL; 
    return cmd;
}

void executeCommand(struct ShellCommand command) {
    // do nothing if they just hit enter
    if (command.args[0] == NULL) {
        return;
    }

    // built in exit command
    if (strcmp(command.args[0], "exit") == 0) {
        exit(0);
    }

    // built in cd command
    if (strcmp(command.args[0], "cd") == 0) {
        if (command.args[1] != NULL) {
            if (chdir(command.args[1]) != 0) {
                printf("Error %d (%s)\n", errno, strerror(errno));
            }
        } else {
            // go home if no args
            char* home = getenv("HOME");
            if (home != NULL && chdir(home) != 0) {
                printf("Error %d (%s)\n", errno, strerror(errno));
            }
        }
        return; 
    }

    // fork a child process for external commands
    pid_t pid = fork();

    if (pid < 0) {
        perror("techshell error: fork failed");
    } else if (pid == 0) {
        // child process handles the actual command

        // handle input redirection
        if (command.inputFile != NULL) {
            int fd_in = open(command.inputFile, O_RDONLY);
            if (fd_in < 0) {
                printf("Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        // handle output redirection
        if (command.outputFile != NULL) {
            int fd_out = open(command.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                printf("Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // run the command
        if (execvp(command.args[0], command.args) == -1) {
            printf("Error %d (%s)\n", errno, strerror(errno));
            exit(1); 
        }
    } else {
        // parent process waits for child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}