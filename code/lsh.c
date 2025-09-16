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

static void intHandler(int dummy);
static void chldHandler();

static void exec_cmd(Command *cmd);
static void exec_single_cmd(Command *cmd);
static void exec_pipeline(Command *cmd, int cmd_count);

static int should_exit = 0; // Flag to indicate if shell should exit
static pid_t foreground = 0; // PID of current foreground process
static pid_t foreground_pgid = 0; // Process group ID of current foreground job

int main(void)
{
  char buff[256];
  for (;;)
  {
    foreground = 0;
    foreground_pgid = 0; // Reset foreground process group ID

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

  // check for built-in commands (only if single command without pipes)
  if (pgm->next == NULL) {
    char **args = pgm->pgmlist;
    if (args == NULL || args[0] == NULL) {
      return;
    }
    
    // handle 'cd' and 'exit' built-in commands
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
    } 
    else if (!strcmp("exit", args[0]))
    {
      should_exit = 1;
      return;
    }
  }

  // Get command count
  int cmd_count = 0;
  while (pgm != NULL) {
    cmd_count++;
    pgm = pgm->next;
  }

  // if only single command, execute directly
  if (cmd_count == 1) {
    exec_single_cmd(cmd);
    return;
  }

  // Handle pipeline
  exec_pipeline(cmd, cmd_count);
}


/*
 * execute a single command (no pipes)
 */
static void exec_single_cmd(Command *cmd)
{
  Pgm *pgm = cmd->pgm;
  char **args = pgm->pgmlist;
  
  pid_t pid = fork();
  
  if (pid == 0) {
    // Child process, handle redirections and execute command
    
    // Create new process group for this job
    // This allows us to control jobs with signals properly
    setpgid(0, 0); // Set child's process group ID to its own PID
    
    if (cmd->rstdin) {
      int fd = open(cmd->rstdin, O_RDONLY);
      if (fd == -1) {
        perror("open rstdin");
        exit(1);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
    }

    if (cmd->rstdout) {
      int fd = open(cmd->rstdout, O_WRONLY | O_CREAT | O_TRUNC, 0744);
      if (fd == -1) {
        perror("open rstdout");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }
    
    execvp(args[0], args);
    perror(args[0]);
    exit(1);
  }
  else if (pid > 0) {
    // Parent process
    // Set the process group ID for proper job control
    setpgid(pid, pid); // Ensure the child is in its own process group

    if (!cmd->background) {
      foreground_pgid = pid; // Store the process group ID for foreground jobs
    }
    
    if (cmd->background) {
      printf("Started background process PID: %d\n", pid);
      return; 
    } else {
      foreground = pid;
      int status;
      waitpid(pid, &status, 0);
      foreground = 0;
      foreground_pgid = 0; // Reset after job completion
    }
  }
  else {
    perror("fork");
  }
}



/*
 * execute a pipeline of commands
 */
static void exec_pipeline(Command *cmd, int cmd_count)
{
  // create pipe array, we need cmd_count - 1 pipes for cmd_count commands
  int pipes[cmd_count-1][2];
  pid_t pids[cmd_count];
  pid_t pipeline_pgid = 0; // Process group ID for the entire pipeline
  
  // create all pipes we need
  for (int i = 0; i < cmd_count - 1; i++) {
    if (pipe(pipes[i]) == -1) {
      perror("pipe");
      return;
    }
  }

  // convert linked list of Pgm to array for easier access, because list is in reverse order
  Pgm *pgm_array[cmd_count];
  Pgm *p = cmd->pgm;
  for (int i = cmd_count - 1; i >= 0; i--) {
    pgm_array[i] = p;
    p = p->next;
  }

  // create processes for each command
  for (int i = 0; i < cmd_count; i++) {
    pids[i] = fork();
    
    if (pids[i] == 0) {
      // child process
      
      // Set up process group for pipeline job control
      // All processes in the pipeline should be in the same process group
      if (i == 0) {
        // First process creates the process group
        setpgid(0, 0); // Create new process group with this process as leader
      } else {
        // Subsequent processes join the first process's group
        setpgid(0, pids[0]); // Join the process group of the first process
      }
      
      // 设置管道连接
      if (i > 0) {
        // Not the first command, read from previous pipe
        dup2(pipes[i-1][READ_END], STDIN_FILENO);
      }
      
      if (i < cmd_count - 1) {
        // Not the last command, write to next pipe
        dup2(pipes[i][WRITE_END], STDOUT_FILENO);
      }
      
      // Close all pipe fds in child
      for (int j = 0; j < cmd_count - 1; j++) {
        close(pipes[j][READ_END]);
        close(pipes[j][WRITE_END]);
      }
      
      // handle input redirection for the first command
      if (i == 0 && cmd->rstdin) {
        int fd = open(cmd->rstdin, O_RDONLY);
        if (fd == -1) {
          perror("open rstdin");
          exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
      }
      
      // handle output redirection for the last command
      if (i == cmd_count - 1 && cmd->rstdout) {
        int fd = open(cmd->rstdout, O_WRONLY | O_CREAT | O_TRUNC, 0744);
        if (fd == -1) {
          perror("open rstdout");
          exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }
      
      // execute command
      char **args = pgm_array[i]->pgmlist;
      execvp(args[0], args);
      perror(args[0]);
      exit(1);
    }
    else if (pids[i] == -1) {
      perror("fork");
      return;
    }
    else {
      // Parent process: ensure all children are in the same process group
      if (i == 0) {
        // Set up the process group using the first process's PID
        pipeline_pgid = pids[0];
        setpgid(pids[0], pids[0]); // Ensure first process is group leader
      } else {
        // Add subsequent processes to the same group
        setpgid(pids[i], pipeline_pgid); // Join the established process group
      }
    }
  }

  // close all pipe fds in parent
  for (int i = 0; i < cmd_count - 1; i++) {
    close(pipes[i][READ_END]);
    close(pipes[i][WRITE_END]);
  }

  // wait for all child processes
  if (cmd->background) {
    printf("Started background pipeline with %d processes\n", cmd_count);
    return;
  } else {
    // Store the pipeline's process group ID for signal handling (foreground only)
    foreground_pgid = pipeline_pgid;
    foreground = pids[cmd_count - 1]; // Keep compatibility with single process tracking

    for (int i = 0; i < cmd_count; i++) {
      int status;
      waitpid(pids[i], &status, 0);
    }
    foreground = 0;
    foreground_pgid = 0; // Reset after pipeline completion
  }
}



/*
 * Handle Ctrl-C signal (SIGINT)
 * With process groups, we can send signal to entire job at once
 */
static void intHandler(int dummy) { 
  if (foreground_pgid > 0) {
    // Send SIGTERM to the entire foreground process group
    // Using negative PID sends signal to process group
    kill(-foreground_pgid, SIGTERM);
    printf("\nTerminated foreground job (process group %d)\n", foreground_pgid);
  } else {
    printf("\n");
  }
  return;
} 


/*
 * Handle Zombies (SIGCHLD)
 * Process group-aware zombie handling
 */
static void chldHandler() {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    // Check if this process belongs to the current foreground job
    pid_t pid_pgid = getpgid(pid);
    
    if (pid_pgid != foreground_pgid) {
      // This is a background process that finished
      printf("Background process %d finished\n", pid);
    }
    // For foreground processes, we don't print anything as the parent
    // will handle them in the wait loop
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