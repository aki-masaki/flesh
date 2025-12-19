#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "defs.h"
#include "utils.h"

fh_cspace create_cspace(fh_inst *inst) {
  fh_cspace cspace = {};
  cspace.command = (char *)malloc(MAX_COMMAND_LEN);

  inst->cspaces[inst->cspaces_cnt++] = cspace;

  return cspace;
}

void exec_cspace(fh_cspace *cspace) {
  if (strcmp(cspace->command, "clear") == 0) {
    if (cspace->output != NULL) {
      free(cspace->output);

      cspace->output = NULL;
    }

    return;
  }

  int pipes[2];
  pipe(pipes);

  pid_t pid = fork();

  char *output = (char *)malloc(MAX_OUTPUT_LEN);

  if (pid == 0) {
    dup2(pipes[1], STDOUT_FILENO);

    close(pipes[0]);
    close(pipes[1]);

    // TODO: parse args from command and pass them here
    execvp(cspace->command, NULL);

    exit(0);
  } else {
    close(pipes[1]);

    int nbytes = read(pipes[0], output, MAX_OUTPUT_LEN);
    cspace->output = output;
    cspace->output_len = nbytes;
  }
}
