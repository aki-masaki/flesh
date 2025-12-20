#include <pty.h>
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

void exec_cspace(fh_cspace *cspace, char **args) {
  if (strcmp(cspace->command, "clear") == 0) {
    if (cspace->output != NULL) {
      free(cspace->output);

      cspace->output = NULL;
    }

    return;
  } else if (strcmp(cspace->command, "exit") == 0) {
    endwin();

    exit(0);
  }

  char slave_name[64];
  int master;
  fd_set inFds;

  ssize_t numRead;

  if (cspace->output != NULL) {
    free(cspace->output);
    cspace->output = NULL;
  }

  char *output = (char *)malloc(MAX_OUTPUT_LEN);
  cspace->output = output;

  pid_t pid = forkpty(&master, slave_name, NULL, NULL);

  if (pid == 0) {
    char columns[8];
    snprintf(columns, 8, "%d", COLS);

    setenv("COLUMNS", columns, 1);

    if (execvp(args[0], args) == -1) {
      printf(":(can't execute '%s'", args[0]);

      exit(1);

      return;
    }
  } else {
    // read(master, output, MAX_OUTPUT_LEN);

    for (;;) {
      FD_ZERO(&inFds);
      FD_SET(STDIN_FILENO, &inFds);
      FD_SET(master, &inFds);

      if (select(master + 1, &inFds, NULL, NULL, NULL) == -1)
        break;

      /* if (FD_ISSET(STDIN_FILENO, &inFds)) { /* stdin --> pty
      ssize_t command_len = strlen(cspace->command);

      if (write(master, cspace->command, command_len) != command_len)
        break;
    }
    */

      if (FD_ISSET(master, &inFds)) {
        numRead = read(master, output, MAX_OUTPUT_LEN - 1);

        if (numRead <= 0)
          break;

        output[numRead - 1] = '\0';
      }
    }
  }
}

char **get_command_args(char *command, int *out_argc) {
  char *command_copy = (char *)malloc(strlen(command) + 1);
  strcpy(command_copy, command);

  int argc = 0;

  char **args = (char **)malloc(MAX_ARGS_COUNT * sizeof(char *));

  char *token = strtok(command_copy, " ");

  while (token != NULL) {
    // expand ~ to $HOME
    if (token[0] == '~') {
      char *home = getenv("HOME");
      int home_len = strlen(home);
      int token_len = strlen(token);

      if (token_len == 2 && token[1] == '\n') {
        token_len = 1;
        token[1] = '\0';
      }

      // its just ~
      if (token_len == 1) {
        args[argc] = malloc(home_len + 1);
        memcpy(args[argc], home, home_len);
        args[argc][home_len] = '\0';
      } else {
        args[argc] = malloc(
            home_len + token_len); // we don't need to add one since token_len
        // has one extra char that we won't use: ~

        snprintf(args[argc], home_len + token_len, "%s%s", home,
                 token + 1); // token + 1 so we skip ~
      }
      argc++;
    } else {
      args[argc] = (char *)malloc(strlen(token) + 1);
      strcpy(args[argc], token);
      argc++;
    }

    token = strtok(NULL, " ");
  }

  free(command_copy);

  args[argc] = NULL;
  *out_argc = argc;

  return args;
}
