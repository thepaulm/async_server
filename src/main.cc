#include <iostream>
#include <functional>
#include <unordered_map>
#include <queue>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "sockets.h"
#include "ioctx.h"

#define PORT 6543

int accept_cb(IOCtx *, int);
int read_cb(IOCtx *, int);
int timeout_cb(IOCtx *, int);

int main(int argc, char *argv[]) {
	int s = listen_on_port(PORT);
	std::cout << "listening on port " << PORT << std::endl;

	IOCtx *ctx = new IOCtx();
	ctx->add_socket(s, accept_cb);
	ctx->ioloop_run(); // never return
	return -1;
}

#define MS_PER_SEC 1000
int accept_cb(IOCtx *ctx, int s) {
	int client = accept(s, NULL, 0);
	if (-1 == client) {
		std::cerr << "got bad accept" << strerror(errno) << std::endl;
	} else {
		ctx->set_timeout(4 * MS_PER_SEC, [client](IOCtx* ctx) -> int{return timeout_cb(ctx, client);});
		ctx->add_socket(client, read_cb);
	}
	return 0;
}

const char *result = "HTTP/1.0\n200 OK\n\n<center>Hello</center>";

#define BUFSIZE 1024
int read_cb(IOCtx *ctx, int s) {
	char buffer[BUFSIZE];
	int got = recv(s, buffer, BUFSIZE, 0); // should probably read the request
	if (got != 0) {
		send(s, result, strlen(result), 0);
	}
	close(s); // In any case we close here because we're done.
	ctx->remove_socket(s);
	return 0;
}

int timeout_cb(IOCtx *ctx, int s) {
	close(s);
	ctx->remove_socket(s);
	return 0;
}
