/**
 * \file legacy2nepe2/main.c
 *
 * \brief Main entry point for the legacy2nepe2 import tool.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <errno.h>
#include <nepe2/error_codes.h>
#include <nepe2/terminal.h>
#include <stdio.h>
#include <sys/stat.h>

#include "legacy_cli_internal.h"

RCPR_IMPORT_resource;

static status read_and_verify_master_passphrase(legacy_cli_instance* inst);
static status verify_master_passphrase(bool* valid, legacy_cli_instance* inst);
static status derive_verification_hash(
    secure_buffer** buffer, legacy_cli_instance* inst);
static status open_database(const char* dbname, legacy_cli_instance* inst);

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
    FILE* input = NULL;

    if (argc != 2)
    {
        fprintf(
            stderr,
            "This program expects one argument: the legacy nepephemeral input "
            "file name.\n");
        retval = 1;
        goto done;
    }

    /* register nepe2 library. */
    retval = nepe2_register();
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Could not register nepe2 library.\n");
        goto done;
    }

    /* open import file for reading. */
    input = fopen(argv[1], "r");
    if (NULL == input)
    {
        perror("Error opening input file");
        retval = 1;
        goto done;
    }

    /* create instance. */
    retval = legacy_cli_instance_create(&inst);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating instance.\n");
        goto cleanup_input;
    }

    /* read salt. */
    printf("Enter salt: ");
    fflush(stdout);
    retval = terminal_readpassphrase(&inst->salt, inst->alloc, 4096, false);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inst;
    }

    /* read master passphrase. */
    retval = read_and_verify_master_passphrase(inst);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inst;
    }

    /* open the database. */
    retval = open_database("legacy2nepe2.db", inst);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inst;
    }

    /* get the encryption salt. */
    retval =
        database_get_or_insert_encryption_salt(
            &inst->encryption_salt, inst->alloc, inst->db);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error getting encryption salt.\n");
        goto cleanup_inst;
    }

    /* derive the encryption key. */
    printf("Deriving database encryption key...\n");
    retval =
        database_create_encryption_key(
            &inst->encryption_key, inst->alloc, inst->master_passphrase,
            inst->encryption_salt);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error deriving encryption key.\n");
        goto cleanup_inst;
    }

    /* import entries. */
    retval = load_metadata_file(inst, input);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error importing legacy metadata.\n");
        goto cleanup_inst;
    }

    /* success. */
    printf("Import successful.\n");
    retval = STATUS_SUCCESS;
    goto cleanup_inst;

cleanup_inst:
    release_retval = resource_release(&inst->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_input:
    fclose(input);

done:
    if (STATUS_SUCCESS != retval)
    {
        return 1;
    }

    return 0;
}

/**
 * \brief Read the master passphrase in a verification loop.
 *
 * \param inst              The instance to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status read_and_verify_master_passphrase(legacy_cli_instance* inst)
{
    status retval;
    bool valid = false;

    /* loop until the user is satisfied. */
    do
    {
        /* read the passphrase. */
        printf("Enter master passphrase: ");
        fflush(stdout);
        retval =
            terminal_readpassphrase(
                &inst->master_passphrase, inst->alloc, 4096, false);
        if (STATUS_SUCCESS != retval)
        {
            goto done;
        }

        /* verify it. */
        retval = verify_master_passphrase(&valid, inst);
        if (STATUS_SUCCESS != retval)
        {
            fprintf(stderr, "Error verifying passphrase.\n");
            goto done;
        }
    } while(!valid);

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

/**
 * \brief Verify the master passphrase by showing the user the verification
 * hash.
 *
 * \param valid             Flag to set to true if this passphrase is valid.
 * \param inst              The instance for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status verify_master_passphrase(bool* valid, legacy_cli_instance* inst)
{
    status retval, release_retval;
    secure_buffer* v;
    const char* v_data;
    size_t v_size;
    int selection;

    /* compute the verification hash. */
    retval = derive_verification_hash(&v, inst);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* loop until we get a valid response. */
    while (1)
    {
        /* output the verification hash. */
        v_data = secure_buffer_data(&v_size, v);
        printf("Verification hash: %s\n", v_data);

        /* prompt the user to verify. */
        retval = terminal_readchoice(&selection, "Look okay", 'y', 'n', -1);
        if (ERROR_TERMINAL_BAD_CHOICE == retval)
        {
            fprintf(stderr, "Invalid response. Try again.\n\n");
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
                        secure_buffer_resource_handle(inst->master_passphrase));
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

done:
    return retval;
}

/**
 * \brief Derive the verification hash string to show the user.
 *
 * \param buffer            Pointer to the \ref secure_buffer pointer where this
 *                          verification hash is stored on success.
 * \param inst              The instance for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status derive_verification_hash(
    secure_buffer** buffer, legacy_cli_instance* inst)
{
    status retval, release_retval;
    secure_buffer *hash1, *tmp;
    transformer* xform;
    mapper* m;

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

    /* perform the legacy transformation to derive the hash. */
    retval =
        transformer_transform(
            &hash1, xform, inst->alloc, inst->master_passphrase, inst->salt,
            NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* map this value into a Base64 fingerprint. */
    retval = mapper_map(&tmp, m, inst->alloc, hash1);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash1;
    }

    /* success. */
    *buffer = tmp;
    retval = STATUS_SUCCESS;
    goto cleanup_hash1;

cleanup_hash1:
    release_retval = resource_release(secure_buffer_resource_handle(hash1));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

/**
 * \brief Open the database using the given database name.
 *
 * \note This function will attempt to create \p dbname as a directory before
 * opening the database.
 *
 * \param dbname                The name of the directory where this database
 *                              lives.
 * \param inst                  The instance for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status open_database(const char* dbname, legacy_cli_instance* inst)
{
    status retval;

    /* create the directory if it does not already exist. */
    retval = mkdir(dbname, S_IRWXU);
    if (retval < 0)
    {
        if (EEXIST != errno)
        {
            fprintf(stderr, "Error creating database directory.\n");
            retval = ERROR_DATABASE_MDB_ENV_OPEN;
            goto done;
        }
    }

    /* open the database. */
    retval = database_open(&inst->db, inst->alloc, dbname);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error opening database.\n");
        goto done;
    }

    /* verify the schema. */
    retval = database_check_or_insert_schema(inst->db);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Invalid database schema version.\n");
        goto done;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}
