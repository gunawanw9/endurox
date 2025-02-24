/**
 * @brief Common/shared data structures between server & client.
 *
 * @file ndrxdcmn.h
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
#ifndef NDRXDCMN_H
#define	NDRXDCMN_H

#ifdef	__cplusplus
extern "C" {
#endif

/*---------------------------Includes-----------------------------------*/
#include <atmi_int.h>
#include <sys/param.h>
#include <nstopwatch.h>
#include <atmi_int.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/

/* Unsigned long */
#define NDRX_MAGIC                              0x62327700L

#define CMD_ARG_MAX                             2048
#define RPLY_ERR_MSG_MAX                        256

/* Client/server commands */
#define NDRXD_COM_MIN               0
#define NDRXD_COM_LDCF_RQ           0    /**< load config req        */
#define NDRXD_COM_LDCF_RP           1    /**< load config replay     */
#define NDRXD_COM_START_RQ          2    /**< start app domain req   */
#define NDRXD_COM_START_RP          3    /**< start app domain reply */
#define NDRXD_COM_SVCINFO_RQ        4    /**< service info from svc  */
#define NDRXD_COM_SVCINFO_RP        5    /**< not used               */
#define NDRXD_COM_PMNTIFY_RQ        6    /**< Process notification   */
#define NDRXD_COM_PMNTIFY_RP        7    /**< not used               */
#define NDRXD_COM_PSC_RQ            8    /**< Print services req     */
#define NDRXD_COM_PSC_RP            9    /**< Print services rsp     */
#define NDRXD_COM_STOP_RQ           10   /**< stop app domain req    */
#define NDRXD_COM_STOP_RP           11   /**< stop app domain reply  */
#define NDRXD_COM_SRVSTOP_RQ        12   /**< server stop req        */
#define NDRXD_COM_SRVSTOP_RP        13   /**< server stop reply      */
#define NDRXD_COM_AT_RQ             14   /**< attach to server req   */
#define NDRXD_COM_AT_RP             15   /**< attach to server reply */
#define NDRXD_COM_RELOAD_RQ         16   /**< reload config req      */
#define NDRXD_COM_RELOAD_RP         17   /**< reload config reply    */
#define NDRXD_COM_TESTCFG_RQ        18   /**< test config req        */
#define NDRXD_COM_TESTCFG_RP        19   /**< test config reply      */
#define NDRXD_COM_SRVINFO_RQ        20   /**< Server info request    */
#define NDRXD_COM_SRVINFO_RP        21   /**< Server info req/rsp    */
    
/*
 * Un Advertise from xadmin console:
 * 
 * xadmin--24-->ndrxd--26-->server
 *              A              |
 *              |              |
 *              +------22------+
 */
/* Server performs un-advertise operation, informs server about this thing      */
#define NDRXD_COM_SRVUNADV_RQ       22   /**< Server unadvertise request        */
#define NDRXD_COM_SRVUNADV_RP       23   /**< Server unadvertise response       */
/* xadmin requests unadvertise */
#define NDRXD_COM_XADUNADV_RQ       24   /**< xadmin request for unadvertise, req */
#define NDRXD_COM_XADUNADV_RP       25   /**< xadmin request for unadvertise, resp*/
/* ndrxd forwards xadmin request to server */
#define NDRXD_COM_NXDUNADV_RQ       26   /**< ndrxd request for unadvertise, req*/
#define NDRXD_COM_NXDUNADV_RP       27   /**< ndrxd request for unadvertise, resp*/

#define NDRXD_COM_SRVADV_RQ         28   /**< server requests advertise, req    */
#define NDRXD_COM_SRVADV_RP         29   /**< server reqeusts un-advertise, resp*/
    
#define NDRXD_COM_XAPPM_RQ          30   /**< print process model, req          */
#define NDRXD_COM_XAPPM_RP          31   /**< print process model, resp         */
    
#define NDRXD_COM_XASHM_PSVC_RQ     32   /**< print SHM services, req           */
#define NDRXD_COM_XASHM_PSVC_RP     33   /**< print SHM services, resp          */
    
#define NDRXD_COM_XASHM_PSRV_RQ     34   /**< print SHM servers, req            */
#define NDRXD_COM_XASHM_PSRV_RP     35   /**< print SHM servers, resp           */

#define NDRXD_COM_NXDREADV_RQ       36   /**< ndrxd request for readvertise, req */
#define NDRXD_COM_NXDREADV_RP       37   /**< ndrxd request for readvertise, resp*/
    
#define NDRXD_COM_XADREADV_RQ       38   /**< xadmin request for readvertise, req*/
#define NDRXD_COM_XADREADV_RP       39   /**< xadmin request for readvertise, resp*/

#define NDRXD_COM_XACABORT_RQ        40  /**< xadmin request for abort, req     */
#define NDRXD_COM_XAABORT_RP        41   /**< xadmin request for abort, resp    */

