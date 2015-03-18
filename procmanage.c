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

// Declare internal function prototypes
int _process_array_count(char* const arr[]);
void _process_array_clear(char*** arr);
void _process_array_push(char*** arr, const char* item);
void _process_string_copy(char** dest, const char* src);

/**
 * @brief Process Array Count
 *
 * Counts the elements in an array until NULL is reached
 *
 * @param arr The array to count
 *
 * @return The number of elements in the array
 */
int _process_array_count(char* const arr[]) {
  // Count the variables in arr
  int arrc = 0;
  for (; arr != NULL && arr[arrc] != NULL; arrc++);
  return arrc;
}

/**
 * @brief Process Array Clear
 *
 * Frees all elements in the array and the array itself
 *
 * @remarks
 * The provided array variable is NULLed
 *
 * @param[out] arr The array to clear
 */
void _process_array_clear(char*** arr) {
  if (arr != NULL) {
    // Free each element
    int count = _process_array_count(*arr);
    for (int i = 0; i < count; i++) {
      free((*arr)[i]);
      (*arr)[i] = NULL;
    }
    // Free array
    free(*arr);
    *arr = NULL;
  }
}

/**
 * @brief Process Array Push
 *
 * Grows an array by one element and copies an item into the new position
 *
 * @param[out] arr  The array to append an item
 * @param      item The item to append
 */
void _process_array_push(char*** arr, const char* item) {
  // Count the variables in arr
  int arrc = _process_array_count(*arr);
  // Reallocate storage
  *arr = realloc(*arr, (arrc + 2) * sizeof(char*));
  // Copy item to new slot
  _process_string_copy(&((*arr)[arrc]), item);
  // NULL the last element
  (*arr)[arrc + 1] = NULL;
}

/**
 * @brief Process String Copy
 *
 * Copies a string into another location
 *
 * @remarks
 * Allocates new storage for destination
 *
 * @param[out] dest The destination pointer
 * @param      src  The source string
 */
void _process_string_copy(char** dest, const char* src) {
  // Allocate space for src at dest
  *dest = calloc(strlen(src) + 1, sizeof(char));
  // Copy src to dest
  memcpy(*dest, src, strlen(src));
}

/**
 * @brief Process Add Argument
 *
 * Adds an argument to an existing Process
 *
 * @param[out] p   The Process object
 * @param      arg The environment variable to append
 */
extern void process_add_arg(struct Process* p, const char* arg) {
  // Push arg on to p->argv
  _process_array_push(&p->argv, arg);
}

/**
 * @brief Process Add Arguments
 *
 * Adds arguments to an existing Process
 *
 * @param[out] p   The Process object
 * @param      arg The environment variable to append
 */
extern void process_add_args(struct Process* p, char* const args[]) {
  // Add arguments
  for (int i = 0; args != NULL && args[i] != NULL; i++) {
    process_add_arg(p, args[i]);
  }
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
  // Push env on to p->envp
  _process_array_push(&p->envp, env);
}

/**
 * @brief Process Add Environment Variables
 *
 * Adds environment variables to an existing Process
 *
 * @param[out] p   The Process object
 * @param      env The environment variable to append
 */
extern void process_add_envs(struct Process* p, char* const envs[]) {
  // Add environment variables
  for (int i = 0; envs != NULL && envs[i] != NULL; i++) {
    process_add_env(p, envs[i]);
  }
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
extern void process_clear_argv(struct Process* p) {
  // Free arguments
  _process_array_clear(&p->argv);
  p->argv = NULL;
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
extern void process_clear_envp(struct Process* p) {
  // Free environment variables
  _process_array_clear(&p->envp);
  p->envp = NULL;
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
  _process_string_copy(&p->path, path);

  // Add arguments
  process_add_args(p, argv);

  // Add environment variables
  process_add_envs(p, envp);

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
    // // Close the process
    // process_close(p);

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

      // Create session and process group
      setsid();

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
