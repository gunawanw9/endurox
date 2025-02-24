/**
 * @brief ATMI lib part for XA api
 *   Responsible for:
 *   - Loading the drivers in app.
 *   Think about automatic open...
 *   RECON: We expect that on first network failure, XA APIs should return XAER_RMFAIL
 *   on such error, if RECON is configured we shall re-establish connection.
 *   In case if last call (if counter is exceeded) still we are getting XAER_RMFAIL
 *   keep the internal status that connection is broken and on next API call we firstly
 *   try to connect to resource, and only the proceed. If proceed fails, attempt
 *   counter continues to count from first attempts (if there was any).
 *
 * @file xa.c
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
#include <errno.h>
#include <dlfcn.h>

#include <atmi.h>
#include <atmi_shm.h>
#include <ndrstandard.h>
#include <ndebug.h>
#include <nstdutil.h>
#include <ndrxdcmn.h>
#include <userlog.h>

/* shm_* stuff, and mmap() */
#include <sys/mman.h>
#include <sys/types.h>
/* exit() etc */
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <xa_cmn.h>
#include <tperror.h>
#include <atmi_tls.h>
#include "Exfields.h"
#include "sys_test.h"
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/

#define XA_API_ENTRY(X) {\
    ATMI_TLS_ENTRY;\
    if (!M_is_xa_init) { \
        if (EXSUCCEED!=(ret = atmi_xa_init()))\
        {\
            goto out;\
        }\
    }\
    if (!G_atmi_tls->M_is_curtx_init)\
    {\
        if (EXSUCCEED!=(ret=atmi_xa_init_thread(X)))\
        {\
            goto out;\
        }\
    }\
}\

/**
 * This is generic API retry engine. Uses global retry
 * counter with specified condition code to retry on.
 * this assumes that XA_OK is expected to be returned by API.
 * TODO: In case of protocl failures and restarts. We might want to track,
 * if we know that connection was open, then reloop on RMFAIL and do not touch XAER_PROTO?
 * Currently if we are in the API, we do not check are we open or not, thus have to relay on XAER_PROTO.
 * @param (call) expression to call on retry
 * @param (retry_condition) error condition on which to retry
 * @param (bad_status) expression indicating that if true, we are still failing
 * @param do_primary shall we call the primary expression?
 */
#define GENERIC_RETRY_CORE(call, retry_condition, bad_status, do_primary) do {\
        /* Check that return code is specified or loop second attempt is it in global retry list */\
        if (G_atmi_env.xa_recon_times && (retry_condition))\
        {\
            if (do_primary)\
            {\
                NDRX_LOG(log_warn, "RECON: Entry of %s() failed with %d", __func__, ret);\
            }\
            while (tries<G_atmi_env.xa_recon_times)\
            {\
                tries++;\
                NDRX_LOG(log_warn, "RECON: >>> Attempt %d type=%s. Sleeping %ld micro-sec", \
                        tries, (do_primary?__func__:"conn-only"), G_atmi_env.xa_recon_usleep);\
                usleep(G_atmi_env.xa_recon_usleep);\
                NDRX_LOG(log_warn, "RECON: Retrying...");\
                /* xa_close */\
                NDRX_LOG(log_warn, "RECON: atmi_xa_close_entry()");\
                atmi_xa_close_entry(EXTRUE);\
                NDRX_LOG(log_warn, "RECON: atmi_xa_open_entry()");\
                /* keep the last error */\
                ndrx_TPunset_error();\
                if (XA_OK==(ret=atmi_xa_open_entry()))\
                {\
                    /* restart... */\
                    NDRX_LOG(log_warn, "RECON: %s() call of atmi_xa_open_entry() OK", __func__);\
                    if (do_primary)\
                    {\
                        NDRX_LOG(log_warn, "RECON: Retry of %s()", __func__);\
                        /* rest the error code? previous errors keeps the original error code?*/\
                        ndrx_TPunset_error();\
                        ret = (call);\
                        if (!(bad_status))\
                        {\
                            NDRX_LOG(log_warn, "RECON: <<< Succeed (%s)", __func__);\
                            break;\
                        }\
                        else\
                        {\
                            if ((retry_condition))\
                            {\
                                NDRX_LOG(log_warn, "RECON: <<< Attempt %d. %s() failed %d", \
                                    tries, __func__, ret);\
                            }\
                            else\
                            { /* this is different error, so not attempting... */\
                                NDRX_LOG(log_warn, "RECON: <<< Attempt %d. %s() failed %d, no continue", \
                                    tries, __func__, ret);\
                                break;\
                            }\
                        }\
                    }\
                    else\
                    {\
                        NDRX_LOG(log_warn, "RECON: <<< Succeed (connection)");\
                        break;\
                    }\
                }\
                else\
                {\
                    NDRX_LOG(log_error, "RECON: <<< Attempt %d. atmi_xa_open_entry() - "\
                       "fail: %d [%s]", tries, ret, atmi_xa_geterrstr(ret));\
                }\
            } /* for tries */\
            /* still failed, let next calls to survive */\
            if (XAER_RMFAIL==ret)\
            {\
                atmi_xa_close_entry(EXTRUE);\
            }\
        } /* if retry supported. */\
    } while (0)


/**
 * Generic retry engine for all others except start
 * see args of GENERIC_RETRY_CORE()
 */
#define GENERIC_RETRY(call, retry_condition, bad_status) do {\
        GENERIC_RETRY_CORE((call), (retry_condition), (bad_status), EXTRUE);\
        if (bad_status)\
        {\
            NDRX_LOG(log_error, "finally %s - fail: %d [%s]", \
                    __func__, ret, atmi_xa_geterrstr(ret));\
            ndrx_TPset_error_fmt_rsn(TPERMERR,  \
                    ret, "finally %s - fail: %d [%s]", \
                    __func__, ret, atmi_xa_geterrstr(ret));\
            goto out;\
        }\
    } while (0)

/**
 * Keep the common attempts counter as for entry connect and later reconnects
 * after bad primary method.
 */
#define GENERIC_RETRY_ENTRY(do_rollback) \
        /* perform retry with common counter */\
        GENERIC_RETRY_CORE(0, G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error, 0, EXFALSE);\
        do\
        {\
            if (XA_OK!=ret)\
            {\
                if (do_rollback)\
                {\
                    ndrx_xa_join_fail(NULL, EXFALSE);\
                    atmi_xa_reset_curtx();\
                }\
                NDRX_LOG(log_error, "finally %s - fail: %d [%s]", \
                        __func__, ret, atmi_xa_geterrstr(ret));\
                ndrx_TPset_error_fmt_rsn(TPERMERR,  \
                        ret, "finally %s - fail: %d [%s]", \
                        __func__, ret, atmi_xa_geterrstr(ret));\
                goto out;\
            }\
        } while (0)

/**
 * Generic define for retry engine
 * keeps the common re-connect attempts counter!
 */
#define GENERIC_RETRY_DEF \
    int tries = 0

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/

/** current library status */
exprivate int volatile M_is_xa_init = EXFALSE;

/** protect from double init... */
exprivate MUTEX_LOCKDECL(M_is_xa_init_lock);

/*---------------------------Prototypes---------------------------------*/
exprivate int atmi_xa_init_thread(int do_open);
exprivate int ndrx_xa_join_fail(int *did_abort, int force_abort);


/******************************************************************************/
/*                          LIB INIT                                          */
/******************************************************************************/

/**
 * Return Enduro/X null switch (special case for aix/golang) - to
 * have atlest null switch available - thus provide internal one
 * @return XA switch or null
 */
exprivate struct xa_switch_t *ndrx_aix_fix(void)
{
    return &tmnull_switch;
}

/**
 * Initialize current thread
 */
exprivate int atmi_xa_init_thread(int do_open)
{
    int ret = EXSUCCEED;
    
    /* ATMI_TLS_ENTRY; - not needed called already from macros which does the init */
    
    memset(&G_atmi_tls->G_atmi_xa_curtx, 0, sizeof(G_atmi_tls->G_atmi_xa_curtx));
    G_atmi_tls->M_is_curtx_init = EXTRUE;
    
out:
    return ret;
}
/**
 * Un-initialize XA lib (for thread)
 * @return 
 */
expublic void atmi_xa_uninit(void)
{
    ATMI_TLS_ENTRY;
    /* do only thread based stuff un-init */
    if (G_atmi_tls->M_is_curtx_init)
    {
        if (G_atmi_tls->G_atmi_xa_curtx.is_xa_open)
        {
            atmi_xa_close_entry(EXFALSE);
            G_atmi_tls->G_atmi_xa_curtx.is_xa_open = EXFALSE;
        }
        G_atmi_tls->M_is_curtx_init = EXFALSE;
    }
}


/**
 * Initialize the XA drivers
 * we should load the Enduro/X driver for target XA resource manager 
 * and get handler for XA api.
 *
 * @return 
 */
