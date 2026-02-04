#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "file.h"

int create_db_file(char *filepath) {
  int fd = open(filepath, O_RDWR);

  if (fd != -1) {
    return fd;
  }

  fd = open(filepath, O_RDWR | O_CREAT, 0644);

  if (fd == -1) {
    perror("open");
    return STATUS_ERROR;
  }

  return fd;
}

int open_db_file(char *filepath) {
  int fd = open(filepath, O_RDWR);

  if (fd == -1) {
    perror("open");
    return STATUS_ERROR;
  }

  return fd;
}
