#define CTEST_MAIN
#include "ctest.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ipc.h"

CTEST_DATA(ipc) {
	char * fname;
};

CTEST_SETUP(ipc) {

	data->fname = malloc(1024);

	srand(time(NULL));
	sprintf(data->fname, "/tmp/%i.socket", rand() % 1000);
	CTEST_LOG("socket: %s", data->fname);
}

CTEST_TEARDOWN(ipc) {
	free(data->fname);
}

CTEST2(ipc, open)
{
	int server;
	server = ipc_server(data->fname);

	ipc_close(server);
}

int main(int argc, const char * argv[])
{
	return ctest_main(argc, argv);
}
