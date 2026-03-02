#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {

  if (argc != 4) {
    printf("Usage: %s <row_size> <number_of_files> <output_filename>\n",
           argv[0]);
    return 1;
  }

  int N = atoi(argv[1]);
  int K = atoi(argv[2]);
  char *input_filename = argv[3];

  if (N <= 0 || K <= 0) {
    printf("Numeric arguments must be positive integers.\n");
    return 1;
  }

  srand((unsigned)time(NULL));

  const int rows = N * N;
  const int cols = N;

  /* ---- Parse filename once ---- */

  char base[512];
  char extension[128] = "";
  int multiple = (K > 1);

  if (multiple) {
    char *dot = strrchr(input_filename, '.');
    if (dot) {
      size_t base_len = dot - input_filename;
      memcpy(base, input_filename, base_len);
      base[base_len] = '\0';
      strcpy(extension, dot);
    } else {
      strcpy(base, input_filename);
    }
  }

  char filename[512];

  /* ---- Generate files ---- */

  for (int file_index = 0; file_index < K; file_index++) {

    if (multiple) {
      snprintf(filename, sizeof(filename), "%s_%d%s", base, file_index,
               extension);
    } else {
      strcpy(filename, input_filename);
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
      printf("Error creating file: %s\n", filename);
      return 1;
    }

    fprintf(f, ".i %d\n", N);
    fprintf(f, ".o 1\n");
    fprintf(f, ".p %d\n\n", rows);

    /* ---- Allocate row buffer ONCE per file ---- */

    int *used = malloc(N * sizeof(int));
    if (!used) {
      fclose(f);
      return 1;
    }

    for (int r = 0; r < rows; r++) {

      /* Reset buffer (faster than calloc per row) */
      memset(used, 0, N * sizeof(int));

      int ones = rand() % (N + 1);

      /* Fill unique random positions */
      for (int i = 0; i < ones; i++) {
        int pos;
        do {
          pos = rand() % N;
        } while (used[pos]);
        used[pos] = 1;
      }

      /* Print row */
      for (int c = 0; c < cols; c++)
        fputc(used[c] ? '1' : '-', f);

      fputs(" 1\n", f);
    }

    free(used);

    fputs("\n.e\n", f);
    fclose(f);

    printf("File '%s' created successfully.\n", filename);
  }

  return 0;
}