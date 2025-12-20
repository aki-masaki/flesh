#pragma once

#include "structs.h"

fh_cspace create_cspace(fh_inst *inst);

char **get_command_args(char *command, int *out_argc);
void exec_cspace(fh_cspace *cspace, char **args);
