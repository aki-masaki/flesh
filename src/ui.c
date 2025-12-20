#include <string.h>

#include "structs.h"

void init_ui(fh_ui *ui) {
  initscr();
  raw();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  start_color();

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

  if (cspace->output != NULL) {
    FILE *fl = fopen("log", "w");
    fputs(cspace->output, fl);
    fclose(fl);
  }

  werase(owin);
  mvwaddstr(owin, 0, 0, cspace->output);
  wrefresh(owin);
}
