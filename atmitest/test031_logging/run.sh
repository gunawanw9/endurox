#!/bin/bash
##
## @brief @(#) Test31 Launcher
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
TESTNAME="test031_logging"

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

export NDRX_ULOG=`pwd`
rm *.log 2>/dev/null
rm ULOG*  2>/dev/null

# Clean up log dir
rm -rf ./logs 2>/dev/null
mkdir ./logs

rm -rf ./logs_cfg 2>/dev/null
mkdir ./logs_cfg

rm -rf ./non_exist
rm -rf ./non_write
mkdir non_write

# no write for others..
chmod 555 non_write

(./atmisv31FIRST -t 4 -i 1 2>&1) > ./atmisv31FIRST.log &
(./atmisv31SECOND -i 1 2>&1) > ./atmisv31SECOND.log &
sleep 2
(./atmiclt31 2>&1) > ./atmiclt31.log

RET=$?

if [ "X$RET" != "X0" ]; then
    echo "atmiclt31 failed!"
    RET=-2
fi

(./atmiclt31_inv 2>&1) > ./atmiclt31_inv.log
XRET=$?

if [ "X$XRET" != "X0" ]; then
    echo "atmiclt31_inv failed!"
    RET=-2
fi

# could lines of "HELLO" in ./atmiclt31_inv.log
# shall be over 1000
CNT = `grep HELLO ./atmiclt31_inv.log | wc -l | awk '{print $1}'`

echo "CNT=$CNT"

if [ "$CNT" -lt "1000" ]; then
    echo "atmiclt31_inv expected at least 1000 HELLO, got $CNT!"
    RET=-2
fi

# Catch is there is test error!!!
if [ "X`grep TESTERROR *.log`" != "X" ]; then
    echo "Test error detected!"
    RET=-2
fi

xadmin killall atmisv31FIRST 2>/dev/null
xadmin killall atmisv31SECOND 2>/dev/null
xadmin killall exbenchsv 2>/dev/null
xadmin killall exbenchcl 2>/dev/null
xadmin killall atmiclt31 2>/dev/null

#killall atmiclt1


# Check the log files
if [ "X`grep 'Hello from NDRX' clt-endurox.log`" == "X" ]; then
        echo "error in clt-endurox.log!"
	RET=-2
fi

#if [ "X`grep 'THIS IS NDRX IN PROCLOG' clt-endurox.log`" == "X" ]; then
#        echo "error in clt-endurox.log missing 'THIS IS NDRX IN PROCLOG'!"

