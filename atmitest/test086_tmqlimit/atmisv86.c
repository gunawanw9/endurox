/**
 * @brief Service reply responder
 *
 * @file atmisv86.c
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
#include <stdio.h>
#include <stdlib.h>
#include <ndebug.h>
#include <atmi.h>
#include <ndrstandard.h>
#include <ubf.h>
#include <test.fd.h>
#include <string.h>
#include <unistd.h>

/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/

exprivate long M_seq = 0;

/*---------------------------Prototypes---------------------------------*/

/**
 * Sequence validator
 */
void SEQVALID (TPSVCINFO *p_svc)
{
    int ret = EXSUCCEED;
    long l;
    UBFH *p_ub = (UBFH *)p_svc->data;
    
    /* read the sequence field T_LONG_FLD must start from 1 */
    if (EXSUCCEED!=Bget(p_ub, T_LONG_FLD, 0, (char *)&l, NULL))
    {
        NDRX_LOG(log_always, "TESTERROR: T_LONG_FLD is missing!");
        EXFAIL_OUT(ret);
    }
    
    /* Validate the sequence of the call: */
    if (M_seq+1!=l)
    {
        NDRX_LOG(log_always, "TESTERROR: Invalid service call sequence: got %ld expected %ld", 
                l, M_seq+1);
        userlog("TESTERROR: Invalid service call sequence: got %ld expected %ld", 
                l, M_seq+1);
        EXFAIL_OUT(ret);
    }
    M_seq = l;

out:
    tpreturn(  (EXSUCCEED==ret?TPSUCCESS:TPFAIL),
                0L,
                (char *)p_ub,
                0L,
                0L);
}

/**
 * Standard service entry
 */
void FAILSVC (TPSVCINFO *p_svc)
{
    UBFH *p_ub = (UBFH *)p_svc->data;

    NDRX_LOG(log_debug, "%s got call", __func__);
    
    
    if (EXSUCCEED!=Bchg(p_ub, T_STRING_FLD, 0, "RSP", 0))
    {
        NDRX_LOG(log_error, "TESTERROR: failed to update buffer: %s",
            Bstrerror(Berror));
    }
    
    
out:
    tpreturn(  TPFAIL,
                0L,
                (char *)p_ub,
                0L,
                0L);
}

/**
 * Standard service entry
 */
void OKSVC (TPSVCINFO *p_svc)
{
    UBFH *p_ub = (UBFH *)p_svc->data;

out:
    tpreturn(  TPSUCCESS,
                0L,
                (char *)p_ub,
                0L,
                0L);
}

/**
 * Standard service entry
 */
void TOUT_SLEEP (TPSVCINFO *p_svc)
{
    UBFH *p_ub = (UBFH *)p_svc->data;
    
    sleep(35);

out:
    tpreturn(  TPSUCCESS,
                0L,
                (char *)p_ub,
                0L,
                0L);
}

/**
 * Do initialisation
 */
int NDRX_INTEGRA(tpsvrinit)(int argc, char **argv)
{
    int ret = EXSUCCEED;
    NDRX_LOG(log_debug, "tpsvrinit called");

    if (EXSUCCEED!=tpadvertise("FAILSVC", FAILSVC))
    {
        NDRX_LOG(log_error, "Failed to initialise FAILSVC!");
        EXFAIL_OUT(ret);
    }

    if (EXSUCCEED!=tpadvertise("OKSVC", OKSVC))
    {
        NDRX_LOG(log_error, "Failed to initialise OKSVC!");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpadvertise("SEQVALID", SEQVALID))
    {
        NDRX_LOG(log_error, "Failed to initialise SEQVALID!");
        EXFAIL_OUT(ret);
    }
    
    
    if (EXSUCCEED!=tpadvertise("T_OK", TOUT_SLEEP))
    {
        NDRX_LOG(log_error, "Failed to initialise T_OK!");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpadvertise("T_NOK", TOUT_SLEEP))
    {
        NDRX_LOG(log_error, "Failed to initialise T_NOK!");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpadvertise("NS_OK", TOUT_SLEEP))
    {
        NDRX_LOG(log_error, "Failed to initialise NS_OK!");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpadvertise("ND_OK", TOUT_SLEEP))
    {
        NDRX_LOG(log_error, "Failed to initialise ND_OK!");
        EXFAIL_OUT(ret);
    }
    
    if (EXSUCCEED!=tpadvertise("ND_NOK", TOUT_SLEEP))
    {
        NDRX_LOG(log_error, "Failed to initialise ND_NOK!");
        EXFAIL_OUT(ret);
    }
    
    
    if (EXSUCCEED!=tpopen())
    {
        NDRX_LOG(log_error, "tpopen() failed: %s", tpstrerror(tperrno));
        EXFAIL_OUT(ret);
    }
    
out:
    return ret;
}

/**
 * Do de-initialisation
 */
void NDRX_INTEGRA(tpsvrdone)(void)
{
    NDRX_LOG(log_debug, "tpsvrdone called");
    tpclose();
}

/* vim: set ts=4 sw=4 et smartindent: */
