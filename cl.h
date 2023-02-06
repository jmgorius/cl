/* Declarative command-line parsing library.
 * This is free and unencumbered software released into the public domain.
 */
#ifndef INCLUDED_CL_H
#define INCLUDED_CL_H

#include <stddef.h>

#ifndef CL_HELP_ALIGNMENT
#define CL_HELP_ALIGNMENT 24
#endif

typedef enum CLType { CL_FLAG, CL_VALUE } CLType;

typedef struct CLOpt CLOpt;
typedef struct CLArg CLArg;

struct CLOpt {
  const char *short_name;
  const char *long_name;
  const char *description;
  const char *value_name;
  const char **storage;
  CLType type;
};

struct CLArg {
  const char *name;
  const char *description;
  const char **storage;
};

typedef struct CLInterfaceDesc CLInterfaceDesc;

struct CLInterfaceDesc {
  const char *program_name;
  const CLOpt *opts;
  const CLArg *positional_args;
  const char *help_header;
  const char *help_footer;
  size_t num_opts;
  size_t num_positional_args;
};

void cl_parse(int argc, char **restrict argv, const CLInterfaceDesc *desc);
void cl_print_help(const CLInterfaceDesc *desc);

#endif /* INCLUDED_CL_H */
