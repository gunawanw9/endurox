/**
 * @brief tpreturn function implementation.
 *
 * @file tpreturn.c
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
#include <setjmp.h>
#include <errno.h>

#include <atmi.h>
#include <ndebug.h>
#include <tperror.h>
#include <typed_buf.h>
#include <atmi_int.h>
#include <srv_int.h>
#include <gencall.h>
#include <atmi_shm.h>

#include <xa_cmn.h>
#include <userlog.h>

#include "atmi_tls.h"
#include <ndrx_ddr.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

/**
 * Detect are we still part of our transaction
 * @return EXSUCCEED/EXFAIL
 */
exprivate int initiated_tran_attached(void)
{
    if (NULL!=G_atmi_tls->G_atmi_xa_curtx.txinfo
            && (G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags & XA_TXINFO_INITIATOR)
            && !(ndrx_get_G_last_call()->sysflags & SYS_FLAG_AUTOTRAN))
    {
        return EXTRUE;
    }
    
    return EXFALSE;
}

/**
 * Internal version to tpreturn.
 * This is
 * TODO: If we are in thread, then disassoc of global txn must happen here!
 * In case if we are initiators of txn, and it is still running, we return
 * TPSVCERR. And transaction is aborted.
 * @param rval
 * @param rcode
 * @param data
 * @param len
 * @param flags
 */
