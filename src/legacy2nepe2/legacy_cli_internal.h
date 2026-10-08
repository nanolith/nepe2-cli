/**
 * \file legacy2nepe2/legacy_cli_internal.h
 *
 * \brief Internals for the legacy2nepe2 conversion utility.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#pragma once

#include <nepe2/database.h>
#include <nepe2/nepe2.h>
#include <stdio.h>

/* C++ compatibility. */
# ifdef   __cplusplus
extern "C" {
# endif /*__cplusplus*/

typedef struct legacy_cli_instance legacy_cli_instance;
struct legacy_cli_instance
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    secure_buffer* salt;
    secure_buffer* master_passphrase;
    secure_buffer* encryption_salt;
    secure_buffer* encryption_key;
    database* db;
};

/******************************************************************************/
/* Start of constructors.                                                     */
/******************************************************************************/

/**
 * \brief Create a legacy_cli_instance with NULL salt and master_passphrase
 * buffer pointers.
 *
 * \param inst              Pointer to the instance pointer to set to this
 *                          instance on success.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status FN_DECL_MUST_CHECK
legacy_cli_instance_create(legacy_cli_instance** inst);

/**
 * \brief Release a \ref legacy_cli_instance resource.
 *
 * \param r                 The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status legacy_cli_instance_resource_release(RCPR_SYM(resource)* r);

/******************************************************************************/
/* Start of protected methods.                                                */
/******************************************************************************/

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
status load_metadata_file(legacy_cli_instance* inst, FILE* input);

/* C++ compatibility. */
# ifdef   __cplusplus
}
# endif /*__cplusplus*/
