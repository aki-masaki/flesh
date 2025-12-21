#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

void init_colors() {
  // -1 means the terminal default's bg color
  init_pair(1, COLOR_BLACK, -1);
  init_pair(2, COLOR_RED, -1);
  init_pair(3, COLOR_GREEN, -1);
  init_pair(4, COLOR_YELLOW, -1);
  init_pair(5, COLOR_BLUE, -1);
  init_pair(6, COLOR_MAGENTA, -1);
  init_pair(7, COLOR_CYAN, -1);
  init_pair(8, COLOR_WHITE, -1);
  init_pair(9, COLOR_WHITE, COLOR_BLACK);
}

void init_ui(fh_ui *ui) {
  initscr();
  raw();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);

  nodelay(stdscr, 1);

  start_color();
  use_default_colors();
  init_colors();

  ESCDELAY = 10;

  ui->dirty = 1;

  ui->status_line = newwin(1, COLS, LINES - 1, 0);
}

void draw_cspace(fh_inst *inst, int index) {
  fh_ui *ui = &inst->ui;
  fh_cspace *cspace = &inst->cspaces[index];

  if (!ui->dirty)
    return;

  WINDOW *cwin = ui->cspace_command_wins[index];
  WINDOW *owin = ui->cspace_output_wins[index];

  wmove(cwin, 0, 0);
  wclrtoeol(cwin);

  if (cspace->focus == 2)
    mvwaddch(cwin, 0, 0, '>');
  else if (cspace->focus == 1)
    mvwaddch(cwin, 0, 0, '*');
  else
    mvwaddch(cwin, 0, 0, '#');

  if (cspace->command != NULL)
    mvwprintw(cwin, 0, 2, "%s", cspace->command);

  wrefresh(cwin);

  wbkgd(owin, COLOR_PAIR(8));

  if (cspace->output == NULL) {
    werase(owin);
    mvwaddstr(owin, 0, 0, "(null)");
    wrefresh(owin);

    return;
  }

  int len = strlen(cspace->output);

  if (len > 2 && cspace->output[len - 2] == '\r') {
    cspace->output[len - 2] = '\0';
  } else if (len > 1 && (cspace->output[len - 1] == '\n' ||
                         cspace->output[len - 1] == '\r')) {
    cspace->output[len - 1] = '\0';
  }

  int newline_cnt = 0;

  for (int i = 0; i < len; i++)
    if (cspace->output[i] == '\n')
      newline_cnt++;

  werase(owin);
  wrefresh(owin);

  if (ui->cspace_wins_tb[index][1] != newline_cnt + 1) {
    ui->cspace_wins_tb[index][1] = newline_cnt + 1;

    delwin(owin);
    ui->cspace_output_wins[index] =
        newwin(newline_cnt + 1, COLS, ui->cspace_wins_tb[index][0] + 1, 0);
    owin = ui->cspace_output_wins[index];
  }

  FILE *fl = fopen("log", "w");
  fputs(cspace->output, fl);
  fclose(fl);

  int crx = 0;
  int cry = 0;
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

    if (cspace->output[i] == '\r') {
      cry++;

      crx = 0;

      i++; // skip \r

      if (i + 1 < len && cspace->output[i] == '\n')
        i++; // skip \n
    }

    if (cspace->output[i] == '\0')
      break;

    wattron(owin, attr);
    mvwaddch(owin, cry, crx++, cspace->output[i]);
    wattroff(owin, attr);
  }

  wrefresh(owin);
}

void create_cspace_ui(fh_inst *inst) {
  fh_ui *ui = &inst->ui;
  int i = inst->cspaces_cnt - 1;

  ui->cspace_wins_tb[i][0] = 0;
  ui->cspace_wins_tb[i][1] = 3;

  if (i > 0) {
    ui->cspace_wins_tb[i][0] = ui->cspace_wins_tb[i - 1][1] + 1;
    ui->cspace_wins_tb[i][1] =
        ui->cspace_wins_tb[i][0] + ui->cspace_wins_tb[i - 1][1] + 1;
  }

  ui->cspace_command_wins[i] = newwin(1, COLS, ui->cspace_wins_tb[i][0], 0);
  ui->cspace_output_wins[i] = newwin(2, COLS, ui->cspace_wins_tb[i][0] + 1, 0);
}

void draw_status_line(fh_inst *inst) {
  fh_ui *ui = &inst->ui;

  if (!ui->dirty)
    return;

  wmove(ui->status_line, 0, 0);
  wclrtoeol(ui->status_line);
  wattron(ui->status_line, COLOR_PAIR(9));
  mvwprintw(ui->status_line, 0, 0, "focus: %d; windows: %d; input: %p",
            inst->cspace_focus_index, inst->cspaces_cnt, inst->focused_input);
  wattroff(ui->status_line, COLOR_PAIR(9));
  wrefresh(ui->status_line);
}