expublic void _tpreturn (int rval, long rcode, char *data, long len, long flags)
{
    int ret=EXSUCCEED;
    char *buf=NULL; /**< physical place where to put the reply */
    size_t buf_len; /**< buf_len */
    tp_command_call_t *call;
    char fn[] = "_tpreturn";
    buffer_obj_t *buffer_info;
    long data_len;
    int return_status=0;
    char reply_to[NDRX_MAX_Q_SIZE+1] = {EXEOS};
    atmi_lib_conf_t *p_atmi_lib_conf = ndrx_get_G_atmi_conf();
    tp_conversation_control_t *p_accept_conn = ndrx_get_G_accepted_connection();
    tp_command_call_t * last_call;
    int was_auto_buf = EXFALSE;
    
    NDRX_LOG(log_debug, "%s enter data=%p", fn, data);
    last_call = ndrx_get_G_last_call();
    
    if (last_call->flags & TPNOREPLY)
    {
        NDRX_LOG(log_debug, "No reply required (TPNOREPLY) - return to main() "
                "flags: %ld", last_call->flags);
        
        /* commit or abort .. if autotran was started */
        if (last_call->sysflags & SYS_FLAG_AUTOTRAN
            && tpgetlev())
        {
            /* try to commit */
            if (rval==TPSUCCESS)
            {
                if (EXSUCCEED!=ndrx_tpcommit(0))
                {
                    NDRX_LOG(log_error, "Auto commit failed: %s ", tpstrerror(tperrno));
                    userlog("Auto commit failed: %s", tpstrerror(tperrno));
                }
            }
            else
            {
                if (EXSUCCEED!=ndrx_tpabort(0, EXTRUE))
                {
                    NDRX_LOG(log_error, "Auto abort failed: %s", tpstrerror(tperrno));
                    userlog("Auto abort failed: %s", tpstrerror(tperrno));
                }
            }
        }
        
        /* If our tran is open, terminate it... */
        if (initiated_tran_attached())
        {
            NDRX_LOG(log_error, "Still in transaction [%s] - aborting",
                    G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
            userlog("Still in transaction [%s] - aborting",
                    G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        
            if (EXSUCCEED!=ndrx_tpabort(0, EXTRUE))
            {
                NDRX_LOG(log_error, "Auto abort failed: %s", tpstrerror(tperrno));
                userlog("Auto abort failed: %s", tpstrerror(tperrno));
            }
        }
        
        goto out;
    }

    /* If our tran is open, terminate it... */
    if (initiated_tran_attached())
    {
        /* return TPESVCERR back to caller */
        rcode=TPESVCERR;
        flags=TPSOFTERR;
        
        NDRX_LOG(log_error, "Still in transaction [%s] - aborting, returning TPESVCERR",
                    G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        userlog("Still in transaction [%s] - aborting, returning TPESVCERR",
                    G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        if (EXSUCCEED!=ndrx_tpabort(0, EXTRUE))
        {
            NDRX_LOG(log_error, "Auto abort failed: %s", tpstrerror(tperrno));
            userlog("Auto abort failed: %s", tpstrerror(tperrno));
        }
    }
    
    /* client with last call is acceptable...! 
     * As it can be a server's companion thread
     */
    if (p_atmi_lib_conf->is_client && !last_call->cd)
    {
        /* this is client */
        NDRX_LOG(log_debug, "tpreturn is not available for clients "
                "(is_client=%d, cd=%d)!!!", p_atmi_lib_conf->is_client, 
                last_call->cd);
        ndrx_TPset_error_fmt(TPEPROTO, "tpreturn - not available for clients!!!");
        return; /* <<<< RETURN */
    }
    
    NDRX_SYSBUF_MALLOC_WERR_OUT(buf, buf_len, ret);
    call =(tp_command_call_t *)buf;

    memset(call, 0, sizeof(*call));

    close_open_client_connections(); /* disconnect any calls when we are clients */

    /* Set descriptor */
    call->cd = last_call->cd;

    if (CONV_IN_CONVERSATION==p_accept_conn->status)
    {
        call->cd-=NDRX_CONV_UPPER_CNT;
        call->msgseq = p_accept_conn->msgseqout;
        p_accept_conn->msgseqout++;
    }

    call->timestamp = last_call->timestamp;
    /* store the client timeout setting, so that bridge can drop on expiry */
    call->clttout = last_call->clttout;
    call->callseq = last_call->callseq;
    call->sysflags = 0; /* reset the flags. */
    
    /* TODO: put our call node id? As source which generated reply? */

    /* Send our queue path back 
    strcpy(call->reply_to, G_atmi_conf.reply_q_str);
    */
    /* Save original reply to path, so that bridge knows what to next */
    NDRX_STRCPY_SAFE(call->reply_to, last_call->reply_to);
    
    /* Mark as service failure. */
    if (TPSUCCESS!=rval)
    {
        return_status|=RETURN_SVC_FAIL;
    }
    
    if (last_call->sysflags & SYS_FLAG_AUTOTRAN
            && tpgetlev()) /* if we are in the transaction. */
    {
        /* tpcommit() or tpabort() 
         * at this point we need to ignore the ownership of the transaction.
         */
        if (rval==TPSUCCESS)
        {
            /* try to commit */
            if (EXSUCCEED!=ndrx_tpcommit(0))
            {
                NDRX_LOG(log_error, "Auto commit failed: %s - returning TPESVCERR", 
                        tpstrerror(tperrno));
                userlog("Auto commit failed: %s - returning TPESVCERR", 
                        tpstrerror(tperrno));
                
                call->sysflags |=SYS_FLAG_REPLY_ERROR;
                call->rcode = TPESVCERR;
                ret=EXFAIL;
                goto out_send;
                
            }
        }
        else
        {
            /* try to abort */
            if (EXSUCCEED!=ndrx_tpabort(0, EXTRUE))
            {
                NDRX_LOG(log_error, "Auto abort failed: %s - returning TPESVCERR", 
                        tpstrerror(tperrno));
                userlog("Auto abort failed: %s - returning TPESVCERR", 
                        tpstrerror(tperrno));
                
                call->sysflags |=SYS_FLAG_REPLY_ERROR;
                call->rcode = TPESVCERR;
                ret=EXFAIL;
                goto out_send;
                
            }
        }
        
    }
    /* work out the XA data */
    else if (ndrx_get_G_atmi_xa_curtx()->txinfo)
    {
        /* Update the list  
        strcpy(call->tmknownrms, ndrx_get_G_atmi_xa_curtx()->txinfo->tmknownrms);
        strcpy(call->tmxid, ndrx_get_G_atmi_xa_curtx()->txinfo->tmxid);
         * */
        XA_TX_COPY(call, ndrx_get_G_atmi_xa_curtx()->txinfo);
    }
    
    /* will override later */
    call->rcode = rcode;
    /* prepare reply buffer */
    if (TPFAIL==rval || TPSUCCESS==rval || TPEXIT==rval)
    {
        /* try convert the data */
        if (NULL==(buffer_info = ndrx_find_buffer(data)))
        {
            NDRX_LOG(log_error, "Err: No buffer as %p registered in system", data);
            /* set reply fail FLAG */
            call->sysflags |=SYS_FLAG_REPLY_ERROR;
            call->rcode = TPESVCERR;
            ret=EXFAIL;
        }
        else
        {
            /* Convert back, if convert flags was set */
            if (SYS_SRV_CVT_ANY_SET(last_call->sysflags))
            {
                if (buffer_info == last_call->autobuf)
                {
                    was_auto_buf=EXTRUE;
                }
                
                NDRX_LOG(log_debug, "about reverse xcvt...");
                /* Convert buffer back.. */
                if (EXSUCCEED!=typed_xcvt(&buffer_info, last_call->sysflags, EXTRUE))
                {
                    NDRX_LOG(log_debug, "Failed to convert buffer back to "
                            "callers format: %llx", last_call->sysflags);
                    userlog("Failed to convert buffer back to "
                            "callers format: %llx", last_call->sysflags);
                    /* set reply fail FLAG */
                    call->sysflags |=SYS_FLAG_REPLY_ERROR;
                    call->rcode = TPESVCERR;
                    ret=EXFAIL;
                }
                else
                {
                    data = buffer_info->buf;
                    /* Assume that length not used for self describing buffers */
                    /* Bug #250 restore auto buf if was so... */
                    if (was_auto_buf)
                    {
                        last_call->autobuf = buffer_info;
                    }
                }
            }
            
            if (EXFAIL!=ret)
            {
                /* otherwise cannot generate error */
                call->data_len = MAX_CALL_DATA_SIZE;
                /* build reply data here */
                if (EXFAIL==ndrx_mbuf_prepare_outgoing (data, 
                            len, call->data, &call->data_len, flags, 0)
                        )
                {
                    /* set reply fail FLAG */
                    call->sysflags |=SYS_FLAG_REPLY_ERROR;
                    call->rcode = TPESYSTEM;
                    ret=EXFAIL;
                }
            }
        }
    }
    else
    {
        /* no data in reply */
        call->data_len = 0;
    }
    
out_send:    
    /* Feature #594 */
    if (TPEXIT==rval)
    {
        call->rval = TPFAIL;
    }
    else
    {
        call->rval = rval;
    }

    data_len = sizeof(tp_command_call_t)+call->data_len;
    call->command_id = ATMI_COMMAND_TPREPLY;
    
    /* If this is gateway timeout, then set the flags accordingly */
    
    if (flags & TPSOFTTIMEOUT)
    {
        NDRX_LOG(log_error, "TPSOFTTIMEOUT present -> returning service error TPETIME!");
        call->sysflags |=SYS_FLAG_REPLY_ERROR;
        call->rcode = TPETIME;
        ret=EXFAIL;
    } 
    else if (flags & TPSOFTENOENT)
    {
        NDRX_LOG(log_error, "TPSOFTENOENT present -> returning service error TPENOENT!");
        call->sysflags |=SYS_FLAG_REPLY_ERROR;
        call->rcode = TPENOENT;
        ret=EXFAIL;
    }
    else if (flags & TPSOFTERR)
    {
        NDRX_LOG(log_error, "TPSOFTERR present -> returning service "
                "error code: %ld", call->rcode);
        call->sysflags |=SYS_FLAG_REPLY_ERROR;
        ret=EXFAIL;
    }
    
    /* keep the timer from last call. */
    call->timer = last_call->timer;
    
    /* Get the reply order... */
    NDRX_STRCPY_SAFE(call->callstack, last_call->callstack);
    if (EXSUCCEED!=fill_reply_queue(call->callstack, last_call->reply_to, reply_to))
    {
        NDRX_LOG(log_error, "ATTENTION!! Failed to get reply queue");
        goto out;
    }
    
    /* Needs some hint for multi-threaded bridge
     * To choose reply thread (all convs goes to thread number = cd % workers_count
     */
    if (CONV_IN_CONVERSATION==p_accept_conn->status)
    {
        call->sysflags |=SYS_CONVERSATION;
    }
    
    /* well if we are in global TX we shall disconnect/end here
     * otherwise tmsrv might get locked txn..
     */
    if (ndrx_get_G_atmi_xa_curtx()->txinfo)
    {
        int end_fail=EXFALSE;
        int prop_fail = EXFALSE;
        
        if (ndrx_get_G_atmi_xa_curtx()->txinfo->tmtxflags & TMTXFLAGS_IS_ABORT_ONLY)
        {
            prop_fail=EXTRUE;
        }
        
        _tp_srv_disassoc_tx(EXFALSE, &end_fail);
        
        if (prop_fail || end_fail)
        {
            if (end_fail)
            {
                NDRX_LOG(log_error, "Marking transaction as abort only "
                        "due to xa_end() failure");
            }
            /* propagate back to caller */
            call->tmtxflags|=TMTXFLAGS_IS_ABORT_ONLY;
        }
    }
    
    /* send the reply back actually */
    NDRX_LOG(log_debug, "Returning to %s cd %d, timestamp %d, callseq: %u, rval: %d, rcode: %ld",
                            reply_to, call->cd, call->timestamp, call->callseq,
                            call->rval, call->rcode);

    /* TODO: Chose the cluster node to send to, Scan the stack, to find
     * the closest reply node... We might event not to pop the stack.
     * But each node searches for closest path, from right to left.
     */
    if (EXFAIL==ndrx_generic_q_send(reply_to, (char *)call, data_len, flags, 0))
    {
        NDRX_LOG(log_error, "ATTENTION!! Reply to queue [%s] failed!",
                                            reply_to);
        goto out;
    }

    /* Wait for ack if we run in conversation */
    if (CONV_IN_CONVERSATION==p_accept_conn->status)
    {
        /* If this is conversation, then we should release conversation queue */
        normal_connection_shutdown(p_accept_conn, EXFALSE, "tpreturn on open conversation");
    }

out:
    
    if (NULL!=buf)
    {
        NDRX_SYSBUF_FREE(buf);
        buf = NULL;
    }

    /* Hmm we can free up the data? 
     * - well mvitolin 16/01/2017 - only auto buffers & this one.
     * Not sure how with Tuxedo multi-threading?
     * - mvitolin 03/03/2017 - will make free any buffer
     * 
     * - mvitolin 01/07/2019 - with NULL buffers (which are real NULLs we have
     * an issue). Because for those there are is no pointer descriptor object.
     * thus if prepare incoming NULL did de-reallocate the object, then we
     * never know it. Thus in this case all prepare incomings if doing buffer
     * free, shall check the last call and reset the auto buf.
     */
    if (NULL!=data)
    {
        if (NULL!=last_call->autobuf && last_call->autobuf->buf==data)
        {
            last_call->autobuf=NULL;
        }
        NDRX_LOG(log_debug, "%s free buffer %p", fn, data);
        ndrx_tpfree(data, NULL);
    }

    if (NULL!=last_call->autobuf)
    {
        NDRX_LOG(log_debug, "%s free auto buffer %p", fn, last_call->autobuf->buf);
        ndrx_tpfree(last_call->autobuf->buf, NULL);
        last_call->autobuf = NULL;
    }

    /* server thread, no long jump... (thread should kill it self.)*/
    if (!(last_call->sysflags & SYS_SRV_THREAD))
    {        
         return_status|=RETURN_TYPE_TPRETURN;
         if (EXFAIL==ret)
             return_status|=RETURN_FAILED;

        if (G_libatmisrv_flags & ATMI_SRVLIB_NOLONGJUMP)
        {
            NDRX_LOG(log_debug, "%s normal return to main - no longjmp", fn);
            G_atmisrv_reply_type = return_status;
        }
        else
        {
            NDRX_LOG(log_debug, "%s about to jump to main()", fn);
            
            /* initiate shutdown ... if requested so ... */
            if (TPEXIT==rval)
            {
                tpexit();
            }
            longjmp(G_atmi_tls->call_ret_env, return_status);
        }
    }
    else
    {
        NDRX_LOG(log_debug, "Thread ending...");
    }

    /* initiate shutdown ... if requested so.. */
    if (TPEXIT==rval)
    {
        tpexit();
    }

    return;
}

/**
 * Forward the call to next service
 * @param svc
 * @param data
 * @param len
 * @param flags - should be managed from parent function (is it real async call
 *                  or tpcall wrapper)
 * @return SUCCEED/FAIL
 */
expublic void _tpforward (char *svc, char *data,
                long len, long flags)
{
    int ret=EXSUCCEED;
    char *buf = NULL;
    size_t buf_len;
    tp_command_call_t *call;
    buffer_obj_t *buffer_info;
    char fn[] = "_tpforward";
    long data_len = MAX_CALL_DATA_SIZE;
    char send_q[NDRX_MAX_Q_SIZE+1];
    long return_status=0;
    int is_bridge;
    tp_command_call_t * last_call;
    int was_auto_buf = EXFALSE;
    atmi_lib_conf_t *p_atmi_lib_conf = ndrx_get_G_atmi_conf();
    int prio = NDRX_MSGPRIO_DEFAULT;
    tp_conversation_control_t *p_accept_conn = ndrx_get_G_accepted_connection();
    char svcddr[XATMI_SERVICE_NAME_LENGTH+1]; /**< routed service name */
    
    NDRX_LOG(log_debug, "%s enter", fn);
    
    NDRX_STRCPY_SAFE(svcddr, svc);
    
    /* abort / tpreturn transaction if we are initiator & transaction
     * is still started
     */
    if (initiated_tran_attached())
    {
        NDRX_LOG(log_error, "Cannot forward to [%s] - initiated transaction [%s] is attached",
                svc, G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        
        /* pass the data fields to, as these shall be freed */
        _tpreturn(TPFAIL, TPESVCERR, data, len, TPSOFTERR);
	return;
    }
    
    /* try the DDR */
    if (EXFAIL==ndrx_ddr_grp_get(svcddr, sizeof(svcddr), data, len,
        &prio))
    {
        /* error shall be set */
        EXFAIL_OUT(ret);
    }
    
    /* client with last call is acceptable...! 
     * It can be servers companion thread.
     * TODO: Add the check.
     */
    last_call = ndrx_get_G_last_call();
    
    if (p_atmi_lib_conf->is_client && !last_call->cd)
    {
        /* this is client */
        NDRX_LOG(log_debug, "tpforward is not available for clients "
                "(is_client=%d, cd=%d)!!!", p_atmi_lib_conf->is_client, 
                last_call->cd);
        ndrx_TPset_error_fmt(TPEPROTO, "tpforward - not available for clients!!!");
        return; /* <<<< RETURN */
    }
    
    /* Cannot do the forward if we are in conversation! */
    if (CONV_IN_CONVERSATION==p_accept_conn->status ||
            have_open_connection())
    {
        ndrx_TPset_error_fmt(TPEPROTO, "tpforward no allowed for conversation server!");
	return;
    }
    
    NDRX_SYSBUF_MALLOC_WERR_OUT(buf, buf_len, ret);
    call = (tp_command_call_t *)buf;
    
    memset(call, 0, sizeof(*call)); /* have some safety net */

    call->data_len = MAX_CALL_DATA_SIZE;

    if (NULL==(buffer_info = ndrx_find_buffer(data)))
    {
        ndrx_TPset_error_fmt(TPEINVAL, "Buffer %p not known to system!", fn);
        ret=EXFAIL;
        goto out;
    }
    
    /* Convert back, if convert flags was set */
    if (SYS_SRV_CVT_ANY_SET(last_call->sysflags))
    {
        if (buffer_info == last_call->autobuf)
        {
            was_auto_buf=EXTRUE;
        }
        
        NDRX_LOG(log_debug, "about reverse xcvt...");
        /* Convert buffer back.. */
        if (EXSUCCEED!=typed_xcvt(&buffer_info, last_call->sysflags, EXTRUE))
        {
            NDRX_LOG(log_debug, "Failed to convert buffer back to "
                    "callers format: %llx", last_call->sysflags);
            userlog("Failed to convert buffer back to "
                    "callers format: %llx", last_call->sysflags);
            ret=EXFAIL;
            goto out;
        }
        else
        {
            data = buffer_info->buf;
            /* Assume that length not used for self describing buffers */
            /* Bug #250 restore auto buf if was so... */
            if (was_auto_buf)
            {
                last_call->autobuf = buffer_info;
            }
        }
    }

    /* prepare buffer for call 
     * TODO: should we check call/buf (buf_len) buffer output size?
     */
    if (EXSUCCEED!=ndrx_mbuf_prepare_outgoing(data, len, call->data, &data_len, flags, 0L))
    {
        /* not good - error should be already set */
        ret=EXFAIL;
        goto out;
    }
    
    /* OK, now fill up the details */
    call->data_len = data_len;

    data_len+=sizeof(tp_command_call_t);

    NDRX_STRCPY_SAFE(call->reply_to, last_call->reply_to); /* <<< main difference from call! */
    
    call->clttout = last_call->clttout; /* store the client timeout setting */
    call->command_id = ATMI_COMMAND_TPCALL;

    NDRX_STRNCPY(call->name, svcddr, XATMI_SERVICE_NAME_LENGTH);
    call->name[XATMI_SERVICE_NAME_LENGTH] = EXEOS;
    call->flags = last_call->flags; /* preserve the original call flags Bug #570 */
    call->cd = last_call->cd; /* <<< another difference from call! */
    call->timestamp = last_call->timestamp;
    call->callseq = last_call->callseq;
    NDRX_STRCPY_SAFE(call->callstack, last_call->callstack);
    
    /* TODO: forward the indication of syste but only if we are in the transaction at the momment */
    call->sysflags|= (last_call->sysflags & SYS_FLAG_AUTOTRAN);
    
    /* work out the XA data */
    if (ndrx_get_G_atmi_xa_curtx()->txinfo)
    {
        /* Copy TX data */
        XA_TX_COPY(call, ndrx_get_G_atmi_xa_curtx()->txinfo);
    }
    
#if 0
    - moved ot tmisabortonly.
    /* If we have global transaction & failed. Then set abort only flag 
     * On the other side, receiver should mark it's global tx too
     * as abort only.
     */
    if (ndrx_get_G_atmi_xa_curtx()->txinfo && ndrx_get_G_atmi_xa_curtx()->txinfo->tmisabortonly)
    {
        call->sysflags|=SYS_XA_ABORT_ONLY;
    }
#endif
    
    /* Want to keep original call time... */
    memcpy(&call->timer, &last_call->timer, sizeof(call->timer));
    
    /* Hmm we can free up the data? - do it here because we still need buffer_info!
     * ???? NOTE HERE! Bug #250 - all job is done bellow!
    if (NULL!=data)
    {
        ndrx_tpfree(data, NULL);
    }
    *
    */
    /* Check is service available? */
    if (EXSUCCEED!=ndrx_shm_get_svc(call->name, send_q, &is_bridge, NULL))
    {
        NDRX_LOG(log_error, "Service is not available %s by shm", 
                call->name);
        ret=EXFAIL;
        ndrx_TPset_error_fmt(TPENOENT, "%s: Service is not available %s by shm", 
                fn, call->name);
                /* we should reply back, that call failed, so that client does not wait */
        reply_with_failure(flags, last_call, NULL, NULL, TPESVCERR);
        goto out;
    }
    
    /* well if we are in global TX we shall disconnect/end here
     * otherwise tmsrv might get locked txn..
     */
    if (ndrx_get_G_atmi_xa_curtx()->txinfo)
    {
        int end_fail=EXFALSE;
        int prop_fail = EXFALSE;
        
        if (ndrx_get_G_atmi_xa_curtx()->txinfo->tmtxflags & TMTXFLAGS_IS_ABORT_ONLY)
        {
            prop_fail=EXTRUE;
        }
        
        _tp_srv_disassoc_tx(EXFALSE, &end_fail);
        
        if (prop_fail || end_fail)
        {
            if (end_fail)
            {
                NDRX_LOG(log_error, "Marking transaction as abort only "
                        "due to xa_end() failure");
            }
            /* propagate back to caller */
            call->tmtxflags|=TMTXFLAGS_IS_ABORT_ONLY;
        }
    }

    NDRX_LOG(log_debug, "Forwarding cd %d, timestamp %d, callseq %u to %s",
                    call->cd, call->timestamp, call->callseq, send_q);
        
    if (EXSUCCEED!=(ret=ndrx_generic_q_send(send_q, (char *)call, data_len, flags, prio)))
    {
        /* reply FAIL back to caller! */
        int err;

        /* basically we override some conditions here! */
        if (ENOENT==ret)
        {
            err=TPENOENT;
        }
        else
        {
            CONV_ERROR_CODE(ret, err);
        }

        ndrx_TPset_error_fmt(err, "%s: Failed to send, os err: %s", fn, strerror(ret));
        userlog("%s: Failed to send, os err: %s", fn, strerror(ret));
        ret=EXFAIL;

        /* we should reply back, that call failed, so that client does not wait */
        reply_with_failure(flags, last_call, NULL, NULL, TPESVCERR);
    }

out:

    if (NULL!=buf)
    {
        NDRX_SYSBUF_FREE(buf);
    }

    if (NULL!=data)
    {
        /* Lookup the buffer infos for data, and then compare with autobuf!
         * as the last_call autobuf might be already free - the same for tpforward
         * for xcv -> update the autobuf if changed auto buf...
         */
        if (last_call->autobuf && last_call->autobuf->buf==data)
        {
            last_call->autobuf=NULL;
        }
        NDRX_LOG(log_debug, "%s free buffer %p", fn, data);
        ndrx_tpfree(data, NULL);
    }

    if (last_call->autobuf)
    {
        NDRX_LOG(log_debug, "%s free auto buffer %p", fn, last_call->autobuf->buf);
        ndrx_tpfree(last_call->autobuf->buf, NULL);
        last_call->autobuf = NULL;
    }

    NDRX_LOG(log_debug, "%s return %d (information only)", fn, ret);

    /* server thread, no long jump... (thread should kill it self.)*/
    if (!(last_call->sysflags & SYS_SRV_THREAD))
    {
        return_status|=RETURN_TYPE_TPFORWARD;
        if (EXFAIL==ret)
            return_status|=RETURN_FAILED;
        
        if (G_libatmisrv_flags & ATMI_SRVLIB_NOLONGJUMP)
        {
            NDRX_LOG(log_debug, "%s normal return to main - no longjmp", fn);
            G_atmisrv_reply_type = return_status;
        }
        else 
        {
            NDRX_LOG(log_debug, "%s longjmp to main()", fn);
            longjmp(G_atmi_tls->call_ret_env, return_status);
        }
    }
    else
    {
        NDRX_LOG(log_debug, "Thread ending...");
    }
    
    return;
}

/**
 * Task is copied to thread.
 * The main thread goes back to polling.
 */
expublic void _tpcontinue (void)
{
    
    /* shall we abort transaction, if not terminated? */
    
    /* mvitolin 11.01.2017 
     * We can do thing when we are not running in integration mode!
     */
    if (G_libatmisrv_flags & ATMI_SRVLIB_NOLONGJUMP)
    {
       NDRX_LOG(log_debug, "Not jumping - as integra mode!");
       G_atmisrv_reply_type|=RETURN_TYPE_THREAD;

    }
    else 
    {
        long return_status=0;
        return_status|=RETURN_TYPE_THREAD;
        
        NDRX_LOG(log_debug, "Long jumping to continue!");
        longjmp(G_atmi_tls->call_ret_env, return_status);
        /* NDRX_LOG(log_error, "doing nothing after long jmp!"); - not reached. */
    }
}
/* vim: set ts=4 sw=4 et smartindent: */
