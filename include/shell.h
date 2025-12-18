#pragma once

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

struct shell {
  // current dir
  char *cr_dir;
  char *home_dir;
};

void sh_init(struct shell *sh);
void sh_chdir(struct shell *sh, char *dir);
void sh_execute(struct shell *sh, char **args);

