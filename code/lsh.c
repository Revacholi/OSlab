/*
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file(s)
 * you will need to modify the CMakeLists.txt to compile
 * your additional file(s).
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Using assert statements in your code is a great way to catch errors early and make debugging easier.
 * Think of them as mini self-checks that ensure your program behaves as expected.
 * By setting up these guardrails, you're creating a more robust and maintainable solution.
 * So go ahead, sprinkle some asserts in your code; they're your friends in disguise!
 *
 * All the best!
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

// The <unistd.h> header is your gateway to the OS's process management facilities.
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#include "parse.h"

#define READ_END 0
#define WRITE_END 1

static void print_cmd(Command *cmd);
static void print_pgm(Pgm *p);
void stripwhite(char *);
static void exec_cmd(Command *cmd);
static void intHandler();
static int should_exit = 0;
static void chldHandler();

static pid_t foreground = 0; // PID of current foreground process

int main(void)
{
  char buff[256];
  for (;;)
  {
    foreground = 0;

    char *line;

    // Potentially allows for an error if buff cannot fit cwd + "> "
    line = readline(strcat(getcwd(buff, 254), "> "));

    // Handle Ctrl-D, readline returns NULL when EOF is 
    if (line == NULL)  
    {
      printf("Ctrl-D (EOF) detected, exit!\n");
      break; // Exit 
    }

    // Handle Ctrl-C, terminate the current foreground process    
    signal(SIGINT, intHandler);
    // Handle Zombies
    signal(SIGCHLD, chldHandler);

    // Remove leading and trailing whitespace from the line
    stripwhite(line);

    // If stripped line not blank
    if (*line)
    {
      add_history(line);

      Command cmd;
      if (parse(line, &cmd) == 1)
      {
        print_cmd(&cmd);
        exec_cmd(&cmd);
      }
      else
      {
        printf("Parse ERROR\n");
      }
    }

    // Clear memory
    free(line);

    if (should_exit)
    {
      break;
    }
  }

  return 0;
}


/*
 * Execute after parse
 */
static void exec_cmd(Command *cmd)
{
  // char *path = getenv("PATH");
  // printf("PATH: %s\n\n", path);


  if (cmd->pgm == NULL) {
    return;
  }

  Pgm *pgm = cmd->pgm;
  
  // Get the command name and arguments
  char **args = pgm->pgmlist;
  if (args == NULL || args[0] == NULL) {
    return;
  }
  
  // check if there are multiple commands (then we have pipes)
  Pgm *ppgm = pgm;
  while (ppgm->next) {
    ppgm = pgm->next;
    // Create array of two pipe "ends". We will write and read to each end.
    int pipe_descriptors[2];
    // Try to create a pipe. If it fails print pipe error and return
    if(pipe(pipe_descriptors) < 0) {
      printf("Pipe error!");
      return;
    }

    pid_t pid_pipe = fork();

    if (pid_pipe > 0) {   // Parent process closes read end
      close(pipe_descriptors[READ_END]);
      
      // set output to pipe
      dup2(pipe_descriptors[WRITE_END], STDOUT_FILENO);  
      close(pipe_descriptors[WRITE_END]);
    }
    else if (pid_pipe == 0) {  // Child process closes write end
      close(pipe_descriptors[WRITE_END]);
      
      // set output to pipe
      dup2(pipe_descriptors[READ_END], STDIN_FILENO);  
      close(pipe_descriptors[READ_END]);
    }
  }


  if (!strcmp("cd", args[0]))
  {
    char *dir;
    if (args[1] != NULL)
    {
      dir = args[1];
    }
    else
    {
      dir = getenv("HOME");
    }
    if (chdir(dir))
    {
      perror(args[1]);
    }
    return;
  } else if (!strcmp("exit", args[0]))
  {
    should_exit = 1;
    return;
  }
  

  pid_t pid = fork();
  
  if (pid == 0) {
    // Child process

    // Handle input redirection
    if (cmd -> rstdin) {
      int fd;
      if ((fd = open(cmd->rstdin, O_RDONLY)) == -1){   // if open fail or no target file
        perror("open rstdin");
        exit(1);
      }
      else {
        dup2(fd, STDIN_FILENO);  // duplicate old fd into stdin
        close(fd);
      }
    }

    // Handle output redirection
    if (cmd -> rstdout) {
      int fd;
      if ((fd = open(cmd->rstdout, O_WRONLY | O_CREAT)) == -1){ // if open fail or no target file
        perror("open rstdout");
        exit(1);
      }
      else {
        dup2(fd, STDOUT_FILENO);  // duplicate old fd into stdout
        close(fd);
      }
    }
    
    execvp(args[0], args);     // Child process execute the cmd
    // execvp fail
    perror(args[0]);
    exit(1);
  }
  else if (pid > 0) {  // Parent process wait for child 
    if (cmd->background) {
      printf("Started background process PID: %d\n", pid);
      return; 
    }
    else {
      foreground = pid;  // Set the foreground process PID
      printf("Waiting for process PID: %d\n", pid);
      int status;
      waitpid(pid, &status, 0);
      foreground = 0; // Reset foreground PID
    }

  }
  else {
    perror("fork");
  }
}



/*
 * Handle Ctrl-C signal (SIGINT)
 */
static void intHandler() { 
  printf("\n");
  return;
} 


/*
 * Handle Zombies (SIGCHLD)
 */
static void chldHandler() {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (pid != foreground) {
      printf("Zombie %d Killed\n", pid);
    }
  }

}

/*
 * Print a Command structure as returned by parse on stdout.
 *
 * Helper function, no need to change. Might be useful to study as inspiration.
 */
static void print_cmd(Command *cmd_list)
{
  printf("------------------------------\n");
  printf("Parse OK\n");
  printf("stdin:      %s\n", cmd_list->rstdin ? cmd_list->rstdin : "<none>");
  printf("stdout:     %s\n", cmd_list->rstdout ? cmd_list->rstdout : "<none>");
  printf("background: %s\n", cmd_list->background ? "true" : "false");
  printf("Pgms:\n");
  print_pgm(cmd_list->pgm);
  printf("------------------------------\n");
  printf("Exec result:\n");
}

/* Print a (linked) list of Pgm:s.
 *
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
static void print_pgm(Pgm *p)
{
  if (p == NULL)
  {
    return;
  }
  else
  {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    print_pgm(p->next);
    printf("            * [ ");
    while (*pl)
    {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}


/* Strip whitespace from the start and end of a string.
 *
 * Helper function, no need to change.
 */
void stripwhite(char *string)
{
  size_t i = 0;

  while (isspace(string[i]))
  {
    i++;
  }

  if (i)
  {
    memmove(string, string + i, strlen(string + i) + 1);
  }

  i = strlen(string) - 1;
  while (i > 0 && isspace(string[i]))
  {
    i--;
  }

  string[++i] = '\0';
}
