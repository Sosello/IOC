#ifndef FLAG_H
#define FLAG_H

enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  int val;
};

#endif