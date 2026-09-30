/**
 * \file main.c
 *
 * \brief Main entry point for the nepe2 CLI tool.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <nepe2/nepe2.h>
#include <stdio.h>

/**
 * \brief Main entry point.
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
    status retval;

    (void)argc;
    (void)argv;

    /* register nepe2 library. */
    retval = nepe2_register();
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Could not register nepe2 library.\n");
        goto done;
    }

    printf("Not yet implemented.\n");
    retval = 1;

done:
    return retval;
}
