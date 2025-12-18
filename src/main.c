#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "shell.h"

int main() {
  struct shell sh = {};
  sh_init(&sh);

  while (1) {
    char *line = NULL;
    size_t size = 0;

    char **args = NULL;
    int argc = 0;

    printf("[%s] -> ", sh.cr_dir);
    getline(&line, &size, stdin);

    args = parse_args(line, &argc, &sh);

    if (strcmp(args[0], "cd") == 0) {
      if (args[1] == NULL) {
        printf("cd where?\n");
      } else {
        sh_chdir(&sh, args[1]);
      }
    } else if (strcmp(args[0], "exit") == 0) {
      exit(0);
    } else {
      sh_execute(&sh, args);
    }

    free(line);
    free(args);
  }

  return 0;
}
