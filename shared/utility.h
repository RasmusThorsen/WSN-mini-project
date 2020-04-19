#include "sys/energest.h"
#include <stdio.h>

static unsigned long
to_seconds(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
}

void
energest_report(void) 
{
    energest_flush();
    printf("\nEnergest:\n");
    printf(" CPU          %lu LPM      %lu DEEP LPM %lu  Total time %lu\n",
        to_seconds(energest_type_time(ENERGEST_TYPE_CPU)),
        to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
        to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
        to_seconds(ENERGEST_GET_TOTAL_TIME()));
    printf(" Radio LISTEN %lu TRANSMIT %lu OFF      %lu \n",
        to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
        to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
        to_seconds(ENERGEST_GET_TOTAL_TIME() - energest_type_time(ENERGEST_TYPE_TRANSMIT) - energest_type_time(ENERGEST_TYPE_LISTEN)));
}

// Removes colons from a char-array.
char* remove_colon(char* input)                                         
{
    int i,j;
    char *output=input;
    for (i = 0, j = 0; i<strlen(input); i++,j++)          
    {
        if (input[i]!=':')                           
            output[j]=input[i];                     
        else
            j--;                                     
    }
    output[j]=0;
    return output;
}

// void string_to_int_array(char *source_array, int *dest_array, unsigned int dest_len) {
//     int i = 0;
//     Get the first token from the string
//     char *tok = strtok(source_array, ",");
//     // Keep going until we run out of tokens
//     while (tok) {
//         // Don't overflow your target array
//         if (i < dest_len) {
//             // Convert to integer and store it
//             // dest_array[i++] = atoi(tok);
//         }
//         // Get the next token from the string - note the use of NULL
//         // instead of the string in this case - that tells it to carry
//         // on from where it left off.
//         tok = strtok(NULL, ",");
//     }
// }

