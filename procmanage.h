/**
 * @file  procmanage.h
 * @brief Process Manager
 *
 * Provides a simple interface to manage process creation, environment,
 *   spawning, and destruction
 *
 * @author     Clay Freeman
 * @date       January 3, 2015
 */

#ifndef _PROCMANAGE_H
#define _PROCMANAGE_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct Process {
  char*  path; // path to binary
  char** argv; // argument array (terminated with NULL pointer)
  char** envp; // environment variables (terminated with NULL pointer)
  int    in;   // stdin  (from Process perspective)
  int    out;  // stdout (from Process perspective)
  int    err;  // stderr (from Process perspective)
  pid_t  pid;  // pid of Process
};

#endif

#ifndef _PROCMANAGE_C
#define _PROCMANAGE_C

extern void process_add_arg(struct Process* p, const char* arg);
extern void process_add_env(struct Process* p, const char* env);
extern void process_close(struct Process* p);
extern struct Process* process_create(const char* path, char* const argv[],
  char* const envp[]);
extern void process_free(struct Process* p);
extern int process_open(struct Process* p);

#endif
