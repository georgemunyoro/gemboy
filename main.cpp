#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/tester.h"
#include "src/cpu.h"
#include "src/mem.h"

Memory g_Memory = Memory();
CPU g_CPU(&g_Memory);

// extern struct tester_operations test_ops;
struct tester_operations tt_ops = {
    .init = mycpu_init,
    .set_state = mycpu_set_state,
    .get_state = mycpu_get_state,
    .step = mycpu_step,
};

static struct tester_flags flags = {
    .keep_going_on_mismatch = 0,
    .enable_cb_instruction_testing = 1,
    .print_tested_instruction = 0,
    .print_verbose_inputs = 0,
};

static void print_usage(char *progname) {
  printf("Usage: %s [option]...\n\n", progname);
  printf("Game Boy Instruction Tester.\n\n");
  printf("Options:\n");
  printf(
      " -k, --keep-going       Skip to the next instruction on a mismatch "
      "(instead of aborting all tests).\n");
  printf(
      " -c, --no-enable-cb     Disable testing of CB prefixed "
      "instructions.\n");
  printf(" -p, --print-inst       Print instruction undergoing tests.\n");
  printf(" -v, --print-input      Print every inputstate that is tested.\n");
  printf(" -h, --help             Show this help.\n");
}

static int parse_args(int argc, char **argv) {
  while (1) {
    static struct option long_options[] = {
        {"keep-going", no_argument, 0, 'k'},
        {"no-enable-cb", no_argument, 0, 'c'},
        {"print-inst", no_argument, 0, 'p'},
        {"print-input", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    char c = getopt_long(argc, argv, "kcpvh", long_options, NULL);

    if (c == -1) break;

    switch (c) {
      case 'k':
        flags.keep_going_on_mismatch = 1;
        break;

      case 'c':
        flags.enable_cb_instruction_testing = 0;
        break;

      case 'p':
        flags.print_tested_instruction = 1;
        break;

      case 'v':
        flags.print_verbose_inputs = 1;
        break;

      case 'h':
        print_usage(argv[0]);
        exit(0);

      default:
        print_usage(argv[0]);
        return 1;
    }
  }

  if (optind != argc) {
    /* We should not have any leftover arguments. */
    print_usage(argv[0]);
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {
  if (parse_args(argc, argv)) return 1;

  g_Memory.shouldWriteToMemory = false;

  return tester_run(&flags, &tt_ops);
}
