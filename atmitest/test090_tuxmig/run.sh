#!/bin/bash
##
## @brief Tuxedo to Enduro/X migration tools - test launcher
##
## @file run.sh
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

export TESTNAME="test090_tuxmig"

PWD=`pwd`
if [ `echo $PWD | grep $TESTNAME ` ]; then
    # Do nothing 
    echo > /dev/null
else
    # started from parent folder
    pushd .
    echo "Doing cd"
    cd $TESTNAME
fi;

. ../testenv.sh

export TESTDIR="$NDRX_APPHOME/atmitest/$TESTNAME"
export PATH=$PATH:$TESTDIR
export NDRX_ULOG=$TESTDIR
export NDRX_TOUT=10
export NDRX_SILENT=Y

#
# Generic exit function
#
function go_out {
    echo "Test exiting with: $1"
    popd 2>/dev/null
    exit $1
}

#
# Check that syntax fails
#
function syntax_chk {    

    rm -rf ./runtime 2>/dev/null
    # must be in path:
    ubb2ex -P ./runtime $1
    RET=$?
    if [ "X$RET" == "X0" ]; then
        echo "$1 must fail, but converted OK"
        go_out -1
    fi

}

# Some preparations
echo "FLDTBLDIR32=${TESTDIR}/../../ubftest/ubftab" > env_common.txt
echo "FIELDTBLS=test.fd" >> env_common.txt

################################################################################
./ubb_logs-run.sh

RET=$?

if [ "X$RET" != "X0" ]; then
    go_out $RET
fi

################################################################################
./ubb_config1-run.sh

RET=$?

if [ "X$RET" != "X0" ]; then
    go_out $RET
fi
################################################################################

./ubb_network-run.sh

RET=$?

if [ "X$RET" != "X0" ]; then
    go_out $RET
fi
################################################################################

syntax_chk "ubb_syntax_err1"
syntax_chk "ubb_syntax_err2"
syntax_chk "ubb_syntax_err3"
syntax_chk "ubb_syntax_err4"

RET=0

#TODO: Check folder cleanup...

go_out $RET


# vim: set ts=4 sw=4 et smartindent:

