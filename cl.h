/* Declarative command-line parsing library.
 * This is free and unencumbered software released into the public domain.
 */
#ifndef INCLUDED_CL_H
#define INCLUDED_CL_H

#include <stddef.h>

#ifndef CL_HELP_ALIGNMENT
#define CL_HELP_ALIGNMENT 24
#endif

typedef enum cl_type { CL_FLAG, CL_VALUE } cl_type;

typedef struct cl_opt {
  const char *short_name;
  const char *long_name;
  const char *description;
  const char *value_name;
  const char** storage;
  cl_type type;
} cl_opt;

typedef struct cl_arg {
  const char *name;
  const char *description;
  const char **storage;
} cl_arg;

typedef struct cl_interface_desc {
  const char *program_name;
  const cl_opt *opts;
  const cl_arg *positional_args;
  const char *help_header;
  const char *help_footer;
  size_t num_opts;
  size_t num_positional_args;
} cl_interface_desc;

void cl_parse(int argc, char **__restrict argv, const cl_interface_desc *desc);
void cl_print_help(const cl_interface_desc *desc);

#endif /* INCLUDED_CL_H */