#define NDRXD_COM_BRCON_RQ          42   /**< bridge, connected, req            */
#define NDRXD_COM_BRCON_RP          43   /**< bridge, connected, resp           */
    
#define NDRXD_COM_BRDISCON_RQ       44   /**< bridge, disconnected, req         */
#define NDRXD_COM_BRDISCON_RP       45   /**< bridge, disconnected, resp        */

#define NDRXD_COM_BRREFERSH_RQ      46   /**< bridge, refersh, req              */
#define NDRXD_COM_BRREFERSH_RP      47   /**< bridge, refersh, resp             */
    
#define NDRXD_COM_BRCLOCK_RQ        48   /**< bridge, monotonic clock exchange, req*/
#define NDRXD_COM_BRCLOCK_RP        49   /**< bridge, monotonic clock exchange, rsp*/

#define NDRXD_COM_SRVGETBRS_RQ      50   /**< Get bridges, request from server  */
#define NDRXD_COM_SRVGETBRS_RP      51   /**< Get bridges, response from ndrxd  */

#define NDRXD_COM_SRVPING_RQ        52   /**< Server ping request               */
#define NDRXD_COM_SRVPING_RP        53   /**< Server ping response              */

#define NDRXD_COM_SRELOAD_RQ        54   /**< xadmin request for server reload, req*/
#define NDRXD_COM_SRELOAD_RP        55   /**< xadmin request for server reload, resp*/

#define NDRXD_COM_XAPQ_RQ           56   /**< xadmin print service queue, req   */
#define NDRXD_COM_XAPQ_RP           57   /**< xadmin print service queue, resp  */
    
#define NDRXD_COM_PE_RQ             58   /**< xadmin print env, req             */
#define NDRXD_COM_PE_RP             59   /**< xadmin print env, resp            */
    
#define NDRXD_COM_SET_RQ            60   /**< xadmin set env, req               */
#define NDRXD_COM_SET_RP            61   /**< xadmin set env, resp              */
    
#define NDRXD_COM_UNSET_RQ          62   /**< xadmin unset env, req             */
#define NDRXD_COM_UNSET_RP          63   /**< xadmin unset env, resp            */

#define NDRXD_COM_SRELOADI_RQ       64   /**< ndrxd request for server reload, req, internal  */
#define NDRXD_COM_SRELOADI_RP       65   /**< ndrxd request for server reload, resp, internal */
    
#define NDRXD_COM_APPCONFIG_RQ      66   /**< ndrxd appconfig, req, internal    */
#define NDRXD_COM_APPCONFIG_RP      67   /**< ndrxd appconfig, rsp, internal    */
    
#define NDRXD_COM_DPING_RQ          68   /**< tprecover ndrxd ping, req, int    */
#define NDRXD_COM_DPING_RP          69   /**< tprecover ndrxd ping, req, int    */
    
#define NDRXD_COM_DSLEEP_RQ         70   /**< tprecover ndrxd ping, req, int    */
#define NDRXD_COM_DSLEEP_RP         71   /**< tprecover ndrxd ping, req, int    */
    
#define NDRXD_COM_BLIST_RQ          72   /**< bridge admin queue listing, req, int */
#define NDRXD_COM_BLIST_RP          73   /**< bridge admin queue listing, rsp int  */
    
#define NDRXD_COM_BRCONINFO_RQ      74   /**< return bridge connection infos, req int */
#define NDRXD_COM_BRCONINFO_RP      75   /**< return bridge connection infos, rsp int */

#define NDRXD_COM_MAX               75
    
/** This is sqv admin thread shutdown priv */
#define NDRXD_COM_SVQADMIN_PRIV     NDRX_COM_SVQ_PRIV

/* Command contexts */
#define NDRXD_CTX_ANY               -1   /**< Any context...                    */
#define NDRXD_CTX_NOCHG             NDRXD_CTX_ANY   /**< Do not change context! */
#define NDRXD_CTX_ZERO              0    /**< Zero context, no command running  */
#define NDRXD_CTX_START             1    /**< Start command is running          */
#define NDRXD_CTX_STOP              2    /**< Stop context/command is running   */

/* Max number of services can be advertised by servers! */
#define MAX_SVC_PER_SVR             50

