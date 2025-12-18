#pragma once

#define ARGS_MAX_COUNT 32

#include <stdlib.h>

#include "shell.h"

char **parse_args(char *line, int *out_argc, struct shell *sh);
