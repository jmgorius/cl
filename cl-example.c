#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cl.h"

int main(int argc, char *argv[]) {
  const char *input_file = 0;
  const char *output_file = "cloutput.txt";
  const char *uppercase = 0;

  cl_interface_desc desc = {
      .program_name = argv[0],
      .help_header = "cl-example - CL library example",
      .help_footer = "This is free and unencumbered software released into the "
                     "public domain.\nFor more information, please visit "
                     "https://github.com/jmgorius/cl.",
      .opts =
          (cl_opt[]){
              [0] = {.short_name = "o",
                     .long_name = "output",
                     .description = "Output file name",
                     .value_name = "file",
                     .type = CL_VALUE,
                     .storage = &output_file},
              [1] = {.short_name = "U",
                     .description = "Make the output uppercase",
                     .type = CL_FLAG,
                     .storage = &uppercase},
          },
      .num_opts = 2,
      .positional_args =
          (cl_arg[]){
              [0] = {.name = "input-file",
                     .description = "Input file name",
                     .storage = &input_file},
          },
      .num_positional_args = 1,
  };

  cl_parse(argc, argv, &desc);

  FILE *input = fopen(input_file, "rb");
  FILE *output = fopen(output_file, "wb");

  char c;
  while ((c = fgetc(input)) != EOF)
    fputc(uppercase ? toupper(c) : c, output);

  fclose(output);
  fclose(input);

  return 0;
}
