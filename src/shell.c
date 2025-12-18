#include <stdlib.h>

#include "shell.h"

void sh_init(struct shell *sh) {
  sh->home_dir = getenv("HOME");

  if (sh->home_dir == NULL) {
    printf("$HOME env variable not set");

    exit(0);
  }

  sh_chdir(sh, sh->home_dir);
}

void sh_chdir(struct shell *sh, char *dir) {
  if (chdir(dir) != 0) {
    perror("flesh");

    return;
  }

  size_t len = strlen(dir);
  sh->cr_dir = (char *)malloc(len + 1);
  strcpy(sh->cr_dir, dir);

  sh->cr_dir[len] = '\0';
}

void sh_execute(struct shell *sh, char **args) {
  pid_t pid = fork();
  int status;

  if (pid == -1) {
    perror("flesh");
  } else if (pid == 0) {
    // child process
    if (execvp(args[0], args) == -1) {
      perror("flesh");

      exit(0);
    }
  } else {
    // parent process
    waitpid(pid, &status, WUNTRACED);

    while (status != WIFEXITED(status) && status != WIFSIGNALED(status)) {
      waitpid(pid, &status, WUNTRACED);
    }
  }
}
