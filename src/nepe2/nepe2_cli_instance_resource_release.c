/**
 * \file nepe2/nepe2_cli_instance_resource_release.c
 *
 * \brief Release a nepe2 cli instance.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#include <stdlib.h>

#include "nepe2_cli_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a \ref nepe2_cli_instance resource.
 *
 * \param r                 The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status nepe2_cli_instance_resource_release(RCPR_SYM(resource)* r)
{
    status retval = STATUS_SUCCESS, release_retval;
    nepe2_cli_instance* inst = (nepe2_cli_instance*)r;

    /* release database if set. */
    if (NULL != inst->db)
    {
        release_retval = resource_release(database_resource_handle(inst->db));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

    /* release salt if set. */
    if (NULL != inst->salt)
    {
        release_retval =
            resource_release(secure_buffer_resource_handle(inst->salt));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

    /* release master_passphrase if set. */
    if (NULL != inst->master_passphrase)
    {
        release_retval =
            resource_release(
                secure_buffer_resource_handle(inst->master_passphrase));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

    /* release session passphrase if set. */
    if (NULL != inst->session_passphrase)
    {
        release_retval =
            resource_release(
                secure_buffer_resource_handle(inst->session_passphrase));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

    /* release allocator if set. */
    if (NULL != inst->alloc)
    {
        release_retval =
            resource_release(rcpr_allocator_resource_handle(inst->alloc));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

    /* reclaim instance memory. */
    explicit_bzero(inst, sizeof(*inst));
    free(inst);

    /* decode result. */
    return retval;
}
