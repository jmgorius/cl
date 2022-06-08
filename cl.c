#include "cl.h"

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_attribute(noreturn)
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#if __has_attribute(format)
#define PRINTF_FORMAT(fmt, args) __attribute__((__format__(printf, fmt, args)))
#else
#define PRINTF_FORMAT(fmt, args)
#endif

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

static const char *set_constant = "set";

static const cl_opt help_opt = {
    .short_name = "h",
    .long_name = "help",
    .description = "Print this help message and exit",
};

NORETURN
PRINTF_FORMAT(2, 3)
static void error_exit(const char *restrict program_name,
                       const char *restrict fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (isatty(STDERR_FILENO))
    fprintf(stderr, "\033[31merror\033[0m: ");
  else
    fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\nTry %s --help for more information\n", program_name);
  exit(1);
}

static void print_opt(const cl_opt *opt) {
  int print_size = 0;
  bool is_value = opt->type == CL_VALUE;
  printf("  %s%s%s%s%s%s%s%s%s %n", opt->short_name ? "-" : " ",
         opt->short_name ? opt->short_name : " ",
         (opt->short_name && opt->long_name) ? ", " : "",
         opt->long_name ? (opt->short_name ? "--" : "  --") : "",
         opt->long_name ? opt->long_name : "",
         is_value ? (opt->long_name ? "=" : " ") : "", is_value ? "<" : "",
         is_value ? (opt->value_name ? opt->value_name : "value") : "",
         is_value ? ">" : "", &print_size);
  if (print_size > CL_HELP_ALIGNMENT) {
    printf("\n%" STRINGIFY(CL_HELP_ALIGNMENT) "s%s\n", "", opt->description);
  } else {
    printf("%*s%s\n", CL_HELP_ALIGNMENT - print_size, "", opt->description);
  }
}

void cl_print_help(const cl_interface_desc *desc) {
  if (desc->help_header)
    printf("%s\n\n", desc->help_header);
  printf("USAGE: %s", desc->program_name);
  if (desc->num_opts > 0)
    printf(" [options] [--]");

  for (size_t i = 0; i < desc->num_positional_args; ++i)
    printf(" <%s>", desc->positional_args[i].name);
  putc('\n', stdout);

  if (desc->num_positional_args > 0) {
    puts("\nARGUMENTS:");
    for (size_t i = 0; i < desc->num_positional_args; ++i)
      printf("  %-*s %s\n", CL_HELP_ALIGNMENT - 3,
             desc->positional_args[i].name,
             desc->positional_args[i].description);
  }

  if (desc->num_opts > 0) {
    puts("\nOPTIONS:");

    print_opt(&help_opt);
    for (size_t i = 0; i < desc->num_opts; ++i)
      print_opt(&desc->opts[i]);
  }

  if (desc->help_footer)
    printf("\n%s\n", desc->help_footer);
}

static bool is_dash_dash(const char *s) {
  return s[0] == '-' && s[1] == '-' && s[2] == '\0';
}
static bool is_short_opt(const char *s) {
  return s[0] == '-' && s[1] != '-' && s[1] != '\0';
}
static bool is_long_opt(const char *s) {
  return s[0] == '-' && s[1] == '-' && s[2] != '-' && s[2] != '\0';
}

static const cl_opt *get_long_option(const cl_interface_desc *desc,
                                     const char *restrict argv_str,
                                     const char **restrict value) {
  for (size_t i = 0; i < desc->num_opts; ++i) {
    if (desc->opts[i].long_name) {
      size_t n = strlen(desc->opts[i].long_name);
      if (strncmp(desc->opts[i].long_name, argv_str, n) == 0) {
        if (argv_str[n] == '=')
          *value = argv_str + n + 1;
        else
          *value = 0;
        return &desc->opts[i];
      }
    }
  }

  if (strcmp(help_opt.long_name, argv_str) == 0) {
    cl_print_help(desc);
    exit(0);
  }

  return 0;
}

static const cl_opt *get_short_option(const cl_interface_desc *desc,
                                      const char *restrict argv_str) {
  for (size_t i = 0; i < desc->num_opts; ++i) {
    if (desc->opts[i].short_name) {
      size_t n = strlen(desc->opts[i].short_name);
      if (strncmp(desc->opts[i].short_name, argv_str, n) == 0)
        return &desc->opts[i];
    }
  }

  if (strcmp(help_opt.short_name, argv_str) == 0) {
    cl_print_help(desc);
    exit(0);
  }

  return 0;
}

void cl_parse(int argc, char **restrict argv, const cl_interface_desc *desc) {
  const char *program_name = argv[0];
  /* Ignore the program name */
  int idx = 1;
  while (idx < argc) {
    if (is_dash_dash(argv[idx])) {
      idx += 1;
      break;
    }
    if (is_long_opt(argv[idx])) {
      const char *value = 0;
      const cl_opt *option = get_long_option(desc, argv[idx] + 2, &value);
      if (!option) {
        error_exit(program_name, "Unknown option %s", argv[idx]);
      } else {
        if ((option->type != CL_VALUE) && value)
          error_exit(program_name, "Unexpected value for option --%s",
                     option->long_name);
        if (option->type == CL_VALUE) {
          if (value)
            *option->storage = value;
          else if (idx + 1 < argc && !is_dash_dash(argv[idx + 1]))
            *option->storage = argv[++idx];
          else
            error_exit(program_name, "Missing option value for --%s",
                       option->long_name);
        } else {
          assert(option->type == CL_FLAG &&
                 "Option with no value isn't a valid flag");
          *option->storage = set_constant;
        }
      }
    } else if (is_short_opt(argv[idx])) {
      const char *options = argv[idx] + 1;
      size_t len = strlen(argv[idx]);
      size_t opt_idx = idx;
      /* Handle potentially grouped short options */
      while (options < argv[opt_idx] + len) {
        const cl_opt *option = get_short_option(desc, options);
        if (!option)
          error_exit(program_name, "Unknown option %s", argv[idx]);
        options += strlen(option->short_name);
        if (option->type == CL_VALUE) {
          if ((options == argv[idx] + len) && (idx + 1 < argc))
            *option->storage = argv[++idx];
          else
            error_exit(program_name, "Missing option value for -%s",
                       option->short_name);
        } else {
          assert(option->type == CL_FLAG &&
                 "Option with no value isn't a valid flag");
          *option->storage = set_constant;
        }
      }
    } else
      break;
    idx += 1;
  }

  if (argc - idx != (int)desc->num_positional_args)
    error_exit(program_name, "Unexpected number of arguments");

  /* From here on, there should only be positional arguments */
  for (int arg = 0; arg < argc - idx; ++arg)
    *desc->positional_args[arg].storage = argv[idx + arg];
}
