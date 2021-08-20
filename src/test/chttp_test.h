/*
 * Copyright (c) 2021 chttp
 *
 */

#ifndef _CHTTP_TEST_H_INCLUDED_
#define _CHTTP_TEST_H_INCLUDED_

#include "chttp.h"
#include "test/chttp_test_cmds.h"
#include "data/tree.h"
#include "data/queue.h"

#include <stdio.h>
#include <pthread.h>

#define CHTTP_TEST_TIMEOUT_SEC			10

enum chttp_test_verbocity {
	CHTTP_LOG_FORCE = -2,
	CHTTP_LOG_ROOT = -1,
	CHTTP_LOG_NONE = 0,
	CHTTP_LOG_VERBOSE,
	CHTTP_LOG_VERY_VERBOSE
};

struct chttp_test_cmdentry {
	unsigned int				magic;
#define CHTTP_TEST_ENTRY			0x52C66713

	RB_ENTRY(chttp_test_cmdentry)		entry;

	const char				*name;
	chttp_test_cmd_f			*func;
};

RB_HEAD(chttp_test_tree, chttp_test_cmdentry);

struct chttp_test;

typedef void (chttp_test_finish_f)(struct chttp_text_context*);

struct chttp_test_finish {
	unsigned int				magic;
#define CHTTP_TEST_FINISH			0x0466CDF2

	TAILQ_ENTRY(chttp_test_finish)		entry;

	const char				*name;
	chttp_test_finish_f			*func;
};

struct chttp_test {
	unsigned int				magic;
#define CHTTP_TEST_MAGIC			0xD1C4671E

	struct chttp_text_context		context;

	int					argc;
	char					**argv;

	pthread_t				thread;
	volatile int				stopped;

	enum chttp_test_verbocity		verbocity;

	struct chttp_test_tree			cmd_tree;
	TAILQ_HEAD(, chttp_test_finish)		finish_list;

	char					*cht_file;
	FILE					*fcht;

	char					*line_raw;
	char					*line_buf;
	size_t					line_raw_len;
	size_t					line_buf_len;
	size_t					lines;
	size_t					lines_multi;

	struct chttp_test_cmd			cmd;

	int					error;
	int					skip;
};

#define CHTTP_TEST_JOIN_INTERVAL_MS		25

void chttp_test_register_finish(struct chttp_text_context *ctx, const char *name,
	chttp_test_finish_f *func);
void chttp_test_run_finish(struct chttp_text_context *ctx, const char *name);
void chttp_test_run_all_finish(struct chttp_test *test);

void chttp_test_cmds_init(struct chttp_test *test);
struct chttp_test_cmdentry *chttp_test_cmds_get(struct chttp_test *test, const char *name);

int chttp_test_readline(struct chttp_test *test, size_t append_len);
void chttp_test_parse_cmd(struct chttp_test *test);

struct chttp_test *chttp_test_convert(struct chttp_text_context *ctx);
void chttp_test_skip(struct chttp_text_context *ctx);
void chttp_test_log(struct chttp_text_context *ctx, enum chttp_test_verbocity level,
	const char *fmt, ...);
void chttp_test_warn(int condition, const char *fmt, ...);
void chttp_test_ERROR(int condition, const char *fmt, ...);
long chttp_test_parse_long(const char *str);
void chttp_test_ERROR_param_count(struct chttp_test_cmd *cmd, size_t count);
void chttp_test_ERROR_string(const char *str);
void chttp_test_sleep_ms(long ms);
int chttp_test_join_thread(pthread_t thread, volatile int *stopped,
	unsigned long timeout_ms);

#define chttp_test_ok(test)						\
	do {								\
		assert(test);						\
		assert((test)->magic == CHTTP_TEST_MAGIC);		\
	} while (0)

#endif /* _CHTTP_TEST_H_INCLUDED_ */
