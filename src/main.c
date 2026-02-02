#include "common.h"
#include "file.h"
#include "parse.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>

void printUsage(char *argv[]) {
  printf("Usage: %s -f <filePath> -n\n", argv[0]);
  printf("\t -n Create a new file\n");
  printf("\t -f <filePath> (required) Path of the file");
}

int main(int argc, char *argv[]) {
  int c = 0;
  char *filepath = NULL;
  bool create_new_file = false;
  int db_fd = -1;
  struct dbheader_t *db_header = NULL;

  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
    case 'f':
      filepath = optarg;
      break;
    case 'n':
      create_new_file = true;
      break;
    case '?':
      printf("unkown flag -%c\n", c);
      printUsage(argv);
      break;
    default:
      return -1;
    }
  }

  if (filepath == NULL) {
    printf("File Path is a required argument\n");
    printUsage(argv);
    return -1;
  }

  if (create_new_file) {
    db_fd = create_db_file(filepath);
    if (db_fd == STATUS_ERROR) {
      printf("Unable to create db file\n");
      return -1;
    }
    if (create_db_header(db_fd, &db_header) == STATUS_ERROR) {
      printf("Failed on creating db header\n");
      return -1;
    }

  } else {
    db_fd = open_db_file(filepath);
    if (db_fd == STATUS_ERROR) {
      printf("Unable to open db file\n");
      return -1;
    }
    if (validate_db_header(db_fd, &db_header) == STATUS_ERROR) {
      printf("Error on validating db header\n");
      return -1;
    }
  }

  if (output_file(db_fd, db_header)) {
    printf("Error on outputing the file.\n");
    return -1;
  }

  return 0;
}
