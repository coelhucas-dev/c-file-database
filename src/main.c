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
  char *filePath = NULL;
  bool createNewFile = false;
  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
    case 'f':
      filePath = optarg;
      break;
    case 'n':
      createNewFile = true;
      break;
    case '?':
      printf("unkown flag -%c\n", c);
      printUsage(argv);
      break;
    default:
      return -1;
    }
  }

  if (filePath == NULL) {
    printf("File Path is a required argument");
    printUsage(argv);
    return -1;
  }

  return 0;
}