if [ "X`grep 'THIS IS NDRX IN PROCLOG' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log missing 'THIS IS NDRX IN PROCLOG'!"
	RET=-2
fi

#if [ "X`grep 'THIS IS UBF IN PROCLOG' clt-endurox.log`" == "X" ]; then
#        echo "error in clt-endurox.log missing 'THIS IS UBF IN PROCLOG'!"
if [ "X`grep 'THIS IS UBF IN PROCLOG' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log missing 'THIS IS UBF IN PROCLOG'!"
	RET=-2
fi

# Feature #470
if [ "X`grep 'THIS IS TP IN PROCLOG' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log missing 'THIS IS TP IN PROCLOG'!"
	RET=-2
fi

if [ "X`grep 'Hello from tp' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log (Hello from tp not found)!"
	RET=-2
fi

if [ "X`grep 'hello from thread 1' clt-tp-th1.log`" == "X" ]; then
        echo "error in clt-tp-th1.log!"
	RET=-2
fi

if [ "X`grep 'hello from thread 2' clt-tp-th2.log`" == "X" ]; then
        echo "error in clt-tp-th2.log!"
	RET=-2
fi

if [ "X`grep 'hello from main thread' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log (hello from main thread not found)!"
	RET=-2
fi

if [ "X`grep 'Thread 1 logs to main' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log (missing Thread 1 logs to main in main)!"
	RET=-2
fi

if [ "X`grep 'Thread 2 logs to main' clt-tp.log`" == "X" ]; then
        echo "error in clt-tp.log (missing Thread 2 logs to main in main)!"
	RET=-2
fi

# There shall be 1000 files in log directory
FILES=` ls -1 ./logs/*.log | wc | awk '{print $1}'`

echo "Got request files: [$FILES]"
if [ "X$FILES" != "X1000" ]; then
        echo "Invalid files count [$FILES] should be 1000!"
	RET=-2
fi

################################################################################
# there shall be in each log file:
# - Hello from SETREQFILE
# - Hello from atmicl31
# - Hello from TEST31_2ND
################################################################################

# Test all 1000 files

for ((i=1;i<=100;i++)); do
echo "Testing sequence: $i"

    if [ "X`grep 'Hello from SETREQFILE' ./logs/request_$i.log`" == "X" ]; then
            echo "Missing 'Hello from SETREQFILE' file $i"
            RET=-2
    fi

    if [ "X`grep 'Hello from atmicl31' ./logs/request_$i.log`" == "X" ]; then
            echo "Missing 'Hello from atmicl31' file $i"
            RET=-2
    fi

    if [ "X`grep 'Hello from TEST31_2ND' ./logs/request_$i.log`" == "X" ]; then
            echo "Missing 'Hello from TEST31_2ND' file $i"
            RET=-2
    fi

    # Feature #470
    if [ "X`grep 'THIS IS NDRX IN REQLOG' ./logs/request_$i.log`" == "X" ]; then
            echo "Missing 'THIS IS NDRX IN REQLOG' file $i"
            RET=-2
    fi

    if [ "X`grep 'THIS IS UBF IN REQLOG' ./logs/request_$i.log`" == "X" ]; then
            echo "Missing 'THIS IS UBF IN REQLOG' file $i"
            RET=-2
    fi

done

if [ "X`grep 'Finishing off' ./clt-tp.log`" == "X" ]; then
        echo "Missing 'Finishing off'"
        RET=-2
fi

# start the exbenchsv to 
(NDRX_DEBUG_CONF="debug_threaded_y.conf" exbenchsv -i 100 2>&1) > ./exbenchsv.log &

# start the benchmark in threads...
(NDRX_DEBUG_CONF="debug_threaded_y.conf" exbenchcl -n5 -P -t20 -b "{}" -f EX_DATA -S1024 -R5 2>&1) > ./exbenchcl.log
XRET=$?
if [ "X$XRET" != "X0" ]; then
    echo "exbenchcl 1 failed"
    RET=$XRET
fi

# start the benchmark in threads...
(NDRX_DEBUG_CONF="debug_threaded_perc.conf" exbenchcl -n5 -P -t20 -b "{}" -f EX_DATA -S1024 -R5 2>&1) > ./exbenchcl.log
XRET=$?
if [ "X$XRET" != "X0" ]; then
    echo "exbenchcl 2 failed"
    RET=$XRET
fi

export SOME_TEST="THIS_IS_ENV"
(NDRX_DEBUG_CONF="debug_env_sub.conf" exbenchcl -n5 -P -t20 -b "{}" -f EX_DATA -S1024 -R5 2>&1) > ./exbenchcl.log
XRET=$?
if [ "X$XRET" != "X0" ]; then
    echo "exbenchcl 3 failed"
    RET=$XRET
fi

export SOME_TEST="THIS_IS_ENV2"
(NDRX_DEBUG_CONF="debug_env_sub_th.conf" exbenchcl -n5 -P -t20 -b "{}" -f EX_DATA -S1024 -R5 2>&1) > ./exbenchcl.log
XRET=$?
if [ "X$XRET" != "X0" ]; then
    echo "exbenchcl 4 failed"
    RET=$XRET
fi

#
# Validate log files..., with context id
# 
if [ ! -f "logs_cfg/exbenchcl.0.log_loger_extension" ]; then
    echo "Missing [exbenchcl.0.log_loger_extension]"
    RET=-2
fi

if [ ! -f "logs_cfg/exbenchcl%d.0.log_perc" ]; then
    echo "Missing [exbenchcl%d.0.log_perc]"
    RET=-2
fi

if [ ! -f "logs_cfg/exbenchcl%d.0.log_perc" ]; then
    echo "Missing [exbenchcl%d.0.log_perc]"
    RET=-2
fi

if [ ! -f "logs_cfg/exbenchcl.log_THIS_IS_ENV" ]; then
    echo "Missing [exbenchcl.log_THIS_IS_ENV]"
    RET=-2
fi

if [ ! -f "logs_cfg/exbenchcl.0.log_THIS_IS_ENV2" ]; then
    echo "Missing [exbenchcl.0.log_THIS_IS_ENV2]"
    RET=-2
fi

if [ ! -f "logs_cfg/exbenchsv.0.log" ]; then
    echo "Missing [exbenchsv.0.log]"
    RET=-2
fi

xadmin killall atmisv31FIRST 2>/dev/null
xadmin killall atmisv31SECOND 2>/dev/null
xadmin killall exbenchsv 2>/dev/null
xadmin killall exbenchcl 2>/dev/null
xadmin killall atmiclt31 2>/dev/null

popd 2>/dev/null

exit $RET

# vim: set ts=4 sw=4 et smartindent:
