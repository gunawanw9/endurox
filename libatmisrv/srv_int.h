/**
 * @brief Internal servers header.
 *
 * @file srv_int.h
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

#ifndef SRV_INT_H
#define	SRV_INT_H

#ifdef	__cplusplus
extern NDRX_API "C" {
#endif
/*---------------------------Includes-----------------------------------*/
#include <ndrx_config.h>
#include <sys_mqueue.h>
#include <atmi.h>
#include <setjmp.h>
#include <ndrxdcmn.h>
#include <exhash.h>
#include <sys_unix.h>
#include <atmi.h>
#include <atmi_int.h>
#include <exthpool.h>
#include <thlock.h>
/*---------------------------Externs------------------------------------*/
extern NDRX_API long G_libatmisrv_flags; /**< present in integra.c or standard.c */
extern NDRX_API int G_atmisrv_reply_type; /**< ATMI server return value (no long jump) */
extern NDRX_API int G_shutdown_req; /**< is shutdown requested? */
/* system call for server init */
extern int (*ndrx_G_tpsvrinit_sys)(int, char **);

/*---------------------------Macros-------------------------------------*/
#define MIN_SVC_LIST_LEN        30
#define SVN_LIST_REALLOC        15

#define RETURN_FAILED             0x00000001
#define RETURN_TYPE_TPRETURN      0x00000002
#define RETURN_TYPE_TPFORWARD     0x00000004
#define RETURN_SVC_FAIL           0x00000008
#define RETURN_TYPE_THREAD        0x00000010  /**< processing sent to thread   */

/* with linux 4.5 this is supported */ 
#ifndef EPOLLEXCLUSIVE

#define EPOLLEXCLUSIVE (1 << 28)

#endif

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/

   
/**
 * Service name/alias entry.
 */
typedef struct svc_entry svc_entry_t;
struct svc_entry
{
    char svc_nm[XATMI_SERVICE_NAME_LENGTH+1];
    char svc_aliasof[XATMI_SERVICE_NAME_LENGTH+1];
    svc_entry_t *next, *prev;
};

/**
 * Hash of buffer conversion functions.
 */
typedef struct xbufcvt_entry xbufcvt_entry_t;
struct xbufcvt_entry
{
    char fn_nm[XATMI_SERVICE_NAME_LENGTH+1]; /* function name */
    long xcvtflags; /* Conversion function */
    EX_hash_handle hh;         /* makes this structure hashable */
};

/**
 * Skip service from advertse Feature #275
 */
typedef struct ndrx_svchash ndrx_svchash_t;
struct ndrx_svchash
{
    char svc_nm[XATMI_SERVICE_NAME_LENGTH+1]; /* function name */
    EX_hash_handle hh;         /* makes this structure hashable */
};

/**
 * Service entry descriptor.
 */
typedef struct svc_entry_fn svc_entry_fn_t;
struct svc_entry_fn
{
    char svc_nm[XATMI_SERVICE_NAME_LENGTH+1]; /* service name */
    char fn_nm[XATMI_SERVICE_NAME_LENGTH+1]; /* function name */
    void (*p_func)(TPSVCINFO *);
    /* listing support */
    svc_entry_fn_t *next, *prev;
    char listen_q[FILENAME_MAX+1]; /* queue on which to listen */
    int is_admin;
    mqd_t q_descr; /* queue descriptor */
    ndrx_stopwatch_t qopen_time;
    long xcvtflags; /* Conversion function */
    
    /* have flags for transaction -> authtran & timeout */
    int autotran;       /**< shall we start transaction upport receving msg?  */
    unsigned long trantime; /**< transaction timeout if doing autotran        */
};

/*
 * Basic call info
 */
struct basic_call_info
{
    char *buf_ptr;
    long len;
    int no;
};
typedef struct basic_call_info call_basic_info_t;

/**
 * Basic server configuration.
 */
struct srv_conf
{
    char binary_name[MAXTIDENT+1];
    int srv_id;
    char err_output[FILENAME_MAX+1];
    int log_work;
    int advertise_all;
    int no_built_advertise; /**< Do not advertise services provided by buildserver */
    svc_entry_t *svc_list;
    svc_entry_t *funcsvc_list;  /**< Function mappings to services -S */
    char q_prefix[FILENAME_MAX+1];
    