expublic int atmi_xa_init(void)
{
    int ret=EXSUCCEED;
    void *handle; /* keep the handle, so that we have a reference */
    ndrx_get_xa_switch_loader func;
    char *error;
    char *xa_flags = NULL; /* dynamically allocated... */
    int has_lock = EXFALSE;
    
    /* Bug #565 avoid double init */
    if (!M_is_xa_init)
    {   
        MUTEX_LOCK_V(M_is_xa_init_lock);
        has_lock=EXTRUE;
        
        /* we are done... (other thread already performed init) */
        if (M_is_xa_init)
        {
            goto out;
        }
    }
    
    /* how about thread safety? */
    NDRX_LOG(log_info, "Loading XA driver: [%s]", G_atmi_env.xa_driverlib);
    handle = dlopen (G_atmi_env.xa_driverlib, RTLD_NOW);
    if (!handle)
    {
        error = dlerror();
        NDRX_LOG(log_error, "Failed to load XA lib [%s]: %s", 
                G_atmi_env.xa_driverlib, error?error:"no dlerror provided");

        ndrx_TPset_error_fmt(TPEOS, "Failed to load XA lib [%s]: %s", 
                G_atmi_env.xa_driverlib, error?error:"no dlerror provided");
        EXFAIL_OUT(ret);
    }

    func = (ndrx_get_xa_switch_loader)dlsym(handle, "ndrx_get_xa_switch");

/* for golang brtl runtime linkage cannot be enabled, thus processes
 * do no see the global Enduro/X variables,
 * we can make special exception here for nullswitch, to use
 * built-in symbol here
 */
#ifdef EX_OS_AIX
    if (ndrx_str_ends_with(G_atmi_env.xa_driverlib, "libndrxxanulls.so"))
    {
        func = ndrx_aix_fix;
    }
#endif

    if (!func) 
    {

        error = dlerror();
        NDRX_LOG(log_error, "Failed to get symbol `ndrx_get_xa_switch' [%s]: %s", 
            G_atmi_env.xa_driverlib, error?error:"no dlerror provided");

        ndrx_TPset_error_fmt(TPESYSTEM, "Failed to get symbol `ndrx_get_xa_switch' [%s]: %s", 
            G_atmi_env.xa_driverlib, error?error:"no dlerror provided");
        EXFAIL_OUT(ret);
    }

    NDRX_LOG(log_info, "About to call ndrx_get_xa_switch()");

    /* Do not deallocate the lib... */
    if (NULL==(G_atmi_env.xa_sw = func()))
    {
        NDRX_LOG(log_error, "Cannot get XA switch handler - "
                        "`ndrx_get_xa_switch()' - returns NULL");

        ndrx_TPset_error_fmt(TPESYSTEM,  "Cannot get XA switch handler - "
                        "`ndrx_get_xa_switch()' - returns NULL");
        EXFAIL_OUT(ret);
    }

    NDRX_LOG(log_info, "Using XA %s", 
            (G_atmi_env.xa_sw->flags&TMREGISTER)?"dynamic registration":"static registration");

    /* why ?
     * still we can suspend / resume and join even TMNOMIGRATE is set
    if (G_atmi_env.xa_sw->flags & TMNOMIGRATE)
    {
        NDRX_LOG(log_warn, "XA Switch has TMNOMIGRATE flag -> fallback to nojoin");
        ndrx_xa_nojoin(EXTRUE);
    }
     */

    /* Parse the flags... and store the config 
     * This is done for Feature #160. Customer have an issue with xa_start
     * after a while. Suspect that firewall closes connections and oracle XA
     * lib looses the connection (fact that there was xa_open()). Thus 
     * at xa_start allow to retry with xa_close(), xa_open() and xa_start.
     */
    NDRX_LOG(log_debug, "xa_flags = [%s]", G_atmi_env.xa_flags);
    G_atmi_env.xa_fsync_flags=0;
    
    /* default other retry is XAER_RMFAIL */
    NDRX_STRCPY_SAFE(G_atmi_env.xa_recon_retcodes_other, ",-7,");
    if (EXEOS!=G_atmi_env.xa_flags[0])
    {
        char *tag_ptr;
        /* Note this will be parsed and EOS inserted. */
        char *tag_first;
        char *tag_token;
        int token_nr = 0;

        char *value_ptr, *value_first, *value_token;

        if (NULL==(xa_flags = NDRX_STRDUP(G_atmi_env.xa_flags)))
        {
            int err = errno;
            ndrx_TPset_error_fmt(TPEOS,  "Failed to allocate xa_flags temp buffer: %s", 
                    strerror(err));

            userlog("Failed to allocate xa_flags temp buffer: %s", strerror(err));

            EXFAIL_OUT(ret);
        }

        tag_first = xa_flags;
        NDRX_LOG(log_debug, "About token: [%s]", tag_first);
        while ((tag_token = strtok_r(tag_first, ";", &tag_ptr)))
        {
            if (NULL!=tag_first)
            {
                tag_first = NULL; /* now loop over the string */
            }

            NDRX_LOG(log_debug, "Got tag [%s]", tag_token);

            /* format: RECON:<1,2,* - error codes>:<tries>:<sleep_millisec>
             * "*" - used for any error.
             * example:
             * RECON:*:3:250
             * Meaning: on any xa_start error, reconnect (tpclose/tpopen/tpbegin)
             * 3x times, between attempts sleep 250ms.
             */
            if (0==strncmp(tag_token, NDRX_XA_FLAG_RECON_TEST, strlen(NDRX_XA_FLAG_RECON_TEST)))
            {
                value_first = tag_token;
                G_atmi_env.xa_recon_usleep = EXFAIL;
                NDRX_LOG(log_warn, "Parsing RECON tag... [%s]", value_first);

                while ((value_token = strtok_r(value_first, ":", &value_ptr)))
                {
                    token_nr++;
                    if (NULL!=value_first)
                    {
                        value_first = NULL; /* now loop over the string */
                    }

                    switch (token_nr)
                    {
                        case 1:
                            /* This is "RECON" */
                            NDRX_LOG(log_debug, "RECON: 1: [%s]", value_token);
                            break;
                        case 2:
                            /* This is list of error codes */
                            NDRX_LOG(log_debug, "RECON: 2: [%s]", value_token);
                            snprintf(G_atmi_env.xa_recon_retcodes, 
                                    sizeof(G_atmi_env.xa_recon_retcodes),
                                    ",%s,", value_token);

                            /* Remove spaces and tabs.. */
                            ndrx_str_strip(G_atmi_env.xa_recon_retcodes, "\t ");

                            break;

                        case 3:
                            NDRX_LOG(log_debug, "RECON: 3: [%s]", value_token);
                            G_atmi_env.xa_recon_times = atoi(value_token);
                            break;
                        case 4:
                            /* so user gives us milliseconds */
                            NDRX_LOG(log_debug, "RECON: 4: [%s]", value_token);
                            G_atmi_env.xa_recon_usleep = atol(value_token)*1000;
                            break;
                        case 5:
                            /* This is list of error codes */
                            NDRX_LOG(log_debug, "RECON: 5: [%s]", value_token);
                            snprintf(G_atmi_env.xa_recon_retcodes_other, 
                                    sizeof(G_atmi_env.xa_recon_retcodes_other),
                                    ",%s,", value_token);

                            /* Remove spaces and tabs.. */
                            ndrx_str_strip(G_atmi_env.xa_recon_retcodes_other, "\t ");

                            break;
                    }
                }

                if (G_atmi_env.xa_recon_usleep < 0)
                {
                    NDRX_LOG(log_error, "Invalid [%s] settings in "
                            "XA_FLAGS [%s] (usleep not set)", 
                            NDRX_XA_FLAG_RECON, G_atmi_env.xa_flags);

                    ndrx_TPset_error_fmt(TPEINVAL, "Invalid [%s] settings in "
                            "XA_FLAGS [%s] (usleep not set)", 
                            NDRX_XA_FLAG_RECON, G_atmi_env.xa_flags);

                    EXFAIL_OUT(ret);
                }

                NDRX_LOG(log_error, "XA flag: [%s]: on xa_start ret codes: [%s],"
                        " recon number of %d times, sleep %ld "
                        "microseconds between attempts",
                        NDRX_XA_FLAG_RECON, 
                        G_atmi_env.xa_recon_retcodes, 
                        G_atmi_env.xa_recon_times, 
                        G_atmi_env.xa_recon_usleep);
            } /* If tag is NOJOIN */
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_NOJOIN))
            {
                ndrx_xa_nojoin(EXTRUE);
            }
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_NOSTARTXID))
            {
                ndrx_xa_nostartxid(EXTRUE);
            }
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_NOSUSPEND))
            {
                ndrx_xa_nosuspend(EXTRUE);
            }
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_FSYNC))
            {
                NDRX_LOG(log_warn, "XA FSYNC flag found");
                G_atmi_env.xa_fsync_flags|=NDRX_FSYNC_FSYNC;
            }
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_FDATASYNC))
            {
                NDRX_LOG(log_warn, "XA FDATASYNC flag found");
                G_atmi_env.xa_fsync_flags|=NDRX_FSYNC_FDATASYNC;
            }
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_DSYNC))
            {
                NDRX_LOG(log_warn, "XA DSYNC flag found");
                G_atmi_env.xa_fsync_flags|=NDRX_FSYNC_DSYNC;
            }
            else if (0==strcmp(tag_token, NDRX_XA_FLAG_BTIGHT))
            {
                ndrx_xa_btight(EXTRUE);
            }

        } /* for tag.. */
    } /* if xa_flags set */
        
    M_is_xa_init = EXTRUE;
    
    if (EXSUCCEED==ret)
    {
        NDRX_LOG(log_info, "XA lib initialized.");
        /* M_is_xa_init = TRUE; */
    }
    
out:
     
    if (has_lock)
    {
        MUTEX_UNLOCK_V(M_is_xa_init_lock);
    }

    if (NULL!=xa_flags)
    {
        NDRX_FREE(xa_flags);
    }

    if (EXSUCCEED!=ret && NULL!=handle)
    {
        /* close the handle */
        dlclose(handle);
    }

    return ret;
}

/******************************************************************************/
/*                          XA ENTRY FUNCTIONS                                */
/******************************************************************************/

/**
 * Wrapper for `open_entry'
 * @return 
 */
