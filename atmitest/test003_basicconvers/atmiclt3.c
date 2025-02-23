/**
 * @brief Basic test client
 *
 * @file atmiclt3.c
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

#include <atmi.h>
#include <ubf.h>
#include <ndebug.h>
#include <test.fd.h>
#include <ndrstandard.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

/*
 * Do the test call to the server
 */
int main(int argc, char** argv) {

    UBFH *p_ub = (UBFH *)tpalloc("UBF", NULL, 4048);
    long rsplen;
    int i;
    int ret=EXSUCCEED;
    double d;
    double dv = 55.66;
    int cd;
    long revent;
    int received = 0;
    char tmp[126];
    long len;

    /* add test case selector normal vs timeout vs invalid arg... */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s normal|timeout\n", argv[0]);
        exit(EXFAIL);
    }
    else if (0==strcmp(argv[1], "normal"))
    {
        
        Badd(p_ub, T_STRING_FLD, "THIS IS TEST FIELD 1", 0);
        Badd(p_ub, T_STRING_FLD, "THIS IS TEST FIELD 2", 0);
        Badd(p_ub, T_STRING_FLD, "THIS IS TEST FIELD 3", 0);

        if (EXFAIL==(cd=tpconnect("CONVSV", (char *)p_ub, 0L, TPRECVONLY)))
        {
            NDRX_LOG(log_error, "TESTSV connect failed!: %s",
                                    tpstrerror(tperrno));
            ret=EXFAIL;
            goto out;
        }

        /* Recieve the stuff back */
        NDRX_LOG(log_debug, "About to tprecv!");

        while (EXSUCCEED==tprecv(cd, (char **)&p_ub, &len, 0L, &revent))
        {
            received++;
            NDRX_LOG(log_debug, "MSG RECEIVED OK!");
        }
        

        /* If we have event, we would like to become recievers if so */
        if (TPEEVENT==tperrno)
        {
            received++;
            snprintf(tmp, sizeof(tmp), "CLT: %d", received);
            
            Bprint(p_ub);
            Badd(p_ub, T_STRING_FLD, tmp, 0L);
            if (TPEV_SENDONLY==revent)
            {
                int i=0;
                /* Start the sending stuff now! */
                for (i=0; i<100 && EXSUCCEED==ret; i++)
                {
                    ret=tpsend(cd, (char *)p_ub, 0L, 0L, &revent);
                }
            }
        }

        /* Now give the control to the server, so that he could finish up */
        if (EXFAIL==tpsend(cd, NULL, 0L, TPRECVONLY, &revent))
        {
            NDRX_LOG(log_debug, "Failed to give server control!!");
            ret=EXFAIL;
            goto out;
        }

        NDRX_LOG(log_debug, "Get response from tprecv!");
        Bfprint(p_ub, stderr);

        /* Wait for return from server */
        ret=tprecv(cd, (char **)&p_ub, &len, 0L, &revent);
        NDRX_LOG(log_error, "tprecv failed with revent=%ld tperrno=%d", revent, tperrno);

        if (EXFAIL==ret && TPEEVENT==tperrno && TPEV_SVCSUCC==revent)
        {
            NDRX_LOG(log_error, "Service finished with TPEV_SVCSUCC!");
            ret=EXSUCCEED;
        }
        
        /* check that we do not core dump when trying to fetch from closed connection */
        ret=tprecv(cd, (char **)&p_ub, &len, 0L, &revent);
        
        if (EXSUCCEED==ret)
        {
            NDRX_LOG(log_error, "TESTERROR ! Error shall be generated when "
                    "fetching from closed connection!");
            EXFAIL_OUT(ret);
        }
        else
        {
            ret = EXSUCCEED;
        }
        
        if (tperrno!=TPEINVAL)
        {
            NDRX_LOG(log_error, "TESTERROR ! Expected err %d got %d!", 
                    TPEINVAL, tperrno);
            EXFAIL_OUT(ret);
        }
        
    } 
    else if (0==strcmp(argv[1], "timeout"))
    {

        /* kill the atmisv -> conn will fail.. 
        system("xadmin killall atmisv3");*/

        /* test for timeout */
        if (EXFAIL!=(cd=tpconnect("TOUTSV", (char *)p_ub, 0L, TPRECVONLY)))
        {
            NDRX_LOG(log_error, "TOUTSV not failed!");
            ret=EXFAIL;
            goto out;
        }

        if (tperrno!=TPETIME)
        {
            NDRX_LOG(log_error, "TESTERROR ! Expected err %d got %d!", 
                    TPETIME, tperrno);
            EXFAIL_OUT(ret);
        }
    }
    else
    {
        NDRX_LOG(log_error, "TESTERROR: invalid test case [%s]", argv[1]);
        ret=EXFAIL;
        goto out;
    }
    
out:

    if (EXSUCCEED!=tpterm())
    {
        NDRX_LOG(log_error, "tpterm failed with: %s", tpstrerror(tperrno));
    }

    return ret;
}

/* vim: set ts=4 sw=4 et smartindent: */