    int app_argc;/**< Arguments passed after -- */
    char **app_argv;
    
    /*<THESE LISTS ARE USED ONLY TILL SERVER GOES ONLINE, STATIC INIT>      */
    svc_entry_fn_t *service_raw_list; /**< As from initialization           */
    int service_raw_list_count; /**< Number of services in raw list         */
    svc_entry_fn_t **service_array; /**< Direct array of items              */
    /*</THESE LISTS ARE USED ONLY TILL SERVER GOES ONLINE, STATIC INIT>     */
    
    svc_entry_fn_t *service_list; /**< Final list used for processing */
    
    int adv_service_count; /**< advertised service count. */
    int flags; /**< Special flags of the server (see: ndrxdcmn.h:SRV_KEY_FLAGS_BRIDGE) */
    int nodeid; /**< Other node id of the bridge */
    int (*p_qmsg)(char **buf, int len, char msg_type); /**< Q message processor for bridge */
    /**************** POLLING *****************/
    struct ndrx_epoll_event *events;
    int epollfd;
    int time_out;
    int max_events; /**< Max epoll events. */
    
    int (*p_periodcb)(void);/**< Periodic callback */
    int periodcb_sec; /**< Number of seconds for each cb call */
    
    int (*p_shutdowncb)(int *shutdown_req);/**< Redirect shutdown request to callback
                               * for advanced shutdown sequences such as tmq */
    
    /** Callback used before server goes in poll state */
    int (*p_b4pollcb)(void);
    xbufcvt_entry_t *xbufcvt_tab; /**< string hashlist for buffer convert funcs */
    
    char rqaddress[NDRX_MAX_Q_SIZE+1]; /**< request address if used... (sysv) */
    
    int is_threaded;            /**< is multi-threaded server       */
    int mindispatchthreads;     /**< minimum dispatch threads       */
    int maxdispatchthreads;     /**< maximum dispatch trheads       */
  
    threadpool dispthreads;     /**< thread pool for dispatch threads*/
    NDRX_SPIN_LOCKDECL (mt_lock);   /**< mt lock for data sync        */
    
    int ddr_keep_grp;           /**< shall we keep DDR group name in svcnm? */
};

typedef struct srv_conf srv_conf_t;


/**
 * Server multi threading, context switching
 */
struct server_ctx_info
{
    tp_conversation_control_t G_accepted_connection;
    tp_command_call_t         G_last_call;
    int                       is_in_global_tx;  /* Running in global tx      */
    TPTRANID                  tranid;           /* Transaction ID  (if used) */
};
typedef struct server_ctx_info server_ctx_info_t;

/**
 * Defer server tpacall
 */
typedef struct ndrx_tpacall_defer ndrx_tpacall_defer_t;
struct ndrx_tpacall_defer
{   
    /** service to call */
    char svcnm[MAXTIDENT+1];
    
    /** data may be null if sending NULL buffer */
    char *data;
    
    /** data len */
    long len;
    
    /** call flags */
    long flags;
    
    ndrx_tpacall_defer_t *next;
    ndrx_tpacall_defer_t *prev;
};

/*---------------------------Globals------------------------------------*/
extern NDRX_API srv_conf_t G_server_conf;
extern NDRX_API shm_srvinfo_t *G_shm_srv;
extern NDRX_API pollextension_rec_t *ndrx_G_pollext;
extern NDRX_API ndrx_svchash_t *ndrx_G_svchash_skip;
extern NDRX_API ndrx_svchash_t *ndrx_G_svchash_funcs;
extern NDRX_API int G_shutdown_req;
extern NDRX_API int G_shutdown_nr_wait;   /* Number of self shutdown messages to wait */
extern NDRX_API int G_shutdown_nr_got;    /* Number of self shutdown messages got  */
extern NDRX_API void (*___G_test_delayed_startup)(void);