expublic int atmi_xa_open_entry(void)
{
    int ret = EXSUCCEED;
    XA_API_ENTRY(EXFALSE); /* already does ATMI_TLS_ENTRY; */
    
    NDRX_LOG(log_debug, "atmi_xa_open_entry RMID=%hd", G_atmi_env.xa_rmid);
    
    if (G_atmi_tls->G_atmi_xa_curtx.is_xa_open 
            && !G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error)
    {
        NDRX_LOG(log_warn, "xa_open_entry already called for context!");
        goto out;
    }
    
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_open_entry(G_atmi_env.xa_open_str, 
                                    G_atmi_env.xa_rmid, 0)))
    {
        /* required for retry engine, for ATMI it does not play big role here. */
        if (XAER_RMERR==ret)
        {
            ret = XAER_RMFAIL;
            NDRX_LOG(log_error, "atmi_xa_open_entry ret XAER_RMERR remapping to XAER_RMFAIL");
        }

        NDRX_LOG(log_error, "atmi_xa_open_entry - fail: %d [%s]", 
                ret, atmi_xa_geterrstr(ret));
        
        /* we should  generate atmi error */
        ndrx_TPset_error_fmt_rsn(TPERMERR,  ret, "atmi_xa_open_entry - fail: %d [%s]", 
                ret, atmi_xa_geterrstr(ret));
        
        goto out;
    }
    
    G_atmi_tls->G_atmi_xa_curtx.is_xa_open = EXTRUE;
    
    /* flag is set only if processing recon */
    if (G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error)
    {
        NDRX_LOG(log_warn, "RECON: Marking resource connection as OK");
        G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error = EXFALSE;
    }
    
    NDRX_LOG(log_info, "XA interface open");
    
out:
    return ret;
}

/**
 * Wrapper for `close_entry'
 * TODO: Maybe we do not need XA_API_ENTRY here...
 * @param for_retry doing close for retry / error attempt
 * @return XA error
 */
expublic int atmi_xa_close_entry(int for_retry)
{
    int ret = EXSUCCEED;
    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ENTRY */
    
    NDRX_LOG(log_debug, "atmi_xa_close_entry");
    
    if (!G_atmi_tls->G_atmi_xa_curtx.is_xa_open)
    {
        NDRX_LOG(log_warn, "xa_close_entry already called for context!");
        goto out;
    }
    
    /* lets assume it is closed... */
    if (for_retry)
    {
        NDRX_LOG(log_warn, "RECON: Marking resource connection as ERROR");
        G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error = EXTRUE;
    }
    else
    {
        G_atmi_tls->G_atmi_xa_curtx.is_xa_open = EXFALSE;
	
	if (G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error)
	{
        	NDRX_LOG(log_warn, "RECON: Resource connection was marked as ERROR. "
					"Normal close, clearing flag");
        	G_atmi_tls->G_atmi_xa_curtx.is_xa_conn_error = EXFALSE;
	}
    }
    
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_close_entry(G_atmi_env.xa_close_str, 
                                    G_atmi_env.xa_rmid, 0)))
    {
        NDRX_LOG(log_error, "atmi_xa_close_entry - fail: %d [%s]", 
                ret, atmi_xa_geterrstr(ret));
        
        if (!for_retry)
        {
            /* we should  generate atmi error */
            ndrx_TPset_error_fmt_rsn(TPERMERR,  ret, "atmi_xa_close_entry - fail: %d [%s]", 
                    ret, atmi_xa_geterrstr(ret));
        }
        goto out;
    }
    
out:
    return ret;
}

/**
 * Test the RECON settings for error 
 * @param list error code checking list
 * @param retcode return code for xa_start to test for
 * @return TRUE - do retry, FALSE - no retry
 */
exprivate int is_error_in_recon_list(char *list, int retcode)
{
    char scanstr[16];
    char scanstr2[4] = ",*,";
    int ret = EXFALSE;
    
    snprintf(scanstr, sizeof(scanstr), ",%d,", retcode);
    
    NDRX_LOG(log_warn, "%s testing return code [%s] in recon list [%s]", 
            __func__, scanstr, list);
    
    if (NULL!=strstr(list, scanstr))
    {
        NDRX_LOG(log_warn, "matched by code - DO RETRY");
        ret = EXTRUE;
        goto out;
    }
    else if (NULL!=strstr(list, scanstr2))
    {
        NDRX_LOG(log_warn, "matched by wildcard - DO RETRY");
        ret = EXTRUE;
        goto out;
    }
    
out:
    return ret;
    
}
/**
 * Start transaction (or join..) depending on flags.
 * @param xid
 * @param flags
 * @param silent_err for XAER_NOTA or XAER_DUPID errors, do not generate errors in log
 * @return EXSUCCEED/EXFAIL
 */
expublic int atmi_xa_start_entry(XID *xid, long flags, int silent_err)
{
    int ret = EXSUCCEED;
    int need_retry;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE);
    
    NDRX_LOG(log_debug, "%s", __func__);
    
    /* Generic Retry entry */
    GENERIC_RETRY_ENTRY(EXFALSE);
    
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_start_entry(xid, 
                                    G_atmi_env.xa_rmid, flags)))
    {
        
        if ((flags & TMJOIN || flags & TMRESUME) && XAER_NOTA==ret)
        {
            need_retry = EXFALSE;
        }
        else
        {
            need_retry = EXTRUE;
        }
        
        if (!silent_err || need_retry)
        {
            NDRX_LOG(log_error, "%s - fail: %d [%s]", 
                    __func__, ret, atmi_xa_geterrstr(ret));
        }

        /* Core retry engine, no final checks please */
        GENERIC_RETRY_CORE(
            (G_atmi_env.xa_sw->xa_start_entry(xid, G_atmi_env.xa_rmid, flags))
            , (need_retry && is_error_in_recon_list(G_atmi_env.xa_recon_retcodes, ret))
            , (XA_OK!=ret)
            , EXTRUE);

        if (XA_OK!=ret)
        {
            if (silent_err && (XAER_NOTA==ret || XAER_DUPID==ret))
            {
                /* needs to set error silently.. */
                ndrx_TPset_error_fmt_rsn_silent(TPERMERR,  
                        ret, "finally %s - fail: %d [%s]", 
                        __func__, ret, atmi_xa_geterrstr(ret));
            }
            else
            {
                NDRX_LOG(log_error, "finally %s - fail: %d [%s]", 
                        __func__, ret, atmi_xa_geterrstr(ret));

                ndrx_TPset_error_fmt_rsn(TPERMERR,  
                        ret, "finally %s - fail: %d [%s]", 
                        __func__, ret, atmi_xa_geterrstr(ret));
            }
            goto out;
        }
    }
    
out:
    return ret;
}


/**
 * Disassociate current thread from transaction
 * @param xid
 * @param flags
 * @param[in] aborting if set to EXTRUE, we know that abort will follow.
 *  used by posgresql to not to do xa_prepare at the end when rollback will
 *  follow
 * @return 
 */
expublic int atmi_xa_end_entry(XID *xid, long flags, int aborting)
{
    int ret = EXSUCCEED;
    char stat;
    UBFH *p_ub = NULL;
    int local_rb = EXFALSE;
    GENERIC_RETRY_DEF;
    
    XA_API_ENTRY(EXTRUE);
    
    NDRX_LOG(log_debug, "atmi_xa_end_entry flags %ld", flags);
    
    GENERIC_RETRY_ENTRY(EXFALSE);
    
    /* we do always success (as TX intiator decides commit or abort...! */
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_end_entry(xid, 
                                    G_atmi_env.xa_rmid, flags)))
    {
        /* generic retry to keep the connection status with us */
        GENERIC_RETRY(
            (G_atmi_env.xa_sw->xa_end_entry(xid, G_atmi_env.xa_rmid, flags))
            , (is_error_in_recon_list(G_atmi_env.xa_recon_retcodes_other, ret))
            , (XA_OK!=ret)
            );
    }
    
    /* If having no start xid, then we must call prepare! (for postgresql) */

    if (G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOSTARTXID)
    {
        NDRX_LOG(log_debug, "NOSTARTXID - preparing at end!");
        if (aborting && G_atmi_env.pf_xa_loctxabort)
        {
            NDRX_LOG(log_info, "Aborting using local rollback func");
            local_rb = EXTRUE;
            ret = G_atmi_env.pf_xa_loctxabort(xid, TMNOFLAGS);
            
            if (XA_OK!=ret)
            {
                NDRX_LOG(log_error, "Failed to disconnect from transaction: %d", ret);
                userlog("Failed to disconnect from transaction: %d", ret);
            }
        }
        /*
        else if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_prepare_entry(xid, 
                                        G_atmi_env.xa_rmid, TMNOFLAGS)))
         * moved to common prepare engine.
         */
        else if (XA_OK!=(ret=atmi_xa_prepare_entry(xid, TMNOFLAGS)) && XA_RDONLY!=ret)
        {
            NDRX_LOG(log_error, "Failed to prepare transaction at NOSTARTXID end");
            goto out;
        }
        
        /* test of transaction automatic rollback */
        if ((XA_OK==ret || XA_RDONLY == ret) &&
                NDRX_SYSTEST_ENBLD && ndrx_systest_case(NDRX_SYSTEST_ENDPREPFAIL))
        {
            NDRX_LOG(log_error, "SYSTEST! Generating end-fail error");
            atmi_xa_rollback_entry(xid, 0L);
            ret = XAER_RMERR;
        }
        
        /* Report status to TMSRV that we performed prepare
         * and report the results back...
         * If there is failure with TMSRV, then rollback prepared transaction
         */
        if (local_rb)
        {
            stat = XA_RM_STATUS_ABORTED;
        }
        else if (XA_OK==ret)
        {
            stat = XA_RM_STATUS_PREP;
        }
        else if (XA_RDONLY==ret)
        {
            stat = XA_RM_STATUS_COMMITTED_RO;
        }
        else
        {
            /* this is rollback */
            stat = XA_RM_STATUS_ABORTED;
        }
        
        /* call tmsrv */
        
        /*
         * if call failed due to transaction not found or status unknown
         * then we rollback the transaction
         */
        
        NDRX_LOG(log_debug, "Reporting branch transaction status: %c", stat);
        p_ub = atmi_xa_call_tm_rmstatus(G_atmi_tls->G_atmi_xa_curtx.txinfo, stat);
        
        /* if there is matching error, then we abort the  */
        if (TPEMATCH==tperrno)
        {
            NDRX_LOG(log_error, "Got matching error! Abort transaction");
            atmi_xa_rollback_entry(xid, 0L);
        }
        
    }
    
    
out:
    if (NULL!=p_ub)
    {
        tpfree((char *)p_ub);
    }
                                
    return ret;
}

/**
 * Rollback the transaction.
 * @param xid
 * @param flags
 * @return XA error code
 */
expublic int atmi_xa_rollback_entry(XID *xid, long flags)
{
    int ret = EXSUCCEED;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE);
    
    NDRX_LOG(log_debug, "atmi_xa_rollback_entry");
    
    GENERIC_RETRY_ENTRY(EXFALSE);
    
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_rollback_entry(xid, 
                                    G_atmi_env.xa_rmid, flags)))
    {
        /* in case of protocol error, we need to re-open, as connection
         * might be is closed.
         */
        GENERIC_RETRY(
            (G_atmi_env.xa_sw->xa_rollback_entry(xid, G_atmi_env.xa_rmid, flags))
            , (is_error_in_recon_list(G_atmi_env.xa_recon_retcodes_other, ret))
            , (XA_OK!=ret)
            );
    }
    
out:
    return ret;
}

/**
 * Prepare for commit current transaction on local resource manager.
 * @param xid
 * @param flags
 * @return XA error code
 */
expublic int atmi_xa_prepare_entry(XID *xid, long flags)
{
    int ret = EXSUCCEED;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE);
    
    NDRX_LOG(log_debug, "atmi_xa_prepare_entry");
    
    GENERIC_RETRY_ENTRY(EXFALSE);
     
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_prepare_entry(xid, 
                                    G_atmi_env.xa_rmid, flags)))
    {
        /* no special log needed! */
        if (XA_RDONLY==ret)
        {
            NDRX_LOG(log_debug, "xa_prepare_entry - fail: %d [%s]", 
                ret, atmi_xa_geterrstr(ret));
            ndrx_TPset_error_fmt_rsn(TPERMERR,  ret, "xa_prepare_entry - fail: %d [%s]", 
                ret, atmi_xa_geterrstr(ret));
        }
        else
        {
            GENERIC_RETRY(
                (G_atmi_env.xa_sw->xa_prepare_entry(xid, G_atmi_env.xa_rmid, flags))
                , (is_error_in_recon_list(G_atmi_env.xa_recon_retcodes_other, ret))
                , (XA_OK!=ret)
                 );

        }
        
        
        goto out;
    }
    
out:
    return ret;
}

/**
 * Forget RM known transaction
 * @param xid
 * @param flags
 * @return XA error code
 */
expublic int atmi_xa_forget_entry(XID *xid, long flags)
{
    int ret = EXSUCCEED;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE);
    
    NDRX_LOG(log_debug, "atmi_xa_forget_entry");
     
    GENERIC_RETRY_ENTRY(EXFALSE);
    
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_forget_entry(xid, 
                                    G_atmi_env.xa_rmid, flags)))
    {
        /* in case of protocol error, we need to re-open, as connection
         * might be is closed.
         */
        GENERIC_RETRY(
               (G_atmi_env.xa_sw->xa_forget_entry(xid, G_atmi_env.xa_rmid, flags))
               , (is_error_in_recon_list(G_atmi_env.xa_recon_retcodes_other, ret))
               , (XA_OK!=ret)
                );
    }
    
out:
    return ret;
}


/**
 * Prepare for commit current transaction on local resource manager.
 * @param xid
 * @param flags
 * @return XA error code
 */
expublic int atmi_xa_commit_entry(XID *xid, long flags)
{
    int ret = EXSUCCEED;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE);
    
    GENERIC_RETRY_ENTRY(EXFALSE);
    NDRX_LOG(log_debug, "atmi_xa_commit_entry");
    if (XA_OK!=(ret = G_atmi_env.xa_sw->xa_commit_entry(xid, 
                                    G_atmi_env.xa_rmid, flags)))
    {
        GENERIC_RETRY(
                (G_atmi_env.xa_sw->xa_commit_entry(xid,G_atmi_env.xa_rmid, flags))
                , (XAER_RMFAIL==ret)
                , (XA_OK!=ret));
    }
    
out:
    return ret;
}

/**
 * Get the list of transactions associate with RMID
 * @param xids xid array
 * @param count array len of xids
 * @param rmid resource manager id
 * @param flags flags
 * @return >= number of trans in xid, 
 */
expublic int atmi_xa_recover_entry(XID *xids, long count, int rmid, long flags)
{
    int ret = EXSUCCEED;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE);
    
    NDRX_LOG(log_debug, "%s", __func__);
    
    GENERIC_RETRY_ENTRY(EXFALSE);
    
    if (0 > (ret = G_atmi_env.xa_sw->xa_recover_entry(xids, count,
                                    G_atmi_env.xa_rmid, flags)))
    {
        GENERIC_RETRY(
            (G_atmi_env.xa_sw->xa_recover_entry(xids, count, G_atmi_env.xa_rmid, flags))
            , (is_error_in_recon_list(G_atmi_env.xa_recon_retcodes_other, ret))
            , (0 > ret)
            );
    }
    
out:
    return ret;
}

/******************************************************************************/
/*                          ATMI API                                          */
/******************************************************************************/




/**
 * Begin the global transaction.
 * - We should check the context already, maybe we run in transaction already?
 * 
 * But basically we should call the server, to get the transaction id.
 * 
 * @param timeout
 * @param flags
 * @return 
 */
expublic int ndrx_tpbegin(unsigned long timeout, long flags)
{
    int ret=EXSUCCEED;
    UBFH *p_ub = atmi_xa_alloc_tm_call(ATMI_XA_TPBEGIN);
    atmi_xa_tx_info_t xai;
    long tmflags = 0;
    GENERIC_RETRY_DEF;
    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ENTRY */
    
    NDRX_LOG(log_debug, "%s enter", __func__);
    
    memset(&xai, 0, sizeof(atmi_xa_tx_info_t));
    
    
    if (!G_atmi_tls->G_atmi_xa_curtx.is_xa_open)
    {
        NDRX_LOG(log_error, "tpbegin: - tpopen() was not called!");
        ndrx_TPset_error_msg(TPEPROTO,  "tpbegin - tpopen() was not called!");
        EXFAIL_OUT(ret);
    }

    if (0!=flags)
    {
        NDRX_LOG(log_error, "tpbegin: flags != 0");
        ndrx_TPset_error_msg(TPEINVAL,  "tpbegin: flags != 0");
        EXFAIL_OUT(ret);
    }
    
    /* If we have active transaction, then we are in txn mode.. */
    if (G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_error, "tpbegin: - already in transaction mode XID: [%s]", 
                G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        ndrx_TPset_error_fmt(TPEPROTO,  "tpbegin: - already in transaction mode XID: [%s]", 
                G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        EXFAIL_OUT(ret);
    }
    
    NDRX_LOG(log_debug, "About to call TM");
    /* Load the timeout param to FB... */
    if (EXSUCCEED!=Bchg(p_ub, TMTXTOUT, 0, (char *)&timeout, 0L))
    {
        ndrx_TPset_error_fmt(TPESYSTEM,  "tpbegin: - failed to fill FB - set TMTXTOUT!");
        EXFAIL_OUT(ret);
    }
    
    if (XA_IS_DYNAMIC_REG)
    {
        /* tell RM, then we use dynamic reg (so that it does not register 
         * local RMs work)
         */
        tmflags|=TMFLAGS_DYNAMIC_REG;
    }
    
    if (G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOSTARTXID)
    {
        tmflags|=TMFLAGS_TPNOSTARTXID;
    }
    
    if (EXSUCCEED!=Bchg(p_ub, TMTXFLAGS, 0, (char *)&tmflags, 0L))
    {
        ndrx_TPset_error_fmt(TPESYSTEM,  "tpbegin: - failed to fill FB - set TMTXFLAGS!");
        EXFAIL_OUT(ret);
    }
    
    /* OK, we should call the server, request for transaction...  */
    if (NULL==(p_ub=atmi_xa_call_tm_generic_fb(ATMI_XA_TPBEGIN, NULL, EXTRUE, EXFAIL, 
            NULL, p_ub)))
    {
        NDRX_LOG(log_error, "Failed to execute TM command [%c]", 
                    ATMI_XA_TPBEGIN);
        /* _TPoverride_code(TPETRAN);  - WHY?*/
        EXFAIL_OUT(ret);
    }
    /* We should load current context with transaction info we got 
     * + we should join the transaction i.e. current thread.
     */
    
    if (EXSUCCEED!=atmi_xa_read_tx_info(p_ub, &xai, 0))
    {
        NDRX_LOG(log_error, "tpbegin: - failed to read TM response");
        ndrx_TPset_error_msg(TPEPROTO,  "tpbegin: - failed to read TM response");
        EXFAIL_OUT(ret);
    }
    
    NDRX_LOG(log_debug, "About to load tx info");
    
    /* Only when we have in transaction, then install the handler */
    if (EXSUCCEED!= atmi_xa_set_curtx_from_xai(&xai))
    {
        NDRX_LOG(log_error, "tpbegin: - failed to set curren tx");
        ndrx_TPset_error_msg(TPEPROTO,  "tpbegin: - failed to set curren tx");
        EXFAIL_OUT(ret);
    }
    /*G_atmi_xa_curtx.is_in_tx = TRUE;*/
    G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags |= XA_TXINFO_INITIATOR;
    
    /* OK... now join the transaction (if we are static...) (only if static) */
    if (!XA_IS_DYNAMIC_REG)
    {
        if (EXSUCCEED!=atmi_xa_start_entry(atmi_xa_get_branch_xid(&xai, xai.btid), 
                TMNOFLAGS, EXFALSE))
        {
            /* got to rollback the curren transaction*/
            NDRX_LOG(log_error, "Failed to join transaction!");
            ndrx_xa_join_fail(NULL, EXFALSE);
            atmi_xa_reset_curtx();
            EXFAIL_OUT(ret);
        }
        
        /* Already set by TM
        atmi_xa_curtx_set_cur_rmid(&xai);
         */
    }
    else
    {
        NDRX_LOG(log_debug, "Working in dynamic mode...");
        /* if connection is bad, please reconect */
        GENERIC_RETRY_ENTRY(EXTRUE);
    }

    NDRX_LOG(log_debug, "Process joined to transaction [%s] OK",
                        G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
    
out:

    /* TODO: We need remove curren transaction from HASH! */
    if (NULL!=p_ub)
    {
        /* save errors */
        atmi_error_t err;
        
        /* Save the original error/needed later! */
        ndrx_TPsave_error(&err);
        tpfree((char *)p_ub);  /* This stuff removes ATMI error!!! */
        ndrx_TPrestore_error(&err);
    }
    return ret;
}


/**
 * Set when to return from commit. Either when transaction is completed
 * or when transaction is logged in persisted device for further background
 * completion.
 * The default is TP_CMT_COMPLETE.
 * @param flags TP_CMT_LOGGED or TP_CMT_COMPLETE
 * @return EXSUCCEED/EXFAIL
 */
expublic int ndrx_tpscmt(long flags)
{
    int ret = EXSUCCEED;
    
    if (TP_CMT_LOGGED!=flags &&
            TP_CMT_COMPLETE!=flags)
    {
        NDRX_LOG(log_error, "Invalid value: commit return %ld", (long)flags);
        ndrx_TPset_error_fmt(TPEINVAL,  "Invalid value: commit return %ld", 
                (long)flags);
        EXFAIL_OUT(ret);
    }
    
    if (TX_COMMIT_COMPLETED==G_atmi_tls->tx_commit_return)
    {
        ret = TP_CMT_COMPLETE;
    }
    else
    {
        ret = TP_CMT_LOGGED;
    }
    
    if (TP_CMT_COMPLETE==flags)
    {
        G_atmi_tls->tx_commit_return = TX_COMMIT_COMPLETED;
    }
    
    if (TP_CMT_LOGGED==flags)
    {
        G_atmi_tls->tx_commit_return = TX_COMMIT_DECISION_LOGGED;
    }
    
    NDRX_LOG(log_info, "Commit return set to %ld (TX) ret %d", 
            (long)G_atmi_tls->tx_commit_return, ret);
    
out:
    return ret;
}

/**
 * API implementation of tpcommit
 *
 * @param timeout
 * @param flags TPTXCOMMITDLOG - return when prepared
 * @return EXSUCCEED/EXFAIL
 */
expublic int ndrx_tpcommit(long flags)
{
    int ret=EXSUCCEED;
    UBFH *p_ub = NULL;
    int do_abort = EXFALSE;
    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ENTRY; */
    
    NDRX_LOG(log_debug, "%s enter", __func__);
    
    if (!G_atmi_tls->G_atmi_xa_curtx.is_xa_open)
    {
        NDRX_LOG(log_error, "tpcommit: - tpopen() was not called!");
        ndrx_TPset_error_msg(TPEPROTO,  "tpcommit - tpopen() was not called!");
        ret=EXFAIL;
        goto out_no_reset;
    }

    if (0!=flags && !(flags & TPTXCOMMITDLOG))
    {
        NDRX_LOG(log_error, "tpcommit: flags != 0 && !TPTXCOMMITDLOG");
        ndrx_TPset_error_msg(TPEINVAL,  "tpcommit: flags != 0 && !TPTXCOMMITDLOG");
        ret=EXFAIL;
        goto out_no_reset;
    }
    
    if (!G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_error, "tpcommit: Not in global TX");
        ndrx_TPset_error_msg(TPEPROTO,  "tpcommit: Not in global TX");
        ret=EXFAIL;
        goto out_no_reset;
        
    }
            
    /* allow commit even, if we are not the initiators,
     * but for auto-tran this is OK
     */
    if (!G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags)
    {
        NDRX_LOG(log_error, "tpcommit: Not not initiator");
        ndrx_TPset_error_msg(TPEPROTO,  "tpcommit: Not not initiator");
        ret=EXFAIL;
        goto out_no_reset;
    }
    
    /* flag is shared with tx: */
    if (TX_COMMIT_DECISION_LOGGED == G_atmi_tls->tx_commit_return)
    {
        flags|=TPTXCOMMITDLOG;
    }
    
    /* Check situation with call descriptors */
    if (atmi_xa_cd_isanyreg(&(G_atmi_tls->G_atmi_xa_curtx.txinfo->call_cds)))
    {
        NDRX_LOG(log_error, "tpcommit: Open call descriptors found - abort!");
        do_abort = EXTRUE;
    }
    
    if (atmi_xa_cd_isanyreg(&(G_atmi_tls->G_atmi_xa_curtx.txinfo->conv_cds)))
    {
        NDRX_LOG(log_error, "tpcommit: Open conversation descriptors found - abort!");
        do_abort = EXTRUE;
    }
    
    if (G_atmi_tls->G_atmi_xa_curtx.txinfo->tmtxflags & TMTXFLAGS_IS_ABORT_ONLY)
    {
        NDRX_LOG(log_error, "tpcommit: Transaction marked as abort only!");
        do_abort = EXTRUE;
    }
    
    /* Disassoc from transaction! */
    
    /* TODO: Detect when we need the END entry. 
     * it should be work_done, or static reg!!!
     */
    if (!XA_IS_DYNAMIC_REG || 
            (XA_TXINFO_AXREG_CLD & G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags))
    {
        if (EXSUCCEED!= (ret=atmi_xa_end_entry(atmi_xa_get_branch_xid(G_atmi_tls->G_atmi_xa_curtx.txinfo, 
                G_atmi_tls->G_atmi_xa_curtx.txinfo->btid), TMSUCCESS, EXFALSE)))
        {
            NDRX_LOG(log_error, "Failed to end XA api: %d [%s] - aborting", 
                    ret, atmi_xa_geterrstr(ret));
            userlog("Failed to end XA api: %d [%s] - aborting", 
                    ret, atmi_xa_geterrstr(ret));
            
            do_abort = EXTRUE;
        }
    }
    
    if (do_abort)
    {
        /* common termination at commit. */
        ret = ndrx_tpabort(0, EXFALSE);
        
        /* in this case the tmsrv might already rolled back
         * thus assume that transaction is aborted.
         */
        if (EXSUCCEED==ret || TPEPROTO==tperrno)
        {
            /* clear current error */
            ndrx_TPunset_error();
            ndrx_TPset_error_msg(TPEABORT,  "tpcommit: Transaction was marked for "
                    "abort and aborted now!");
            ret=EXFAIL;
        }
        
        return ret; /*<<<<<<<<<< RETURN!!! */
    }
    
    NDRX_LOG(log_debug, "About to call TM flags=%ld", flags);
    /* OK, we should call the server, request for transaction...  */
    
    /* TODO: pass flags to call struct! */
    if (NULL==(p_ub=atmi_xa_call_tm_generic(ATMI_XA_TPCOMMIT, EXFALSE, EXFAIL, 
            G_atmi_tls->G_atmi_xa_curtx.txinfo, flags, EXFAIL)))
    {
        NDRX_LOG(log_error, "Failed to execute TM command [%c]", 
                    ATMI_XA_TPCOMMIT);
        
        /* _TPoverride_code(TPETRAN); */
        
        EXFAIL_OUT(ret);
    }

    NDRX_LOG(log_debug, "Transaction [%s] commit OK",
                        G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        
out:
                            
    /* reset global transaction info */
    atmi_xa_reset_curtx();

out_no_reset:

    if (NULL!=p_ub)
    {
        /* save errors */
        atmi_error_t err;
        
        /* Save the original error/needed later! */
        ndrx_TPsave_error(&err);
        tpfree((char *)p_ub);  /* This stuff removes ATMI error!!! */
        ndrx_TPrestore_error(&err);
    }


    return ret;
}


/**
 * API implementation of tpabort
 * @param timeout
 * @param flags
 * @param call_xa_end shall the xa_end() be called?
 * @return 
 */
expublic int ndrx_tpabort(long flags, int call_xa_end)
{
    int ret=EXSUCCEED;
    UBFH *p_ub = NULL;
    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ENTRY; */
    
    NDRX_LOG(log_debug, "_tpabort enter");
    
    if (!G_atmi_tls->G_atmi_xa_curtx.is_xa_open)
    {
        NDRX_LOG(log_error, "tpabort: - tpopen() was not called!");
        ndrx_TPset_error_msg(TPEPROTO,  "tpabort - tpopen() was not called!");
        ret=EXFAIL;
        goto out_no_reset;
    }

    if (0!=flags)
    {
        NDRX_LOG(log_error, "tpabort: flags != 0");
        ndrx_TPset_error_msg(TPEINVAL,  "tpabort: flags != 0");
        ret=EXFAIL;
        goto out_no_reset;
    }
    
    if (!G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_error, "tpabort: Not in global TX");
        ndrx_TPset_error_msg(TPEPROTO,  "tpabort: Not in global TX");
        ret=EXFAIL;
        goto out_no_reset;
    }
            
    if (!(XA_TXINFO_INITIATOR & G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags))
    {
        NDRX_LOG(log_error, "tpabort: Not not initiator");
        ndrx_TPset_error_msg(TPEPROTO,  "tpabort: Not not initiator");
        ret=EXFAIL;
        goto out_no_reset;
    }
    
    /* Disassoc from transaction! */
    if (call_xa_end)
    {
        if (!XA_IS_DYNAMIC_REG || 
                (XA_TXINFO_AXREG_CLD & G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags))
        {
            /* abort anyway... */
            if (EXSUCCEED!= atmi_xa_end_entry(
                    atmi_xa_get_branch_xid(G_atmi_tls->G_atmi_xa_curtx.txinfo,
                    G_atmi_tls->G_atmi_xa_curtx.txinfo->btid), TMSUCCESS, EXTRUE))
            {
                NDRX_LOG(log_error, "Failed to end XA api: %d [%s]", 
                        ret, atmi_xa_geterrstr(ret));
                userlog("Failed to end XA api: %d [%s]", 
                        ret, atmi_xa_geterrstr(ret));
            }
        }
    }
    
    NDRX_LOG(log_debug, "About to call TM");
    /* OK, we should call the server, request for transaction...  */
    if (NULL==(p_ub=atmi_xa_call_tm_generic(ATMI_XA_TPABORT, EXFALSE, EXFAIL, 
            G_atmi_tls->G_atmi_xa_curtx.txinfo, 0L, EXFAIL)))
    {
        NDRX_LOG(log_error, "Failed to execute TM command [%c]", 
                    ATMI_XA_TPBEGIN);
        
        /* _TPoverride_code(TPETRAN); */
        
        EXFAIL_OUT(ret);
    }

    NDRX_LOG(log_debug, "Transaction [%s] abort OK",
                        G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
out:
    /* reset global transaction info */
    atmi_xa_reset_curtx();

out_no_reset:

    if (NULL!=p_ub)
    {
        /* save errors */
        atmi_error_t err;
        
        /* Save the original error/needed later! */
        ndrx_TPsave_error(&err);
        tpfree((char *)p_ub);  /* This stuff removes ATMI error!!! */
        ndrx_TPrestore_error(&err);
    }



    return ret;
}

/**
 * Open the entry to XA.
 * @return 
 */
expublic int ndrx_tpopen(void)
{
    int ret=EXSUCCEED;
    XA_API_ENTRY(EXTRUE);
   
    ret = atmi_xa_open_entry();
    
out:
    return ret;
}

/**
 * Close the entry to XA.
 * @return EXSUCCEED/EXFAIL
 */
expublic int ndrx_tpclose(void)
{
    int ret=EXSUCCEED;
    
    XA_API_ENTRY(EXTRUE);

    if (G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_error, "tpclose: - cannot close as process in TX: [%s]", 
                G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        ndrx_TPset_error_fmt(TPEPROTO, "tpclose: - cannot close as process in TX: [%s]", 
                G_atmi_tls->G_atmi_xa_curtx.txinfo->tmxid);
        EXFAIL_OUT(ret);
    }
   
    ret = atmi_xa_close_entry(EXFALSE);
    
out:
    return ret;
}
 
/**
 * Suspend the current transaction in progress.
 * Note we do not care is it server or client. Global transaction (even participants)
 * can be moved to another external process.
 * 
 * NODE: we might get additional error code:TPERMERR when xa_end fails.
 * @param tranid
 * @param flags
 * @param[in] is_contexting are we doing context swtching? 
 *  and if driver flags sys that no api suspend needed, then do not call the xa-commands
 *  particulary this is needed for java api for which transaction state is kept in
 *  java object and not in C TLS.
 * @return SUCCEED/FAIL
 */
expublic int ndrx_tpsuspend (TPTRANID *tranid, long flags, int is_contexting)
{
    int ret=EXSUCCEED;
    long xa_flags = TMSUCCESS;
    
    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ENTRY; */
    NDRX_LOG(log_info, "Suspending global transaction: atmi flags %lx", flags);
    if (NULL==tranid)
    {
        ndrx_TPset_error_msg(TPEINVAL,  "_tpsuspend: trandid = NULL!");
        EXFAIL_OUT(ret);
    }
    
    if (0!= (flags & ~TPTXTMSUSPEND) )
    {
        ndrx_TPset_error_msg(TPEINVAL,  "_tpsuspend: flags is not 0, nor TPTXTMSUSPEND");
        EXFAIL_OUT(ret);
    }
    
    if (!G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_error, "_tpsuspend: Not in global TX");
        ndrx_TPset_error_msg(TPEPROTO,  "_tpsuspend: Not in global TX");
        EXFAIL_OUT(ret);
    }
    
    /* 
     * for no-join we will continue as is with TMSUCCESS, as TMSUSPEND
     * is not supported 100%
     */
    if ( (flags & TPTXTMSUSPEND) && !(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOJOIN))
    {
        xa_flags = TMSUSPEND;
    }
    
#if 0
    - I guess this is not a problem. Must be able to suspend abort only transaction
    because of object-api
    if (G_atmi_tls->G_atmi_xa_curtx.txinfo->tmtxflags & TMTXFLAGS_IS_ABORT_ONLY)
    {
        NDRX_LOG(log_error, "_tpsuspend: Abort only transaction!");
        ndrx_TPset_error_msg(TPEPROTO,  "_tpsuspend: Abort only transaction!");
        EXFAIL_OUT(ret);
    }
#endif
    
    /* Check situation with call descriptors */
    if (!is_contexting  /* do not check call descriptors in contexting mode */
            && atmi_xa_cd_isanyreg(&(G_atmi_tls->G_atmi_xa_curtx.txinfo->call_cds)))
    {
        NDRX_LOG(log_error, "_tpsuspend: Call descriptors still open!");
        ndrx_TPset_error_msg(TPEPROTO,  "_tpsuspend: Call descriptors still open!");
        EXFAIL_OUT(ret);
    }
    
    if (!is_contexting /* do not check call descriptors in contexting mode */
            && atmi_xa_cd_isanyreg(&(G_atmi_tls->G_atmi_xa_curtx.txinfo->conv_cds)))
    {
        NDRX_LOG(log_error, "_tpsuspend: Conversation descriptors still open!");
        ndrx_TPset_error_msg(TPEPROTO,  "_tpsuspend: Conversation descriptors still open!");
        EXFAIL_OUT(ret);
    }
    
    /* Now transfer current transaction data from one struct to another... */
    
    XA_TX_COPY(tranid, G_atmi_tls->G_atmi_xa_curtx.txinfo);
    tranid->is_tx_initiator = G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags;
    
    /* TODO: if join is not supported, then we terminate current transaction/BTID
     * and that shall be removed from list.
     * That is done by atmi_xa_reset_curtx.
     * Thus at this point we just end our journey wit this BTID
     */
    
    /* Disassoc from transaction! */
    if (!XA_IS_DYNAMIC_REG || 
            (XA_TXINFO_AXREG_CLD & G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags))
    {
        /*
	 * causes ORA-24775 error
        long xaflags = TMSUSPEND;
        
        if (!(G_atmi_env.xa_sw->flags & TMNOMIGRATE))
        {
            NDRX_LOG(log_debug, "Setting migrate flag for suspend");
            xaflags|=TMMIGRATE;
        }
        */
        
        if (EXSUCCEED!= (ret=atmi_xa_end_entry(
                atmi_xa_get_branch_xid(G_atmi_tls->G_atmi_xa_curtx.txinfo,
                G_atmi_tls->G_atmi_xa_curtx.txinfo->btid), xa_flags, EXFALSE)))
        {
            int did_abort = EXFALSE;
            NDRX_LOG(log_error, "Failed to end XA api: %d [%s] flags: %lx", 
                    ret, atmi_xa_geterrstr(ret), xa_flags);
            userlog("Failed to end XA api: %d [%s] flags: %lx", 
                    ret, atmi_xa_geterrstr(ret), xa_flags);
            
            
            ndrx_xa_join_fail(&did_abort, EXFALSE);
            
            if (did_abort)
            {
                ndrx_TPoverride_code(TPEABORT);
            }
            else
            {
                ndrx_TPoverride_code(TPESYSTEM);
            }
            
            goto out;
        }
    }

    atmi_xa_reset_curtx();
    
    NDRX_LOG(log_debug, "Suspend ok xid: [%s] xa flags: %lx", 
            tranid->tmxid, xa_flags);
out:

    return ret;
}

/**
 * Resume suspended transaction
 * @param tranid transaction id object
 * @param flags TPTXNOOPTIM - do not use optimization known RMs
 * @return EXUSCCEED/EXFAIL
 */
expublic int  ndrx_tpresume (TPTRANID *tranid, long flags)
{
    int ret=EXSUCCEED;
    int was_join = EXFALSE;
    long join_flag = TMJOIN;
    atmi_xa_tx_info_t xai;
    
    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ETNRY; */
    NDRX_LOG(log_info, "Resuming global transaction...");
    
    if (NULL==tranid)
    {
        ndrx_TPset_error_msg(TPEINVAL,  "_tpresume: trandid = NULL!");
        EXFAIL_OUT(ret);
    }
       
    if (0!= (flags & ~ (TPTXNOOPTIM | TPTXTMSUSPEND)) )
    {
        ndrx_TPset_error_msg(TPEINVAL,  "_tpresume: flags is not 0, "
                "nor TPTXNOOPTIM, nor TPTXTMSUSPEND");
        EXFAIL_OUT(ret);
    }
    
    /* Resume the trany 
     * Also if operating in nojoin mode, then there is no need for this flag.
     * Thought all could be handled in _tp_srv_join_or_new
     * but anyway for consistency will leave this peace of code here.
     */
    if ( (flags & TPTXTMSUSPEND)
            && !(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOJOIN))
    {
        join_flag = TMRESUME;
    }
    
    /* NOTE: TPEMATCH - not tracked. */
    if (G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        ndrx_TPset_error_msg(TPEPROTO,  "_tpresume: Already in global TX!");
        EXFAIL_OUT(ret);
    }
    
    /* Copy off the tx info to call */
    XA_TX_COPY((&xai), tranid);
    
    /* do not use optimization data... */
    if (flags & TPTXNOOPTIM)
    {
        xai.tmknownrms[0]=EXEOS;
    }
    
    if (EXSUCCEED!=_tp_srv_join_or_new(&xai, EXFALSE, &was_join, join_flag,
            tranid->is_tx_initiator))
    {
        ndrx_TPset_error_msg(TPESYSTEM,  "_tpresume: Failed to enter in global TX!");
        EXFAIL_OUT(ret);
    }
    
    NDRX_LOG(log_debug, "Resume ok xid: [%s] is_tx_initiator: %d abort_only: %d", 
            tranid->tmxid, tranid->is_tx_initiator, 
            G_atmi_tls->G_atmi_xa_curtx.txinfo->tmtxflags & TMTXFLAGS_IS_ABORT_ONLY);
    
out:
    return ret;
}

/**
 * Database is registering transaction in progress...
 * @param rmid
 * @param xid
 * @param flags
 * @return 
 */
expublic int ax_reg(int rmid, XID *xid, long flags)
{
    int ret = TM_OK;
    int was_join = EXFALSE;
    ATMI_TLS_ENTRY;
    
    NDRX_LOG(log_info, "ax_reg called");
    if (NULL==G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_error, "ERROR: No global transaction registered "
                "with process/thread!");
        userlog("ERROR: No global transaction registered with process/thread!");
        memset(xid, 0, sizeof(XID));
        ret = TMER_TMERR;
        goto out;
    }
    
    if (EXSUCCEED!=_tp_srv_join_or_new(G_atmi_tls->G_atmi_xa_curtx.txinfo, 
            EXTRUE, &was_join, TMJOIN, G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags))
    {
        ret = TMER_TMERR;
        goto out;
    }
    
    if (was_join)
    {
        ret = TM_JOIN;
    }
    
    memcpy(xid, atmi_xa_get_branch_xid(G_atmi_tls->G_atmi_xa_curtx.txinfo, 
            G_atmi_tls->G_atmi_xa_curtx.txinfo->btid), sizeof(*xid));
    
    /* why? already handled by _tp_srv_join_or_new()
    G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags |= XA_TXINFO_AXREG_CLD;
    */
  
out:
    NDRX_LOG(log_info, "ax_reg returns: %d", ret);
    return ret;
}
 
