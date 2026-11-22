#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s $  ", cwd);
    else
        printf("$  ");
    fflush(stdout);
}

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    printf("Welcome to the terminal.\n\n");

    while (1) {
        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break; // Ctrl+D
        }

        // remove trailing newline
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0') continue;

        // tokenize: split by spaces
        int i = 0;
        char *tok = strtok(line, " \t");
        while (tok != NULL && i < MAX_ARGS - 1) {
            args[i++] = tok;
            tok = strtok(NULL, " \t");
        }
        args[i] = NULL;
        if (i == 0) continue;

        // jonbk was here
        // --- BUILTINS ---

        if (strcmp(args[0], "exit") == 0) {
            printf("exiting the terminal...\n");
            return 0;
        }
        else if (strcmp(args[0], "clear") == 0) {
            printf("\033[2J\033[1;1H");
            continue;
        }
        else if (strcmp(args[0], "help") == 0) {
            printf("Available commands:\n");
            printf("  exit - Exit The Shell\n");
            printf("  clear - Clear The Shell\n");
            printf("  cd <dir> - Change Directory\n");
            printf("  pwd - Show Current Directory\n");
            printf("  mkdir <dir> - Create A Directory\n");
            printf("  rmdir <dir> - Remove A Directory\n");
            printf("  touch <file> - Create A File\n");
            printf("  rmfile <file> - Remove A File\n");
            printf("  echo <text> - Echo to the Terminal\n");
            printf("  help - Show This Menu\n");
            printf("  + any other FreeBSD command (ls, git, cc, figlet, etc.)\n");
            continue;
        }
        else if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                printf("cd: missing directory. Usage: cd <path>\n");
            } else if (chdir(args[1]) == 0) {
                printf("Inside Directory: %s\n", args[1]);
            } else {
                perror("cd");
            }
            continue;
        }
        else if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s\n", cwd);
            else
                perror("pwd");
            continue;
        }
        else if (strcmp(args[0], "mkdir") == 0) {
            if (args[1] == NULL) {
                printf("mkdir: missing directory name\n");
            } else if (mkdir(args[1], 0777) == 0) {
                printf("Directory Created: %s\n", args[1]);
            } else {
                perror("mkdir");
            }
            continue;
        }
        else if (strcmp(args[0], "rmdir") == 0) {
            if (args[1] == NULL) {
                printf("rmdir: missing directory name\n");
                continue;
            }
            printf("Are you sure you want to remove the directory? [Y/n]: ");
            fflush(stdout);
            char ans[16];
            if (fgets(ans, sizeof(ans), stdin) == NULL) continue;
            if (ans[0] == 'Y' || ans[0] == 'y') {
                if (rmdir(args[1]) == 0)
                    printf("Directory Removed: %s\n", args[1]);
                else
                    perror("rmdir");
            } else {
                printf("rmdir: operation cancelled\n");
            }
            continue;
        }
        else if (strcmp(args[0], "touch") == 0) {
            if (args[1] == NULL) {
                printf("touch: missing file name\n");
                continue;
            }
            // "a" = create if not exists, DO NOT truncate if exists
            FILE *fp = fopen(args[1], "a");
            if (fp != NULL) {
                fclose(fp);
                printf("File Created: %s\n", args[1]);
            } else {
                perror("touch");
            }
            continue;
        }
        else if (strcmp(args[0], "rmfile") == 0) {
            if (args[1] == NULL) {
                printf("rmfile: missing file name\n");
                continue;
            }
            printf("Are you sure you want to remove the file? [Y/n]: ");
            fflush(stdout);
            char ans[16];
            if (fgets(ans, sizeof(ans), stdin) == NULL) continue;
            if (ans[0] == 'Y' || ans[0] == 'y') {
                if (remove(args[1]) == 0)
                    printf("File Removed: %s\n", args[1]);
                else
                    perror("rmfile");
            } else {
                printf("rmfile: operation cancelled\n");
            }
            continue;
        }
        else if (strcmp(args[0], "echo") == 0) {
            for (int j = 1; j < i; j++) {
                printf("%s", args[j]);
                if (j < i - 1) printf(" ");
            }
            printf("\n");
            continue;
        }

        // --- NOT A BUILTIN: RUN AS EXTERNAL COMMAND ---
        pid_t pid = fork();
        if (pid == 0) {
            // child
            execvp(args[0], args);
            // if we get here, it failed
            printf("es: %s: command not found.\n", args[0]);
            exit(1);
        } else if (pid > 0) {
            // parent waits
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
        }
    }
    return 0;
}
