#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>

#include "defs.h"
#include "structs.h"
#include "ui.h"
#include "utils.h"

int main() {
  fh_inst instance = {.mode = 1, .cspace_focus_index = 0, .last_char = '\0'};

  fh_cspace cspace = create_cspace(&instance);
  cspace.focus = 2;
  instance.focused_input = cspace.command;

  init_ui(&instance.ui);

  fh_ui *ui = &instance.ui;

  create_cspace_ui(&instance);

  int c;
  int i = 0;
  while (c = getch()) {
    if (instance.mode == 3) {
      if (instance.last_char == 'f') {
        int n = c - '0';

        if (n >= 0 && n <= instance.cspaces_cnt) {
          instance.cspaces[instance.cspace_focus_index].focus = 0;

          instance.cspace_focus_index = n;
          instance.cspaces[instance.cspace_focus_index].focus = 1;

          ui->dirty = 1;
        }
      }

      nodelay(stdscr, 1);
      instance.mode = 0;
    } else if (instance.mode == 0) {
      if (c == 'n' && instance.cspaces_cnt <= 9) {
        fh_cspace cspace = create_cspace(&instance);
        create_cspace_ui(&instance);

        instance.focused_input = cspace.command;

        ui->dirty = 1;
      } else if (c == 'f') {
        nodelay(stdscr, 0); // make getch blocking

        instance.mode = 3;
        instance.last_char = 'f';

        continue;
      } else if (c == 'a' || c == 'i') {
        instance.mode = 1;
        instance.focused_input =
            instance.cspaces[instance.cspace_focus_index].command;
        instance.cspaces[instance.cspace_focus_index].focus = 2;

        ui->dirty = 1;
      }
    } else if (instance.mode == 1) {
      if (isprint(c) && i < MAX_COMMAND_LEN) {
        instance.focused_input[i++] = c;

        ui->dirty = 1;
      } else if (c == KEY_BACKSPACE && i > 0) {
        instance.focused_input[--i] = '\0';

        ui->dirty = 1;
      } else if (c == 0x0A) { // Line feed (enter)
        int argc = 0;
        char **args = get_command_args(
            instance.cspaces[instance.cspace_focus_index].command, &argc);
        exec_cspace(&instance.cspaces[instance.cspace_focus_index], args);

        ui->dirty = 1;
      } else if (c == 0x1B) { // Escape
        instance.focused_input = NULL;
        instance.mode = 0;
        instance.cspaces[instance.cspace_focus_index].focus = 1;

        ui->dirty = 1;
      }
    }

    for (int i = 0; i < instance.cspaces_cnt; i++) {
      draw_cspace(&instance, i);
    }

    draw_status_line(&instance);

    ui->dirty = 0;
  }

  endwin();

  return 0;
}
