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

  ui->cspace_command_wins[0] = newwin(1, COLS, 0, 0);
  ui->cspace_output_wins[0] = newwin(2, COLS, 1, 0);
  ui->cspace_delim_lines[0][0] = -1;
  ui->cspace_delim_lines[0][1] = -1;

  ui->dirty = 1;
}

void draw_cspace(fh_ui *ui, fh_cspace *cspace, int index) {
  if (!ui->dirty)
    return;

  ui->dirty = 0;

  WINDOW *cwin = ui->cspace_command_wins[index];
  WINDOW *owin = ui->cspace_output_wins[index];

  wmove(cwin, 0, 0);
  wclrtoeol(cwin);
  mvwprintw(cwin, 0, 0, "> %s", cspace->command);
  wrefresh(cwin);

  if (cspace->output == NULL) {
    werase(owin);
    mvwaddstr(owin, 0, 0, "(null)");
    wrefresh(owin);

    if (ui->cspace_delim_lines[0][0] != -1 ||
        ui->cspace_delim_lines[0][1] != -1) {
      move(ui->cspace_delim_lines[0][0], 0);
      clrtoeol();
      move(ui->cspace_delim_lines[0][1], 0);
      clrtoeol();
      refresh();

      ui->cspace_delim_lines[0][0] = -1;
      ui->cspace_delim_lines[0][1] = -1;
    }

    ui->cspace_delim_lines[0][0] = 1;
    ui->cspace_delim_lines[0][1] = 3;

    mvhline(ui->cspace_delim_lines[0][0], 0, '-', COLS);
    mvhline(ui->cspace_delim_lines[0][1], 0, '-', COLS);
    refresh();

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

  if (ui->cspace_delim_lines[0][0] != -1 ||
      ui->cspace_delim_lines[0][1] != -1) {
    move(ui->cspace_delim_lines[0][0], 0);
    clrtoeol();
    move(ui->cspace_delim_lines[0][1], 0);
    clrtoeol();
    refresh();

    ui->cspace_delim_lines[0][0] = -1;
    ui->cspace_delim_lines[0][1] = -1;
  }

  werase(owin);
  wrefresh(owin);

  delwin(owin);
  ui->cspace_output_wins[index] = newwin(newline_cnt + 1, COLS, 2, 0);
  owin = ui->cspace_output_wins[index];

  FILE *fl = fopen("log", "w");
  fputs(cspace->output, fl);
  fclose(fl);

  ui->cspace_delim_lines[0][0] = 1;
  ui->cspace_delim_lines[0][1] = newline_cnt + 3;

  mvhline(ui->cspace_delim_lines[0][0], 0, '-', COLS);
  mvhline(ui->cspace_delim_lines[0][1], 0, '-', COLS);
  refresh();

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
