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

  int c;
  int i = 0;
  while ((c = getch()) != 'q') {
    if (isprint(c) && i < MAX_COMMAND_LEN)
      cspace.command[i++] = c;
    else if (c == KEY_BACKSPACE && i > 0)
      cspace.command[--i] = '\0';

    move(0, 0);
    clrtoeol();
    mvprintw(0, 0, "> %s", cspace.command);
    refresh();
  }

  endwin();

  return 0;
}
