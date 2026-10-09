/**
 * \file legacy2nepe2/load_metadata_file.c
 *
 * \brief Load a metadata file into the database.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <rcpr/string.h>
#include <stdlib.h>

#include "legacy_cli_internal.h"

RCPR_IMPORT_string_as(rcpr);
RCPR_IMPORT_resource;

static status insert_line(legacy_cli_instance* inst, char* line);
static bool is_field_separator(int ch);

/**
 * \brief Read lines from the given metadata file, inserting them into the
 * database.
 *
 * \param inst              The instance for this operation.
 * \param input             The input file from which these lines are read.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status load_metadata_file(legacy_cli_instance* inst, FILE* input)
{
    status retval;
    char* line = NULL;
    size_t n = 0, line_len = 0;

    while (!feof(input))
    {
        retval = getline(&line, &n, input);
        if (retval < 0)
        {
            goto done;
        }

        rcpr_trim(line);
        line_len = strlen(line);

        retval = insert_line(inst, line);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_line;
        }

        explicit_bzero(line, line_len);
        free(line);
        line = NULL;
        n = 0;
    }

    goto done;

cleanup_line:
    explicit_bzero(line, line_len);
    free(line);

done:
    return retval;
}

/**
 * \brief Return true if the token is a field separator (';').
 */
static bool is_field_separator(int ch)
{
    return (ch == ';');
}

/**
 * \brief Convert the given line to a metadata instance, then to a database
 * value, and insert this into the database.
 *
 * \param inst              The legacy instance for this operation.
 * \param line              The line to convert and insert.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status insert_line(legacy_cli_instance* inst, char* line)
{
    status retval, release_retval;
    metadata* meta;
    const char *hash_id, *field, *key, *value;
    char* data;
    rcpr_string_iterator iter;
    secure_buffer* hash_id_buffer;
    database_value* dbval;

    /* create an empty metadata instance for receiving this data. */
    retval = metadata_create(&meta, inst->alloc);
    if (STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating metadata instance.\n");
        goto done;
    }

    /* first, extract the id. */
    retval = rcpr_split(&hash_id, (const char**)&data, line, ':');
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_meta;
    }

    /* create a buffer for this hash id. */
    retval =
        secure_buffer_create_from_base64(
            &hash_id_buffer, inst->alloc, hash_id, strlen(hash_id));
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_meta;
    }

    /* set this id. */
    retval = metadata_hash_id_set_from_secure_buffer(meta, hash_id_buffer);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* iterate through all fields. */
    while (
        STATUS_SUCCESS
            == rcpr_multisplit(
                    (const char**)&field, &iter, data, &is_field_separator))
    {
        /* split the field into key and value. */
        retval = rcpr_split(&key, &value, (char*)field, ':');
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_hash_id_buffer;
        }

        if (!strcmp(key, "pwLen"))
        {
            retval = metadata_password_length_set(meta, atoi(value));
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_hash_id_buffer;
            }
        }
        else if (!strcmp(key, "gen"))
        {
            retval = metadata_generation_set(meta, atoi(value));
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_hash_id_buffer;
            }
        }
    }

    /* set the version. */
    retval = metadata_version_set(meta, 1);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the creation date. */
    retval = metadata_creation_date_set(meta, 0);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the revocation date. */
    retval = metadata_revocation_date_set(meta, 0);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the expiration date. */
    retval = metadata_expiration_date_set(meta, 0);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the legacy flag. */
    retval = metadata_legacy_flag_set(meta, true);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the KDF name. */
    retval = metadata_kdf_name_set(meta, "legacy");
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the encoding. */
    retval = metadata_encoding_set(meta, "SYMBOLIC-Base64");
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* set the iterations. */
    retval = metadata_iterations_set(meta, 500000);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* create a database value from this metadata. */
    retval =
        database_value_create_with_hash_id(
            &dbval, inst->alloc, meta, hash_id_buffer, inst->encryption_key);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_hash_id_buffer;
    }

    /* insert this value into the database. */
    retval = database_upsert_value(inst->db, inst->alloc, dbval, false);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_dbval;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_dbval;

cleanup_dbval:
    release_retval = resource_release(database_value_resource_handle(dbval));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_hash_id_buffer:
    release_retval =
        resource_release(secure_buffer_resource_handle(hash_id_buffer));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_meta:
    release_retval = resource_release(metadata_resource_handle(meta));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
