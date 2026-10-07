/**
 * \file legacy2nepe2/main.c
 *
 * \brief Main entry point for the legacy2nepe2 import tool.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <nepe2/terminal.h>
#include <stdio.h>

#include "legacy_cli_internal.h"

RCPR_IMPORT_resource;

/**
 * \brief legacy2nepe2 entry point.
 *
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - 0 on success.
 *      - non-zero on failure.
 */
int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    status retval, release_retval;
    legacy_cli_instance* inst;

    /* register nepe2 library. */
    retval = nepe2_register();
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Could not register nepe2 library.\n");
        goto done;
    }

    /* create instance. */
    retval = legacy_cli_instance_create(&inst);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating instance.\n");
        goto done;
    }

    /* read salt. */
    printf("Enter salt: ");
    fflush(stdout);
    retval = terminal_readpassphrase(&inst->salt, inst->alloc, 4096, false);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inst;
    }

    printf("Not yet implemented.\n");
    retval = 1;
    goto cleanup_inst;

cleanup_inst:
    release_retval = resource_release(&inst->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    if (STATUS_SUCCESS != retval)
    {
        return 1;
    }

    return 0;
}
