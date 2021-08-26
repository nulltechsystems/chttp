/*
 * Copyright (c) 2021 chttp
 *
 */

#include "chttp.h"

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

/*
static void
_tcp_set_nonblocking(int sock)
{
	int val, ret;

	val = 1;
	ret = ioctl(sock, FIONBIO, &val);
	(void)ret;
}

static void
_tcp_set_blocking(int sock)
{
	int val, ret;

	val = 0;
	ret = ioctl(sock, FIONBIO, &val);
	(void)ret;
}
*/

void
chttp_tcp_import(struct chttp_context *ctx, int sock)
{
	chttp_context_ok(ctx);
	assert_zero(ctx->addr.magic);
	assert_zero(ctx->addr.len);
	assert_zero(ctx->addr.sa.sa_family);
	assert(sock >= 0);

	ctx->addr.magic = CHTTP_ADDR_MAGIC;
	ctx->addr.sock = sock;

	chttp_addr_ok(ctx);
}

void
chttp_tcp_connect(struct chttp_context *ctx)
{
	struct chttp_addr *addr;
	int val;

	chttp_context_ok(ctx);

	addr = &ctx->addr;

	assert(addr->magic == CHTTP_ADDR_MAGIC);
	assert(addr->sock == -1);

	addr->sock = socket(addr->sa.sa_family, SOCK_STREAM, 0);

	if (addr->sock < 0) {
		ctx->error = CHTTP_ERR_CONNECT;
		return;
	}

	ctx->state = CHTTP_STATE_CONNECTING;

	val = 1;
	setsockopt(addr->sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
	val = 1;
	setsockopt(addr->sock, IPPROTO_TCP, TCP_FASTOPEN, &val, sizeof(val));

	val = connect(addr->sock, &addr->sa, addr->len);

	// TODO non blocking timeout (EINPROGRESS)

	if (val) {
		chttp_tcp_close(ctx);
		ctx->error = CHTTP_ERR_CONNECT;

		return;
	}

	ctx->state = CHTTP_STATE_CONNECTED;

	return;
}

void
chttp_tcp_read(struct chttp_context *ctx)
{
	size_t ret;

	chttp_context_ok(ctx);
	chttp_dpage_ok(ctx->data_last);
	assert(ctx->data_last->offset < ctx->data_last->length);

	ret = chttp_tcp_read_buf(ctx, ctx->data_last->data + ctx->data_last->offset,
		ctx->data_last->length - ctx->data_last->offset);

	ctx->data_last->offset += ret;
	assert(ctx->data_last->offset <= ctx->data_last->length);
}

size_t
chttp_tcp_read_buf(struct chttp_context *ctx, void *buf, size_t buf_len)
{
	ssize_t ret;

	chttp_context_ok(ctx);
	chttp_addr_ok(ctx);
	assert(buf);
	assert(buf_len);

	ret = recv(ctx->addr.sock, buf, buf_len, 0);

	if (ret <= 0) {
		// TODO other errors
		chttp_finish(ctx);

		return 0;
	}

	return ret;
}

void
chttp_tcp_close(struct chttp_context *ctx)
{
	chttp_context_ok(ctx);
	chttp_addr_ok(ctx);

	assert_zero(close(ctx->addr.sock));

	ctx->addr.sock = -1;
}
