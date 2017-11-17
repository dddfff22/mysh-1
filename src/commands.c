#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "commands.h"
#include "built_in.h"

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

int back=0;

int setback(int i){
  if(i==0){
    back=0;
  }else if(i==1){
    back=1;
  }
  return back;
}

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
 
  if (n_commands == 1) {
    struct single_command* com = (*commands);

    assert(com->argc != 0);

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    }else if (validate_dir(com->argc,com->argv)){
      int pid;
      int status;
      int getpid;
      pid=fork();
      if(pid==0){
          execv(com->argv[0],com->argv);
          exit(1);
      }else{
        getpid=wait(status);
        return 0;
      }
    }else{
     
      fprintf(stderr, "%s: command not found\n", com->argv[0]);
      return -1;
      
    }
  }else if(n_commands>1){

    struct single_command *com = (*commands);
    struct single_command *com2 = (*commands)+1;
    assert(com->argc != 0);
    int built_in_pos = is_built_in_command(com->argv[0]);
    int p[2];
    pipe(p); // Creates a pipe with file descriptors Eg. input = 3 and output = 4 (Since, 0,1 and 2 are not available)

    if (fork() == 0) {
    // Child process
        close(0); // Release fd no - 0
        close(p[0]); // Close pipe fds since useful one is duplicated
        close(p[1]);
        dup(p[0]); // Create duplicate of fd - 3 (pipe read end) with fd 0.
        execv(com2->argv[0],com2->argv);
    } else {
        //Parent process
        close(1); // Release fd no - 1
        close(p[0]); // Close pipe fds since useful one is duplicated
        close(p[1]);
        dup2(p[1]); // Create duplicate of fd - 4 (pipe write end) with fd 1.
        execv(com->argv[0],com->argv);
    }



   
  
  }

 
  return 0;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
