#!/bin/bash
##
## @brief @(#) Test086 - testing on MQ limitations
##
## @file run-dom.sh
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

export TESTNO="086"
export TESTNAME_SHORT="tmqlimit"
export TESTNAME="test${TESTNO}_${TESTNAME_SHORT}"

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
# Override timeout!
export NDRX_TOUT=90
export NDRX_LIBEXT="so"
export NDRX_ULOG=$TESTDIR
export NDRX_SILENT=Y

# minimum performance enq/deq to proceed with test
# where the absolute values are used
TEST_PERF_MIN=70


#
# Domain 1 - here client will live
#
function set_dom1 {
    echo "Setting domain 1"
    . ../dom1.sh
    export NDRX_CONFIG=$TESTDIR/ndrxconfig-dom1.xml
    export NDRX_DMNLOG=$TESTDIR/ndrxd-dom1.log
    export NDRX_LOG=$TESTDIR/ndrx-dom1.log
    export NDRX_DEBUG_CONF=$TESTDIR/debug-dom1.conf

# XA config, mandatory for TMQ:
    export NDRX_XA_RES_ID=1
    export NDRX_XA_OPEN_STR="datadir=./QSPACE1,qspace=MYSPACE"
    export NDRX_XA_CLOSE_STR=$NDRX_XA_OPEN_STR
# Used from parent
    export NDRX_XA_DRIVERLIB=$NDRX_XA_DRIVERLIB_FILENAME

    export NDRX_XA_RMLIB=libndrxxaqdisk.so
if [ "$(uname)" == "Darwin" ]; then
    export NDRX_XA_RMLIB=libndrxxaqdisk.dylib
    export NDRX_LIBEXT="dylib"
fi
    export NDRX_XA_LAZY_INIT=0
}

#
# Generic exit function
#
function go_out {
    echo "Test exiting with: $1"
    
    set_dom1;
    xadmin stop -y
    xadmin down -y

    # If some alive stuff left...
    xadmin killall atmiclt86

    popd 2>/dev/null
    exit $1
}

function clean_logs {

    # clean-up the logs for debbuging at the error.
    for f in `ls *.log`; do
         echo > $f
    done

}

UNAME=`uname`

#
# export the library path.
#
case $UNAME in

  Darwin)
    export NDRX_PLUGINS=libt86_lcf.dylib
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$TESTDIR
    ;;

  AIX)
    export NDRX_PLUGINS=libt86_lcf.so
    export LIBPATH=$LIBPATH:$TESTDIR
    ;;

  *)
    export NDRX_PLUGINS=libt86_lcf.so
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TESTDIR
    ;;
esac


# Where to store TM logs
rm -rf ./RM1
mkdir RM1

rm -rf ./RM2
mkdir RM2

# Where to store Q messages (QSPACE1)
rm -rf ./QSPACE1
mkdir QSPACE1

set_dom1;
# clean up anything left from prevoius tests...
xadmin down -y
# let ndrxd to finish
sleep 2
xadmin start -y || go_out 1

# Run the client test...
xadmin psc
xadmin psvc
xadmin ppm
clean_logs;
rm ULOG*


################################################################################
echo "Transaction timeouts"
################################################################################
# use custom timeout
export NDRX_TOUT=10
xadmin stop -y
xadmin start -y

# Load message in each Q to finish up
exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sT_OK -E -R1
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "exbenchcl failed T_OK load fail"
    go_out $RET
fi

exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sT_NOK -E -R1
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "exbenchcl failed T_NOK load fail"
    go_out $RET
fi

exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sNS_OK -E -R1
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "exbenchcl failed NS_OK load fail"
    go_out $RET
fi

exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sND_OK -E -R1
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "exbenchcl failed ND_OK load fail"
    go_out $RET
fi

exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sND_NOK -E -R1
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "exbenchcl failed ND_NOK load fail"
    go_out $RET
fi

echo "Wait 60, all tout Qs shall complete..."
sleep 60
xadmin mqlq
xadmin psc

