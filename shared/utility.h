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
char* trim(char* input, char trim)                                         
{
    int i,j;
    char *output=input;
    for (i = 0, j = 0; i<strlen(input); i++,j++)          
    {
        if (input[i]!=(int)trim)                           
            output[j]=input[i];                     
        else
            j--;                                     
    }
    output[j]=0;
    return output;
}

// stdlib atoi causes ram/rom overflow
// https://www.geeksforgeeks.org/write-your-own-atoi/
int simple_atoi(char* str) 
{ 
    int res = 0; // Initialize result 
  
    // Iterate through all characters of input string and 
    // update result 
    int i;
    for (i = 0; str[i] != '\0'; ++i) 
        res = res * 10 + str[i] - '0'; 
  
    // return result. 
    return res; 
} 

float simple_atof(const char* s){
  float rez = 0, fact = 1;
  if (*s == '-'){
    s++;
    fact = -1;
  };
  int point_seen;
  for (point_seen = 0; *s; s++){
    if (*s == '.'){
      point_seen = 1; 
      continue;
    };
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) fact /= 10.0f;
      rez = rez * 10.0f + (float)d;
    };
  };
  return rez * fact;
}