#define MAX_NDRXD_ERROR_LEN         2048
/* NDRXD Error sesction */
#define NDRXD_EMINVAL            0
#define NDRXD_ESRVCIDDUP         1          /**< Duplicat service ID          */
#define NDRXD_ESRVCIDINV         2          /**< Invalid service ID           */
#define NDRXD_EOS                3          /**< Operating System failure     */
#define NDRXD_ECFGLDED           4          /**< Configuration already loaded */
#define NDRXD_ECFGINVLD          5          /**< Invalid configuration file   */
#define NDRXD_EPMOD              6          /**< Process model failed         */
#define NDRXD_ESHMINIT           7          /**< Shared memory not initialized*/
#define NDRXD_NOTSTARTED         8          /**< App domain not started       */
#define NDRXD_ECMDNOTFOUND       9          /**< Command not found            */
#define NDRXD_ENONICONTEXT       10         /**< Non interractive context     */
#define NDRXD_EREBBINARYRUN      11         /**< Renamed binary in run state  */
#define NDRXD_EBINARYRUN         12         /**< Removed binary in run state  */
#define NDRXD_ECONTEXT           13         /**< Invalid command context      */
#define NDRXD_EINVPARAM          14         /**< Invalid paramters            */
#define NDRXD_EABORT             15         /**< Abort requested              */
#define NDRXD_EENVFAIL           16         /**< putenv failed                */
#define NDRXD_EINVAL             17         /**< Invalid argument             */
#define NDRXD_ENORMAL            18         /**< Normal state expected        */
#define NDRXD_ECFGDEFAULTS          19         /**< Invalid defaults             */
#define NDRXD_ECFGSERVER            20         /**< Invalid server settigns      */
#define NDRXD_ECFGAPPCONFIG         21         /**< Invalid appconfig settings   */
#define NDRXD_EACCES             22         /**< No access to file            */
#define NDRXD_ESYNTAX            23         /**< Syntax error during parsing  */
#define NDRXD_ESYSTEM             24         /**< E/X system failure            */
#define NDRXD_EMAXVAL            24

/* This section list call types */
#define NDRXD_CALL_TYPE_GENERIC         0   /**< Generic call type        */
#define NDRXD_CALL_TYPE_PM_INFO         1   /**< Process model info       */
#define NDRXD_CALL_TYPE_SVCINFO         2   /**< Service info             */
#define NDRXD_CALL_TYPE_PM_STARTING     3   /**< Process model info (starting process)*/
#define NDRXD_CALL_TYPE_PM_STARTED      4   /**< Process model info  (started)*/
#define NDRXD_CALL_TYPE_PM_STOPPING     5   /**< Process model info  (stop initiated)*/
#define NDRXD_CALL_TYPE_PM_STOPPED      6   /**< Process model info  (stopped)*/
#define NDRXD_CALL_TYPE_PM_RELERR       7   /**< Reload error                 */
#define NDRXD_CALL_TYPE_PM_PPM          8   /**< Print process model          */
#define NDRXD_CALL_TYPE_PM_SHM_PSVC     9   /**< Print services from SHM      */
#define NDRXD_CALL_TYPE_PM_SHM_PSRV     10  /**< Print servers from SHM       */
#define NDRXD_CALL_TYPE_BRIDGEINFO      11  /**< Bridge info command          */
#define NDRXD_CALL_TYPE_BRIDGESVCS      12  /**< Bridge services command      */
#define NDRXD_CALL_TYPE_BRBCLOCK        13  /**< Bridge clock info            */
#define NDRXD_CALL_TYPE_GETBRS          14  /**< Get connected bridges        */
#define NDRXD_CALL_TYPE_PQ              15  /**< Response struct for `pq' cmd */
#define NDRXD_CALL_TYPE_PE              16  /**< Response struct for `pe' cmd */
#define NDRXD_CALL_TYPE_APPCONFIG       17  /**< Response to appconfig command*/
#define NDRXD_CALL_TYPE_DPING           18  /**< NDRXD ping response type     */
#define NDRXD_CALL_TYPE_DSLEEP          19  /**< Put NDRXD in sleep mode      */
#define NDRXD_CALL_TYPE_BLIST           20  /**< List bridge admin queues     */
#define NDRXD_CALL_TYPE_BRCONINFO       21  /**< Connection info messages     */

#define NDRXD_SRC_NDRXD                 0   /**< Call source is daemon       */
#define NDRXD_SRC_ADMIN                 1   /**< Call source is admin utility*/
#define NDRXD_SRC_SERVER                2   /**< EnduroX server              */
#define NDRXD_SRC_BRIDGE                3   /**< EnduroX bridge server       */

/**
 * NDRXD flags/state:
 */
#define NDRXD_STATE_CFG_OK      0x00000001  /**< Configuration loaded       */
#define NDRXD_STATE_SHUTDOWN	0x00000002  /**< About to shutdown          */
#define NDRXD_STATE_DOMSTART	0x00000004  /**< Domain startup in progress */
#define NDRXD_STATE_DOMSTARTED  0x00000008  /**< Domain started             */
#define NDRXD_STATE_SHUTDOWNED	0x00000010  /**< Domain shutdowned          */
    
/**
 * Process state flags
 */
