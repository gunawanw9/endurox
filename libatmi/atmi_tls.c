/**
 * @brief Globals/TLS for libatmi
 *
 * @file atmi_tls.c
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

/*---------------------------Includes-----------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <ndrstandard.h>
#include <atmi.h>
#include <atmi_tls.h>
#include <string.h>
#include "thlock.h"
#include "userlog.h"
#include "utlist.h"
#include <typed_buf.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
__thread atmi_tls_t *G_atmi_tls = NULL; /* single place for library TLS storage */
/*---------------------------Statics------------------------------------*/
exprivate pthread_key_t M_atmi_tls_key;
exprivate pthread_key_t M_atmi_switch_key; /* switch the structure */

exprivate MUTEX_LOCKDECL(M_thdata_init);
exprivate int M_first = EXTRUE;
/*---------------------------Prototypes---------------------------------*/

/**
 * Unlock, unset G_atmi_tls, return pointer to G_atmi_tls
 * @return 
 */
expublic void * ndrx_atmi_tls_get(long priv_flags)
{
    atmi_tls_t *tls = G_atmi_tls;
    char *fn = "ndrx_atmi_tls_get";
    if (NULL!=tls)
    {
        /*
         * Unset the destructor
         */
        if (tls->is_auto)
        {
            pthread_setspecific( M_atmi_tls_key, NULL );
        }

        /* suspend the transaction if any in progress: similar to tpsrvgetctxdata() */
#ifdef NDRX_OAPI_DEBUG
        
        NDRX_LOG(log_debug, "%s: G_atmi_xa_curtx.txinfo: %p", 
                     __func__, tls->G_atmi_xa_curtx.txinfo);
#endif
        
        /* well for java we do not need to suspend the transaction,,
         * do we?
         */
        if (priv_flags & CTXT_PRIV_TRAN && 
                !(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOAPISUSP))
        {
            tls->global_tx_suspended = EXFALSE;
            
            if (tls->G_atmi_xa_curtx.txinfo)
            {
                atmi_error_t aerr;
                int aerr_loaded=EXFALSE;
                
                /* suspend current error */
                if (tls->M_atmi_error)
                {
                    aerr_loaded=EXTRUE;
                    ndrx_TPsave_error(&aerr);
                }
                
                tls->M_atmi_error = 0;
                if (EXSUCCEED!=ndrx_tpsuspend(&tls->tranid, 0, EXTRUE))
                {
                    /*
                     * Nothing to do here! it will fail next time when user
                     * will try to do some DB operation... 
                     */
                    userlog("ndrx_atmi_tls_get: Failed to suspend transaction: [%s]", 
                            tpstrerror(tperrno));
                }
                else
                {
                    tls->global_tx_suspended = EXTRUE;
                }
                
                if (aerr_loaded)
                {
                    ndrx_TPrestore_error(&aerr);
                }
                
            }
        }
        
        /* Disable current thread TLS... */
        G_atmi_tls = NULL;

        /* unlock object */
        MUTEX_UNLOCK_V(tls->mutex);
    }
out:
    return (void *)tls;
}

/**
 * Get the lock & set the G_atmi_tls to this one
 * @param tls
 */
expublic int ndrx_atmi_tls_set(void *data, int flags, long priv_flags)
{
    int ret = EXSUCCEED;
    atmi_tls_t *tls = (atmi_tls_t *)data;
    char *fn = "ndrx_atmi_tls_set";
   
    if (NULL!=tls)
    {
        /* extra control... */
        if (ATMI_TLS_MAGIG!=tls->magic)
        {
            userlog("atmi_tls_set: invalid magic! expected: %x got %x", 
                    ATMI_TLS_MAGIG, tls->magic);
        }

        /* Lock the object 
         * TODO: We need PTHREAD_MUTEX_RECURSIVE
         * so that we can acquire multiple locks (for example server process calls 
         * back tpsrvinit or done which internally may acquire some more locks!
         */
        MUTEX_LOCK_V(tls->mutex);

        /* Add the additional flags to the user. */
        tls->G_last_call.sysflags |= flags;
        
#ifdef NDRX_OAPI_DEBUG
        NDRX_LOG(log_debug, "%s: G_atmi_xa_curtx.txinfo: %p", 
                     __func__, tls->G_atmi_xa_curtx.txinfo);
#endif
        G_atmi_tls = tls; /* Must be set, so that tpresume works () - not allocate new.. */
        
        /* Resume the transaction only if flag is set
         * For Object API some of the operations do not request transaction to
         * be open.
         */
        if (priv_flags & CTXT_PRIV_TRAN && 
                !(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOAPISUSP))
        {
            if(tls->global_tx_suspended)
            {   
                /* reset error */
                tls->M_atmi_error = 0;
                if (EXSUCCEED!=ndrx_tpresume(&tls->tranid, 0))
                {
                    userlog("Failed to resume transaction: [%s]", tpstrerror(tperrno));
                }
                else
                {
                    tls->global_tx_suspended = EXFALSE;
                }
            }
        }
        
        /*
         * Destruct automatically if it was auto-tls 
         */
        if (tls->is_auto)
        {
            pthread_setspecific( M_atmi_tls_key, (void *)tls );
        }
    }
    else
    {
        G_atmi_tls = NULL;
    }
    
out:
    return ret;
}

