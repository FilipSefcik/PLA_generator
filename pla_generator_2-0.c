#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {

  if (argc != 4) {
    printf("Usage: %s <row_size> <number_of_files> <output_filename>\n", argv[0]);
    return 1;
  }

  int N = atoi(argv[1]);
  int K = atoi(argv[2]);
  char *input_filename = argv[3];

  if (N <= 0 || K <= 0) {
    printf("Numeric arguments must be positive integers.\n");
    return 1;
  }

  srand(time(NULL));

  char base[512];
  char extension[128] = "";

  if (K > 1) {
    char *dot = strrchr(input_filename, '.');

    if (dot) {
      size_t base_len = dot - input_filename;
      strncpy(base, input_filename, base_len);
      base[base_len] = '\0';
      strcpy(extension, dot);
    } else {
      strcpy(base, input_filename);
    }
  }

  for (int file_index = 0; file_index < K; file_index++) {

    char filename[512];

    if (K == 1) {
      strcpy(filename, input_filename);
    } else {
      snprintf(filename, sizeof(filename),
               "%s_%d%s", base, file_index, extension);
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
      printf("Error creating file: %s\n", filename);
      return 1;
    }

    int rows = N * N;
    int cols = N;

    fprintf(f, ".i %d\n", N);
    fprintf(f, ".o 1\n");
    fprintf(f, ".p %d\n\n", rows);

    for (int r = 0; r < rows; r++) {

      int ones = rand() % (N + 1);
      int *used = calloc(N, sizeof(int));
      if (!used) {
        fclose(f);
        return 1;
      }

      for (int i = 0; i < ones; i++) {
        int pos;
        do {
          pos = rand() % N;
        } while (used[pos]);
        used[pos] = 1;
      }

      for (int c = 0; c < cols; c++)
        fprintf(f, used[c] ? "1" : "-");

      fprintf(f, " 1\n");
      free(used);
    }

    fprintf(f, "\n.e\n");
    fclose(f);

    printf("File '%s' created successfully.\n", filename);
  }

  return 0;
}