STATS=`xadmin mqlq | grep "MYSPACE   T_OK          0     0     1     1     1     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Invalid T_OK stats!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "MYSPACE   T_NOK         0     0     1     1     0     1"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Invalid T_NOK stats!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "MYSPACE   NS_OK         0     0     1     1     1     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Invalid NS_OK stats!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "MYSPACE   ND_OK         0     0     1     1     1     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Invalid ND_OK stats!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "MYSPACE   ND_NOK        0     0     1     1     0     1"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Invalid ND_NOK stats!"
    go_out -1
fi

clean_logs;
rm ULOG*
# restore tout:
export NDRX_TOUT=90
xadmin stop -y
xadmin start -y

################################################################################
echo "Forward thread wakup test"
################################################################################
rm wakeup.out 2>/dev/null
export NDRX_BENCH_FILE="wakeup.out"
#
# Check the system performance, if we can do 40 enq/deq sec, then continue with the
# given test
#
exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sTEST1

NR_CALLS=`tail -1 wakeup.out  | cut -d ' '  -f3`
# remember the performance indication
# as on slow systems, say slower than 70 tps we will not be able to complete
# test in time

TEST_PERF=$NR_CALLS

if [ "$TEST_PERF" -ge "$TEST_PERF_MIN" ]; then

    rm wakeup.out 2>/dev/null
    exbenchcl -n1 -P -t30 -b "{}" -f EX_DATA -S1 -QMYSPACE -sWAKEUP -E -I

    RET=$?
    if [[ "X$RET" != "X0" ]]; then
        echo "exbenchcl failed"
        go_out $RET
    fi

    # validate that there is more messages than say 65
    cat wakeup.out

    NR_CALLS=`tail -1 wakeup.out  | cut -d ' '  -f3`

    if [ "$NR_CALLS" -lt "65" ]; then
        echo "Expected more calls than 80 got $NR_CALLS"
        go_out -1
    fi

else
    echo "Skipping thread wakup test as disk performance $TEST_PERF < $TEST_PERF_MIN"
fi

################################################################################
# Crash loop of autoq=T
################################################################################

if [ `xadmin poller` != "emq" ]; then
    echo "Testing crashloop_t"
    # use custom timeout
    export NDRX_TOUT=30
    xadmin stop -y
    xadmin start -y
    (./atmiclt86 crashloop_t 2>&1) >> ./atmiclt-dom1.log
    RET=$?
    if [[ "X$RET" != "X0" ]]; then
        xadmin psc
        go_out $RET
    fi

    # print what's left in q...
    xadmin mqlq
    xadmin pt

    STATS=`xadmin mqlq | grep "ERROR         0     0"`

    echo "Stats: [$STATS]"

    if [[ "X$STATS" == "X" ]]; then
        echo "Expecting ERROR queue to be fully empty!"
        go_out -1
    fi

    # restore tout:
    export NDRX_TOUT=90
    xadmin stop -y
    xadmin start -y

    clean_logs;
    rm ULOG*
fi


################################################################################
echo "Validate transactional sequence..."
################################################################################

(./atmiclt86 seqvalid 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "seqvalid failed"
    go_out $RET
fi

xadmin mqch -n1 -i 100 -qSEQVALID,autoq=T

RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "mqch failed"
    go_out $RET
fi

i=0
# wait to complete...
while [[ $i -lt 90 ]]
do
    echo "Wait completion ($i)"
    sleep 1

    ((i++))

    xadmin mqlq | grep "SEQVALID      0     0    1K    1K    1K     0"
    STATS=`xadmin mqlq | grep "SEQVALID      0     0    1K    1K    1K     0"`
    if [ "X$STATS" != "X" ]; then
        break;
    fi
done

# validate the result finally.
xadmin mqlq | grep SEQVALID
STATS=`xadmin mqlq | grep "SEQVALID      0     0    1K    1K    1K     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting SEQVALID to be completed!"
    go_out -1
fi

################################################################################
echo "QoS test... last Q slow"
################################################################################

# Load first fast.. (just to get forward order)
exbenchcl -n1 -P -t9999 -b "{}" -f EX_DATA -S1 -QMYSPACE -sLASTOK -R5 -E

RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "Failed to load QoS messages..."
    go_out $RET
fi

# Load second slow...
exbenchcl -n1 -P -t9999 -b "{}" -f EX_DATA -S1 -QMYSPACE -sLASTSLOW -R60 -E
RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "Failed to load QoS messages..."
    go_out $RET
fi

echo "Current situation:"
xadmin mqlq

# Load fast queue, fast load
exbenchcl -n40 -P -t9999 -b "{}" -f EX_DATA -S1 -QMYSPACE -sLASTOK -R5 -E

RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "Failed to load QoS messages..."
    go_out $RET
fi

echo "Wait 20..."
sleep 20

echo "Current situation:"
xadmin mqlq

# Fast queue shall be done with 200 msgs...
STATS=`xadmin mqlq | grep "LASTOK        0     0   205   205   205     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting LASTOK to be completed!"
    go_out -1
fi

echo "Wait 60 for slow to complete..."
sleep 60
xadmin mqlq
STATS=`xadmin mqlq | grep "LASTSLOW      0     0    60    60    60     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting LASTSLOW to be completed!"
    go_out -1
fi

################################################################################
echo "TPETRAN join on timed out transaction ..."
################################################################################
(NDRX_CCONFIG=${TESTDIR}/nulltm.ini NDRX_CCTAG=NULL ./atmiclt86 txtout 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    xadmin psc
    go_out $RET
fi

################################################################################
echo "Massive global transaction load ..."
################################################################################

# test cases checks for any possiblity of deadlock due to TM sending
# completions to QSP, but QSP requests join to TM.

#export NDRX_XA_FLAGS="FSYNC;DSYNC"
# Do we need to fsync?
#xadmin stop -y
#xadmin start -y

# Run the load...
NDRX_CCONFIG=${TESTDIR}/nulltm.ini NDRX_CCTAG=NULL exbenchcl -n60 -P -t160 -b "{}" -f EX_DATA -S1024 -QMYSPACE -sEXB -N5 -T60

RET=$?
if [ "X$RET" != "X0" ]; then
    echo "Global transaction load test..."
    go_out $RET
fi

#unset NDRX_XA_FLAGS
#xadmin stop -y
#xadmin start -y

################################################################################
echo "QoS test... (slow queue does not slow down all other Qs)"
################################################################################

if [ "$TEST_PERF" -ge "$TEST_PERF_MIN" ]; then


exbenchcl -n5 -P -t9999 -b "{}" -f EX_DATA -S1 -QMYSPACE -sQOS -R100 -E -N5

RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "Failed to load QoS messages..."
    go_out $RET
fi

# load some more to single q, so that not all Qs finish in the same time
# which might introduce some issues with busy all/any flag
exbenchcl -n1 -P -t9999 -b "{}" -f EX_DATA -S1 -QMYSPACE -sQOS003 -R100 -E

RET=$?
if [[ "X$RET" != "X0" ]]; then
    echo "Failed to load QoS messages..."
    go_out $RET
fi

# Check the queue stats... QOS000 is slow queue, shall be ready 200ms*100
# all other queues shall be flushed immediatlly, say 10 sec should be sufficient
# OK?

# all shall complete within 10 sec, not?
echo "Wait 30..."
sleep 30
xadmin mqlq


STATS=`xadmin mqlq | grep "QOS000        0     0   100   100   100     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" != "X" ]; then
    echo "Expecting QOS000 to be in-completed!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "QOS001        0     0   100   100   100     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting QOS001 to be completed!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "QOS002        0     0   100   100   100     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting QOS002 to be completed!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "QOS003        0     0   200   200   200     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting QOS003 to be completed!"
    go_out -1
fi

STATS=`xadmin mqlq | grep "QOS004        0     0   100   100   100     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting QOS004 to be completed!"
    go_out -1
fi


# give some 30 sec total for other to complete... 
echo "Wait 65..."
sleep 65
xadmin mqlq

STATS=`xadmin mqlq | grep "QOS000        0     0   100   100   100     0"`
echo "Stats: [$STATS]"
if [ "X$STATS" == "X" ]; then
    echo "Expecting QOS000 to be completed!"
    go_out -1
fi

else
    echo "System too slow... skipping the test"
fi

################################################################################
echo "AutoQ performance test..."
################################################################################
(./atmiclt86 autoperf 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    xadmin psc
    go_out $RET
fi

################################################################################
echo "Testing rmrollback - rollback due to internal timeout / no activity"
################################################################################
(./atmiclt86 rmrollback 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    xadmin psc
    go_out $RET
fi

################################################################################
echo "Testing rmnorollback - no rollback, due to having slow, but actvity"
################################################################################
(./atmiclt86 rmnorollback 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    xadmin psc
    go_out $RET
fi

################################################################################
# Forward crash / commit fails, thus timeout rollback and when new tmsrv
# is available, messages shall go normally to errorq and all msgs shall be
# downloable
################################################################################

echo "Testing fwdcrash"
(./atmiclt86 fwdcrash 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    xadmin psc
    go_out $RET
fi

################################################################################
# Test the fsync flags...
################################################################################

export NDRX_XA_FLAGS="FSYNC;DSYNC"

xadmin stop -y
xadmin start -y

echo "Testing FSYNC;DSYNC"
(./atmiclt86 enqdeq 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

export NDRX_XA_FLAGS="FDATASYNC;DSYNC"

xadmin stop -y
xadmin start -y

echo "Testing FDATASYNC;DSYNC"
(./atmiclt86 enqdeq 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

unset NDRX_XA_FLAGS

xadmin stop -y
xadmin start -y

################################################################################

###################################################
# for darwin /emq missing robost locks may stall the testing
# thus it is not possible to test this func here
###################################################

if [ `xadmin poller` != "emq" ]; then
    echo "Testing crashloop"
    # use custom timeout
    export NDRX_TOUT=30
    xadmin stop -y
    xadmin start -y
    (./atmiclt86 crashloop 2>&1) >> ./atmiclt-dom1.log
    RET=$?
    if [[ "X$RET" != "X0" ]]; then
        xadmin psc
        go_out $RET
    fi

    # print what's left in q...
    xadmin mqlq
    xadmin pt

    STATS=`xadmin mqlq | grep "ERROR         0     0"`

    echo "Stats: [$STATS]"

    if [[ "X$STATS" == "X" ]]; then
        echo "Expecting ERROR queue to be fully empty!"
        go_out -1
    fi

    # restore tout:
    export NDRX_TOUT=90
    xadmin stop -y
    xadmin start -y

    clean_logs;
    rm ULOG*
fi
###################################################

echo "Testing errorq"
(./atmiclt86 errorq 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    xadmin psc
    go_out $RET
fi

echo "Testing abortrules"
(./atmiclt86 abortrules 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing loadprep"
(./atmiclt86 loadprep 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

# after loadprep log folders shall be cleaned up...
xadmin stop -y

# Where to store TM logs
rm -rf ./RM1
mkdir RM1

rm -rf ./RM2
mkdir RM2

# Where to store Q messages (QSPACE1)
rm -rf ./QSPACE1
mkdir QSPACE1

xadmin start -y

echo "Testing diskfull"
clean_logs;
xadmin help lcf 

(./atmiclt86 diskfull 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing tmsrvdiskerr"
(./atmiclt86 tmsrvdiskerr 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing commit_crash"
(./atmiclt86 commit_crash 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing badmsg"
(./atmiclt86 badmsg 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing tmqrestart"
(./atmiclt86 tmqrestart 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing tmsrvrestart"
(./atmiclt86 tmsrvrestart 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing commit_shut"
(./atmiclt86 commit_shut 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Testing deqwriteerr"
(./atmiclt86  deqwriteerr 2>&1) >> ./atmiclt-dom1.log
RET=$?
if [[ "X$RET" != "X0" ]]; then
    go_out $RET
fi

echo "Cannot start as damaged prepared files... (manual resolve)"
dd if=/dev/zero of=./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-010 count=1024 bs=1
touch ./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-011
touch ./QSPACE1/prepared/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-012

xadmin stop -s tmqueue
xadmin start -s tmqueue
sleep 5

if [[ ! -f ./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-010 ]]; then
    echo "./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-010 must not be removed!"
    go_out -1
fi

if [[ ! -f ./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-011 ]]; then
    echo "./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-011 must not be removed!"
    go_out -1
fi

if [[ ! -f ./QSPACE1/prepared/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-012 ]]; then
    echo "./QSPACE1/prepared/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-012 must not be removed!"
    go_out -1
fi

echo "Remove damaged prepare file, shall abort all ok"
rm ./QSPACE1/prepared/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-012
xadmin stop -s tmqueue
xadmin start -s tmqueue
sleep 5

if [[ -f ./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-010 ]]; then
    echo "./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-010 must be removed!"
    go_out -1
fi

if [[ -f ./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-011 ]]; then
    echo "./QSPACE1/active/YZT3oUBAwBhirg3IRi2wqkSZp6IitwEAAQAy-011 must be removed!"
    go_out -1
fi

go_out $RET

# vim: set ts=4 sw=4 et smartindent:
