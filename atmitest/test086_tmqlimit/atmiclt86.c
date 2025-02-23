/**
 * @brief TMQ test client /limits
 *  TODO: add test case for: tmq session timeout 
 *   1. no commit in time => TPEABORT.
 *   2. forward tpenqueue() (ERRORQ not defined). Define after a while, msg shall be put in errorq
 *
 * @file atmiclt86.c
 */
/* -----------------------------------------------------------------------------
 * Enduro/X Middleware Platform for Distributed Transaction Processing
 * Copyright (C) 2009-2016, ATR Baltic, Ltd. All Rights Reserved.
 * Copyright (C) 2017-2019, Mavimax, Ltd. All Rights Reserved.
 * This software is released under one of the following licenses:
 * AGPL (with Java and Go exceptions) or Mavimax's license for commercial use.
 * See LICENSE file for full text.
 * -----------------------------------------------------------------------------
 * AGPL license:
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License, version 3 as published
 * by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Affero General Public License, version 3
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * -----------------------------------------------------------------------------
 * A commercial use license is available from Mavimax, Ltd
 * contact@mavimax.com
 * -----------------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#include <atmi.h>
#include <ubf.h>
#include <ndebug.h>
#include <test.fd.h>
#include <ndrstandard.h>
#include <ubfutil.h>
#include <nstopwatch.h>
#include <nstdutil.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/
exprivate int basic_tmqrestart(int maxmsg);
exprivate int basic_tmsrvrestart(int maxmsg);
exprivate int basic_diskfull(int maxmsg);
exprivate int basic_commit_shut(int maxmsg);
exprivate int basic_loadprep(int maxmsg);
exprivate int basic_tmsrvdiskerr(int maxmsg);
exprivate int basic_badmsg(int maxmsg);
exprivate int basic_commit_crash(int maxmsg);
exprivate int basic_deqwriteerr(int maxmsg);
exprivate int basic_enqdeq(int maxmsg);
exprivate int basic_rmrollback(int maxmsg);
exprivate int basic_rmnorollback(int maxmsg);
exprivate int basic_fwdcrash(int maxmsg);
exprivate int basic_autoperf(int maxmsg);
exprivate int basic_txtout(int maxmsg);
exprivate int basic_seqvalid(int maxmsg);

extern int basic_abort_rules(int maxmsg);
extern int basic_errorq(void);
extern int basic_crashloop(char *qname);

int main(int argc, char** argv)
{
    int ret = EXSUCCEED;
    
    if (argc<=1)
    {
        NDRX_LOG(log_error, "usage: %s <test_case: qfull>", argv[0]);
        return EXFAIL;
    }
    NDRX_LOG(log_error, "\n\n\n\n\n !!!!!!!!!!!!!! TEST CASE %s !!!!!!!! \n\n\n\n\n\n", argv[1]);
    
    if (EXSUCCEED!=tpopen())
    {
        EXFAIL_OUT(ret);
    }
    
    if (0==strcmp(argv[1], "rmrollback"))
    {
        return basic_rmrollback(1200);
    }
    else if (0==strcmp(argv[1], "rmnorollback"))
    {
        return basic_rmnorollback(1200);
    }
    else if (0==strcmp(argv[1], "autoperf"))
    {
        return basic_autoperf(200);
    }
    else if (0==strcmp(argv[1], "tmqrestart"))
    {
        return basic_tmqrestart(1200);
    }
    else if (0==strcmp(argv[1], "fwdcrash"))
    {
        return basic_fwdcrash(1200);
    }
    else if (0==strcmp(argv[1], "tmsrvrestart"))
    {
        return basic_tmsrvrestart(1200);
    }
    else if (0==strcmp(argv[1], "commit_shut"))
    {
        return basic_commit_shut(1200);
    }
    else if (0==strcmp(argv[1], "loadprep"))
    {
        return basic_loadprep(1200);
    }
    else if (0==strcmp(argv[1], "diskfull"))
    {
        return basic_diskfull(10);
    }
    else if (0==strcmp(argv[1], "tmsrvdiskerr"))
    {
        return basic_tmsrvdiskerr(100);
    }
    else if (0==strcmp(argv[1], "badmsg"))
    {
        return basic_badmsg(100);
    }
    else if (0==strcmp(argv[1], "commit_crash"))
    {
        return basic_commit_crash(100);
    }
    else if (0==strcmp(argv[1], "deqwriteerr"))
    {
        return basic_deqwriteerr(100);
    }
    else if (0==strcmp(argv[1], "abortrules"))
    {
        return basic_abort_rules(1);
    }
    else if (0==strcmp(argv[1], "errorq"))
    {
        return basic_errorq();
    }
    else if (0==strcmp(argv[1], "crashloop"))
    {
        return basic_crashloop("ERROR");
    }
    else if (0==strcmp(argv[1], "crashloop_t"))
    {
        return basic_crashloop("ERROR_T");
    }
    else if (0==strcmp(argv[1], "enqdeq"))
    {
        return basic_enqdeq(200);
    }
    else if (0==strcmp(argv[1], "txtout"))
    {
        return basic_txtout(1);
    }
    else if (0==strcmp(argv[1], "seqvalid"))
    {
        return basic_seqvalid(1000);
    }
    else
    {
        NDRX_LOG(log_error, "Invalid test case!");
        return EXFAIL;
    }
    
out:

    tpclose();

    return ret;   
}


/**
 * TMQ resource manager performs rollback due to timeout
 * @param maxmsg max messages to be ok
 */
