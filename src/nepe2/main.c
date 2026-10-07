/**
 * \file nepe2/main.c
 *
 * \brief Main entry point for the nepe2 CLI tool.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <ctype.h>
#include <nepe2/error_codes.h>
#include <nepe2/terminal.h>
#include <stdio.h>

#include "nepe2_cli_internal.h"

RCPR_IMPORT_resource;

static status verify_master_passphrase(bool* valid, nepe2_cli_instance* inst);

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
    (void)argc;
    (void)argv;
    status retval, release_retval;
    nepe2_cli_instance* inst;
    bool master_passphrase_valid = false;

    /* register nepe2 library. */
    retval = nepe2_register();
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Could not register nepe2 library.\n");
        goto done;
    }

    /* create instance. */
    retval = nepe2_cli_instance_create(&inst);
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

    do
    {
        /* read master passphrase. */
        printf("Enter master passphrase: ");
        fflush(stdout);
        retval =
            terminal_readpassphrase(
                &inst->master_passphrase, inst->alloc, 4096, false);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_inst;
        }

        /* verify that this passphrase looks right. */
        retval = verify_master_passphrase(&master_passphrase_valid, inst);
        if (STATUS_SUCCESS != retval)
        {
            fprintf(stderr, "Error verifying passphrase.\n");
            goto cleanup_inst;
        }
    } while (!master_passphrase_valid);

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

/**
 * \brief Verify the master passphrase by showing the user the verification
 * token.
 *
 * \param valid         Flag to set to true if this passphrase is valid.
 * \param inst          The instance for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status verify_master_passphrase(bool* valid, nepe2_cli_instance* inst)
{
    status retval, release_retval;
    secure_buffer *hash1, *v;
    transformer* xform;
    mapper* m;
    const char* v_data;
    size_t v_size;
    int selection;

    /* look up the legacy transformer. */
    retval = transformer_registry_lookup(&xform, "legacy");
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* look up the base64 mapper. */
    retval = mapper_registry_lookup(&m, "SYMBOLIC-Base64");
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* perform the legacy transformation. */
    retval =
        transformer_transform(
            &hash1, xform, inst->alloc, inst->master_passphrase,
            inst->salt, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* map this value. */
    retval = mapper_map(&v, m, inst->alloc, hash1);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash1;
    }

    while (1)
    {
        /* output the verification. */
        v_data = secure_buffer_data(&v_size, v);
        printf("Verification hash: %s\n", v_data);

        /* prompt the user. */
        retval = terminal_readchoice(&selection, "Look okay", 'y', 'n', -1);
        if (ERROR_TERMINAL_BAD_CHOICE == retval)
        {
            printf("Invalid response. Try again.\n\n");
            continue;
        }
        else if (STATUS_SUCCESS != retval)
        {
            goto cleanup_v;
        }

        switch (selection)
        {
            case 'y':
                *valid = true;
                goto cleanup_v;

            case 'n':
            default:
                retval =
                    resource_release(
                        secure_buffer_resource_handle(
                            inst->master_passphrase));
                inst->master_passphrase = NULL;
                *valid = false;
                goto cleanup_v;
        }
    }

cleanup_v:
    release_retval = resource_release(secure_buffer_resource_handle(v));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_hash1:
    release_retval = resource_release(secure_buffer_resource_handle(hash1));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
