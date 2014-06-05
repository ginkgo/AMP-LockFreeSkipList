#include "../init.h"

#ifdef UTS_TEST

#include <stdlib.h>

void impl_abort(int err) {
  exit(err);
}

char * impl_getName() {
	static char name[] = "pheet";
	return name;
}

// construct string with all parameter settings 
int impl_paramsToStr(char *, int ind) {
  return ind;
}

int impl_parseParam(char *, char *) {
  int err = 1;  // Return 0 on a match, nonzero on an error
  return err;
}

void impl_helpMessage() {
}

#endif