/**
 * Free up the TLS data
 * @param tls
 * @return 
 */
expublic void ndrx_atmi_tls_free(void *data)
{   
    atmi_tls_t *tls = (atmi_tls_t *)data;
    tpmemq_t *el, *elt;
    if (NULL!=data)
    {
        if (data == G_atmi_tls)
        {
            if (G_atmi_tls->is_auto)
            {
                pthread_setspecific( M_atmi_tls_key, NULL );
            }
            G_atmi_tls = NULL;
        }
        
        /* de-init mutex & spinlock */
        
        MUTEX_DESTROY_V(tls->mutex);
        
        /* shouldn't we free up any  tls->memq ? */
        
        DL_FOREACH_SAFE(tls->memq, el, elt)
        {
            if (NULL!=(el->buf))
            {
                NDRX_SYSBUF_FREE(el->buf);
            }
            
            NDRX_FPFREE(el);
        }
        
        if (NULL!=tls->qdisk_tls)
        {
            NDRX_FPFREE(tls->qdisk_tls);
        }
        
        NDRX_FREE((char*)data);
    }
}

/**
 * Get the lock & init the data
 * @param auto_destroy if set to 1 then when tried exits, thread data will be made free
 * @return 
 */
expublic void * ndrx_atmi_tls_new(void *tls_in, int auto_destroy, int auto_set)
{
    int ret = EXSUCCEED;
    atmi_tls_t *tls  = NULL;
    
    /* init they key storage */
    if (M_first)
    {
        MUTEX_LOCK_V(M_thdata_init);
        if (M_first)
        {
            pthread_key_create( &M_atmi_tls_key, 
                    &ndrx_atmi_tls_free );
            
            /* perform first time library inits..., locks, etc  */
            ndrx_tpcall_init_once();
            M_first = EXFALSE;
        }
        MUTEX_UNLOCK_V(M_thdata_init);
    }
    
    if (NULL!=tls_in)
    {
        tls = (atmi_tls_t *)tls_in;
        NDRX_LOG(log_debug, "%s: Reusing TLS storage", __func__);
    }
    else
    {
        if (NULL==(tls = (atmi_tls_t *)NDRX_MALLOC(sizeof(atmi_tls_t))))
        {
            userlog ("%s: failed to malloc", __func__);
            EXFAIL_OUT(ret);
        }
    }
    
    /* do the common init... */
    tls->magic = ATMI_TLS_MAGIG;
    
    
    /* init.c */    
    tls->conv_cd=0;/*  Lets start from 0 now... */
    
    /* reset client info  */
    memset(&tls->client_init_data, 0, sizeof(tls->client_init_data));
    
    /* tls->callseq = 0; ???? */
    tls->G_atmi_is_init= 0;/*  Is environment initialised */
    memset (tls->G_call_state, 0, sizeof(tls->G_call_state));
    tls->tpcall_get_cd=MAX_ASYNC_CALLS-2; /* first available, we want test overlap!*/
    tls->memq = NULL; /* In memory messages when tpchkunsol are performed... */
    /* tls->tpcall_callseq=0; */
    
    
    memset (tls->G_tp_conversation_status, 0, sizeof(tls->G_tp_conversation_status));
    
    /* tpcall.c */
    tls->M_svc_return_code = 0;
    tls->tpcall_first = EXTRUE;
    
    /* tperror.c */
    tls->M_atmi_error_msg_buf[0] = EXEOS;
    tls->M_atmi_error = TPMINVAL;
    tls->M_atmi_reason = NDRX_XA_ERSN_NONE;
    tls->errbuf[0] = EXEOS;
    tls->is_associated_with_thread = EXFALSE;
    /* xa.c */
    tls->M_is_curtx_init = EXFALSE;
    tls->global_tx_suspended = EXFALSE;
    memset(&tls->G_atmi_conf, 0, sizeof(tls->G_atmi_conf));
    memset(&tls->G_atmi_xa_curtx, 0, sizeof(tls->G_atmi_xa_curtx));
    
    /* unsol msgs */
    tls->p_unsol_handler = NULL;
    
    /* tx related: */
    tls->tx_commit_return = TX_COMMIT_COMPLETED;
    tls->tx_transaction_control = TX_UNCHAINED;
    tls->tx_transaction_timeout = 0;
    
    tls->nullbuf.autoalloc = EXFALSE;
    tls->nullbuf.size=0;
    tls->nullbuf.subtype[0]=EXEOS;
    tls->nullbuf.type_id=BUF_TYPE_NULL;
    tls->nullbuf.callinfobuf=NULL;
    tls->nullbuf.callinfobuf_len=0;
    memset(&tls->nullbuf.hh, 0, sizeof(tls->nullbuf.hh));
    
    memset(&tls->integpriv, 0, sizeof(tls->integpriv));
    
    MUTEX_VAR_INIT(tls->mutex);
    
    /* reset the hook */
    tls->pf_tpacall_noservice_hook = NULL;
    
    /* no priority set ... */
    tls->prio = 0;
    tls->prio_flags=0;
    tls->prio_last = NDRX_MSGPRIO_DEFAULT;
    tls->tmnull_is_open=EXFALSE;
    tls->tmnull_rmid=EXFAIL;

    tls->tout = EXFAIL;
    tls->tout_next = EXFAIL;
    tls->tout_next_eff = EXFAIL;
    
    tls->qdisk_is_open=EXFALSE;
    tls->qdisk_rmid=EXFAIL;
    tls->qdisk_tls=NULL;
    
    /* set callback, when thread dies, we need to get the destructor 
     * to be called
     */
    if (auto_destroy)
    {
        tls->is_auto = EXTRUE;
        pthread_setspecific( M_atmi_tls_key, (void *)tls );
    }
    else
    {
        tls->is_auto = EXFALSE;
    }
    
    if (auto_set)
    {
        ndrx_atmi_tls_set(tls, 0, 0);
    }
    
out:

    if (EXSUCCEED!=ret && NULL!=tls)
    {
        ndrx_atmi_tls_free((char *)tls);
        tls = NULL;
    }

    return (void *)tls;
}