extern NDRX_API int (*G_tpsvrinit__)(int, char **);
extern NDRX_API int (*ndrx_G_tpsvrinit_sys)(int, char **);
extern NDRX_API void (*G_tpsvrdone__)(void);
extern NDRX_API int (*ndrx_G_tpsvrthrinit)(int, char **);
extern NDRX_API void (*ndrx_G_tpsvrthrdone)(void);

/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/
extern NDRX_API int sv_open_queue(void);
extern NDRX_API int sv_wait_for_request(void);
extern NDRX_API int unadvertse_to_ndrxd(char *srvcnm);

/* Server specific functions: */
extern NDRX_API void _tpreturn (int rval, long rcode, char *data, long len, long flags);
extern NDRX_API void _tpforward (char *svc, char *data,
                long len, long flags);
extern NDRX_API void _tpcontinue (void);

extern NDRX_API int ndrx_sv_set_autojoin(int new_flag);
extern NDRX_API int ndrx_sv_latejoin(void);

/* ndrd api */
extern NDRX_API int advertse_to_ndrxd(svc_entry_fn_t *entry);
extern NDRX_API int advertse_to_ndrxd(svc_entry_fn_t *entry);
extern NDRX_API int report_to_ndrxd(void);
extern NDRX_API void ndrx_set_report_to_ndrxd_cb(int (*report_to_ndrxd_callback) (void));
/* Return list of connected bridge nodes. */
extern NDRX_API int ndrxd_get_bridges(char *nodes_out);
extern NDRX_API int pingrsp_to_ndrxd(command_srvping_t *ping);
    
/* Advertise & unadvertise */
extern NDRX_API int dynamic_unadvertise(char *svcname, int *found, svc_entry_fn_t *copy);
extern NDRX_API int	dynamic_advertise(svc_entry_fn_t *entry_new, 
                    char *svc_nm, void (*p_func)(TPSVCINFO *), char *fn_nm);
/* We want to re-advertise the stuff, this could be used for race conditions! */
extern NDRX_API int dynamic_readvertise(char *svcname);

/* Polling extension */
extern NDRX_API void ndrx_ext_pollsync(int flag);
extern NDRX_API pollextension_rec_t * ext_find_poller(int fd);
extern NDRX_API int _tpext_addpollerfd(int fd, uint32_t events, 
        void *ptr1, int (*p_pollevent)(int fd, uint32_t events, void *ptr1));
extern NDRX_API int _tpext_delpollerfd(int fd);
extern NDRX_API int _tpext_addperiodcb(int secs, int (*p_periodcb)(void));
extern NDRX_API int _tpext_delperiodcb(void);
extern NDRX_API int _tpext_addb4pollcb(int (*p_b4pollcb)(void));
extern NDRX_API int _tpext_delb4pollcb(void);
extern NDRX_API int process_admin_req(char **buf, long len, 
        int *shutdown_req);

/* auto buffer convert: */
extern NDRX_API long ndrx_xcvt_lookup(char *fn_nm);

extern NDRX_API int atmisrv_build_advertise_list(void);
extern NDRX_API int atmisrv_initialise_atmi_library(void);
extern NDRX_API void atmisrv_un_initialize(int fork_uninit);

extern NDRX_API int atmisrv_array_remove_element(void *arr, int elem, int len, int sz);

extern NDRX_API int ndrx_svchash_add(ndrx_svchash_t **hash, char *svc_nm);
extern NDRX_API int ndrx_svchash_chk(ndrx_svchash_t **hash, char *svc_nm);
extern NDRX_API void ndrx_svchash_cleanup(ndrx_svchash_t **hash);

extern NDRX_API int ndrx_svc_entry_fn_cmp(svc_entry_fn_t *a, svc_entry_fn_t *b);
extern NDRX_API void ndrx_sv_advertise_lock();
extern NDRX_API void ndrx_sv_advertise_unlock();
extern NDRX_API void ndrx_sv_do_shutdown(char *requester, int *shutdown_req);
extern NDRX_API int ndrx_tpext_addbshutdowncb(int (*p_shutdowncb)(int *shutdown_req));
#ifdef	__cplusplus
}
#endif

#endif	/* SRV_INT_H */

/* vim: set ts=4 sw=4 et smartindent: */
