#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>

#include "defs.h"
#include "structs.h"
#include "ui.h"
#include "utils.h"

int main() {
  fh_inst instance = {};

  fh_cspace cspace = create_cspace(&instance);

  fh_ui ui;
  init_ui(&ui);

  draw_cspace(&ui, &cspace, 0);
  refresh();

  int c;
  int i = 0;
  while (c = getch()) {
    if (isprint(c) && i < MAX_COMMAND_LEN) {
      cspace.command[i++] = c;

      ui.dirty = 1;
    } else if (c == KEY_BACKSPACE && i > 0) {
      cspace.command[--i] = '\0';

      ui.dirty = 1;
    } else if (c == 10) { // Enter key
      // https://stackoverflow.com/questions/11067800/ncurses-key-enter-is-fail

      int argc = 0;
      char **args = get_command_args(cspace.command, &argc);
      exec_cspace(&cspace, args);

      ui.dirty = 1;
    }

    draw_cspace(&ui, &cspace, 0);
  }

  endwin();

  return 0;
}
