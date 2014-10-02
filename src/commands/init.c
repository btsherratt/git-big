#include <stdio.h>

#include "init.h"
#include "../repo.h"
#include "../db/init.h"
#include "../filters/init.h"
#include "../patterns/init.h"

int initRun(int argc, char *argv[])
{
	int error;

	error = git_repository_config(&gRepoConfigHandle, gRepoHandle);

	if(error == 0)
	{
		error = dbInit();
		error = filtersInit();
		error = patternsInit();

		git_config_free(gRepoConfigHandle);
		gRepoConfigHandle = NULL;

		return 0;
	}
	else
	{
		// Fatal error...
		fprintf(stderr, "Fatal git-big error when loading git configuration\n"
		                "Is your repository corrupt?");

		return 1;
	}
}
