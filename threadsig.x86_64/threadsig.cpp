/* **********************************************************
 * Copyright (c) 2011-2018 Google, Inc.  All rights reserved.
 * Copyright (c) 2003-2008 VMware, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Google, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/* Build with:
 * g++ -o threadsig threadsig.cpp -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <vector>

volatile double pi = 0.0;  /* Approximation to pi (shared) */
pthread_mutex_t pi_lock;   /* Lock for above */
static double intervals;   /* How many intervals? */
static int num_threads;
static int alarm_count;

static void
signal_handler(int sig, siginfo_t *siginfo, void *ucxt)
{
    if (sig == SIGVTALRM)
        ++alarm_count;
}

void *
process(void *arg)
{
    register double width, localsum;
    int iproc = (int)(intptr_t)arg;
    char buf1[512], buf2[512];
    
    /* Set width */
    width = 1.0 / intervals;

    /* Do the local computations */
    localsum = 0;
    for (int i = iproc; i < intervals; i += num_threads) {
        register double x = (i + 0.5) * width;
        localsum += 4.0 / (1.0 + x * x);

        /* Arbitrary work trying to get string operations. */
        memset(buf1, 0, sizeof(buf1));
        memcpy(buf2, buf1, sizeof(buf2));
    }
    localsum *= width;
    
    /* Lock pi for update, update it, and unlock */
    pthread_mutex_lock(&pi_lock);
    pi += localsum;
    pthread_mutex_unlock(&pi_lock);

    return NULL;
}

int
main(int argc, char **argv)
{
    std::vector<pthread_t> threads;
    void * retval;

    /* Get the number of intervals and threads */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <threads> <intervals>\n", argv[0]);
        exit(0);
    }
    num_threads = atoi(argv[1]);
    intervals = atoi(argv[2]);

    /* Initialize the lock on pi */
    pthread_mutex_init(&pi_lock, NULL);

    /* Add an itimer to get signals into the mix */
    struct sigaction act;
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO | SA_ONSTACK; /* send 3 args to handler */
    int rc = sigaction(SIGVTALRM, &act, NULL);
    if (rc) {
        fprintf(stderr, "sigaction failed: %d\n", rc);
        exit(1);
    }
    struct itimerval t;
    t.it_interval.tv_sec = 0;
    t.it_interval.tv_usec = 5000;
    t.it_value.tv_sec = 0;
    t.it_value.tv_usec = 5000;
    rc = setitimer(ITIMER_VIRTUAL, &t, NULL);
    if (rc) {
        fprintf(stderr, "setitimer failed: %d\n", rc);
        exit(1);
    }

    /* Make the threads */
    for (int i = 0; i < num_threads; ++i) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, process, (void *)(intptr_t)i)) {
            fprintf(stderr, "%s: failed to create thread\n", argv[0]);
            exit(1);
        }
        threads.push_back(thread);
    }

    /* Join (collapse) the threads */
    for (auto thread : threads) {
        if (pthread_join(thread, &retval)) {
            fprintf(stderr, "%s: thread join failed\n", argv[0]);
            exit(1);
        }
    }

    printf("Estimation of pi is %16.15f\n", pi);
    printf("Received %d alarms\n", alarm_count);

    return 0;
}
