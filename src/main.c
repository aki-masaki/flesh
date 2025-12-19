#include <ctype.h>
#include <ncurses.h>

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
  while ((c = getch()) != 'q') {
    if (isprint(c) && i < MAX_COMMAND_LEN) {
      cspace.command[i++] = c;
    } else if (c == KEY_BACKSPACE && i > 0) {
      cspace.command[--i] = '\0';
    } else if (c == 10) { // Enter key
      // https://stackoverflow.com/questions/11067800/ncurses-key-enter-is-fail

      exec_cspace(&cspace);
    }

    draw_cspace(&ui, &cspace, 0);
  }

  endwin();

  return 0;
}
