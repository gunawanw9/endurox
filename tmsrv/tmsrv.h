/**
 * @brief Transaction Monitor for XA
 *
 * @file tmsrv.h
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

#ifndef TMSRV_H
#define	TMSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

/*---------------------------Includes-----------------------------------*/
#include <xa_cmn.h>
#include <exthpool.h>
/*---------------------------Externs------------------------------------*/
extern pthread_t G_bacground_thread;
extern int G_bacground_req_shutdown;    /* Is shutdown request? */
/*---------------------------Macros-------------------------------------*/
#define SCAN_TIME_DFLT          10  /* Every 10 sec try to complete TXs    */
#define MAX_TRIES_DFTL          100 /* Try count for transaction completion */
#define TOUT_CHECK_TIME         1   /* Check for transaction timeout, sec  */
#define THREADPOOL_DFLT         10  /* Default number of threads spawned   */

#define XA_RETRIES_DFLT         3   /* number of foreground retries */
#define TMSRV_HOUSEKEEP_DEFAULT   (90*60)     /**< houskeep 1 hour 30 min  */

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/

/*
 * TM ndrx_config.handler
 */
typedef struct
{
    long dflt_timeout; /**, how long monitored transaction can be open        */
    char tlog_dir[PATH_MAX]; /* Where to write tx log files                 */
    int scan_time;      /**< Number of seconds retries */
    long max_tries;      /**< Number of tries for running session for single 
                         * transaction, until stop processing it 
                         * (in this process session) */
    int tout_check_time; /**< seconds used for detecting transaction timeout   */
    int threadpoolsize; /**< thread pool size */
    /** Number of foreground retries in stage for XA_RETRY */
    int xa_retries;
    
    int ping_time; /**< Number of seconds for interval of doing "pings" to db */
    int ping_mode_jointran; /**< PING with join non existent transaction */
    threadpool thpool;
    
    int housekeeptime;        /**< Number of seconds for corrupted log cleanup*/
    
} tmsrv_cfg_t;

struct thread_server
{
    char *context_data; /* malloced by enduro/x */
    int cd;
    char *buffer; /* buffer data, managed by enduro/x */
};
/* note we must malloc this struct too. */
typedef struct thread_server thread_server_t;

/*---------------------------Prototypes---------------------------------*/
/* init */
extern void tm_thread_init(void);
extern void tm_thread_uninit(void);
extern void tm_thread_shutdown(void *ptr, int *p_finish_off);

extern void tm_ping_db(void *ptr, int *p_finish_off);

extern tmsrv_cfg_t G_tmsrv_cfg;

extern void atmi_xa_new_xid(XID *xid);

extern int tms_unlock_entry(atmi_xa_log_t *p_tl);
extern atmi_xa_log_t * tms_log_get_entry(char *tmxid, int dowait, int *is_tout);
extern int tms_log_start(atmi_xa_tx_info_t *xai, int txtout, long tmflags, long *btid);
extern int tms_log_addrm(atmi_xa_tx_info_t *xai, short rmid, int *p_is_already_logged, 
        long *btid, long flags);
extern int tms_log_chrmstat(atmi_xa_tx_info_t *xai, short rmid, 
        long btid, char rmstatus, UBFH *p_ub);
extern int tms_open_logfile(atmi_xa_log_t *p_tl, char *mode);
extern int tms_is_logfile_open(atmi_xa_log_t *p_tl);
extern void tms_close_logfile(atmi_xa_log_t *p_tl);
extern void tms_remove_logfree(atmi_xa_log_t *p_tl, int hash_rm);
extern void tms_remove_logfile(atmi_xa_log_t *p_tl, int hash_rm);
extern int tms_log_info(atmi_xa_log_t *p_tl);
extern int tms_log_stage(atmi_xa_log_t *p_tl, short stage, int forced);
extern int tms_log_rmstatus(atmi_xa_log_t *p_tl, atmi_xa_rm_status_btid_t *bt, 
        char rmstatus, int rmerrorcode, short rmreason);
