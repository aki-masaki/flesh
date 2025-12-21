#pragma once

#include <ncurses.h>

struct _fh_ui {
  WINDOW *status_line;

  WINDOW *cspace_command_wins[10];
  WINDOW *cspace_output_wins[10];
  int cspace_wins_tb[10][2]; // top and bottom

  int dirty; // needs rerender
};

typedef struct _fh_ui fh_ui;

struct _fh_cspace {
  char *command;
  char *output;

  int output_len;

  int focus; // 0 - nofocus, 1 - focus, 2 - input focus
};

typedef struct _fh_cspace fh_cspace;

struct _fh_inst {
  fh_ui ui;

  fh_cspace cspaces[10];
  int cspaces_cnt;

  int mode; // 0 - normal, 1 - insert, 2 - visual, 3 - wait for char
  char last_char; // for use when mode == 3
  char *focused_input;
  int cspace_focus_index;
};

typedef struct _fh_inst fh_inst;