exprivate int basic_rmrollback(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case rmrollback");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* sleep 55, as tout is set to 45 see -T */
    sleep(55);
    
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit must fail, as tmq does not know the tran");
        EXFAIL_OUT(ret);
    }

    if (TPEABORT!=tperrno)
    {
        NDRX_LOG(log_error, "TESTERROR: expected %d got %d", TPEABORT, tperrno);
        EXFAIL_OUT(ret);
    }
    
    /* no messages are available */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, must be none (all aborted)!");
            EXFAIL_OUT(ret);
        }
        
        if (TPEDIAGNOSTIC!=tperrno)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d", TPEDIAGNOSTIC, tperrno);
            EXFAIL_OUT(ret);
        }
        
        if (QMENOMSG!=qc.diagnostic)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %ld", QMENOMSG, qc.diagnostic);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}


/**
 * Validate the sequence of the message (transaction sync)
 * Just load the messages. Activation / Validation is done from shell script
 * @param maxmsg max messages to be ok
 */
exprivate int basic_seqvalid(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    long i;
    
    NDRX_LOG(log_error, "case seqvalid");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=1; i<maxmsg+1; i++)
    {
        UBFH *testbuf_ref = (UBFH *)tpalloc("UBF", "", 1024);
        
        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }
        
        if (EXSUCCEED!=Bchg(testbuf_ref, T_LONG_FLD, 0, (char *)&i, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: Bchg failed %s", 
                    Bstrerror(Berror));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "SEQVALID", &qc, (char *)testbuf_ref, 0, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree((char *)testbuf_ref);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}


/**
 * Transaction timeout - TPETRAN test (cannot join as rolled back)
 * @param maxmsg max messages to be ok
 */
exprivate int basic_txtout(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc, o_qc;
    int i;
    
    NDRX_LOG(log_error, "case txtout");
    
    /* add 1 msg.. with COR & MSGID */
    
    if (EXSUCCEED!=tpbegin(60, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&o_qc, 0, sizeof(o_qc));
        /* set corid & msgid.. */
        o_qc.flags|=(TPQCORRID);
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &o_qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), o_qc.diagnostic, o_qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* OK ... */
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit");
        EXFAIL_OUT(ret);
    }
    
    /* try to late deq join... */
    
    if (EXSUCCEED!=tpbegin(1, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    sleep(5);
    
    do
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, must be none (all aborted)!");
            EXFAIL_OUT(ret);
        }
        
        if (TPETRAN!=tperrno)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d", TPETRAN, tperrno);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    } while (0);
    
    tpabort(0);
    
    
    /* try to late deq join... corid */
    
    if (EXSUCCEED!=tpbegin(1, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    sleep(5);
    
    do
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));
        
        qc.flags|=TPQGETBYCORRID;

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, must be none (all aborted)!");
            EXFAIL_OUT(ret);
        }
        
        if (TPETRAN!=tperrno)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d", TPETRAN, tperrno);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    } while (0);
    
    
    tpabort(0);
    
    /* try to late deq join... msgid */
    
    if (EXSUCCEED!=tpbegin(1, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    sleep(5);
    
    do
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));
        
        qc.flags|=TPQGETBYMSGID;
        
        memcpy(qc.msgid, o_qc.msgid, sizeof(qc.msgid));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, must be none (all aborted)!");
            EXFAIL_OUT(ret);
        }
        
        if (TPETRAN!=tperrno)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d", TPETRAN, tperrno);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
        
    } while (0);
    
    
    tpabort(0);
    
    /* try enq to expired tran... */
    
    if (EXSUCCEED!=tpbegin(1, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    sleep(5);
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED==tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() must fail but was OK!");
            EXFAIL_OUT(ret);
        }

        /* Must be TPETRAN (but only for static mode...) 
         * For dynamic mode it is TPESVCFAIL (probably shall fix in future to TPETRAN)
         */
        if (TPETRAN!=tperrno)
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() Expected TPETRAN, got %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    
    tpabort(0);
    
    /* clean up the q finally... */
    
    if (EXSUCCEED!=tpbegin(60, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    do
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));
        
        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
        
    } while (0);
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "Failed to commit: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * We slowly add new msgs... over the 30 sec session -> no rollback as
 * counter is being reset
 * @param maxmsg max messages to be ok
 */
