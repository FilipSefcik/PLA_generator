#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  // Expect exactly 2 arguments: size and number_of_files
  if (argc != 4) {
    printf("Usage: %s <row_size> <number_of_files> <output_file>\n", argv[0]);
    return 1;
  }

  // Convert arguments from string to integer
  int N = atoi(argv[1]);  // row size
  int K = atoi(argv[2]);  // number of files to generate
  char *prefix = argv[3]; // filename prefix (can include path)

  // Basic validation
  if (N <= 0 || K <= 0) {
    printf("Both arguments must be positive integers.\n");
    return 1;
  }

  printf("Row size (N): %d\n", N);
  printf("Number of files to generate (K): %d\n", K);
  printf("Output prefix: %s\n", prefix);

  srand(time(NULL));

  for (int i = 0; i < K; i++) {

    int rows = N * N; // N^2 rows
    int cols = N;     // N columns

    // create filename
    char filename[256];
    snprintf(filename, sizeof(filename), "plas/generated_%d_%d.pla", N, i);

    FILE *f = fopen(filename, "w");
    if (!f) {
      printf("Error creating file.\n");
      return 1;
    }

    // PLA header
    fprintf(f, ".i %d\n", N);
    fprintf(f, ".o 1\n");
    fprintf(f, ".p %d\n\n", rows);

    // generate rows
    for (int r = 0; r < rows; r++) {

      // decide how many '1's will be in this row
      int ones = rand() % (N + 1); // from 0 to N

      // track unique positions
      int *used = (int *)calloc(N, sizeof(int));
      if (!used) { /* handle OOM */
      }

      for (int i = 0; i < ones; i++) {
        int pos;

        // find unused random position
        do {
          pos = rand() % N;
        } while (used[pos] == 1);

        used[pos] = 1;
      }

      // print row
      for (int c = 0; c < cols; c++) {
        if (used[c] == 1)
          fprintf(f, "1");
        else
          fprintf(f, "-");
      }

      fprintf(f, " 1\n"); // output column
      free(used);
    }

    fprintf(f, "\n.e\n");
    fclose(f);

    printf("File '%s' created successfully.\n", filename);
  }
  //   int N;
  //   printf("Enter array size (N): ");
  //   scanf("%d", &N);

  //   int rows = N * N; // N^2 rows
  //   int cols = N;     // N columns

  //   srand(time(NULL));

  //   // create filename
  //   time_t t = time(NULL);
  //   char filename[256];
  //   snprintf(filename, sizeof(filename), "generated_%d_%ld.pla", N, t);

  //   FILE *f = fopen(filename, "w");
  //   if (!f) {
  //     printf("Error creating file.\n");
  //     return 1;
  //   }

  //   // PLA header
  //   fprintf(f, ".i %d\n", N);
  //   fprintf(f, ".o 1\n");
  //   fprintf(f, ".p %d\n\n", rows);

  //   // generate rows
  //   for (int r = 0; r < rows; r++) {

  //     // decide how many '1's will be in this row
  //     int ones = rand() % (N + 1); // from 0 to N

  //     // track unique positions
  //     int *used = calloc(N, sizeof(int));

  //     for (int i = 0; i < ones; i++) {
  //       int pos;

  //       // find unused random position
  //       do {
  //         pos = rand() % N;
  //       } while (used[pos] == 1);

  //       used[pos] = 1;
  //     }

  //     // print row
  //     for (int c = 0; c < cols; c++) {
  //       if (used[c] == 1)
  //         fprintf(f, "1");
  //       else
  //         fprintf(f, "-");
  //     }

  //     fprintf(f, " 1\n"); // output column
  //     free(used);
  //   }

  //   fprintf(f, "\n.e\n");
  //   fclose(f);

  //   printf("File '%s' created successfully.\n", filename);
  return 0;
}
