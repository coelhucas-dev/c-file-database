#include "parse.h"
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int add_employee(struct dbheader_t *header, struct employee_t *employees,
                 char *addstring) {

  char *name = strtok(addstring, ",");
  char *address = strtok(NULL, ",");
  char *hours = strtok(NULL, ",");

  strncpy(employees[header->count - 1].name, name,
          sizeof(employees[header->count - 1].name));
  strncpy(employees[header->count - 1].address, address,
          sizeof(employees[header->count - 1].address));
  employees[header->count - 1].hours = atoi(hours);

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header,
                   struct employee_t **employeesOut) {
  if (fd == -1) {
    printf("Bad file descriptor, returning STATUS_ERROR\n");
    return STATUS_ERROR;
  }

  int count = header->count;
  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    printf("Failed allocating memory for employees\n");
    return STATUS_ERROR;
  }

  if (read(fd, employees, count * sizeof(struct employee_t)) < 0) {
    printf("Failed reading employees from file\n");
    free(employees);
    return STATUS_ERROR;
  }
  int i = 0;
  for (; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;
  return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }
  header->version = 1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Bad file descriptor, returning STATUS_ERROR\n");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

  if (header == NULL) {
    printf("Failed to allocate memory for DB Header\n");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) !=
      sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->filesize = ntohl(header->filesize);
  header->magic = ntohl(header->magic);

  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }
  if (header->magic != HEADER_MAGIC) {
    printf("Improper header magic\n");
    free(header);
    return STATUS_ERROR;
  }

  struct stat dbstat = {};
  fstat(fd, &dbstat);

  if (header->filesize != dbstat.st_size) {
    printf("File corrupted\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *header,
                struct employee_t *employees) {
  if (fd < 0) {
    printf("Bad file descriptor, returning STATUS_ERROR\n");
    return STATUS_ERROR;
  }

  int realCount = header->count;

  header->magic = htonl(header->magic);
  header->count = htons(header->count);
  header->filesize =
      htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realCount);
  header->version = htons(header->version);

  lseek(fd, 0, SEEK_SET);

  if (write(fd, header, sizeof(struct dbheader_t)) < 0) {
    perror("write header");
    return STATUS_ERROR;
  }

  int i = 0;

  for (; i < realCount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    if (write(fd, &employees[i], sizeof(struct employee_t)) < 0) {
      perror("write employee");
      return STATUS_ERROR;
    }

    write(1, &employees[i], sizeof(struct employee_t));
  }
  printf("Success\n");
  return STATUS_SUCCESS;
}
