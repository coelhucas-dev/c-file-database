#include "common.h"
#include "file.h"
#include "parse.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void printUsage(char *argv[]) {
  printf("Usage: %s -f <filePath> -n\n", argv[0]);
  printf("\t -n Create a new file\n");
  printf("\t -f <filePath> (required) Path of the file\n");
  printf("\t -a <, separated values> Struct employee_t separated by ,\n");
}

int main(int argc, char *argv[]) {
  int c = 0;
  char *addstring = NULL;
  char *filepath = NULL;
  bool create_new_file = false;
  int db_fd = -1;
  struct dbheader_t *db_header = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nf:a:")) != -1) {
    switch (c) {
    case 'f':
      filepath = optarg;
      break;
    case 'n':
      create_new_file = true;
      break;
    case 'a':
      addstring = optarg;
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
    if (create_db_header(&db_header) == STATUS_ERROR) {
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

  if (read_employees(db_fd, db_header, &employees) == STATUS_ERROR) {
    printf("Failed to read employees\n");
    close(db_fd);
    return -1;
  }

  if (addstring != NULL) {
    db_header->count++;
    printf("db header%d\n", db_header->count);
    employees =
        realloc(employees, db_header->count * sizeof(struct employee_t));
    printf("emplyees reallocated");
    add_employee(db_header, employees, addstring);
    write(1, employees, sizeof(struct employee_t));
  }

  if (output_file(db_fd, db_header, employees)) {
    printf("Error on outputing the file.\n");
    return -1;
  }

  return 0;
}
