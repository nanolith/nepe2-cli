/**
 * \file nepe2cli/config.h
 *
 * \brief Generated configuration file data for nepe2cli.
 *
 * \copyright 2026 Justin Handville.  Please see License.txt in this
 * distribution for the license terms under which this software is distributed.
 */

#pragma once

#define MAKE_C_VERSION(X,Y) V ## X ## _ ## Y
#define NEPE2CLI_VERSION_SYM \
    MAKE_C_VERSION(@NEPE2CLI_VERSION_MAJOR@, @NEPE2CLI_VERSION_MINOR@)

#define NEPE2CLI_VERSION_STRING \
   "@NEPE2CLI_VERSION_MAJOR@.@NEPE2CLI_VERSION_MINOR@.@NEPE2CLI_VERSION_REL@"
