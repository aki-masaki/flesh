#include <stdlib.h>

#include "defs.h"
#include "utils.h"

fh_cspace create_cspace(fh_inst *inst) {
  fh_cspace cspace = {};
  cspace.command = (char *)malloc(MAX_COMMAND_LEN);

  inst->cspaces[inst->cspaces_cnt++] = cspace;

  return cspace;
}
