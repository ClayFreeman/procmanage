/**
 * @file  procmanage.c
 * @brief Process Manager
 *
 * Provides a simple interface to manage process creation, environment,
 *   spawning, and destruction
 *
 * @author     Clay Freeman
 * @date       January 3, 2015
 */

#ifndef _PROCMANAGE_C
#define _PROCMANAGE_C

#define _POSIX_SOURCE
#include "procmanage.h"

/**
 * @brief Process Add Argument
 *
 * Adds an argument to an existing Process
 *
 * @param[out] p   The Process object
 * @param      arg The environment variable to append
 */
extern void process_add_arg(struct Process* p, const char* arg) {
  // Count the variables in p->argv
  int argc = 0;
  for (; p->argv != NULL && p->argv[argc] != NULL; argc++);
  // Reallocate storage
  p->argv = realloc(p->argv, (argc + 2) * sizeof(char*));
  // Allocate memory for a char* in the new slot
  p->argv[argc] = calloc(strlen(arg) + 1, sizeof(char));
  // Copy the provided char* into the new char*
  memcpy(p->argv[argc], arg, strlen(arg));
  // NULL the last element
  p->argv[argc + 1] = NULL;
}

/**
 * @brief Process Add Environment Variable
 *
 * Adds an environment variable to an existing Process
 *
 * @param[out] p   The Process object
 * @param      env The environment variable to append
 */
extern void process_add_env(struct Process* p, const char* env) {
  // Count the variables in p->envp
  int envc = 0;
  for (; p->envp != NULL && p->envp[envc] != NULL; envc++);
  // Reallocate storage
  p->envp = realloc(p->envp, (envc + 2) * sizeof(char*));
  // Allocate memory for a char* in the new slot
  p->envp[envc] = calloc(strlen(env) + 1, sizeof(char));
  // Copy the provided char* into the new char*
  memcpy(p->envp[envc], env, strlen(env));
  // NULL the last element
  p->envp[envc + 1] = NULL;
}

/**
 * @brief Process Clear argv
 *
 * Clears the Process object's arguments
 *
 * @remarks
 * Frees all arguments and the argv property
 *
 * @param[out] p The Process object
 */
void process_clear_argv(struct Process* p) {
  // Free arguments
  if (p->argv != NULL) {
    // Count environment variables
    int argc = 0;
    for (; p->argv[argc] != NULL; argc++);
    // Free each environment variable
    for (int i = 0; i < argc; i++) {
      free(p->argv[i]);
      p->argv[i] = NULL;
    }
    // Free argument list
    free(p->argv);
    p->argv = NULL;
  }
}

/**
 * @brief Process Clear envp
 *
 * Clears the Process object's environment variables
 *
 * @remarks
 * Frees all environment variables and the envp property
 *
 * @param[out] p The Process object
 */
void process_clear_envp(struct Process* p) {
  // Free environment variables
  if (p->envp != NULL) {
    // Count environment variables
    int envc = 0;
    for (; p->envp[envc] != NULL; envc++);
    // Free each environment variable
    for (int i = 0; i < envc; i++) {
      free(p->envp[i]);
      p->envp[i] = NULL;
    }
    // Free environment variable list
    free(p->envp);
    p->envp = NULL;
  }
}

/**
 * @brief Process Close
 *
 * Closes the Process' pipes, kills the process, and reaps its zombie
 *
 * @remarks
 * After closing a Process, it can be reused
 *
 * @param[out] p The Process object
 */
extern void process_close(struct Process* p) {
  // Close pipes
  if (p->in != -1) {
    close(p->in);
    p->in  = -1;
  }
  if (p->out != -1) {
    close(p->out);
    p->out = -1;
  }
  if (p->err != -1) {
    close(p->err);
    p->err = -1;
  }

  // Kill process
  if (p->pid != -1) {
    kill(p->pid, SIGKILL);
    waitpid(p->pid, NULL, WNOHANG);
    p->pid = -1;
  }
}

/**
 * @brief Process Create
 *
 * Creates a Process object with a binary, arguments, and environment
 *
 * @remarks
 *  - This function inserts the path as the first argument (standard on linux)
 *  - It is recommended to obtain the envp argument from `int main(...)`
 *  - argv and envp must end with a NULL pointer
 *
 * @param path The path to the binary
 * @param argv The argument list
 * @param envp The environment variable list
 *
 * @return The Process object
 */
extern struct Process* process_create(const char* path, char* const argv[],
    char* const envp[]) {
  // Allocate and initialize a new Process
  struct Process* p = malloc(sizeof(struct Process));
  p->path = NULL;
  p->argv = NULL;
  p->envp = NULL;
  p->in   = -1;
  p->out  = -1;
  p->err  = -1;
  p->pid  = -1;

  // Copy the provided path to the Process
  p->path = calloc(strlen(path) + 1, sizeof(char));
  memcpy(p->path, path, strlen(path));

  // Add path as first argument
  process_add_arg(p, path);

  // Add arguments
  for (int i = 0; argv != NULL && argv[i] != NULL; i++) {
    process_add_arg(p, argv[i]);
  }

  // Add environment variables
  for (int i = 0; envp != NULL && envp[i] != NULL; i++) {
    process_add_env(p, envp[i]);
  }

  return p;
}

/**
 * @brief Process Free
 *
 * Closes and destroys the Process object
 *
 * @remarks
 * After freeing a Process, it cannot be reused
 *
 * @param[out] p The process object
 */
extern void process_free(struct Process* p) {
  if (p != NULL) {
    // Close the process
    process_close(p);

    // Free path
    if (p->path != NULL) {
      free(p->path);
      p->path = NULL;
    }

    // Clear arguments
    process_clear_argv(p);

    // Clear environment variables
    process_clear_envp(p);

    // Free the process
    free(p);
  }
}

/**
 * @brief Process Open
 *
 * Launches the given Process object via fork/exec
 *
 * @param[out] p The Process object
 *
 * @return 1 upon success, 0 upon failure
 */
extern int process_open(struct Process* p) {
  int retVal = 0;
  if (p->pid == -1) {
    // Prepare pipes
    int ipipe[2], opipe[2], epipe[2];
    pipe(ipipe);
    pipe(opipe);
    pipe(epipe);

    // Fork and exec
    p->pid = fork();
    if (p->pid == 0) {
      // Prepare pipes
      close(ipipe[1]);
      close(opipe[0]);
      close(epipe[0]);
      dup2(ipipe[0], STDIN_FILENO);
      dup2(opipe[1], STDOUT_FILENO);
      dup2(epipe[1], STDERR_FILENO);
      close(ipipe[0]);
      close(opipe[1]);
      close(epipe[1]);

      // Run command
      execve(p->path, p->argv, p->envp);

      // Exit if something goes wrong
      _exit(1);
    }
    else {
      // Prepare pipes
      close(ipipe[0]);
      close(opipe[1]);
      close(epipe[1]);
      p->in  = ipipe[1];
      p->out = opipe[0];
      p->err = epipe[0];
    }

    // Update return value
    retVal = (p->pid != -1 ? 1 : 0);
  }

  return retVal;
}

#endif
