/**
* GPU-energy: Display the total energy consumption for NVIDIA's GPU in mJ
*             since the driver was last reloaded.
*
* args.c : Parse program arguments
*
* URL       https://github.com/jyvet/gpu-energy
* License   MIT
* Author    Jean-Yves VET <contact[at]jean-yves.vet>
* Copyright (c) 2022
******************************************************************************/

#ifndef GPU_ENERGY_ARGS_H
#define GPU_ENERGY_ARGS_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define GPU_ENERGY_VERSION      "0.1"
#define GPU_ENERGY_CONTACT      "contact[at]jean-yves.vet"
#define GPU_ENERGY_MAX_DEVICES  16

typedef struct ge_args
{
    bool ids[GPU_ENERGY_MAX_DEVICES];
    bool is_detailed;
    bool is_subset;
} ge_args_t;

void ge_args_retrieve(int argc, char **argv, ge_args_t *out_args);

#endif /* GPU_ENERGY_ARGS_H */