#define NDRXD_PM_MIN_EXIT           0       /**< Minimum dead process       */
#define NDRXD_PM_NOT_STARTED        0       /**< process not started (used for req state too)*/
#define NDRXD_PM_DIED               1       /**< process died for some reason */
#define NDRXD_PM_EXIT               2       /**< normal exit, shutdown      */
#define NDRXD_PM_ENOENT             3       /**< Binary not found           */
#define NDRXD_PM_EACCESS            4       /**< Access denied to execute   */
#define NDRXD_PM_EBADFILE           5       /**< Bad executable             */
#define NDRXD_PM_ELIMIT             6       /**< Limits exceeded            */
#define NDRXD_PM_EENV               7       /**< Failed to prepare env      */
#define NDRXD_PM_EARGSLIM           8       /**< Arguments or env too long  */
#define NDRXD_PM_ESYSTEM            9       /**< Bad executable             */
#define NDRXD_PM_MAX_EXIT           19      /**< Maximum dead process       */
    
#define NDRXD_PM_MIN_RUNNING        20      /**< Minimum running process    */
#define NDRXD_PM_STARTING           20      /**< startup in progres...      */
#define NDRXD_PM_RUNNING_OK         21      /**< process running OK (used for req state too!)*/
#define NDRXD_PM_STOPPING           22      /**< About to shutdown          */
#define NDRXD_PM_MAX_RUNNING        39      /**< Max running process        */
    
/* Macro for testing not-running process */
#define PM_NOT_RUNNING(X)       (NDRXD_PM_MIN_EXIT <= ( X ) && ( X ) <= NDRXD_PM_MAX_EXIT)
#define PM_RUNNING(X)       (NDRXD_PM_MIN_RUNNING <= ( X ) && ( X ) <= NDRXD_PM_MAX_RUNNING)
    
/**
 * Flags for shm_svcinfo_t.flags
 * Indicates service info entry state.
 */
#define NDRXD_SVCINFO_INIT              0x00000001  /**< initialized          */


#define NDRXD_SVC_STATUS_AVAIL          0       /**< Service is available     */
#define NDRXD_SVC_STATUS_BUSY           1       /**< Service is busy          */

    
#define NDRXD_CALL_FLAGS_DEADQ          0x0001  /**< Reply queue is dead....! */
#define NDRXD_CALL_FLAGS_PAGE2          0x0002  /**< Second page from call    */
#define NDRXD_CALL_FLAGS_RSPHAVE_MORE   0x0004	/**< Have more stuff to wait for */

#define SRV_KEY_FLAGS_BRIDGE            0x0001  /**< This server is bridge server                 */
#define SRV_KEY_FLAGS_SENDREFERSH       0x0002  /**< Bridge requires that we send refersh to them */
#define SRV_KEY_FLAGS_CONNECTED         0x0004  /**< Is bridge connected?                         */
    
    
/*
 *  values for bridge_refresh_svc_t.mode
 */
#define BRIDGE_REFRESH_MODE_FULL        'F' /**< Full replacement arrived.      */
#define BRIDGE_REFRESH_MODE_DIFF        'D' /**< Contains diff +count or -count */

/*
 * Values for cmd_br_net_call_t.msg_type
 */
#define BR_NET_CALL_MSG_TYPE_ATMI       'A' /**< This is ATMI call                */
#define BR_NET_CALL_MSG_TYPE_NOTIF      'N' /**< This is ATMI, notif/broadcast    */
#define BR_NET_CALL_MSG_TYPE_NDRXD      'X' /**< This is EnduroX call             */

#define BR_NET_CALL_MAGIC               0x6A12CC51L /**< Magic of the netcall  */
    
    
#define PING_MAX_SEQ                    65536    /**< Max sequence number of ping */
    
    
/* Data types describing bellow data structures */
#define EXF_MIN         0       /**< Minimum suported type */
#define EXF_SHORT	0	/**< short int */
#define EXF_LONG	1	/**< long int */
#define EXF_CHAR	2	/**< character */
#define EXF_FLOAT	3	/**< single-precision float */
#define EXF_DOUBLE	4	/**< double-precision float */
#define EXF_STRING	5	/**< string - null terminated */
#define EXF_CARRAY	6	/**< character array */
#define EXF_NONE        7       /**< Data type - none */ 
    
#define EXF_INT         8       /**< Data type - int */ 
#define EXF_ULONG       9       /**< Data type - unsigned long */ 
#define EXF_UINT        10      /**< Data type - unsigned */ 
#define EXF_NTIMER      11      /**< Data type - n_timer_t */ 
#define EXF_TIMET       12      /**< Data type - time_t */ 
#define EXF_USHORT      13      /**< Data type - unsigned short */ 
#define EXF_CARRAYFIX   14      /**< Fixed length carray, use MIN len */ 
#define EXF_MAX         14      /**< Maximum suported type */


#define PQ_LEN                  12        /**< The len of last print queue data */    
#define EX_ENV_MAX              4096      /**< max env name/value size */
    

#define NDRX_BRCLOCK_MODE_ASYNC       1   /**< Async clock data               */
#define NDRX_BRCLOCK_MODE_REQ         2   /**< Request for clock data         */
#define NDRX_BRCLOCK_MODE_RSP         3   /**< Response clock data            */

    
#define NDRX_CONMODE_ACTIVE         'A'         /**< This is client */
#define NDRX_CONMODE_PASSIVE        'P'         /**< This is server */

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/

