procmanage
==========

procmanage is a process management library for Unix-based operating systems.
This library drastically reduces the overhead of managing processes from within
your application.

## Usage

To use procmanage, simply link it to your project and include the `procmanage.h`
header.  All of the interface methods should become available to you.

#### Functions

* `void process_add_arg(struct Process* p, const char* arg)` - Appends an
argument to the argument list.
* `void process_add_args(struct Process* p, char* const args[])` - Appends
multiple arguments to the argument list.
* `void process_add_env(struct Process* p, const char* env)` - Appends an
environment variable to the environment variable list.
* `void process_add_envs(struct Process* p, char* const envs[])` - Appends
multiple environment variables to the environment variable list.
* `void process_clear_argv(struct Process* p)` - Clears the argument list.
* `void process_clear_envp(struct Process* p)` - Clears the environment variable
list.
* `void process_close(struct Process* p)` - Kills a `Process` and closes its
pipes.
* `struct Process* process_create(const char* path, char* const argv[], char*
const envp[])` - Creates a `Process` object with the given path.  `argv` and
`envp` are optional parameters (just use `NULL`).
* `void process_free(struct Process* p)` - Destroys a `Process` object.
* `int process_open(struct Process* p)` - Launches a `Process` object.

#### Examples

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "procmanage/procmanage.h"

#ifdef __APPLE__
#define BINARY "/sbin/ping"
#else
#define BINARY "/bin/ping"
#endif

int main() {
  // argv and envp params must be NULL terminated; path should be first argument
  char* const argv[] = { BINARY, "-c", "4", NULL };
  // Setup storage for read buffer
  char*       buf    = calloc(1025, sizeof(char));

  // Create a process object (automatically sets first argument to binary)
  struct Process* p  = process_create(BINARY, argv, NULL);
  // Add argument to Process object
  process_add_arg(p, "google.com");
  // Launch the Process object
  process_open(p);
  // Read data until the Process object closes
  while (read(p->out, buf, 1024) != 0) {
    printf("%s", buf);
    buf = calloc(1025, sizeof(char));
  }
  // Read data until the Process object closes
  while (read(p->err, buf, 1024) != 0) {
    printf("%s", buf);
    buf = calloc(1025, sizeof(char));
  }
  // Close the Process object (kill process and close pipes)
  process_close(p);
  // Destroy the Process object
  process_free(p);

  // Clean memory
  free(buf);
  buf = NULL;
  p = NULL;

  return 0;
}
```