/**
 * Kill the given context,
 * Do we need tpterm here?
 * @param context
 * @param flags
 * @return 
 */
expublic void ndrx_tpfreectxt(TPCONTEXT_T context)
{
    atmi_tls_t * ctx = (atmi_tls_t *)context;
    
    if (NULL!=ctx)
    {
        /* Close any open loggers... */
        if (G_atmi_tls && G_atmi_tls==context)
        {
            tplogclosereqfile();
            tplogclosethread();
            tpterm();
        }
        
        if (NULL!=ctx->p_nstd_tls)
        {
            ndrx_nstd_tls_free(ctx->p_nstd_tls);
        }
        
        if (NULL!=ctx->p_ubf_tls)
        {
            ndrx_ubf_tls_free(ctx->p_ubf_tls);
        }
        
        ndrx_atmi_tls_free(ctx);
    }
}

/**
 * Internal version of get context
 * @param context
 * @param flags
 * @param priv_flags private flags (for sharing the functionality)
 * @return 
 */
expublic int ndrx_tpsetctxt(TPCONTEXT_T context, long flags, long priv_flags)
{
    int ret = EXSUCCEED;
    atmi_tls_t * ctx;
    
#ifdef NDRX_OAPI_DEBUG
    NDRX_LOG(log_debug, "ENTRY: %s enter, context: %p, current: %p",  __func__, 
            context, G_atmi_tls);
    
    if (NULL!=context)
    {
        NDRX_LOG(log_debug, "ENTRY: is_associated_with_thread = %d", 
            ((atmi_tls_t *)context)->is_associated_with_thread);
    }

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_NSTD = %d", 
        (priv_flags) & CTXT_PRIV_NSTD );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_UBF = %d", 
        (priv_flags) & CTXT_PRIV_UBF );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_ATMI = %d", 
        (priv_flags) & CTXT_PRIV_ATMI );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_TRAN = %d", 
        (priv_flags) & CTXT_PRIV_TRAN );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_NOCHK = %d", 
        (priv_flags) & CTXT_PRIV_NOCHK );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_IGN = %d", 
        (priv_flags) & CTXT_PRIV_IGN );