/**
 * Shared memory entry for server ID
 */
typedef struct shm_srvinfo shm_srvinfo_t;
struct shm_srvinfo
{
    int srvid;

    unsigned svc_fail[MAX_SVC_PER_SVR];
    unsigned svc_succeed[MAX_SVC_PER_SVR];

    unsigned min_rsp_msec[MAX_SVC_PER_SVR];
    unsigned max_rsp_msec[MAX_SVC_PER_SVR];
    unsigned last_rsp_msec[MAX_SVC_PER_SVR];
    short svc_status[MAX_SVC_PER_SVR];     /**< The status of the service     */

    char last_reply_q[NDRX_MAX_Q_SIZE+1];  /**< Last queue on it should reply */
    /** See NDRXD_PM_E error codes */
    unsigned execerr;                      /**< Last exec error               */
    short status;                          /**< Global status, avail or busy  */
    short last_command_id;                 /**< Last command ID received      */
};

/**
 * Basic cluster node info
 */
typedef struct cnodeinfo cnodeinfo_t;
struct cnodeinfo
{
    int latency;        /**< Latency in ms */
    int srvs;           /**< Number of serves on this cluster node */
};


/**
 * Shared memory resource id
 */
typedef struct ndrx_shm_resid ndrx_shm_resid_t;
struct ndrx_shm_resid
{
    short cnt;                          /**< number of instances installed  */
    int resid;                          /**< Resource id                    */
};

/**
 * Shared memory entry for service
 */
typedef struct shm_svcinfo shm_svcinfo_t;
struct shm_svcinfo
{
    char service[MAXTIDENT+1];          /**< name of the service                  */
    int srvs;                           /**< Count of servers advertising this service*/
    int flags;                          /**< service flags                        */
    int csrvs;                          /**< Number of advertises in cluster      */
    int totclustered;                   /**< Total clustered nodes                */
    int cnodes_max_id;                  /**< Max id of cluster nodes in list (for fast search) */
    cnodeinfo_t cnodes[CONF_NDRX_NODEID_COUNT];    /**< List of cluster nodes */
    /**
     * Number of resources, because there could be rqaddr servers, where
     * srvs is large number, but there is only on queue.
     */
    short resnr;                          
    unsigned int resrr;                          /**< round robin server */
    
    /* THIST MUST BE LAST IN STRUCT (AS IT WILL SCALE DEPENDING ON SERVERS): */
    ndrx_shm_resid_t resids[0];         /**<  Servers id's offering this service */
};

/* Macros for shm service size */
#define SHM_SVCINFO_SIZEOF  (sizeof(shm_svcinfo_t) + sizeof(ndrx_shm_resid_t)*G_atmi_env.maxsvcsrvs)

/* memory access index: */
#define SHM_SVCINFO_INDEX(MEM, IDX) ((shm_svcinfo_t*)(((char*)MEM)+(int)(SHM_SVCINFO_SIZEOF*IDX)))

/*
 * Generic command request structure
 */
typedef struct
{
    /* <standard comms header:> */
#if defined(EX_USE_SYSVQ) || defined(EX_USE_SVAPOLL)
    long mtype; /* mandatory for System V queues */
#endif
    short command_id;
    char proto_ver[4];
    int proto_magic;
    /* </standard comms header> */
    
    unsigned long magic;                /**< Packed magic                     */
    int command;                        /**< Request command                  */
    short msg_type;                     /**< Message type                     */
    short msg_src;                      /**< Message source                   */
    char reply_queue[NDRX_MAX_Q_SIZE+1];/**< Queue (str) on which to pass back reply*/
    int flags;                          /**< Flags for command call           */
    int caller_nodeid;                  /**< Node id of the caller            */
} command_call_t;

/**
 * Special structure for stop command.
 */
typedef struct
{
    command_call_t call;

    /* some additional attribs */
    short complete_shutdown;            /**< Id of the server */
    int srvid;
    char binary_name[MAXTIDENT+1];

} command_startstop_t;


/**
 * Set/unset call
 */
typedef struct
{
    command_call_t call;
    char env[EX_ENV_MAX+1];
} command_setenv_t;

/**
 * Configure appconfig, dynamically
 */
typedef struct
{
    command_call_t call;            /**< Standard command call              */
    char setting[MAXTIDENT+1];      /**< Setting name                       */
    char svalue[MAXTIDENT+1];       /**< Request/Response value             */
} command_appconfig_t;


/**
 * Configure ndrxd daemon sleep, for debug
 */
typedef struct
{
    command_call_t call;            /**< Standard command call              */
    int secs;                       /**< Sleep seconds                      */
} command_dsleep_t;

/**
 * Dynamic un/advertise structure
 */