/**
 * DB is un-registering the transaction
 * @param rmid
 * @param flags
 * @return 
 */
int ax_unreg(int rmid, long flags)
{
    NDRX_LOG(log_info, "ax_unreg called");
    return EXSUCCEED;
}

/**
 * Wrapper with call structure 
 * @param call
 * @return 
 */
expublic int _tp_srv_join_or_new_from_call(tp_command_call_t *call,
        int is_ax_reg_callback)
{
    int is_known = EXFALSE;
    atmi_xa_tx_info_t xai;
    memset(&xai, 0, sizeof(xai));
    /* get the xai struct */
    /*
    atmi_xa_xai_from_call(&xai, call);
     */
    XA_TX_COPY((&xai), call)
    
    return _tp_srv_join_or_new(&xai, is_ax_reg_callback, &is_known, TMJOIN, 
            XA_TXINFO_NOFLAGS);
}

/**
 * In case of for any process join have failed we shall report this
 * to TMSRV so that it can rollback the transaction as soon as possible.
 * Otherwise the caller might have lost the control of the transaction
 * and thus it would not be able to rollback it, and thus it will wait
 * timeout at tmsrv (which could be long) so better notify tmsrv to
 * perform abort. This assume that current transaction is set.
 * @param did_abort is transaction aborted at the end?
 * @param force_abort shall abort happen?
 * @return EXSUCCED/EXFAIL
 */
