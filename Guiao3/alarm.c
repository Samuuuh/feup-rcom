#include "alarm.h"

int received_UA = FALSE;
int resent_times = 0;

void alarm_handler() {
  if (!received_UA) {
    send_SET();
    resent_times++;
    if (resent_times < 3)
      alarm(3);
    else
      exit(1);
  }
}