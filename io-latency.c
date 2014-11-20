/*
 * Compile with: gcc -Wall -O2 main.c -o io-latency -lrt
 */

#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <time.h>

void run_direct_io_test(int runs)
{
    int i = 0, f;
    unsigned long total_sec = 0, total_nsec = 0;
    long tmp_nsec;
    int psize = getpagesize();
    char *c;
    char path[32];
    struct timespec tstart, tstop;
    double total_time = 0.0;

    printf("running O_DIRECT test...\n");

    sprintf(path, "run_direct_io_test.tmp.%d", rand());
    f = open(path, O_CREAT | O_RDWR | O_DIRECT, 0666);
    if(f < 1) {
        perror("open");
        exit(1);
    }

    c = memalign(psize, psize);
    if(!c)
        perror("malloc");
    memset(c, '\0', psize);
    while(i < psize) {
        int j;
        for(j = 'a'; j < 'z' && i < psize; j++,i++)
            c[i] = j;
    }
    for(i = 0; i < runs; i++) {
        if(clock_gettime(CLOCK_REALTIME, &tstart) != 0) {
            perror("clock_gettime");
            i--;
            continue;
        }
        if(write(f, c, psize) != psize) {
            perror("write");
            exit(1);
        }
        clock_gettime(CLOCK_REALTIME, &tstop);
        tmp_nsec = tstop.tv_nsec - tstart.tv_nsec;
        if(tmp_nsec < 1) {
            //printf("tmp_nsec < 1: %li - %li\n", tstop.tv_nsec, tstart.tv_nsec);
            i--; continue;
        }
        total_sec += tstop.tv_sec - tstart.tv_sec;
        total_nsec += tstop.tv_nsec - tstart.tv_nsec;
    }
    total_time = ((double)total_sec) + ((double)total_nsec/1000000000);
    printf("%4.10f seconds to run %d loops. avgtime=%4.5fms\n\n", total_time, runs, ((double)total_nsec/(double)runs)/1000000);
    close(f);
    unlink(path);
}

void run_sync_io_test(int runs)
{
    int i = 0, f; 
    unsigned long total_sec = 0, total_nsec = 0;
    long tmp_nsec;
    char path[32];
    char c[] = "a";
    struct timespec tstart, tstop;
    double total_time = 0.0;

    printf("running O_SYNC test...\n");

    sprintf(path, "run_sync_io_test.tmp.%d", rand());
    f = open(path, O_CREAT | O_RDWR | O_SYNC, 0666);
    if(f < 1) {
        perror("open");
        exit(1);
    }
    for(i = 0; i < runs; i++) {
        if(clock_gettime(CLOCK_REALTIME, &tstart) != 0) {
            perror("clock_gettime");
            i--;
            continue;
        }
        /* getting negative values here on the start struct */
        if(tstart.tv_nsec < 0) {
            perror("clock_gettime tv_nsec < 0");
            i--;
            continue;
        }
        if(write(f, c, 1) != 1) {
            perror("write");
            exit(1);
        }
        clock_gettime(CLOCK_REALTIME, &tstop);
        tmp_nsec = tstop.tv_nsec - tstart.tv_nsec;
        if(tmp_nsec < 1) {
            //printf("tmp_nsec < 1: %li - %li\n", tstop.tv_nsec, tstart.tv_nsec);
            i--; continue;
        }
        total_sec += tstop.tv_sec - tstart.tv_sec;
        total_nsec += tstop.tv_nsec - tstart.tv_nsec;
        //printf("tmp_nsec: %li - %li = %li\n", tmp_nsec, tstop.tv_nsec, tstart.tv_nsec);
    }
    total_time = ((double)total_sec) + ((double)total_nsec/1000000000);
    printf("%4.10f seconds to run %d loops. avgtime=%4.5fms\n\n", total_time, runs, ((double)total_nsec/(double)runs)/1000000);
    close(f);
    unlink(path);
}


int main(int argc, char **argv)
{
    srand(time(NULL));
    run_direct_io_test(atoi(argv[1]));
    run_sync_io_test(atoi(argv[1]));
    fflush(stdout);
    fflush(stderr);
    return 0;
}