extern int tms_load_logfile(char *logfile, char *tmxid, atmi_xa_log_t **pp_tl);
extern int tm_chk_tx_status(atmi_xa_log_t *p_tl);
extern atmi_xa_log_list_t* tms_copy_hash2list(int copy_mode);
extern void tms_tx_hash_lock(void);
extern void tms_tx_hash_unlock(void);
extern int tms_log_cpy_info_to_fb(UBFH *p_ub, atmi_xa_log_t *p_tl, int inc_rm_stat);
        
extern int tm_drive(atmi_xa_tx_info_t *p_xai, atmi_xa_log_t *p_tl, int master_op,
                        short rmid, long flags);

/* Prepare API */
extern int tm_prepare_local(UBFH *p_ub, atmi_xa_tx_info_t *p_xai, long btid);
extern int tm_prepare_remote_call(atmi_xa_tx_info_t *p_xai, short rmid, long btid);
extern int tm_prepare_combined(atmi_xa_tx_info_t *p_xai, short rmid, long btid);

/* Rollback API */
extern int tm_rollback_local(UBFH *p_ub, atmi_xa_tx_info_t *p_xai, long btid);
extern int tm_rollback_remote_call(atmi_xa_tx_info_t *p_xai, short rmid, long btid);
extern int tm_rollback_combined(atmi_xa_tx_info_t *p_xai, short rmid, long btid);

/* Forget API */
extern int tm_forget_local(UBFH *p_ub, atmi_xa_tx_info_t *p_xai, long btid);
extern int tm_forget_remote_call(atmi_xa_tx_info_t *p_xai, short rmid, long btid);
extern int tm_forget_combined(atmi_xa_tx_info_t *p_xai, short rmid, long btid);

/* Commit API */
extern int tm_commit_local(UBFH *p_ub, atmi_xa_tx_info_t *p_xai, long btid);
extern int tm_commit_remote_call(atmi_xa_tx_info_t *p_xai, short rmid, long btid);
extern int tm_commit_combined(atmi_xa_tx_info_t *p_xai, short rmid, long btid);

extern int tm_tpbegin(UBFH *p_ub);
extern int tm_tpcommit(UBFH *p_ub);
extern int tm_tpabort(UBFH *p_ub);

extern int tm_tmprepare(UBFH *p_ub);
extern int tm_tmcommit(UBFH *p_ub);
extern int tm_tmabort(UBFH *p_ub);
extern int tm_tmforget(UBFH *p_ub);
extern int tm_tmregister(UBFH *p_ub);
extern int tm_rmstatus(UBFH *p_ub);

/* Background API */
extern int background_read_log(void);
extern void background_wakeup(void);
extern int background_process_init(void);
extern void background_lock(void);
extern void background_unlock(void);

/* Admin functions */
extern int tm_tpprinttrans(UBFH *p_ub, int cd);
extern int tm_aborttrans(UBFH *p_ub);
extern int tm_status(UBFH *p_ub);
extern int tm_committrans(UBFH *p_ub);
extern int tm_recoverlocal(UBFH *p_ub, int cd);
extern int tm_proclocal(char cmd, UBFH *p_ub, int cd);

/* Branch TID manipulations */
extern long tms_btid_gettid(atmi_xa_log_t *p_tl, short rmid);


extern atmi_xa_rm_status_btid_t *tms_btid_find(atmi_xa_log_t *p_tl, 
        short rmid, long btid);
extern int tms_btid_add(atmi_xa_log_t *p_tl, short rmid, 
            long btid, char rmstatus, int  rmerrorcode, short rmreason,
            atmi_xa_rm_status_btid_t **bt);
extern int tms_btid_addupd(atmi_xa_log_t *p_tl, short rmid, 
            long *btid, char rmstatus, int  rmerrorcode, short rmreason, int *exists,
            atmi_xa_rm_status_btid_t **bt);


#ifdef	__cplusplus
}
#endif

#endif	/* TMSRV_H */

/* vim: set ts=4 sw=4 et smartindent: */