typedef struct
{
    command_call_t call;
    
    int srvid;
    char svc_nm[MAXTIDENT+1];       /**< Service name                       */
    char fn_nm[MAXTIDENT+1];        /**< Function name                      */
    ndrx_stopwatch_t   qopen_time;  /**< Timer when q was open              */

} command_dynadvertise_t;

/**
 * Server ping structure
 */
typedef struct
{
    command_call_t call;
    int srvid;                      /**< server ID to be pinged             */
    int seq;                        /**< sequence number in ping            */
} command_srvping_t;

/**
 * Generic command reply structure
 * If changing, see ndrx_thstop_command_call_t. The command_id must match.
 */
typedef struct
{
    /* <standard comms header:> */
#if defined(EX_USE_SYSVQ) || defined(EX_USE_SVAPOLL)
    long mtype;             /**< mandatory for System V queues              */
#endif
    short command_id;
    char proto_ver[4];
    int proto_magic;
    /* </standard comms header> */
    unsigned long magic;    /**< Packed macking...                          */
    int command;            /**< replay command                             */
    short msg_type;         /**< Message source                             */
    short msg_src;          /**< Message source                             */
    long flags;             /**< Response flags                             */
    int status;             /**< Reply status                               */
    int error_code;         /**< Error code, if status if is faulty         */
    long userfld1;          /**< User field                                 */
    char error_msg[RPLY_ERR_MSG_MAX];/**< Error message in reply            */
    char subtype[0];        /**< Pointer to subtype                         */
} command_reply_t;

/*
 * Generic bridge to ndrxd command.
 * Actually now command_call have already nodeid inside, but ok...
 */
typedef struct
{
    command_call_t call;
    int nodeid;
} bridge_info_t;

/**
 * Entry of the refresh
 */
typedef struct
{
    char mode; /**< This is diff (+X, -X) or full */
    char svc_nm[MAXTIDENT+1];
    int count;
} bridge_refresh_svc_t;

/**
 * Refresh call.
 * Probably this can be directly forwarded to other node via bridge.
 */
typedef struct
{
    command_call_t call;
    char mode;   /**< Refresh mode, full or partial??? */
    int count; /**< count of bellow entries */
    bridge_refresh_svc_t svcs[0]; /**< The entries. */
} bridge_refresh_t;


/**
 * When connection is established, both nodes exchanges with their
 * monotonic clocks.
 */
typedef struct
{
    /* Clock sync */
    command_call_t call;
    ndrx_stopwatch_t time;
    int mode;           /**< 1 - async, 2 - request, 3 - reply                      */
    long orig_seq;           /**< sequence number for the request (if with reply         */
    int orig_nodeid;    /**< originator of the message (or caller in case of reply  */
    time_t orig_timestamp;/**< Originatic clock (for the reply match)               */
} cmd_br_time_sync_t;

/**
 * Generic handler for bridge message
 */
typedef struct
{
    long br_magic;
    char msg_type;  /**< A - ATMI, X - NDRX                                 */
    int command_id; /**< either ATMI or NDRX command_id/command             */
    long len;
    char buf[0];
} cmd_br_net_call_t;

/***************** List of reply types/subtypes ***************************/


/**
 * This is used only for ndrxd responses
 * reply struct to ndrxd pings...
 */
typedef struct
{
    command_reply_t rply;           /**< Reply record                       */
    int srvid;                      /**< server ID to be pinged             */
    int seq;                        /**< sequence number in ping            */
} command_reply_srvping_t;

/**
 * Reply command of the appconfig
 */
typedef struct
{
    command_reply_t rply;           /**< Reply record                       */
    char setting[MAXTIDENT+1];      /**< Setting name                       */
    char svalue[MAXTIDENT+1];       /**< Request/Response value             */
} command_reply_appconfig_t;

/**
 * Reply for start/stop processing...
 */
typedef struct
{
    command_reply_t rply;
    /* list some process ifo */
    char binary_name[MAXTIDENT+1];  /**< Name of the binary                 */
    int srvid;
    char clopt[PATH_MAX];
    long state;                     /**< process state code                 */
    pid_t pid;                      /**< PID of the process                 */
} command_reply_pm_t;

/**
 * Details entry about services
 */
typedef struct
{
    char svc_nm[MAXTIDENT+1];       /**< Service name                       */
    char fn_nm[MAXTIDENT+1];        /**< Function name                      */
    long done;                      /**< how many requests are finished     */
    long fail;                      /**< how many requests are finished     */
    long min;                       /**< min response time                  */
    long max;                       /**< max response time                  */
    long last;                      /**< last response time                 */
    short status;                   /**< service status                     */
} command_reply_psc_det_t;

/**
 * Reply for psc (print services) command
 */
