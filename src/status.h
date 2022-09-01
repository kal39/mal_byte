#ifndef STATUS_H
#define STATUS_H

#include "common.h"

typedef struct Status {
	bool ok;
	char *errorMessage;
} Status;

Status ok();
Status error(char *errorMessage);

#endif