#endif

    
    if (context == TPNULLCONTEXT)
    {
        TPCONTEXT_T tmp;
        int dealloc = EXFALSE;
        
        /* deallocate the context (if needed so...) */
        
        if (NULL!=G_atmi_tls && G_atmi_tls->is_auto)
        {
            dealloc = EXTRUE;
        }
        
        /* Bug #311: disassociate context */
        ndrx_tpgetctxt(&tmp, 0L, priv_flags);
        
        if (dealloc)
        {
            /* free the current thread context data (only in case if it was auto) */
            ndrx_tpfreectxt((TPCONTEXT_T)tmp);
        }
        
        /* In case if we are already in NULL context, then go out... */
        goto out; /* we are done. */
    }
    
    ctx = (atmi_tls_t *)context;
    
    if (!(priv_flags & CTXT_PRIV_NOCHK))
    {
        /* have a deep checks */
        if (priv_flags & CTXT_PRIV_ATMI && ATMI_TLS_MAGIG!=ctx->magic)
        {
            ndrx_TPset_error_fmt(TPENOENT, "_tpsetctxt: invalid atmi magic: "
                    "expected: %x got %x!", ATMI_TLS_MAGIG, ctx->magic);
            EXFAIL_OUT(ret);
        }

        if (priv_flags & CTXT_PRIV_NSTD && NULL!=ctx->p_nstd_tls 
                && NSTD_TLS_MAGIG!=ctx->p_nstd_tls->magic)
        {
            ndrx_TPset_error_fmt(TPENOENT, "_tpsetctxt: invalid nstd magic: "
                    "expected: %x got %x!", NSTD_TLS_MAGIG, ctx->p_nstd_tls->magic);
            EXFAIL_OUT(ret);
        }

        if (priv_flags & CTXT_PRIV_UBF && NULL!=ctx->p_ubf_tls 
                && UBF_TLS_MAGIG!=ctx->p_ubf_tls->magic)
        {
            ndrx_TPset_error_fmt(TPENOENT, "_tpsetctxt: invalid ubf magic: "
                    "expected: %x got %x!", UBF_TLS_MAGIG, ctx->p_ubf_tls->magic);
            EXFAIL_OUT(ret);
        }
    }
    
    /* free the current context (with tpterm?) 
     * if one in progress 
     */
    if (!(priv_flags & CTXT_PRIV_IGN) && G_atmi_tls!=ctx && NULL!=G_atmi_tls)
    {
        NDRX_LOG(log_warn, "Free up context %p", G_atmi_tls);
        tpterm();
        tpfreectxt((TPCONTEXT_T)G_atmi_tls);
    }
    
    
    if (priv_flags & CTXT_PRIV_NSTD && NULL!=ctx->p_nstd_tls &&
            EXSUCCEED!=ndrx_nstd_tls_set((void *)ctx->p_nstd_tls))
    {
        ndrx_TPset_error_fmt(TPESYSTEM, "_tpsetctxt: failed to restore libnstd context");
        EXFAIL_OUT(ret);
    }
    
    if (priv_flags & CTXT_PRIV_UBF &&  NULL!=ctx->p_ubf_tls &&
            EXSUCCEED!=ndrx_ubf_tls_set((void *)ctx->p_ubf_tls))
    {
        ndrx_TPset_error_fmt(TPESYSTEM, "_tpsetctxt: failed to restore libubf context");
        EXFAIL_OUT(ret);
    }
    
    if (priv_flags & CTXT_PRIV_ATMI)
    {
        if (EXSUCCEED!=ndrx_atmi_tls_set((void *)ctx, flags, priv_flags))
        {
            ndrx_TPset_error_fmt(TPESYSTEM, "_tpsetctxt: failed to restore libatmi context");
            EXFAIL_OUT(ret);
        }
        
        ctx->is_associated_with_thread = EXTRUE;
    }
    
