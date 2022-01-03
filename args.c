/**
* GPU-energy: Display the total energy consumption for NVIDIA's GPU in mJ
*             since the driver was last reloaded.
* args.c : Parse program arguments
*
* URL       https://github.com/jyvet/gpu-energy
* License   MIT
* Author    Jean-Yves VET <contact[at]jean-yves.vet>
* Copyright (c) 2022
******************************************************************************/

#include <argp.h>
#include "args.h"

/* Expand macro values to string */
#define STR_VALUE(var)   #var
#define STR(var)         STR_VALUE(var)

const char *argp_program_version = "GPU-energy "GPU_ENERGY_VERSION;
const char *argp_program_bug_address = "<"GPU_ENERGY_CONTACT">";

/* Program documentation */
static char doc[] = "This application displays the total energy consumption of "
    "GPU(s) in mJ since the driver was last reloaded. By default, total energy "
    "consumption counters for all available NVIDIA GPUs are added up. Use the --gpu "
    "argument to select a subset of GPU(s). You'll need to execute this application "
    "twice in order to estimate the power consumption during a given time frame. "
    "It accepts the following optional arguments:";

/* A description of the arguments we accept */
static char args_doc[] = "";

/* Options */
static struct argp_option options[] =
{
    {"gpu", 'g', "<list of ids>", 0, "Provide comma-separated GPU IDs to speficy which "
                                     "GPUs to use (use all available GPUs by default)."},
    {"csv", 'c', 0,               0, "Detailed output in CSV format (one line per GPU)."},
    { 0 }
};

/* Parse a single option */
static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
    const char* tok;
    ge_args_t *args = state->input;

    switch (key)
    {
        case 'c':
            args->is_detailed = true;
            break;
        case 'g':
            args->is_subset = true;

            /* Parse list and mark all enabled IDs */
            for (tok = strtok(arg, ","); tok && *tok; tok = strtok(NULL, ",\n"))
            {
                char *endptr;
                long pos = strtol(tok, &endptr, 10);
                if (errno == EINVAL || errno == ERANGE || tok == endptr ||
                    pos < 0 || pos >= GPU_ENERGY_MAX_DEVICES)
                {
                    fprintf(stderr, "Error: --gpu argument only accepts a list of ids "
                                    "(comma-separated) from 0 to %d. Exit.\n",
                                    GPU_ENERGY_MAX_DEVICES - 1);
                    exit(1);
                }
                args->ids[pos] = true;
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* Argp parser */
static struct argp argp = { options, parse_opt, args_doc, doc };

/**
 * Parse arguments
 *
 * @param   argc[in]        Amount of arguments
 * @param   argv[in]        Array of arguments
 * @param   out_args[out]   Parsed arguments
 */
void
ge_args_retrieve(int argc, char **argv, ge_args_t *out_args)
{
    memset(out_args, 0, sizeof(ge_args_t));
    argp_parse(&argp, argc, argv, 0, 0, out_args);
}
