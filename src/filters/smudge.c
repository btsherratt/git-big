#include <stdio.h>

#include "smudge.h"
#include "../db.h"
#include "../hooks/big_pull.h"

static const char * const HOOK_NAME = "big-pull";

static void passthrough(void);

enum Error filter_smudge_run(int argc, char *argv[])
{
	enum Error r = ERROR_NONE;
	char *filename = argv[0];
	char id[DB_ID_SIZE] = { '\0' };
	size_t id_size = 0;

	id_size = fread(id, 1, sizeof(id), stdin);

	if(id_size != DB_ID_SIZE)
	{
		/* Passthrough... */
		fwrite(id, 1, id_size, stdout);

		goto error_passthrough;
	}

	r = db_file_query(stdout, id);

	if(r == ERROR_DB_FILE_QUERY_COULD_NOT_FIND_FILE)
	{
		char hook_path[1024]; // XXX
		char hash[DB_ID_HASH_SIZE + 1] = { '\0' }; // +1 for null
		char db_path[1024];
		char tmp[1024];
		enum Error hook_error = ERROR_NONE;

		if(!hook_big_pull_exists())
		{
			goto skip_pull;
		}

		db_id_parse(NULL, hash, id);
		hash[DB_ID_HASH_SIZE] = '\0';

		db_file_path(db_path, sizeof(db_path), id);

		hook_error = hook_big_pull_run(hash, db_path);

		if(hook_error != ERROR_NONE)
		{
			r = ERROR_INTERNAL;
			goto error_system;
		}

		r = db_file_query(stdout, id);
	}

	if(r == ERROR_DB_ID_PARSE_HEADER_INVALID)
	{
		/* Passthrough... */
		fwrite(id, 1, id_size, stdout);
		passthrough();

		r = ERROR_NONE;
		goto error_passthrough;
	}

error_system:
error_passthrough:

skip_pull:

	return r;

}

static void passthrough(void)
{
	char buffer[1024];
	size_t read_size;

	do
	{
		read_size = fread(buffer, 1, sizeof(buffer), stdin);
		fwrite(buffer, 1, read_size, stdout);
	}
	while(read_size == sizeof(buffer));
}