out:

#ifdef NDRX_OAPI_DEBUG
    NDRX_LOG(log_debug, "RETURN: %s returns %d, context: %p, current: %p",  __func__, 
            ret, context, G_atmi_tls);
#endif
    return ret;
}

/**
 * Return data from context
 * This assumes that ATMI Context currently IS set!
 * @param[out] data output data where to store the result
 */
expublic ndrx_ctx_priv_t* ndrx_ctx_priv_get(void)
{
    if (NULL==G_atmi_tls)
    {
        return NULL;
    }
    else
    {
        return &G_atmi_tls->integpriv;
    }
}

/**
 * Internal version of get full context
 * This disconnects current thread from TLS.
 * @param flags
 * @return 
 */
expublic int ndrx_tpgetctxt(TPCONTEXT_T *context, long flags, long priv_flags)
{
    int ret = TPMULTICONTEXTS; /* default */
    atmi_tls_t * ctx;
    char *fn="_tpgetctxt";
    
#ifdef NDRX_OAPI_DEBUG
    NDRX_LOG(log_debug, "ENTRY: %s enter, context: %p, current: %p", 
                 __func__, *context, G_atmi_tls);
    
    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_NSTD = %d", 
        (priv_flags) & CTXT_PRIV_NSTD );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_UBF = %d", 
        (priv_flags) & CTXT_PRIV_UBF );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_ATMI = %d", 
        (priv_flags) & CTXT_PRIV_ATMI );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_TRAN = %d", 
        (priv_flags) & CTXT_PRIV_TRAN );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_NOCHK = %d", 
        (priv_flags) & CTXT_PRIV_NOCHK );

    NDRX_LOG(log_debug, "ENTRY: CTXT_PRIV_IGN = %d", 
        (priv_flags) & CTXT_PRIV_IGN );
#endif
    if (NULL==context)
    {
        ndrx_TPset_error_msg(TPEINVAL, "_tpgetctxt: context must not be NULL!");
        EXFAIL_OUT(ret);
    }
    
    if (0!=flags)
    {
        ndrx_TPset_error_msg(TPEINVAL, "_tpgetctxt: flags must be 0!");
        EXFAIL_OUT(ret);
    }
    
    /* if not using ATMI, then use existing context object */
    if (priv_flags & CTXT_PRIV_ATMI)
    {
        ctx = (atmi_tls_t *)ndrx_atmi_tls_get(priv_flags);
    }
    else   
    {
        ctx = (TPCONTEXT_T)*context;
    }
    
    if (NULL!=ctx)
    {
        /* Dis associate */
        ctx->is_associated_with_thread = EXFALSE;

        if (priv_flags & CTXT_PRIV_NSTD)
        {
            ctx->p_nstd_tls = ndrx_nstd_tls_get();
        }
        
        if (priv_flags & CTXT_PRIV_UBF)
        {
            ctx->p_ubf_tls = ndrx_ubf_tls_get();
        }
    }
    
    if (priv_flags & CTXT_PRIV_ATMI)
    {
        *context = (TPCONTEXT_T)ctx;
    }
    
    if (NULL==ctx)
    {
        ret = TPNULLCONTEXT;
    }
out:

#ifdef NDRX_OAPI_DEBUG
    NDRX_LOG(log_debug, "RETURN: %s returns %d, context: %p, current: %p",  __func__, 
            ret, *context, G_atmi_tls);
#endif

    return ret;
}

/**
 * Reconfigure context.
 * Current context must be set.
 * @param is_auto TRUE -> automatic dealloc, FALSE -> manual context dealloc
 */
expublic void ndrx_ctx_auto(int is_auto)
{
    G_atmi_tls->is_auto = is_auto;
}

/* vim: set ts=4 sw=4 et smartindent: */
