#pragma once

#include <ncurses.h>

struct _fh_ui {
  WINDOW *status_line;

  WINDOW *cspace_command_wins[10];
  WINDOW *cspace_output_wins[10];
  int cspace_delim_lines[10][2]; // they will be in the stdscr window
  int cspace_wins_cnt;

  int dirty; // needs rerender
};

typedef struct _fh_ui fh_ui;

struct _fh_cspace {
  char *command;
  char *output;

  int output_len;
};

typedef struct _fh_cspace fh_cspace;

struct _fh_inst {
  fh_ui ui;

  fh_cspace cspaces[10];
  int cspaces_cnt;
};

typedef struct _fh_inst fh_inst;

