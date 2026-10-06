/**
 * \file nepe2/nepe2_cli_internal.h
 *
 * \brief Internals for the CLI.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#pragma once

#include <nepe2/nepe2.h>

/* C++ compatibility. */
# ifdef   __cplusplus
extern "C" {
# endif /*__cplusplus*/

typedef struct nepe2_cli_instance nepe2_cli_instance;
struct nepe2_cli_instance
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    secure_buffer* salt;
    secure_buffer* master_passphrase;
    secure_buffer* session_passphrase;
};

/* C++ compatibility. */
# ifdef   __cplusplus
}
# endif /*__cplusplus*/
