#include <stdio.h>
#include <stdlib.h>
#define log_warn printf
#define log_warnx printf
#define log_debug printf

#include "compress.c" // yes.

int main()
{
    char  original_data[4096];
//  char original_data * = malloc(4096);
    char  *word = "Sakura";
    for (int i = 0;
         i < sizeof(original_data) * sizeof(*original_data) ; ++i)
        original_data[i] = word[i % strlen(word) + 1]; // let's compress the trailing \0
    size_t compressed_size;
    uchar * compressed_data = tnt_compress_sized(original_data,
      sizeof(original_data) * sizeof(*original_data), &compressed_size);
    size_t uncompressed_size;
    uchar *deflated_data = tnt_uncompress_sized(compressed_data, compressed_size, &uncompressed_size);
    if (memcmp(deflated_data, original_data, sizeof(original_data) * sizeof(*original_data)))
    {
        free(deflated_data);
        free(compressed_data);
        printf("FAILED\n");
        return EXIT_FAILURE;
    }
    free(deflated_data);
    free(compressed_data);
    printf("SUCCESS\n");
    return EXIT_SUCCESS;
}