exprivate int ndrx_xa_join_fail(int *did_abort, int force_abort)
{
    UBFH *p_ub = NULL;
    int ret = EXSUCCEED;
    /* save errors */
    atmi_error_t err;
    
    /* no action, as we did not start the transaction
     * so that client can finalize
     */
    if (!(G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags & XA_TXINFO_INITIATOR) &&
            !force_abort)
    {
        return EXSUCCEED;
    }
        
    /* Save the original error/needed later! */
    ndrx_TPsave_error(&err);
        
    NDRX_LOG(log_error, "xa_start() or xa_end() failed, aborting to TMSRV");
    
    if (NULL==(p_ub=atmi_xa_call_tm_generic(ATMI_XA_TPABORT, EXFALSE, EXFAIL, 
            G_atmi_tls->G_atmi_xa_curtx.txinfo, 0L, EXFAIL)))
    {
        NDRX_LOG(log_error, "Failed to execute TM command [%c]", 
                    ATMI_XA_TPABORT);
        
        /* _TPoverride_code(TPETRAN); */
        
        EXFAIL_OUT(ret);
    }
    
    if (NULL!=did_abort)
    {
        *did_abort = EXTRUE;
    }
    
out:
    if (NULL!=p_ub)
    {
        tpfree((char *)p_ub);  /* This stuff removes ATMI error!!! */
        
    }
    ndrx_TPrestore_error(&err);
    
    return ret;
}

/**
 * Process should try to join the XA, if fails, then create new transaction
 * @param call
 * @param join_flag override the default join setting (for TMRESUME)
 *  default for callers shall be TMJOIN
 * @param tranid_flags local / tranid flags of the transaction
 * @return SUCCEED/FAIL
 */
expublic int _tp_srv_join_or_new(atmi_xa_tx_info_t *p_xai,
        int is_ax_reg_callback, int *p_is_known, long join_flag, int tranid_flags)
{
    int ret = EXSUCCEED;
    UBFH *p_ub = NULL;
    short reason;
    int new_rm = EXFALSE;
    char src_tmknownrms[2];
    long tmflags = 0;
    GENERIC_RETRY_DEF;

    XA_API_ENTRY(EXTRUE); /* already does ATMI_TLS_ENTRY; */
    
    /* Do the same static flow if ax_reg was already called in dynamic mode
     * this might be true if coming back from tpsuspend()
     * thus we shall perform xa_start.
     */
    if (!(XA_IS_DYNAMIC_REG) || (tranid_flags & XA_TXINFO_AXREG_CLD))
    {
        
        if (EXSUCCEED!=atmi_xa_set_curtx_from_xai(p_xai))
        {
            EXFAIL_OUT(ret);
        }
        
        /* keep the origin flag. */
        G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags = tranid_flags;
    }
    else
    {
        /* 
         * this is Dynamic registration.
         * 
         * This is first time server joins, no work yet done.
         */
        if (!is_ax_reg_callback)
        {
            NDRX_LOG(log_debug, "Dynamic reg + process start "
                                "just remember the transaction");
            
            /* if connection is bad, please reconect */
            GENERIC_RETRY_ENTRY(EXTRUE);

            /* OK, but how BTID is filled?,
             * BTID is set on second pass by bellow common source
             */
            if (EXSUCCEED!=atmi_xa_set_curtx_from_xai(p_xai))
            {
                EXFAIL_OUT(ret);
            }

            /* keep the origin flag. */
            G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags = tranid_flags;

            /* if connection is bad, please reconect */
            GENERIC_RETRY_ENTRY(EXTRUE);

            /* Do not do anything more... */
            goto out;
        }
        else
        {
            /*
             * At this point resource is modified. Note that if we perform
             * resume on such transaction, we shall do the xa_start()
             * mark current thread as involved (needs xa_end()!) 
             * actual join to the transaction
             */
            NDRX_LOG(log_debug, "Dynamic reg work started");
            p_xai->tranid_flags|=XA_TXINFO_AXREG_CLD;
        }
    }
    
    if (!(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOJOIN) &&
            atmi_xa_is_current_rm_known(p_xai->tmknownrms))
    {    
        *p_is_known=EXTRUE;
        
        /* use default btid as in join mode */
        G_atmi_tls->G_atmi_xa_curtx.txinfo->btid = 0;
        
        /* in case if doing resume, then allow the xa_start entry */
        if (XA_IS_DYNAMIC_REG && !(join_flag & TMRESUME))
        {
            NDRX_LOG(log_debug, "Dynamic reg - no start/join!");
        }
        /* Continue with static ...  ok it is known, then just join the transaction */
        else if (EXSUCCEED!=atmi_xa_start_entry(atmi_xa_get_branch_xid(p_xai, 
                G_atmi_tls->G_atmi_xa_curtx.txinfo->btid), 
                join_flag, EXFALSE))
        {
            NDRX_LOG(log_error, "Failed to join transaction!");
	    ndrx_xa_join_fail(NULL, EXFALSE);
            EXFAIL_OUT(ret);
        }
        else
        {
            NDRX_LOG(log_debug, "tx join ok!");
        }
    }
    else
    {
        long btid = EXFAIL;
        NDRX_LOG(log_info, "RM not aware of this transaction");
        
        /* - if have JOIN, then we use default BTID 0
         * - if no JOIN, then ATMI_XA_TMREGISTER will give new TID
         */
        if (!(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOJOIN))
        {
            btid = 0;
        }
        
        if (G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOSTARTXID)
        {
            tmflags|=TMFLAGS_TPNOSTARTXID;
        }

        
        /* register new tx branch/rm */
        if (NULL==(p_ub=atmi_xa_call_tm_generic(ATMI_XA_TMREGISTER, 
                EXFALSE, EXFAIL, p_xai, tmflags, btid)))
        {
            NDRX_LOG(log_error, "Failed to execute TM command [%c]", 
                        ATMI_XA_TMREGISTER);   
            EXFAIL_OUT(ret);
        }

        if (EXSUCCEED!=Bget(p_ub, TMTXFLAGS, 0, (char *)&tmflags, 0L))
        {
            NDRX_LOG(log_error, "Failed to read TMTXFLAGS!");   

            EXFAIL_OUT(ret);
        }
        
        if (tmflags & TMFLAGS_RMIDKNOWN)
        {
            *p_is_known = EXTRUE;
        }
        else
        {
            if (EXSUCCEED!=Bget(p_ub, TMTXBTID, 0, (char *)&btid, 0L))
            {
                NDRX_LOG(log_error, "Failed to read TMTXBTID!");   

                EXFAIL_OUT(ret);
            }
        }
        
        G_atmi_tls->G_atmi_xa_curtx.txinfo->btid = btid;

        if (XA_IS_DYNAMIC_REG)
        {
            /* really? how RM knows about our XID? */
            NDRX_LOG(log_debug, "Dynamic reg - no new tx start!");
        }
        /* Continue with static... */
        else if (*p_is_known)
        {
            if (EXSUCCEED!=atmi_xa_start_entry(atmi_xa_get_branch_xid(p_xai, btid), 
                    join_flag, EXFALSE))
            {
                NDRX_LOG(log_error, "Failed to join transaction!");
		ndrx_xa_join_fail(NULL, EXFALSE);
                EXFAIL_OUT(ret);
            }
            else
            {
                NDRX_LOG(log_debug, "tx join ok!");
            }
        }
        /* Open new transaction in branch */
        else if (EXSUCCEED!=atmi_xa_start_entry(atmi_xa_get_branch_xid(p_xai, btid), 
                TMNOFLAGS, EXTRUE)) /* silent attempt...*/
        {
            reason=atmi_xa_get_reason();
            NDRX_LOG(log_error, "Failed to create new tx under local RM (reason: %hd): %s!", 
                    reason, atmi_xa_geterrstr(reason));
            if (XAER_DUPID == (reason=atmi_xa_get_reason()))
            {
                /* It is already known... then join... */
                *p_is_known=EXTRUE;
                
                if (EXSUCCEED!=atmi_xa_start_entry(atmi_xa_get_branch_xid(p_xai, btid), 
                        join_flag, EXFALSE))
                {
                    NDRX_LOG(log_error, "Failed to join transaction!");
		    ndrx_xa_join_fail(NULL, EXFALSE);
                    EXFAIL_OUT(ret);
                }
                else
                {
                    NDRX_LOG(log_debug, "tx join ok!");
                }
            }
            else
            {
                NDRX_LOG(log_error, "Failed to start transaction!");
                ndrx_xa_join_fail(NULL, EXFALSE);
                EXFAIL_OUT(ret);
            }
        }
        new_rm = EXTRUE;
    }
        
    
    if (!(G_atmi_env.xa_flags_sys & NDRX_XA_FLAG_SYS_NOJOIN) && new_rm)
    {
        src_tmknownrms[0] = G_atmi_env.xa_rmid;
        src_tmknownrms[1] = EXEOS;
        
        if (EXSUCCEED!=atmi_xa_update_known_rms(
                G_atmi_tls->G_atmi_xa_curtx.txinfo->tmknownrms, 
                src_tmknownrms))
        {
            EXFAIL_OUT(ret);
        }
    }
    
out:

    if (EXSUCCEED!=ret)
    {
        /* Remove current, if was set... */
        atmi_xa_reset_curtx();
    }

    if (NULL!=p_ub)
    {
        tpfree((char *)p_ub);
    }

    return ret;
}