exprivate int basic_rmnorollback(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case rmnorollback");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<6; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
        
        /* overall 60 sec... and shall complete all OK */
        sleep(10);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
    /* Download all msgs OK */
    for (i=0; i<6; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeue failed!: %s %ld", 
                    tpstrerror(tperrno), qc.diagnostic);
            EXFAIL_OUT(ret);
        }
        tpfree(buf);
    }


out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}


/**
 * Check out message performance -> no sleep if have any task.
 * @param maxmsg max messages to be ok
 */
exprivate int basic_autoperf(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    char *buf = NULL;
    char *testbuf_ref = tpalloc("CARRAY", "", 10);
    long len=10;

    /* alloc output buffer */
    if (NULL==testbuf_ref)
    {
        NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }

    testbuf_ref[0]=0;
    testbuf_ref[1]=1;
    testbuf_ref[2]=2;
    testbuf_ref[3]=3;
    testbuf_ref[4]=4;
    testbuf_ref[5]=5;
    testbuf_ref[6]=6;
    testbuf_ref[7]=7;
    testbuf_ref[8]=8;
    testbuf_ref[9]=9;

    NDRX_LOG(log_error, "case autoperf");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<200; i++)
    {        
        /* add 1 queues, to pull in the forward lists... */
        if (i<1)
        {
            memset(&qc, 0, sizeof(qc));
            if (EXSUCCEED!=tpenqueue("MYSPACE", "PERF1", &qc, testbuf_ref, 
                len, 0))
            {
                NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
                EXFAIL_OUT(ret);
            }
        }
        
        /* middle queues have more, had issues that trailing queues made sleep */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "PERF2", &qc, testbuf_ref, 
            len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "PERF3", &qc, testbuf_ref, 
            len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        /* order of Q is important here as it spawns queues... and later uses
         * scan in that order
         */
        if (i<1)
        {
            memset(&qc, 0, sizeof(qc));
            if (EXSUCCEED!=tpenqueue("MYSPACE", "PERF4", &qc, testbuf_ref, 
                len, 0))
            {
                NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
                EXFAIL_OUT(ret);
            }
        }

    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    /* all queues must be empty! */
    sleep(60);
    
    buf = tpalloc("CARRAY", "", 100);

    if (NULL==buf)
    {
        NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }

    len=100;
    memset(&qc, 0, sizeof(qc));
    if (EXSUCCEED==tpdequeue("MYSPACE", "PERF1", &qc, (char **)&buf, &len, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: PERF1 must be empty!");
        EXFAIL_OUT(ret);
    }

    memset(&qc, 0, sizeof(qc));
    if (EXSUCCEED==tpdequeue("MYSPACE", "PERF2", &qc, (char **)&buf, &len, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: PERF2 must be empty!");
        EXFAIL_OUT(ret);
    }

    memset(&qc, 0, sizeof(qc));
    if (EXSUCCEED==tpdequeue("MYSPACE", "PERF3", &qc, (char **)&buf, &len, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: PERF3 must be empty!");
        EXFAIL_OUT(ret);
    }

    memset(&qc, 0, sizeof(qc));
    if (EXSUCCEED==tpdequeue("MYSPACE", "PERF4", &qc, (char **)&buf, &len, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: PERF4 must be empty!");
        EXFAIL_OUT(ret);
    }

out:
    if (NULL!=testbuf_ref)
    {
        tpfree(testbuf_ref);
    }

    if (NULL!=buf)
    {
        tpfree(buf);
    }
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * Verify that we can process number of messages
 * @param maxmsg max messages to be ok
 */
exprivate int basic_tmqrestart(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case tmqrestart");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* restart tmqueue.... 
     * it shall be able to commit OK
     */
    if (EXSUCCEED!=system("xadmin stop -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop tmqueue");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=system("xadmin start -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start tmqueue");
        EXFAIL_OUT(ret);
    }
    
    /* no messages available OK */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, even already in progress!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit must fail, as tmq does not know the tran");
        EXFAIL_OUT(ret);
    }

    if (TPEABORT!=tperrno)
    {
        NDRX_LOG(log_error, "TESTERROR: expected %d got %d", TPEABORT, tperrno);
        EXFAIL_OUT(ret);
    }
    
    /* check that number of messages are available... */
    NDRX_LOG(log_error, "Try to dequeue messages after tran restart...");
    
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* all messages shall be removed even after tran restart */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, but all must be aborted!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit");
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * Verify that we can handle tmsrv restarts in the middle of transaction
 * @param maxmsg max messages to be ok
 */
exprivate int basic_tmsrvrestart(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_tmsrvrestart");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* restart tmsrv.... 
     * it shall be able to commit OK
     */
    if (EXSUCCEED!=system("xadmin stop -s tmsrv"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop tmsrv");
        EXFAIL_OUT(ret);
    }

    /* restart tmqueue.... / reload msgs... */
    if (EXSUCCEED!=system("xadmin stop -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop tmqueue");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=system("xadmin start -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start tmqueue");
        EXFAIL_OUT(ret);
    }

    /* also.. here all messages shall be locked */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        /* This shall be updated so that we do not need to use TPNOABORT  */
        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, even already in progress!");
            EXFAIL_OUT(ret);
        }

        tpfree(buf);
    }
        
    /* dequeue must fail as we have already dequeued the messages */
    /* try just 1 msg. */
    
    if (EXSUCCEED!=system("xadmin start -s tmsrv"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start tmsrv");
        EXFAIL_OUT(ret);
    }

    /* let tmsrv to load the logs in background... and perform abort... */
    sleep(20);
    
    /* also.. here all message shall be removed. */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        /* This shall be updated so that we do not need to use TPNOABORT  */
        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, even already in progress!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit");
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}


/**
 * Try commit, queue is shutdown
 * It shall abort with retries... boot tmqueue back, no messages shall be available
 * @param maxmsg max messages to be ok
 */
exprivate int basic_commit_shut(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_commit_shut");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* restart tmqueue.... 
     * it shall be able to commit OK
     */
    if (EXSUCCEED!=system("xadmin stop -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop tmqueue");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit must fail");
        EXFAIL_OUT(ret);
    }
    
    /* we shall get abort error... */
    if (TPEHAZARD!=tperrno)
    {
        NDRX_LOG(log_error, "TESTERROR: invalid error, expected TPEHAZARD got %d",
                tperrno);
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=system("xadmin start -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start tmqueue");
        EXFAIL_OUT(ret);
    }
    
    /* no messages shall be available, refactored after initial design. */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeue OK, must fail!");
            EXFAIL_OUT(ret);
        }

        /* check error no msg */
        if (tperrno!=TPEDIAGNOSTIC)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d err!", TPEDIAGNOSTIC, tperrno);
            EXFAIL_OUT(ret);
        }

        if (qc.diagnostic!=QMENOMSG)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d err (diag)!", QMENOMSG, qc.diagnostic);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * Dequeue write error (disk full on command file) must be reported
 * @param maxmsg max messages to be ok
 */
exprivate int basic_deqwriteerr(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_deqwriteerr");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit failed: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=system("xadmin lcf qwriterr -A 1 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: xadmin lcf qwriterr -A 1 -a failed");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* all messages must be available */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeue must fail!!");
            EXFAIL_OUT(ret);
        }
        
        if (tperrno!=TPEDIAGNOSTIC)
        {
            NDRX_LOG(log_error, "TESTERROR: TPEDIAGNOSTIC expected, got: %d!", tperrno);
            EXFAIL_OUT(ret);
        }
        
        if (QMEOS!=qc.diagnostic)
        {
            NDRX_LOG(log_error, "TESTERROR: QMEOS expected, got: %d!", qc.diagnostic);
            EXFAIL_OUT(ret);
        }   
        tpfree(buf);

    }
    
    /* terminate the transaction */
    tpabort(0);
    
    
    if (EXSUCCEED!=system("xadmin lcf qwriterr -A 0 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: xadmin lcf qwriterr -A 0 -a failed");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* all messages must be available */
    for (i=0; i<maxmsg; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeue failed!: %s %ld", 
                    tpstrerror(tperrno), qc.diagnostic);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
        
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to tpcommit");
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * Load prepared messages (also tmq scans this twice, thus check that no problem)
 * @param maxmsg
 * @return 
 */
exprivate int basic_loadprep(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_loadprep");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* restart tmqueue.... 
     * it shall be able to commit OK
     */
    if (EXSUCCEED!=system("xadmin stop -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop tmqueue");
        EXFAIL_OUT(ret);
    }
    
    /* move files from active to prep... 
     * we will not commit, but tmqueue startup shall not fail.
     */
    
    if (EXSUCCEED!=system("mv QSPACE1/active/* QSPACE1/prepared/"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to move transaction files...");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=system("xadmin start -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start tmqueue");
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * Check the case when disk is full, shall fail to enqueue
 * @param maxmsg max messages to be ok
 */
exprivate int basic_diskfull(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_diskfull");
    
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Set disk failure on for all processes...
     */
    if (EXSUCCEED!=system("xadmin lcf qwriterr -A 1 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to enable write failure");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        
        if (EXSUCCEED==tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() must fail - disk full");
            EXFAIL_OUT(ret);
        }
        
        NDRX_LOG(log_error, "tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
        
        if (TPEDIAGNOSTIC!=tperrno || QMEOS!=qc.diagnostic)
        {
            NDRX_LOG(log_error, "TESTERROR: expected tperrno==TPEDIAGNOSTIC got %d and qc.diagnostic==QMESYSTEM got %d",
                    tperrno, qc.diagnostic);
            EXFAIL_OUT(ret);
        }
        
        tpfree(testbuf_ref);
    }
    
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: it shall fail to commit, as transactions ar marked for abort!");
        EXFAIL_OUT(ret);
    }
    
    if (TPEABORT!=tperrno)
    {
        NDRX_LOG(log_error, "TESTERROR: Expected TPEABORT got %d", tperrno);
        EXFAIL_OUT(ret);
    }
    
    /* reset write error back to norm. */
    if (EXSUCCEED!=system("xadmin lcf qwriterr -A 0 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to enable write failure");
        EXFAIL_OUT(ret);
    }
    
    /* restart tmqueue.... no message shall be available as no files
     * are saved due to write error
     */
    if (EXSUCCEED!=system("xadmin stop -s tmqueue; xadmin start -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop tmqueue");
        EXFAIL_OUT(ret);
    }
    
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        /* This shall be updated so that we do not need to use TPNOABORT  */
        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, even already in progress!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * tmsrv fails to log transaction - disk full  / abort
 * @param maxmsg
 * @return 
 */
exprivate int basic_tmsrvdiskerr(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_tmsrvdiskerr");
    
    if (EXSUCCEED!=tpbegin(20, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;
        
        if (i==1)
        {
            /* Set disk failure for tmsrv - now tmqueue is joined...
             */
            if (EXSUCCEED!=system("xadmin lcf twriterr -A 1 -a"))
            {
                NDRX_LOG(log_error, "TESTERROR: failed to enable write failure");
                EXFAIL_OUT(ret);
            }
        }

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }
        
        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* commit shall fail, as failed to log stuff
     * abort can complete with out disk
     */
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: it shall fail to commit, as transactions ar marked for abort!");
        EXFAIL_OUT(ret);
    }
    
    /* stuff shall be rolled back... */
    /* maybe better it would be to have TPEABORT, but current if storage is not working
     * we will give TPEOS error
     */
    if (TPEABORT!=tperrno)
    {
        NDRX_LOG(log_error, "TESTERROR: Expected TPEABORT got %d", tperrno);
        EXFAIL_OUT(ret);
    }
    
    /* no messages in queue, as rolled back.. */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        /* This shall be updated so that we do not need to use TPNOABORT  */
        if (EXSUCCEED==tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, TPNOABORT))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 dequeued, even already in progress!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }

    
out:
    
    /* reset write error back to norm. */
    if (EXSUCCEED!=system("xadmin lcf twriterr -A 0 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to enable write failure");
        EXFAIL_OUT(ret);
    }

    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/**
 * Skip bad messages on the disk
 * @param maxmsg max messages to be ok
 */
exprivate int basic_badmsg(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_badmsg");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    /* create a bad file */
    if (EXSUCCEED!=system("touch QSPACE1/active/some_bad_message_file"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to create bad message file...");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit got: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=system("xadmin sreload -s tmqueue"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to retart tmqueue");
        EXFAIL_OUT(ret);
    }
    
    /* all messages must be available - bad file ignored...*/
    for (i=0; i<maxmsg; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 failed dequeue!");
            EXFAIL_OUT(ret);
        }
        tpfree(buf);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    /* create a bad file */
    if (system("rm QSPACE1/active/some_bad_message_file"))
    {
        /* avoid warning... */
    }
    
    return ret;
}


/**
 * Simulate commit crash & recovery
 * We enqueue data.
 * Commit fails (change stage to committing, thus we perform automatic
 * rollback)
 * @param maxmsg max messages to be ok
 */
exprivate int basic_commit_crash(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_commit_crash");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit failed: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
    /* start to dequeue... */
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* all messages must be available */
    for (i=0; i<maxmsg; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 failed dequeue!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
    /* 
     * Set crash point
     */
    if (EXSUCCEED!=system("xadmin lcf tcrash -A 50 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to enable crash");
        EXFAIL_OUT(ret);
    }
    
    /* set timeout time  
     * commit will fail...
     * and all records will be rolled back (assuming in 30 sec)
     */
    tptoutset(30);
    
    if (EXSUCCEED==tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit must fail");
        EXFAIL_OUT(ret);
    }
    
    if (tperrno!=TPETIME)
    {
        NDRX_LOG(log_error, "TESTERROR: expected TPETIME got %d", tperrno);
        EXFAIL_OUT(ret);
    }
    
    /* set timeout back... */
    tptoutset(90);
    
    if (EXSUCCEED!=system("xadmin lcf tcrash -A 0 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to disable crash");
        EXFAIL_OUT(ret);
    }
    
    /* let tmsrv to flush all stuff.... */
    sleep(20);
    /* all messages must be available (rolled back after crash recovery) */
    for (i=0; i<maxmsg; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 failed dequeue!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}


/**
 * Test normal enqueue/dequeue operation
 * @param maxmsg max messages to be ok
 */
exprivate int basic_enqdeq(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case enqdeq");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=i%128;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "TEST1", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit");
        EXFAIL_OUT(ret);
    }
    
    /* check that number of messages are available... */
    NDRX_LOG(log_error, "About to dequeue %d messages", maxmsg);
    
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* all messages must be available */
    for (i=0; i<maxmsg; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "TEST1", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: TEST1 failed dequeue!");
            EXFAIL_OUT(ret);
        }
        
        /* verify the message number is it correct? 
         * as we have fifo queue...
         */
        if (buf[0]!=i%128)
        {
            NDRX_LOG(log_error, "TESTERROR: Expected %d at %i, got %d",
                    (int)i%128, i, (int)buf[0]);
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to commit");
        EXFAIL_OUT(ret);
    }
    
out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}


/**
 * Transaction forward crashes. Also ensure that tmsrv instance does not boot
 * back. But we start normal tmsrv instances, to ensure that errorq op can
 * be completed.
 * @param maxmsg max messages to be ok
 */
exprivate int basic_fwdcrash(int maxmsg)
{
    int ret = EXSUCCEED;
    TPQCTL qc;
    int i;
    
    NDRX_LOG(log_error, "case basic_commit_crash");
    if (EXSUCCEED!=tpbegin(9999, 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to begin");
        EXFAIL_OUT(ret);
    }
    
    /* Initial test... */
    for (i=0; i<maxmsg; i++)
    {
        char *testbuf_ref = tpalloc("CARRAY", "", 10);
        long len=10;

        testbuf_ref[0]=0;
        testbuf_ref[1]=1;
        testbuf_ref[2]=2;
        testbuf_ref[3]=3;
        testbuf_ref[4]=4;
        testbuf_ref[5]=5;
        testbuf_ref[6]=6;
        testbuf_ref[7]=7;
        testbuf_ref[8]=8;
        testbuf_ref[9]=9;

        /* alloc output buffer */
        if (NULL==testbuf_ref)
        {
            NDRX_LOG(log_error, "TESTERROR: tpalloc() failed %s", 
                    tpstrerror(tperrno));
            EXFAIL_OUT(ret);
        }

        /* enqueue the data buffer */
        memset(&qc, 0, sizeof(qc));
        if (EXSUCCEED!=tpenqueue("MYSPACE", "CRASHQ", &qc, testbuf_ref, 
                len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: tpenqueue() failed %s diag: %d:%s", 
                    tpstrerror(tperrno), qc.diagnostic, qc.diagmsg);
            EXFAIL_OUT(ret);
        }

        tpfree(testbuf_ref);
    }
    
    if (EXSUCCEED!=tpcommit(0))
    {
        NDRX_LOG(log_error, "TESTERROR: commit failed: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
    /* prepare crash setup */
    if (EXSUCCEED!=system("xadmin stop -i 50"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop 50 inst");
        EXFAIL_OUT(ret);
    }

    if (EXSUCCEED!=system("xadmin start -i 60"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start 60 inst");
        EXFAIL_OUT(ret);
    }

    /* prepare will crash the tmsrv. thus no msgs will stay prepard & 
     * and tout dmn will rollback
     */
    if (EXSUCCEED!=system("xadmin lcf tcrash -A 40 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to enable commit crash");
        EXFAIL_OUT(ret);
    }

    /* enable forward */
    if (EXSUCCEED!=system("xadmin mqch -n1 -i 100 -qCRASHQ,autoq=y"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed enable forward");
        EXFAIL_OUT(ret);
    }

    /* Let some messages to stuck...*/ 
    sleep(15);

    if (EXSUCCEED!=system("xadmin lcf tcrash -A 0 -a"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to disable commit crash");
        EXFAIL_OUT(ret);
    }

    if (EXSUCCEED!=system("xadmin stop -i 60"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to stop 60 inst");
        EXFAIL_OUT(ret);
    }

    if (EXSUCCEED!=system("xadmin start -i 50"))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to start 50 inst");
        EXFAIL_OUT(ret);
    }

    /* let messages to unlock & forward to CRASHERR
     * 90 - is tout.. thus have some more time for stucked msgs...
     */
    sleep(120);

    /* all messages must be available */
    for (i=0; i<maxmsg; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED!=tpdequeue("MYSPACE", "CRASHERR", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: CRASHERR failed dequeue!");
            EXFAIL_OUT(ret);
        }
        
        tpfree(buf);
    }

    /* no messages shall be available */
    for (i=0; i<1; i++)
    {
        long len=0;
        char *buf;
        buf = tpalloc("CARRAY", "", 100);
        memset(&qc, 0, sizeof(qc));

        if (EXSUCCEED==tpdequeue("MYSPACE", "CRASHERR", &qc, (char **)&buf, &len, 0))
        {
            NDRX_LOG(log_error, "TESTERROR: CRASHERR must not dequeue!");
            EXFAIL_OUT(ret);
        }

        /* check error no msg */
        if (tperrno!=TPEDIAGNOSTIC)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d err!", TPEDIAGNOSTIC, tperrno);
            EXFAIL_OUT(ret);
        }

        if (qc.diagnostic!=QMENOMSG)
        {
            NDRX_LOG(log_error, "TESTERROR: expected %d got %d err (diag)!", QMENOMSG, qc.diagnostic);
            EXFAIL_OUT(ret);
        }

        tpfree(buf);
    }

out:
    
    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
        ret=EXFAIL;
        goto out;
    }

    return ret;
}

/* vim: set ts=4 sw=4 et smartindent: */
