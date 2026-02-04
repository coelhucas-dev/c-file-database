#include "parse.h"
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void list_employees(struct dbheader_t *header, struct employee_t *employees) {
  int i = 0;
  for (; i < header->count; i++) {
    printf("Employee %d:\n", i);
    printf("\tName: %s\n", employees[i].name);
    printf("\tAddress: %s\n", employees[i].address);
    printf("\tHours worked: %d\n", employees[i].hours);
  }
}

int add_employee(struct dbheader_t *header, struct employee_t **employees,
                 char *addstring) {

  if (header == NULL)
    return STATUS_ERROR;
  if (employees == NULL)
    return STATUS_ERROR;
  if (*employees == NULL)
    return STATUS_ERROR;
  if (addstring == NULL)
    return STATUS_ERROR;

  struct employee_t *e = *employees;
  e = realloc(e, sizeof(struct employee_t) * header->count + 1);

  if (e == NULL) {
    printf("Error while allocating memory to add employee\n");
    return STATUS_ERROR;
  }

  header->count++;

  char *name = strtok(addstring, ",");
  if (name == NULL)
    return STATUS_ERROR;
  char *address = strtok(NULL, ",");
  if (address == NULL)
    return STATUS_ERROR;
  char *hours = strtok(NULL, ",");
  if (hours == NULL)
    return STATUS_ERROR;

  strncpy(e[header->count - 1].name, name, sizeof(e[header->count - 1].name));
  e[header->count - 1].name[NAME_LEN - 1] = '\0';
  strncpy(e[header->count - 1].address, address,
          sizeof(e[header->count - 1].address));

  e[header->count - 1].address[ADDRESS_LEN - 1] = '\0';
  e[header->count - 1].hours = atoi(hours);

  *employees = e;

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
  }
  return STATUS_SUCCESS;
}
