#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGS_MAX_COUNT 32

char **get_args(char *line, int *out_argc) {
  int argc = 0;
  char **args = (char **)malloc(ARGS_MAX_COUNT * sizeof(char *));
  char *token = strtok(line, " ");

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
      args[argc++] = token;
    }

    token = strtok(NULL, " ");
  }

  if (argc > 0) {
    int arg_len = strlen(args[argc - 1]);

    if (args[argc - 1][arg_len - 1] == '\n')
      args[argc - 1][arg_len - 1] = '\0'; // replace \n with \0
  }

  args[argc] = NULL;

  *out_argc = argc;
  return args;
}

int execute(char **args) {
  pid_t pid = fork();
  int status;

  if (pid == -1) {
    return -1;
  } else if (pid == 0) {
    // child process
    if (execvp(args[0], args) == -1) {
      exit(0);
    }
  } else {
    // parent process
    waitpid(pid, &status, WUNTRACED);

    while (status != WIFEXITED(status) && status != WIFSIGNALED(status)) {
      waitpid(pid, &status, WUNTRACED);
    }
  }

  return 0;
}

int main() {
  char *line = NULL;
  size_t size = 0;

  int argc = 0;
  char **args = NULL;

  int status = 0;

  while (1) {
    free(line);
    line = NULL;

    printf("-> ");
    getline(&line, &size, stdin);

    argc = 0;
    free(args);
    args = NULL;

    args = get_args(line, &argc);

    if (strcmp(args[0], "cd") == 0) {
      if (args[1] == NULL) {
        printf("cd where?\n");
      } else {
        if (chdir(args[1]) != 0)
          perror("flesh");
      }
    } else if (strcmp(args[0], "exit") == 0) {
      exit(0);
    } else {
      status = execute(args);

      if (status == -1) {
        perror("flesh");
      }
    }
  }

  return 0;
}
