/**
 * \file legacy2nepe2/legacy_cli_instance_create.c
 *
 * \brief Create a legacy cli instance.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <rcpr/vtable.h>
#include <stdlib.h>

#include "legacy_cli_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

RCPR_VTABLE resource_vtable legacy_cli_instance_vtable = {
    .release = &legacy_cli_instance_resource_release
};

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
legacy_cli_instance_create(legacy_cli_instance** inst)
{
    status retval, release_retval;
    legacy_cli_instance* tmp;

    /* allocate memory for this instance. */
    tmp = (legacy_cli_instance*)malloc(sizeof(*tmp));
    if (NULL == tmp)
    {
        retval = ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* initialize resource. */
    explicit_bzero(tmp, sizeof(*tmp));
    resource_init(&tmp->hdr, &legacy_cli_instance_vtable);

    /* create malloc allocator. */
    retval = rcpr_malloc_allocator_create(&tmp->alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* success. */
    *inst = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