typedef struct
{
    command_reply_t rply;
    /* list some process info */
    char binary_name[MAXTIDENT+1]; /**< Name of the binary          */
    int nodeid;           /**< Other node id of the bridge          */
    int srvid;
    pid_t pid;            /**< PID of the process                   */
    long state;           /**< process state code                   */
    int svc_count;        /**< count of services (for belloow array)*/
    command_reply_psc_det_t svcdet[0];
} command_reply_psc_t;

/**
 * Queue list 
 */
typedef struct
{
    command_reply_t rply;
    char qstr[NDRX_MAX_Q_SIZE+1];  /**< Queueu name                    */
} command_reply_blist_t;


/**
 * Bridge infos
 */
typedef struct
{
    command_reply_t rply;
    long locnodeid; /**< local node id                      */
    int srvid;      /**< Server id generating resposne      */
    long remnodeid; /**< remove node id                     */
    char mode;      /**< Connection mode                    */   
    int fd;         /**< socket FD number                   */
    
    long time;      /**< local monotonic time sec           */
    long timems;    /**< local monotonic time ms            */
    
    /* Clock infos: */
    long lastsync;  /**< last sync time ago (seconds)       */
    long timediffs; /**< time diff in seconds between hosts */
    long timediffms; /**< time diff in milliseconds between hosts */
    long roundtrip; /**< roundtrip in milliseconds          */
    
} command_reply_brconinfo_t;


/**
 * Reply for ppm (print process model) command
 * Data fields really are taken from pm_node_t/ndrxd.h!!
 */
typedef struct
{
    command_reply_t rply;
    /** list some process ifo */
    char binary_name[MAXTIDENT+1]; /**< Name of the binary*/
    
    /** Real binary name (reported by server it self */
    char binary_name_real[MAXTIDENT+1];
    
    /** request address, used by system v */
    char rqaddress[NDRX_MAX_Q_SIZE+1];
    
    int srvid;
    long state;             /**< process state code (current)  */
    long reqstate;          /**< Requested state               */
    short autostart;        /**< Start automatically after "start" cmd */
    int exec_seq_try;       /**< Sequental start try           */
    long last_startup;      /**< Cycle count for last start   */

    int num_term_sigs;      /**< Number of times to send term sig, before -9 */
    long last_sig;          /**< Time when signal was sent     */
    int autokill;           /**< Kill the process if not started in time */
    /** Parent process PID (or if server if only process then it is real PID */
    pid_t pid;
    /** Server process PID (if parent is script, then this is real one) */
    pid_t svpid;
    long state_changed;     /**< Timer for state changed       */
    int flags;              /**< Flags sent by server info     */
    short   nodeid;         /**< other node id, if this is bridge */
    
} command_reply_ppm_t;

/**
 * Packet for shm_psvc, fields from shm_svcinfo_t
 */
typedef struct
{
    command_reply_t rply;
    int slot;                       /**< Position in SHM                      */
    char service[MAXTIDENT+1];      /**< name of the service                  */
    int srvs;                       /**< Count of servers advertising this service*/
    int flags;                      /**< service flags                        */
    
    int csrvs;                      /**< Number of advertises in cluster      */
    int totclustered;               /**< Total clustered nodes                */
    int cnodes_max_id;              /**< Max id of cluster nodes in list (for fast search) */
    cnodeinfo_t cnodes[CONF_NDRX_NODEID_COUNT]; /**< List of cluster nodes */
    /**< Server ID (fixed number xadmin output) */
    ndrx_shm_resid_t resids[CONF_NDRX_MAX_SRVIDS_XADMIN];
    int resnr;                                  /**< Number of local resources              */
    
} command_reply_shm_psvc_t;


/**
 * Packet for shm_psvc, fields from shm_svcinfo_t
 */
typedef struct
{
    command_reply_t rply;
    char service[MAXTIDENT+1];  /**< name of the service                    */
    int pq_info[PQ_LEN];        /**< Print queues,  statistics              */
} command_reply_pq_t;

/**
 * Packet for shm_psrv, fields from shm_srvinfo_t
 */
typedef struct
{
    command_reply_t rply;
    int slot;                              /**< Position in shm.              */
    int srvid;
    unsigned execerr;                      /**< Flags for last call           */
    short status;                          /**< Glboal status, avail or busy  */
    short last_command_id;                 /**< Last command ID received      */
    char last_reply_q[NDRX_MAX_Q_SIZE+1];  /**< Last queue on it should reply */
    
} command_reply_shm_psrv_t;

/**
 * This reply contains additional info about configuration loading process & errors
 */
typedef struct
{
    command_reply_t rply;
    int srvid;                      /**< Server ID with problem  */
    char old_binary[MAXTIDENT+1];   /**< Old binary in config    */
    char new_binary[MAXTIDENT+1];   /**< New binary in config    */
    int error;                      /**< Associated error code   */
    char msg[256];                  /**< additional errro msg    */
} command_reply_reload_t;

/**
 * Reply to command getbrs - GETBRIDGES
 */
