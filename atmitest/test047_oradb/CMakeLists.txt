##
## @brief Test server connection to Oracle DB from C - makefile
##
## @file CMakeLists.txt
##
## -----------------------------------------------------------------------------
## Enduro/X Middleware Platform for Distributed Transaction Processing
## Copyright (C) 2009-2016, ATR Baltic, Ltd. All Rights Reserved.
## Copyright (C) 2017-2019, Mavimax, Ltd. All Rights Reserved.
## This software is released under one of the following licenses:
## AGPL (with Java and Go exceptions) or Mavimax's license for commercial use.
## See LICENSE file for full text.
## -----------------------------------------------------------------------------
## AGPL license:
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU Affero General Public License, version 3 as published
## by the Free Software Foundation;
##
## This program is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
## PARTICULAR PURPOSE. See the GNU Affero General Public License, version 3
## for more details.
##
## You should have received a copy of the GNU Affero General Public License along 
## with this program; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##
## -----------------------------------------------------------------------------
## A commercial use license is available from Mavimax, Ltd
## contact@mavimax.com
## -----------------------------------------------------------------------------
##
cmake_minimum_required(VERSION 3.1)

# Make sure the compiler can find include files from UBF library
include_directories (${ENDUROX_SOURCE_DIR}/libubf
					 ${ENDUROX_SOURCE_DIR}/include
					 ${ENDUROX_SOURCE_DIR}/libnstd
					 ${ENDUROX_SOURCE_DIR}/ubftest
                                         $ENV{ORACLE_HOME}/precomp/public)

# Add debug options
# By default if RELEASE_BUILD is not defined, then we run in debug!
IF ($ENV{RELEASE_BUILD})
	# do nothing
ELSE ($ENV{RELEASE_BUILD})
	ADD_DEFINITIONS("-D NDRX_DEBUG")
ENDIF ($ENV{RELEASE_BUILD})

# Make sure the linker can find the UBF library once it is built.
link_directories (${ENDUROX_BINARY_DIR}/libubf $ENV{ORACLE_HOME}/lib)

############################# Test - executables ###############################

#
# NOTE: *proc* binary must be in path
# Also we use PARSE=PARTIAL because of ndebug.h requires C99 compiler and
# proc does not support it.
# See: https://stackoverflow.com/questions/18465187/why-the-proc-c-oracle-precompiler-can-not-parse-the-windows-h-header-file
#
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/atmisv47.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/atmisv47.proc
    COMMAND proc iname=${CMAKE_CURRENT_SOURCE_DIR}/atmisv47.proc SQLCHECK=SYNTAX
        MODE=ANSI CODE=ANSI_C IRECLEN=255 ORECLEN=255 PARSE=PARTIAL INCLUDE=${ENDUROX_SOURCE_DIR}/include INCLUDE=$ENV{ORACLE_HOME}/precomp/public
        ONAME=${CMAKE_CURRENT_SOURCE_DIR}/atmisv47.c)

add_executable (atmi.sv47 atmisv47.c ../../libatmisrv/rawmain_integra.c)
add_executable (atmiclt47 atmiclt47.c)
add_executable (atmiclt47_loop atmiclt47_loop.c)
################################################################################
############################# Test - executables ###############################
# Link the executable to the ATMI library & others...
# libatmi shall come first, as it exposes ax_reg() if doing dynamic-reg tests
target_link_libraries (atmi.sv47 atmisrvinteg atmi clntsh ubf nstd m pthread ${RT_LIB})
target_link_libraries (atmiclt47 atmiclt atmi ubf nstd m pthread ${RT_LIB})
target_link_libraries (atmiclt47_loop atmiclt atmi ubf nstd m pthread ${RT_LIB})

set_target_properties(atmi.sv47 PROPERTIES LINK_FLAGS "$ENV{MYLDFLAGS}")
set_target_properties(atmiclt47 PROPERTIES LINK_FLAGS "$ENV{MYLDFLAGS}")
set_target_properties(atmiclt47_loop PROPERTIES LINK_FLAGS "$ENV{MYLDFLAGS}")
################################################################################


# vim: set ts=4 sw=4 et smartindent:
