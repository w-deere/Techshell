# Techshell
Members: William Deere(w-deere) and Drew Sylve(DJ-Sylve)


# Responsibilites
We have each created out techshells based on the project instructions, so you may inspect both. 
However, for trying the code out we decided you use the file indicated:

DEEREtechshell.c - use this file for demo of code

SYLVEtechshell.c


# Project Description
TechShell is a custom, simplistic Bash-like shell written in C for our CSC 222 Systems Programming course. The program repeatedly prompts the user for input, dynamically displaying the current working directory. It parses the user's command line string—carefully handling edge cases like escaped spaces and quoted strings—and executes external system commands by spawning child processes. It also directly supports standard input (<) and output (>) redirection, as well as the built-in cd and exit commands directly within the parent process.

## Features & Working Components
Dynamic Prompt: Displays the current working directory flawlessly.

External Command Execution: Successfully forks and uses execvp to run standard binaries (e.g., ls, ps).

Built-in Commands: The cd (including cd with no arguments to return to HOME) and exit commands work as expected without forking.

I/O Redirection: Standard input (<) and standard output (>) redirection work seamlessly. Output redirection creates new files or truncates existing ones.

Advanced Parsing: The custom parser correctly interprets spaces inside quotes (e.g., "My Documents") and escaped spaces (e.g., \ ) without breaking the command arguments.

Error Handling: System errors are captured using <errno.h> and printed in the exact required format -> Error # (MESSAGE).

Memory Management: Dynamically allocated memory for user input and token strings is properly freed during the shell's execution cycle.



# Instructions: How to Clone, Compile, and Run

1. Clone the Repository

To get a copy of this project on your local machine, run the following command in your terminal: 

git clone https://github.com/w-deere/Techshell.git

cd Techshell


2. Compile and Run the Program

Use the GCC compiler to build the executable. The following command compiles the techshell.c source file into an executable named techshell. and the next runs it:

```sh
gcc techshell.c -o techshell

./techshell
```

To exit the shell simply type exit or ctrl+d.




**This project was done in our second year of college, during our Systems Programming class.**
