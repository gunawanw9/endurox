/**
 * @brief Persistent queue commons
 *
 * @file qcommon.h
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
#ifndef QCOMMON_H
#define	QCOMMON_H

#ifdef	__cplusplus
extern "C" {
#endif
/*---------------------------Includes-----------------------------------*/
#include <stdint.h>
#include <ubf.h>
#include <atmi.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
#define TMQ_DEFAULT_BUFSZ               1024 /* default buffer size     */
    
/* Commands loaded into EX_QCMD: */
#define TMQ_CMD_ENQUEUE         'E'      /**< Enqueue                     */
#define TMQ_CMD_DEQUEUE         'D'      /**< Dequeue                     */
#define TMQ_CMD_MQLQ            'P'      /**< Print queue                 */
#define TMQ_CMD_MQLC            'C'      /**< List configuration of q     */
#define TMQ_CMD_MQLM            'M'      /**< List messages               */
#define TMQ_CMD_MQRC            'R'      /**< Reload config               */
#define TMQ_CMD_MQCH            'H'      /**< Change Q  config (runtime)  */
    
/* XA Commands: */
#define TMQ_CMD_STARTTRAN       'b'      /**< Start new transaction       */
#define TMQ_CMD_ABORTTRAN       'a'      /**< Rollback entry              */
#define TMQ_CMD_PREPARETRAN     'p'      /**< Prepare entry               */
#define TMQ_CMD_COMMITRAN       'c'      /**< Commit entry                */

#define TMQ_QDEF_MAX            512      /**< max buffer size for Q def   */
    
/**
 * This is list of commands to manage files by tmqueue instead of xa driver
 * so that state is fully in sync
 * @defgroup tmqfilecmd
 * @{
 */
#define TMQ_FILECMD_RENAME      'R'      /**< Rename files              */
#define TMQ_FILECMD_UNLINK      'U'      /**< Unlink files              */
/** @} */ /* end of tmqfilecmd */
    
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/
    
extern NDRX_API int tmq_tpqctl_to_ubf_enqreq(UBFH *p_ub, TPQCTL *ctl);
extern NDRX_API int tmq_tpqctl_from_ubf_enqreq(UBFH *p_ub, TPQCTL *ctl);

extern NDRX_API int tmq_tpqctl_to_ubf_enqrsp(UBFH *p_ub, TPQCTL *ctl);
extern NDRX_API int tmq_tpqctl_from_ubf_enqrsp(UBFH *p_ub, TPQCTL *ctl);

extern NDRX_API int tmq_tpqctl_to_ubf_deqreq(UBFH *p_ub, TPQCTL *ctl);
extern NDRX_API int tmq_tpqctl_from_ubf_deqreq(UBFH *p_ub, TPQCTL *ctl);

extern NDRX_API int tmq_tpqctl_to_ubf_deqrsp(UBFH *p_ub, TPQCTL *ctl);
extern NDRX_API int tmq_tpqctl_from_ubf_deqrsp(UBFH *p_ub, TPQCTL *ctl);

extern NDRX_API char * tmq_msgid_serialize(char *msgid_in, char *msgid_str_out);
extern NDRX_API char * tmq_msgid_deserialize(char *msgid_str_in, char *msgid_out);

/* API: */
extern NDRX_API int ndrx_tpenqueue (char *qspace, short nodeid, short srvid, char *qname, TPQCTL *ctl, 
        char *data, long len, long flags);

extern NDRX_API int ndrx_tpdequeue (char *qspace, short nodeid, short srvid, char *qname, TPQCTL *ctl, 
        char **data, long *len, long flags);
#ifdef	__cplusplus
}
#endif

#endif	/* QCOMMON_H */

/* vim: set ts=4 sw=4 et smartindent: */
