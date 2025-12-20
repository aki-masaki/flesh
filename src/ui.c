#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

void init_colors() {
  init_pair(1, COLOR_BLACK, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
  init_pair(5, COLOR_BLUE, COLOR_BLACK);
  init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(7, COLOR_CYAN, COLOR_BLACK);
  init_pair(8, COLOR_WHITE, COLOR_BLACK);
}

void init_ui(fh_ui *ui) {
  initscr();
  raw();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  start_color();
  init_colors();

  ui->cspace_command_wins[0] = newwin(1, COLS, 0, 0);
  ui->cspace_output_wins[0] = newwin(10, COLS, 1, 0);
}

void draw_cspace(fh_ui *ui, fh_cspace *cspace, int index) {
  WINDOW *cwin = ui->cspace_command_wins[index];
  WINDOW *owin = ui->cspace_output_wins[index];

  wmove(cwin, 0, 0);
  wclrtoeol(cwin);
  mvwprintw(cwin, 0, 0, "> %s", cspace->command);
  wrefresh(cwin);

  if (cspace->output != NULL) {
    size_t output_len = strlen(cspace->output);

    if (output_len > 2 && cspace->output[output_len - 2] == '\r')
      cspace->output[strlen(cspace->output) - 2] = '\0';
  }

  if (cspace->output == NULL) {
    werase(owin);
    mvwaddstr(owin, 0, 0, "(null)");
    wrefresh(owin);

    return;
  }

  FILE *fl = fopen("log", "w");
  fputs(cspace->output, fl);
  fclose(fl);

  werase(owin);

  int len = strlen(cspace->output);
  int crx = 0;
  int attr = COLOR_PAIR(8);

  for (int i = 0; i < len; i++) {
    // ESC
    while (cspace->output[i] == 0x1b) {
      // [
      if (i + 1 < len && cspace->output[++i] == 0x5B) {
        // 38 (foreground)
        if (i + 1 < len && cspace->output[++i] == 0x33 && i + 1 < len &&
            cspace->output[++i] == 0x38) {
          i += 4; // skip the next three characters (for me they were 'q5;')

          if (cspace->output[i] > 0x29 && cspace->output[i] < 0x38 &&
              i + 1 < len && cspace->output[i + 1] == 0x6D) {
            attr = COLOR_PAIR(cspace->output[i] - '0' + 1);

            i += 2;
          }
        } else if (cspace->output[i] == 0x31 && i + 1 < len &&
                   cspace->output[++i] == 0x6D) {
          attr |= A_BOLD;

          i++;
        } else if (cspace->output[i] == 0x30 && i + 1 < len) {
          if (cspace->output[i + 1] == 0x6D) {
            attr = COLOR_PAIR(8);

            i += 2;
          } else if (cspace->output[++i] == 0x31 && i + 1 < len &&
                     cspace->output[++i] == 0x3B) {
            attr = COLOR_PAIR(3);

            // starts with 3, so fg color
            if (i + 1 < len && cspace->output[++i] == 0x33) {
              // between 30 and 37 (both included)
              if (i + 1 < len && cspace->output[i + 1] < 0x38 &&
                  cspace->output[i + 1] > 0x29) {
                attr = COLOR_PAIR(cspace->output[i + 1] - '0' + 1);

                i += 2;
              }
            }

            i++;
          }
        }
      }
    }

    wattron(owin, attr);
    mvwaddch(owin, 0, crx++, cspace->output[i]);
    wattroff(owin, attr);
  }

  wrefresh(owin);
}
