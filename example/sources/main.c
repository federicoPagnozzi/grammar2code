#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "general.h"
#include "specific.h"

int main(int argc, char **argv)
{
    load_from_file(argv[1]);
    int max_steps = atoi(argv[2]);
    srand(atoi(argv[3]));

    float result = sls_algorithm(max_steps);

    printf("Result: %f\n", result);
}