typedef struct
{
    command_reply_t rply;
    char nodes[CONF_NDRX_NODEID_COUNT+1];
} command_reply_getbrs_t;

/**
 * Packet for print environment (limited to: FILENAME_MAX)
 */
typedef struct
{
    command_reply_t rply;
    char env[EX_ENV_MAX+1];         /* Env value */
} command_reply_pe_t;

/***************** List of request types/subtypes ***********************/
/**
 * Service entry
 */
typedef struct
{
    char svc_nm[MAXTIDENT+1];       /**< Service name                   */
    char fn_nm[MAXTIDENT+1];        /**< Function name                  */
    ndrx_stopwatch_t   qopen_time;  /**< Timer when q was open          */
} svc_inf_t;

/**
 * Server Key structure - key to identify server sending the info to us!
 */
typedef struct
{
   int srvid;              /**< server id sending this info                   */
   pid_t  pid;             /**< server's process id (for crosscheck alarming) */
   /** real server PID */
   pid_t svpid;
   /** Real name of server process */
   char binary_name_real[MAXTIDENT+1];
   int state;              /**< server's state (the same as for process       */
   int flags;              /**< servers flags                                 */
   int nodeid;             /**< Other node id of the bridge                   */
   /** 
    * Have a request address of the server so that we do not zap them during
    * housekeeping
    */
   char rqaddress[NDRX_MAX_Q_SIZE+1];
   
   int resid;             /**< Resource id server id for poll, qid for sysv   */
   
} srv_key_t;

/**
 * Server status type.
 * Also lists all services advertised.
 */
typedef struct
{
   command_call_t call;    /**< Generic call data                           */
   srv_key_t srvinfo;      /**< Server key                                  */
   short svc_count;        /**< Service count                               */
   svc_inf_t svcs[0];      /**< Service names, should be in proper order!   */
} srv_status_t;
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

/* ATMI helpers here (to avoid header recursion): */
extern NDRX_API int cmd_generic_call(int ndrxd_cmd, int msg_src, int msg_type,
                            command_call_t *call, size_t call_size,
                            char *reply_q,
                            mqd_t reply_queue, /* listen for answer on this! */
                            mqd_t admin_queue, /* this might be FAIL! */
                            char *admin_q_str, /* should be set! */
                            int argc, char **argv,
                            int *p_have_next,
                            int (*p_rsp_process)(command_reply_t *reply, size_t reply_len),
                            void (*p_put_output)(char *text),
                            int need_reply);

extern NDRX_API int cmd_generic_call_2(int ndrxd_cmd, int msg_src, int msg_type,
                            command_call_t *call, size_t call_size,
                            char *reply_q,
                            mqd_t reply_queue, /* listen for answer on this! */
                            mqd_t admin_queue, /* this might be FAIL! */
                            char *admin_q_str, /* should be set! */
                            int argc, char **argv,
                            int *p_have_next,
                            int (*p_rsp_process)(command_reply_t *reply, size_t reply_len),
                            void (*p_put_output)(char *text),
                            int need_reply,
                            int reply_only,
                            char *rply_buf_out,             /* might be a null  */
                            int *rply_buf_out_len,          /* if above is set, then must not be null */
                            int flags,
                            int (*p_rply_request)(char **buf, long len)
);

extern NDRX_API int cmd_generic_bufcall(int ndrxd_cmd, int msg_src, int msg_type,
                            command_call_t *call, size_t call_size,
                            char *reply_q,
                            mqd_t reply_queue, /* listen for answer on this! */
                            mqd_t admin_queue, /* this might be FAIL! */
                            char *admin_q_str, /* should be set! */
                            int argc, char **argv,
                            int *p_have_next,
                            int (*p_rsp_process)(command_reply_t *reply, size_t reply_len),
                            void (*p_put_output)(char *text),
                            int need_reply,
                            int reply_only,
                            char *rply_buf_out,             /* might be a null  */
                            int *rply_buf_out_len,          /* if above is set, then must not be null */
                            int flags,
                            int (*p_rply_request)(char **buf, long len));

extern NDRX_API int cmd_generic_callfl(int ndrxd_cmd, int msg_src, int msg_type,
                            command_call_t *call, size_t call_size,
                            char *reply_q,
                            mqd_t reply_queue, /* listen for answer on this! */
                            mqd_t admin_queue, /* this might be FAIL! */
                            char *admin_q_str, /* should be set! */
                            int argc, char **argv,
                            int *p_have_next,
                            int (*p_rsp_process)(command_reply_t *reply, size_t reply_len),
                            void (*p_put_output)(char *text),
                            int need_reply,
                            int flags);
extern NDRX_API void cmd_generic_init(int ndrxd_cmd, int msg_src, int msg_type,
                            command_call_t *call, char *reply_q);
#ifdef	__cplusplus
}
#endif

#endif	/* NDRXDCMN_H */

/* vim: set ts=4 sw=4 et smartindent: */
