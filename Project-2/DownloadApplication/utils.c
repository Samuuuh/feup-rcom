#include <stdio.h>
#include <string.h>

#include "utils.h"

int parseArguments(char* argument, char* user, char* pass, char* host, char* url_path) {

  // Parse the initial part of the argument: "ftp://"
  if(strncmp(argument, "ftp://", 6) != 0) {
    fprintf(stderr,"Invalid Argument: it should start with 'ftp://'\n");
		return -1;
  }

  // Parse the part of the argument relative to the user
  int i = 6;
  while (argument[i] != ':') {
    user[i - 6] = argument[i];
    i++;
  }
  user[i - 6] = '\0';

  // Parse the part of the argument relative to the pass
  i++;
  int j = i;
  while (argument[j] != '@') {
    pass[j - i] = argument[j];
    j++;
  }
  pass[j - i] = '\0';

  // Parse the part of the argument relative to the host
  j++;
  int k = j;
  while (argument[k] != '/') {
    host[k - j] = argument[k];
    k++;
  }
  host[k - j] = '\0';

  // Parse the part of the argument relative to the url path
  k++;
  int l = k;
  while (argument[l] != '\0') {
    url_path[l - k] = argument[l];
    l++;
  }
  url_path[l - k] = '\0';

  return 0;
}