/**
 * Disassociate current process from transaction 
 * TODO: What about CD's?
 * @param force_rollback shall we rollback the transaction?
 * @param end_fail did the end failed?
 * @return EXSUCCEED/EXFAIL
 */
expublic int _tp_srv_disassoc_tx(int force_rollback, int *end_fail)
{
    int ret = EXSUCCEED;
    ATMI_TLS_ENTRY;
    
    
    NDRX_LOG(log_debug, "into %s() force_rollback=%d", __func__, force_rollback);
    if (NULL==G_atmi_tls->G_atmi_xa_curtx.txinfo)
    {
        NDRX_LOG(log_warn, "Not in global tx!");
        goto out;
    }
    
    /* Only for static...  or if work done */
    if ( !XA_IS_DYNAMIC_REG || 
            (XA_TXINFO_AXREG_CLD & G_atmi_tls->G_atmi_xa_curtx.txinfo->tranid_flags))
    {
        if (EXSUCCEED!= (ret=atmi_xa_end_entry(
                atmi_xa_get_branch_xid(G_atmi_tls->G_atmi_xa_curtx.txinfo,
                G_atmi_tls->G_atmi_xa_curtx.txinfo->btid), TMSUCCESS, EXFALSE)))
        {
            NDRX_LOG(log_error, "Failed to end XA api: %d [%s]", 
                    ret, atmi_xa_geterrstr(ret));
            userlog("Failed to end XA api: %d [%s]", 
                    ret, atmi_xa_geterrstr(ret));
            
            *end_fail = EXTRUE;
        }
    }
    
    /* rollback the transaction if required */
    if (force_rollback)
    {
        ndrx_xa_join_fail(NULL, EXTRUE);
    }

    /* Remove current transaction from list */
    atmi_xa_curtx_del(G_atmi_tls->G_atmi_xa_curtx.txinfo);    
    G_atmi_tls->G_atmi_xa_curtx.txinfo = NULL;
    
out:
    return ret;
}

/**
 * Tell master TM that current transaction have been failed.
 * (so that TM can mark transaction as abort only)
 * @return SUCCEED/FAIL
 */
expublic int _tp_srv_tell_tx_fail(void)
{
    int ret = EXSUCCEED;
    
    /* TODO: */
    
out:
    return ret;
}

/**
 * Disable suspend at context switching
 * @param val EXTRUE/EXFALSE
 */
expublic void ndrx_xa_noapisusp(int val)
{
    if (val)
    {
        NDRX_LOG(log_debug, "No Context tran suspend");
        G_atmi_env.xa_flags_sys|=NDRX_XA_FLAG_SYS_NOAPISUSP;
    }
    else
    {
        G_atmi_env.xa_flags_sys=G_atmi_env.xa_flags_sys & ~NDRX_XA_FLAG_SYS_NOAPISUSP;
    }
}

/**
 * XA Driver does not support join call
 * Such us Mysql/Posgresql
 * @param val EXTRUE/EXFALSE
 */
expublic void ndrx_xa_nojoin(int val)
{
    if (val)
    {
        NDRX_LOG(log_debug, "XA No JOIN");
        G_atmi_env.xa_flags_sys|=NDRX_XA_FLAG_SYS_NOJOIN;
    }
    else
    {
        G_atmi_env.xa_flags_sys=G_atmi_env.xa_flags_sys & ~NDRX_XA_FLAG_SYS_NOJOIN;
    }
}

/**
 * Do not suspend transaction
 * This is basically optimizatoin to avoid overhead for resources which does not require
 * suspend
 * @param val EXTRUE/EXFALSE
 */
expublic void ndrx_xa_nosuspend(int val)
{
    if (val)
    {
        NDRX_LOG(log_debug, "XA No Automatic suspend");
        G_atmi_env.xa_flags_sys|=NDRX_XA_FLAG_SYS_NOSUSPEND;
    }
    else
    {
        G_atmi_env.xa_flags_sys=G_atmi_env.xa_flags_sys & ~NDRX_XA_FLAG_SYS_NOSUSPEND;
    }
}

/**
 * XA Driver does not mark the transaction at start, thus no start XID
 * Thus in this case before process calls "end", the prepare statement will be
 * issued. Also when we request the new BTID, we shall report, that branch
 * is in prepared state...
 * @param val EXTRUE/EXFALSE
 */
expublic void ndrx_xa_nostartxid(int val)
{
    if (val)
    {
        NDRX_LOG(log_debug, "XA No STAR XID");
        G_atmi_env.xa_flags_sys|=NDRX_XA_FLAG_SYS_NOSTARTXID;
    }
    else
    {
        G_atmi_env.xa_flags_sys=G_atmi_env.xa_flags_sys & ~NDRX_XA_FLAG_SYS_NOSTARTXID;
    }
}

/**
 * Set local abort function. Recommended for NOSTARTXID to avoid prepare before
 * abort. Set by XA Driver.
 * @param pf_xa_loctxabort NULL or ptr to local transaction abort function.
 */
expublic void ndrx_xa_setloctxabort(int (*pf_xa_loctxabort)(XID *xid, long flags))
{
    G_atmi_env.pf_xa_loctxabort = pf_xa_loctxabort;
    NDRX_LOG(log_debug, "xa_loctxabort set to %p", G_atmi_env.pf_xa_loctxabort);
}


/**
 * Set function for returning current connection object.
 * This might be used for custom XA Switches
 * @param pf_xa_getconn Callback for requesting the connection object
 */
expublic void ndrx_xa_setgetconnn(void *(*pf_xa_getconn)(void))
{
    G_atmi_env.pf_getconn= pf_xa_getconn;
    NDRX_LOG(log_debug, "pf_getconn set to %p", G_atmi_env.pf_getconn);
}

/**
 * Set tight branching flag
 * @param val EXTRUE/EXFALSE
 */
expublic void ndrx_xa_btight(int val)
{
    if (val)
    {
        NDRX_LOG(log_debug, "XA BTIGHT");
        G_atmi_env.xa_flags_sys|=NDRX_XA_FLAG_SYS_BTIGHT;
    }
    else
    {
        G_atmi_env.xa_flags_sys=G_atmi_env.xa_flags_sys & ~NDRX_XA_FLAG_SYS_BTIGHT;
    }
}


/* vim: set ts=4 sw=4 et smartindent: */
