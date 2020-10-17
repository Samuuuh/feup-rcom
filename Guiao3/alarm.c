#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "alarm.h"
#include "const_defines.h"
#include "messages.h"

int received_UA = FALSE;
int resent_times = 0;

void alarm_handler() {
  if (!received_UA) {
    write_SET(fd_write);
    resent_times++;
    if (resent_times < 3)
      alarm(3);
    else
      exit(1);
  }
}
