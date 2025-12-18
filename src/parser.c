#include "parser.h"

char **parse_args(char *line, int *out_argc, struct shell *sh) {
  char *line_copy = (char *)malloc(strlen(line) + 1);
  strcpy(line_copy, line);

  int argc = 0;
  char **args = (char **)malloc(ARGS_MAX_COUNT * sizeof(char *));
  char *token = strtok(line_copy, " ");

  while (token != NULL) {
    // expand ~ to $HOME
    if (token[0] == '~') {
      char *home = sh->home_dir;

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

  free(line_copy);

  if (argc > 0) {
    int arg_len = strlen(args[argc - 1]);

    if (args[argc - 1][arg_len - 1] == '\n')
      args[argc - 1][arg_len - 1] = '\0'; // replace \n with \0
  }

  args[argc] = NULL;

  *out_argc = argc;
  return args;
}
