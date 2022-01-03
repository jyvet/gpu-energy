/**
* GPU-energy: Display the total energy consumption for NVIDIA's GPU in mJ
*             since the driver was last reloaded.
*
* URL       https://github.com/jyvet/gpu-energy
* License   MIT
* Author    Jean-Yves VET <contact[at]jean-yves.vet>
* Copyright (c) 2022
******************************************************************************/

#include "dcgm_agent.h"
#include "dcgm_structs.h"
#include <stdio.h>
#include <stdlib.h>

//gcc gpu_total_energy.c -o gpu_total_energy -ldcgm

/**
 * Without providing the -d argument, aggregated total cosumption will be provided
 * (sum of the TOTAL_ENERGY_CONSUMPTION counters from selected GPUs).
 * You can use the provided script to display the power consumption of your GPU(s) for a given JOB
 */

/* TODO:
 * -a, --all (all available GPUs) and -g, --gpu (given indexes GPUs)
 * -d, --detailed for detailed view per GPU (csv format)
 * ID, serial number, and GPU model in detailed view
 **/

static uint64_t total_energy[DCGM_MAX_NUM_DEVICES] = { 0 };

/**
 * Initialize DCGM with embedded mode.
 */
static void
dcgm_ini(dcgmHandle_t *handle)
{
    dcgmReturn_t ret;

    /* Initialize DCGM */
    ret = dcgmInit();
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Unable initialize DCGM engine: %s\n", errorString(ret));
        exit(EXIT_FAILURE);
    }

    /* Use embedded mode here */
    ret = dcgmStartEmbedded(DCGM_OPERATION_MODE_MANUAL, handle);
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Unable to start embedded DCGM engine: %s\n", errorString(ret));
        dcgmShutdown();
        exit(EXIT_FAILURE);
    }
}

/**
 * Cleanup DCGM.
 */
/**
 * Cleanup DCGM.
 */
static void
dcgm_fini(dcgmHandle_t *handle)
{
    dcgmStopEmbedded(*handle);
    dcgmShutdown();
}

/**
 * Fetch all available GPUs and select proper subset.
 */
int
dgcm_select_devices(dcgmHandle_t *handle)
{
    uint32_t gpu_ids[DCGM_MAX_NUM_DEVICES];
    dcgmReturn_t ret;

    /* Fetch all available devices */
    int count;
    ret = dcgmGetAllSupportedDevices(*handle, gpu_ids, &count);
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Unable to list devices: %s\n", errorString(ret));
        dcgm_fini(handle);
        exit(EXIT_FAILURE);
    }

    if (count == 0)
    {
        fprintf(stderr, "No supported GPU: %s\n", errorString(ret));
        dcgm_fini(handle);
        exit(EXIT_FAILURE);
    }

    return count;
}

static int
get_total_energy(unsigned int gpu_id, dcgmFieldValue_v1 *field, int num_values, void *user_data)
{
    total_energy[gpu_id] = field[0].value.i64;
    return 0;
}

/**
 * Create a group, a field group for the total energy consuption and fetch
 * values for each GPUs.
 */
void
dgcm_fetch_energy_metric(dcgmHandle_t *handle)
{
    dcgmGpuGrp_t group_id = { 0 };
    char group_name[]     = "energy_group";
    dcgmFieldGrp_t field_group_id;

    /* Total energy consumption for the GPU in mJ since the driver was last reloaded */


    unsigned short field_id = DCGM_FI_DEV_TOTAL_ENERGY_CONSUMPTION;
    dcgmReturn_t ret;

    /* Create a group. */
    ret = dcgmGroupCreate(*handle, DCGM_GROUP_DEFAULT, group_name, &group_id);
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Cannot create a DGCM group: %s\n", errorString(ret));
        goto exit;
    }

    /* Create a field group. */
    ret = dcgmFieldGroupCreate(*handle, 1, &field_id, (char *)"TOTAL_ENERGY", &field_group_id);
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Cannot create a DGCM field group: %s\n", errorString(ret));
        goto exit;
    }

    /* Set a watch on the energy consumption field */
    ret = dcgmWatchFields(*handle, group_id, field_group_id, 100000, 60.0, 100);
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Cannot set DGCM field watch: %s\n", errorString(ret));
        goto exit;
    }

    /* Force fields update */
    dcgmUpdateAllFields(*handle, 1);

    /* Stop the watch */
    dcgmUnwatchFields(*handle, group_id, field_group_id);

    /* Retrieve the total energy consumption for all selected devices */
    ret = dcgmGetLatestValues(*handle, group_id, field_group_id, &get_total_energy, NULL);
    if (ret != DCGM_ST_OK)
    {
        fprintf(stderr, "Cannot get latest values: %s\n", errorString(ret));
        goto exit;
    }

exit:
    dcgmGroupDestroy(*handle, group_id);

    if (ret != DCGM_ST_OK)
    {
    dcgm_fini(handle);
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char *argv[])
{
    uint64_t sum_total_energy = 0;
    dcgmHandle_t handle = { 0 };
    ge_args_t args;
    ge_args_retrieve(argc, argv, &args);

    dcgm_ini(&handle);

    const int count = dgcm_select_devices(&handle);

    dgcm_fetch_energy_metric(&handle);

    /* Print CSV header */
    //printf("#GPU ID;Total energy(mJ)\n");
    for (int i = 0; i <= DCGM_MAX_NUM_DEVICES; i++)
    {
        if (total_energy[i] != 0)
        //printf("%d;%lu\n", i, total_energy[i]);
        sum_total_energy += total_energy[i];
    }

    //printf("%lu (sum %d GPUs)\n", sum_total_energy, count);
    printf("%lu\n", sum_total_energy);

    dcgm_fini(&handle);
    return 0;
}
