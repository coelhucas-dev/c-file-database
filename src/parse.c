#include "parse.h"
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int create_db_header(int fd, struct dbheader_t **headerOut) {
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

  return STATUS_SUCESS;
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
  header->count = ntohl(header->count);
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

  return STATUS_SUCESS;
}

int output_file(int fd, struct dbheader_t *header) {
  if (fd < 0) {
    printf("Bad file descriptor, returning STATUS_ERROR\n");
    return STATUS_ERROR;
  }

  header->magic = htonl(header->magic);
  header->count = htonl(header->count);
  header->filesize = htonl(header->filesize);
  header->version = htons(header->version);

  lseek(fd, 0, SEEK_SET);

  write(fd, header, sizeof(struct dbheader_t));

  return STATUS_SUCESS;
}
