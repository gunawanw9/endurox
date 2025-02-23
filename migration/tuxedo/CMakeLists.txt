##
## @brief Tuxedo Ubbconfig importer
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

# Set a minimum version
cmake_minimum_required(VERSION 3.1)

# Add debug options
# By default if RELEASE_BUILD is not defined, then we run in debug!
IF ($ENV{RELEASE_BUILD})
	# do nothing
ELSE ($ENV{RELEASE_BUILD})
	ADD_DEFINITIONS("-D NDRX_DEBUG")
ENDIF ($ENV{RELEASE_BUILD})

# Osx has very outdated bison,
# thus install by brew install bison
if(PLATFORM STREQUAL "Darwin")
    set(BISON_EXECUTABLE "/usr/local/opt/bison/bin/bison" CACHE PATH "Bison executable")
endif()

find_package(BISON)
find_package(FLEX)
#find_package(pscript)
include(pscript)

# Make sure the compiler can find include files from UBF library
include_directories (${ENDUROX_SOURCE_DIR}/include)

pscript_embed(ubb2ex.pscript ubb2ex_bytecode)

BISON_TARGET(EXPRPARSER ubb.y ${CMAKE_CURRENT_BINARY_DIR}/ubb.tab.c)
FLEX_TARGET(EXPRSCANNER ubb.l ${CMAKE_CURRENT_BINARY_DIR}/ubb.lex.c)

BISON_TARGET(EXPRPARSER ddr_range.y ${CMAKE_CURRENT_BINARY_DIR}/ddr.tab.c)
FLEX_TARGET(EXPRSCANNER ddr_range.l ${CMAKE_CURRENT_BINARY_DIR}/ddr.lex.c)

add_executable (ubb2ex ubb.c ubb.tab.c ubb.lex.c ddr.tab.c ddr.lex.c script.c getrm.c ubb2ex_bytecode.c)
target_compile_definitions(ubb2ex PUBLIC NDRX_TMLOADCF)
target_link_libraries (ubb2ex psstdlib ps nstd ${RT_LIB} pthread m)

install (TARGETS 
    ubb2ex
    DESTINATION bin)

install (FILES
    ubb2ex.pscript
    DESTINATION share/endurox/config)
# vim: set ts=4 sw=4 et smartindent:
