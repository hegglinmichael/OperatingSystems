#ifndef THREAD_AHEAD_H
#define THREAD_AHEAD_H

#include "lwp.h"

#define DEBUG 0
#define MAX_THREADS 100000
#define LONG_SIZE sizeof(unsigned long)
#define RFILE_SIZE sizeof(rfile)
#define THREAD_SIZE sizeof(context)
#define NODE_SIZE sizeof(struct node)

void rr_init(void);
void rr_shutdown(void);
void rr_admit(thread new);
void rr_remove(thread victim);
thread rr_next(void);
void copy_a_thread_over(thread old, thread new);


#endif
