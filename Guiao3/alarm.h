// ----- Alarm Handler -----

int received_UA = FALSE;

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

// ---------------------------------