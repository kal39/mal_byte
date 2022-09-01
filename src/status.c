#include "status.h"

Status ok() {
	return (Status){.ok = true, .errorMessage = NULL};
}

Status error(char *message) {
	Status error = (Status){.ok = false, .errorMessage = NULL};
	if (message != NULL) {
		error.errorMessage = malloc(strlen(message) + 1);
		memcpy(error.errorMessage, message, strlen(message));
	}
	return error;
}