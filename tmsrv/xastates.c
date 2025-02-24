/**
 * @brief State transition handling of XA transactions
 *
 * @file xastates.c
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
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <dlfcn.h>
#include <limits.h>

#include <atmi.h>
#include <atmi_shm.h>
#include <ndrstandard.h>
#include <ndebug.h>
#include <ndrxdcmn.h>
#include <userlog.h>
#include <xa_cmn.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/

/**
 * Static branch driver
 * We will have two drives of all of this:
 * 1. Get new RM status (driven by current stage, status, operation and outcome)
 * 2. Get new TX state (Driven by current TX stage, and RM new status)
 * And the for file TX outcome we should select the stage with lower number...
 */

/* =========================================================================
 * Driving of the PREPARING: 
 * =========================================================================
 */
exprivate rmstatus_driver_t M_rm_status_driver_preparing[] =
{  
    /* ok: */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ACTIVE, XA_OP_PREPARE, XA_OK,     XA_OK,     XA_RM_STATUS_PREP,          XA_TX_STAGE_COMMITTING},
    /* read only assumed as committed: */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ACTIVE, XA_OP_PREPARE, XA_RDONLY, XA_RDONLY, XA_RM_STATUS_COMMITTED_RO,  XA_TX_STAGE_COMMITTING},
    /* If no transaction, then assume committed, read only: */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ACTIVE, XA_OP_PREPARE, XAER_NOTA, XAER_NOTA, XA_RM_STATUS_ABORTED,       XA_TX_STAGE_ABORTING},
    /* Shall we perform any action here? */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ACTIVE, XA_OP_PREPARE, XA_RBBASE, XA_RBEND,  XA_RM_STATUS_ABORTED,       XA_TX_STAGE_ABORTING},
    /* Any error out of the range (catched, we scan from the start) causes abort sequence to run */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ACTIVE, XA_OP_PREPARE, INT_MIN,INT_MAX,      XA_RM_STATUS_ACT_AB,        XA_TX_STAGE_ABORTING},
    /* for PostgreSQL we have strange situation, that only case to work in distributed way is to mark the transaction as
     * prepared once the processing thread disconnects. Thus even transaction is active, the resource is prepared.
     */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_PREP,   XA_OP_NOP,     XA_OK,XA_OK,          XA_RM_STATUS_PREP,          XA_TX_STAGE_COMMITTING},
    /* If recovered from logs where decision is not yet logged, but was logged that this particular RM wants abort: */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ACT_AB, XA_OP_NOP,     XA_OK,XA_OK,          XA_RM_STATUS_ACT_AB,        XA_TX_STAGE_ABORTING},
    /* If restarted, vote for abort, thought after the restart we enter automatically in abort sequence */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_ABORTED,XA_OP_NOP,     XA_OK,XA_OK,          XA_RM_STATUS_ABORTED,       XA_TX_STAGE_ABORTING},
    /* if restarted, vote for commit */
    {XA_TX_STAGE_PREPARING, XA_RM_STATUS_COMMITTED_RO,XA_OP_NOP,XA_OK,XA_OK,          XA_RM_STATUS_COMMITTED_RO,  XA_TX_STAGE_COMMITTING},
    
    {EXFAIL}
};

/* =========================================================================
 * Driving of the COMMITTING 
 * =========================================================================
 */
exprivate rmstatus_driver_t M_rm_status_driver_committing[] =
{  
    /* ok: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_OK,      XA_OK,      XA_RM_STATUS_COMMITTED,     XA_TX_STAGE_COMMITTED},
    /* In case of error: assuming heuristic results: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XAER_RMERR, XAER_RMERR, XA_RM_STATUS_COMMIT_HEURIS, XA_TX_STAGE_COMMITTED_HEURIS},
    /* If RM failed, retry (will be hazard error): */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XAER_RMFAIL,XAER_RMFAIL,XA_RM_STATUS_PREP,          XA_TX_STAGE_COMMITTING},
    /* If RM failed, retry (will be hazard error): */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_RETRY,   XA_RETRY,   XA_RM_STATUS_PREP,          XA_TX_STAGE_COMMITTING},
    /* TPEHAZARD + xa_forget */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_HEURHAZ, XA_HEURHAZ, XA_RM_STATUS_COMFORGET_HAZ, XA_TX_STAGE_COMFORGETTING},
    /* TPEHEURISTIC + xa_forget */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_HEURCOM, XA_HEURCOM, XA_RM_STATUS_COMFORGET_HEU, XA_TX_STAGE_COMFORGETTING},
    /* TPEHEURISTIC + xa_forget */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_HEURRB,  XA_HEURRB,  XA_RM_STATUS_COMFORGET_HEU, XA_TX_STAGE_COMFORGETTING},
    /* TPEHEURISTIC + xa_forget */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_HEURMIX, XA_HEURMIX, XA_RM_STATUS_COMFORGET_HEU, XA_TX_STAGE_COMFORGETTING},
    /* Rolled back: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  XA_RBBASE,  XA_RBEND,   XA_RM_STATUS_ABORTED,       XA_TX_STAGE_COMMITTED_HEURIS},
    /* All unknown statuses will vote for committed: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_PREP,          XA_OP_COMMIT,  INT_MIN,    INT_MAX,    XA_RM_STATUS_COMMITTED,     XA_TX_STAGE_COMMITTED},
    /* In case of restart - same results: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_COMMIT_HEURIS, XA_OP_NOP,     XA_OK,  XA_OK,          XA_RM_STATUS_COMMIT_HEURIS, XA_TX_STAGE_COMMITTED_HEURIS},
    /* In case of restart - same results: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_ABORTED,       XA_OP_NOP,     XA_OK,  XA_OK,          XA_RM_STATUS_ABORTED,       XA_TX_STAGE_COMMITTED_HEURIS},
    /* In case of restart - same results: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_COMMIT_HAZARD, XA_OP_NOP,     XA_OK,  XA_OK,          XA_RM_STATUS_COMMIT_HAZARD, XA_TX_STAGE_COMMITTED_HAZARD},
    /* In case of restart - same results: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_COMFORGET_HAZ, XA_OP_NOP,     XA_OK,  XA_OK,          XA_RM_STATUS_COMFORGET_HAZ, XA_TX_STAGE_COMFORGETTING},
    /* In case of restart - same results: */
    {XA_TX_STAGE_COMMITTING, XA_RM_STATUS_COMFORGET_HEU, XA_OP_NOP,     XA_OK,  XA_OK,          XA_RM_STATUS_COMFORGET_HEU, XA_TX_STAGE_COMFORGETTING},
    {EXFAIL}
};

/* =========================================================================
 * Driving of the COMMITTED, FORGETTGIN
 * =========================================================================
 */
exprivate rmstatus_driver_t M_rm_status_driver_comforgetting[] =
{  
    /* OK */
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HAZ,   XA_OP_FORGET,  XA_OK,       XA_OK,       XA_RM_STATUS_COMMIT_HAZARD, XA_TX_STAGE_COMFORGOT_HAZ},
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HEU,   XA_OP_FORGET,  XA_OK,       XA_OK,       XA_RM_STATUS_COMMIT_HEURIS, XA_TX_STAGE_COMFORGOT_HEU},
    /* retry on failure: */
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HAZ,   XA_OP_FORGET,  XAER_RMFAIL, XAER_RMFAIL, XA_RM_STATUS_COMFORGET_HAZ, XA_TX_STAGE_COMFORGETTING},
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HEU,   XA_OP_FORGET,  XAER_RMFAIL, XAER_RMFAIL, XA_RM_STATUS_COMFORGET_HEU, XA_TX_STAGE_COMFORGETTING},
    /* All other assume completed: */
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HAZ,   XA_OP_FORGET,  INT_MIN,    INT_MAX,      XA_RM_STATUS_COMMIT_HAZARD, XA_TX_STAGE_COMFORGOT_HAZ},
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HEU,   XA_OP_FORGET,  INT_MIN,    INT_MAX,      XA_RM_STATUS_COMMIT_HEURIS, XA_TX_STAGE_COMFORGOT_HEU},
    /* Status reporting in restart: */
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMMIT_HAZARD,   XA_OP_NOP,     XA_OK,       XA_OK,       XA_RM_STATUS_COMMIT_HAZARD, XA_TX_STAGE_COMFORGOT_HAZ},
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMMIT_HEURIS,   XA_OP_NOP,     XA_OK,       XA_OK,       XA_RM_STATUS_COMMIT_HEURIS, XA_TX_STAGE_COMFORGOT_HEU},
    
    {EXFAIL}
};
/* =========================================================================
 * Driving of ABORTING:  
 * =========================================================================
 */
exprivate rmstatus_driver_t M_rm_status_driver_aborting[] =
{  
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XA_OK,      XA_OK,      XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XA_OK,      XA_OK,      XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XA_OK,      XA_OK,      XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    /* these are RO reported by end prep */
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_COMMITTED_RO,XA_OP_NOP, XA_OK,      XA_OK,      XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    /* Really for active only, for other already it is a NOP: */
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XA_RDONLY,  XA_RDONLY,  XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XA_RDONLY,  XA_RDONLY,  XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XA_RDONLY,  XA_RDONLY,  XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XA_HEURHAZ, XA_HEURHAZ, XA_RM_STATUS_ABFORGET_HAZ, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XA_HEURHAZ, XA_HEURHAZ, XA_RM_STATUS_ABFORGET_HAZ, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XA_HEURHAZ, XA_HEURHAZ, XA_RM_STATUS_ABFORGET_HAZ, XA_TX_STAGE_ABFORGETTING},
    
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XA_HEURRB,  XA_HEURRB,  XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XA_HEURRB,  XA_HEURRB,  XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XA_HEURRB,  XA_HEURRB,  XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XA_HEURCOM, XA_HEURCOM, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XA_HEURCOM, XA_HEURCOM, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XA_HEURCOM, XA_HEURCOM, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XA_HEURMIX, XA_HEURMIX, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XA_HEURMIX, XA_HEURMIX, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XA_HEURMIX, XA_HEURMIX, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XAER_RMFAIL,XAER_RMFAIL,XA_RM_STATUS_ACT_AB,       XA_TX_STAGE_ABORTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XAER_RMFAIL,XAER_RMFAIL,XA_RM_STATUS_ACTIVE,       XA_TX_STAGE_ABORTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XAER_RMFAIL,XAER_RMFAIL,XA_RM_STATUS_PREP,         XA_TX_STAGE_ABORTING},
/*  well kind of not correct can retry via RECON flag
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XAER_RMERR,XAER_RMERR,XA_RM_STATUS_ACT_AB,         XA_TX_STAGE_ABORTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XAER_RMERR,XAER_RMERR,XA_RM_STATUS_ACTIVE,         XA_TX_STAGE_ABORTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XAER_RMERR,XAER_RMERR,XA_RM_STATUS_PREP,           XA_TX_STAGE_ABORTING},
*/
    /* This is OK: */
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, XAER_NOTA,  XAER_NOTA,  XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, XAER_NOTA,  XAER_NOTA,  XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, XAER_NOTA,  XAER_NOTA,  XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    
    /* default all to aborted (reported to ulog error) */
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACT_AB, XA_OP_ROLLBACK, INT_MIN,    INT_MAX,    XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ACTIVE, XA_OP_ROLLBACK, INT_MIN,    INT_MAX,    XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_PREP,   XA_OP_ROLLBACK, INT_MIN,    INT_MAX,    XA_RM_STATUS_ABORTED,      XA_TX_STAGE_ABORTED},
    
    /* in case of restart keep the states to next */
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ABFORGET_HEU, XA_OP_NOP, XA_OK,     XA_OK,      XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABORTING, XA_RM_STATUS_ABFORGET_HAZ, XA_OP_NOP, XA_OK,     XA_OK,      XA_RM_STATUS_ABFORGET_HAZ, XA_TX_STAGE_ABFORGETTING},
    
    {EXFAIL}
};

/* =========================================================================
 * Driving of the ABORTED, forgetting
 * =========================================================================
 */
exprivate rmstatus_driver_t M_rm_status_driver_abforgetting[] =
{  
    /* OK */
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABFORGET_HAZ,   XA_OP_FORGET,  XA_OK,       XA_OK,       XA_RM_STATUS_ABORT_HAZARD, XA_TX_STAGE_ABFORGOT_HAZ},
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABFORGET_HEU,   XA_OP_FORGET,  XA_OK,       XA_OK,       XA_RM_STATUS_ABORT_HEURIS, XA_TX_STAGE_ABFORGOT_HEU},
    /* retry on failure: */
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABFORGET_HAZ,   XA_OP_FORGET,  XAER_RMFAIL, XAER_RMFAIL, XA_RM_STATUS_ABFORGET_HAZ, XA_TX_STAGE_ABFORGETTING},
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABFORGET_HEU,   XA_OP_FORGET,  XAER_RMFAIL, XAER_RMFAIL, XA_RM_STATUS_ABFORGET_HEU, XA_TX_STAGE_ABFORGETTING},
    /* All other assume completed: */
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABFORGET_HAZ,   XA_OP_FORGET,  INT_MIN,     INT_MAX,     XA_RM_STATUS_ABORT_HAZARD, XA_TX_STAGE_ABFORGOT_HAZ},
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABFORGET_HEU,   XA_OP_FORGET,  INT_MIN,     INT_MAX,     XA_RM_STATUS_ABORT_HEURIS, XA_TX_STAGE_ABFORGOT_HEU},
    /* Status reporting in restart: */
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABORT_HAZARD,   XA_OP_NOP,     XA_OK,       XA_OK,       XA_RM_STATUS_ABORT_HAZARD, XA_TX_STAGE_ABFORGOT_HAZ},
    {XA_TX_STAGE_ABFORGETTING, XA_RM_STATUS_ABORT_HEURIS,   XA_OP_NOP,     XA_OK,       XA_OK,       XA_RM_STATUS_ABORT_HEURIS, XA_TX_STAGE_ABFORGOT_HEU},
    
    {EXFAIL}
};

/**
 * If Stage/State not in list, then assume XA_OP_NOP
 */
expublic txaction_driver_t G_txaction_driver[] =
{  
    {XA_TX_STAGE_PREPARING,     XA_RM_STATUS_ACTIVE, 		XA_OP_PREPARE},
    {XA_TX_STAGE_COMMITTING,    XA_RM_STATUS_PREP, 	        XA_OP_COMMIT},
    {XA_TX_STAGE_ABORTING,      XA_RM_STATUS_ACTIVE, 		XA_OP_ROLLBACK},
    {XA_TX_STAGE_ABORTING,      XA_RM_STATUS_ACT_AB, 		XA_OP_ROLLBACK},
    {XA_TX_STAGE_ABORTING,      XA_RM_STATUS_PREP, 	        XA_OP_ROLLBACK},
    /* drive the forget of committed: */
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HAZ,     XA_OP_FORGET},
    {XA_TX_STAGE_COMFORGETTING, XA_RM_STATUS_COMFORGET_HEU,     XA_OP_FORGET},
    /* drive the  forget of aborted: */
    {XA_TX_STAGE_ABFORGETTING,  XA_RM_STATUS_ABFORGET_HAZ,      XA_OP_FORGET},
    {XA_TX_STAGE_ABFORGETTING,  XA_RM_STATUS_ABFORGET_HEU,      XA_OP_FORGET},
    {EXFAIL}
};

/**
 * State descriptors
 */
expublic txstage_descriptor_t G_state_descriptor[] =
{
/* txstage                     txs_stage_min                 txs_min_complete             txs_max_complete  descr   allow_jump */
{XA_TX_STAGE_NULL,             XA_TX_STAGE_NULL,             XA_TX_STAGE_NULL,            XA_TX_STAGE_NULL,             "NULL",                       EXFALSE},
/* we get outside the ACTIVE state by API call or timeout */
{XA_TX_STAGE_ACTIVE,           XA_TX_STAGE_NULL,             XA_TX_STAGE_NULL,            XA_TX_STAGE_NULL,             "ACTIVE",                     EXFALSE},
{XA_TX_STAGE_ABORTING,         XA_TX_STAGE_ABORTING,         XA_TX_STAGE_ABORTED_HAZARD,  XA_TX_STAGE_ABORTED,          "ABORTING",                   EXFALSE},
/* Left for compliance: */
{XA_TX_STAGE_ABORTED_HAZARD,   XA_TX_STAGE_ABORTED_HAZARD,   XA_TX_STAGE_ABORTED_HAZARD,  XA_TX_STAGE_ABORTED_HAZARD,   "ABORTED_HAZARD",             EXFALSE},
/* Left for compliance: */
{XA_TX_STAGE_ABORTED_HEURIS,   XA_TX_STAGE_ABORTED_HEURIS,   XA_TX_STAGE_ABORTED_HEURIS,  XA_TX_STAGE_ABORTED_HEURIS,   "ABORTED_HEURIS",             EXFALSE},
/* Left for compliance: */
{XA_TX_STAGE_ABORTED,          XA_TX_STAGE_ABORTED,          XA_TX_STAGE_ABORTED,         XA_TX_STAGE_ABORTED,          "ABORTED",                    EXFALSE},
/* Normally we jump out from PREPARING, in case if state not switched */
{XA_TX_STAGE_PREPARING,        XA_TX_STAGE_PREPARING,        XA_TX_STAGE_PREPRO,          XA_TX_STAGE_PREPRO,           "PREPARING",                  EXTRUE},
/* no participants: */
{XA_TX_STAGE_PREPRO,           XA_TX_STAGE_PREPRO,           XA_TX_STAGE_PREPRO,          XA_TX_STAGE_PREPRO,           "NO_PARTICIPANTS",            EXFALSE},
{XA_TX_STAGE_COMMITTING,       XA_TX_STAGE_COMMITTING,       XA_TX_STAGE_COMMITTED_HAZARD,XA_TX_STAGE_COMMITTED,        "COMMITTING",                 EXFALSE},
/* Left for compliance: */
{XA_TX_STAGE_COMMITTED_HAZARD, XA_TX_STAGE_COMMITTED_HAZARD, XA_TX_STAGE_COMMITTED_HAZARD,XA_TX_STAGE_COMMITTED_HAZARD, "COMMITTED_HAZARD",           EXFALSE},
/* Left for compliance: */
{XA_TX_STAGE_COMMITTED_HEURIS, XA_TX_STAGE_COMMITTED_HEURIS, XA_TX_STAGE_COMMITTED_HEURIS,XA_TX_STAGE_COMMITTED_HEURIS, "COMMITTED_HEURIS",           EXFALSE},
/* Left for compliance: */
{XA_TX_STAGE_COMMITTED,        XA_TX_STAGE_COMMITTED,        XA_TX_STAGE_COMMITTED,       XA_TX_STAGE_COMMITTED,        "COMMITTED",                  EXFALSE},
/* Forgetting state of commit */
{XA_TX_STAGE_COMFORGETTING,    XA_TX_STAGE_COMFORGETTING,    XA_TX_STAGE_COMFORGOT_HAZ,   XA_TX_STAGE_COMFORGOT_HEU,    "FORGETTING_COMMITTED",       EXFALSE},
{XA_TX_STAGE_COMFORGOT_HAZ,    XA_TX_STAGE_COMFORGOT_HAZ,    XA_TX_STAGE_COMFORGOT_HAZ,   XA_TX_STAGE_COMFORGOT_HAZ,    "FORGOT_HAZARD_COMMITTED",    EXFALSE},
{XA_TX_STAGE_COMFORGOT_HEU,    XA_TX_STAGE_COMFORGOT_HEU,    XA_TX_STAGE_COMFORGOT_HEU,   XA_TX_STAGE_COMFORGOT_HEU,    "FORGOT_HEURISTIC_COMMITTED", EXFALSE},

/* Forgetting state of abort */
{XA_TX_STAGE_ABFORGETTING,     XA_TX_STAGE_ABFORGETTING,     XA_TX_STAGE_ABFORGOT_HAZ,    XA_TX_STAGE_ABFORGOT_HEU,     "FORGETTING_ABORTED",         EXFALSE},
{XA_TX_STAGE_ABFORGOT_HAZ,     XA_TX_STAGE_ABFORGOT_HAZ,     XA_TX_STAGE_ABFORGOT_HAZ,    XA_TX_STAGE_ABFORGOT_HAZ,     "FORGOT_HAZARD_ABORTED",      EXFALSE},
{XA_TX_STAGE_ABFORGOT_HEU,     XA_TX_STAGE_ABFORGOT_HEU,     XA_TX_STAGE_ABFORGOT_HEU,    XA_TX_STAGE_ABFORGOT_HEU,     "FORGOT_HEURISTIC_ABORTED",   EXFALSE},


{EXFAIL, 0, 0, 0, "FAIL"}
};

/**
 * Needs new table: state-to-tpreturn code mapper. 
 */
expublic txstate2tperrno_t G_txstage2tperrno[] =
{
{XA_TX_STAGE_NULL,             XA_OP_COMMIT,    TPESYSTEM},
{XA_TX_STAGE_ACTIVE,           XA_OP_COMMIT,    TPESYSTEM},
{XA_TX_STAGE_ABORTING,         XA_OP_COMMIT,    TPEHAZARD},
{XA_TX_STAGE_ABORTED_HAZARD,   XA_OP_COMMIT,    TPEHAZARD},
{XA_TX_STAGE_ABORTED_HEURIS,   XA_OP_COMMIT,    TPEHEURISTIC},
{XA_TX_STAGE_ABORTED,          XA_OP_COMMIT,    TPEABORT},
{XA_TX_STAGE_PREPARING,        XA_OP_COMMIT,    TPESYSTEM},
{XA_TX_STAGE_PREPRO,           XA_OP_COMMIT,    EXSUCCEED},
{XA_TX_STAGE_COMMITTING,       XA_OP_COMMIT,    TPEHAZARD},
{XA_TX_STAGE_COMMITTED_HAZARD, XA_OP_COMMIT,    TPEHAZARD},
{XA_TX_STAGE_COMMITTED_HEURIS, XA_OP_COMMIT,    TPEHEURISTIC},
{XA_TX_STAGE_COMMITTED,        XA_OP_COMMIT,    EXSUCCEED},
/* Heuristic completion, commit */
{XA_TX_STAGE_COMFORGETTING,    XA_OP_COMMIT,    TPEHAZARD},
{XA_TX_STAGE_COMFORGOT_HAZ,    XA_OP_COMMIT,    TPEHAZARD},
{XA_TX_STAGE_COMFORGOT_HEU,    XA_OP_COMMIT,    TPEHEURISTIC},
/* Heuristic completion, abort */
{XA_TX_STAGE_ABFORGETTING,     XA_OP_COMMIT,    TPEHAZARD},
/* Test the HEU/HAZ abort outcomes on commit... */
{XA_TX_STAGE_ABFORGOT_HAZ,     XA_OP_COMMIT,    TPEABORT},
{XA_TX_STAGE_ABFORGOT_HEU,     XA_OP_COMMIT,    TPEABORT},
{XA_TX_STAGE_NULL,             XA_OP_ROLLBACK,  TPESYSTEM},
{XA_TX_STAGE_ACTIVE,           XA_OP_ROLLBACK,  TPESYSTEM},
{XA_TX_STAGE_ABORTING,         XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_ABORTED_HAZARD,   XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_ABORTED_HEURIS,   XA_OP_ROLLBACK,  TPEHEURISTIC},
{XA_TX_STAGE_ABORTED,          XA_OP_ROLLBACK,  EXSUCCEED},
{XA_TX_STAGE_PREPARING,        XA_OP_ROLLBACK,  TPESYSTEM},
/* Not expected prepare completed with abort call:  */
{XA_TX_STAGE_PREPRO,           XA_OP_ROLLBACK,  TPESYSTEM},
{XA_TX_STAGE_COMMITTING,       XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_COMMITTED_HAZARD, XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_COMMITTED_HEURIS, XA_OP_ROLLBACK,  TPEHEURISTIC},
{XA_TX_STAGE_COMMITTED,        XA_OP_ROLLBACK,  TPEHEURISTIC},
/* Heuristic completion, commit */
{XA_TX_STAGE_COMFORGETTING,    XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_COMFORGOT_HAZ,    XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_COMFORGOT_HEU,    XA_OP_ROLLBACK,  TPEHEURISTIC},
/* Heuristic completion, abort */
{XA_TX_STAGE_ABFORGETTING,     XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_ABFORGOT_HAZ,     XA_OP_ROLLBACK,  TPEHAZARD},
{XA_TX_STAGE_ABFORGOT_HEU,     XA_OP_ROLLBACK,  TPEHEURISTIC},


{EXFAIL}
};

/**
 * Swtich the operations table
 */
#define OP_TABLE_SWITCH     do { switch (txstage)\
    {\
        case XA_TX_STAGE_PREPARING:\
            ret = M_rm_status_driver_preparing;\
            break;\
        case XA_TX_STAGE_COMMITTING:\
            ret = M_rm_status_driver_committing;\
            break;    \
        case XA_TX_STAGE_ABORTING:\
            ret = M_rm_status_driver_aborting;\
            break;\
        case XA_TX_STAGE_COMFORGETTING:\
            ret = M_rm_status_driver_comforgetting;\
            break;\
        case XA_TX_STAGE_ABFORGETTING:\
            ret = M_rm_status_driver_abforgetting;\
            break;\
        default:\
            return NULL;\
    }} while (0)

/*---------------------------Prototypes---------------------------------*/

/**
 * Get next RM status/txstage by current operation.
 * @param txstage - current tx stage
 * @param rmstatus - current RM status
 * @param op - operation done
 * @param op_retcode - operation return code
 * @param p_xai transaction identifier (used for debug)
 * @param rmid resource manager id (for debug)
 * @param btid branch at RM (for debug)
 * @return NULL or transition descriptor
 */
expublic rmstatus_driver_t* xa_status_get_next_by_op(short txstage, char rmstatus, 
                                                    int op, int op_retcode,
                                                    atmi_xa_tx_info_t *p_xai, 
                                                    short rmid, long btid)
{
    rmstatus_driver_t *ret=NULL;
    
    OP_TABLE_SWITCH;
    
    while (EXFAIL!=ret->txstage)
    {
        if (ret->txstage == txstage &&
                ret->rmstatus == rmstatus &&
                ret->op == op &&
                op_retcode>=ret->min_retcode && 
                op_retcode<=ret->max_retcode
            )
        {
            break;
        }
        ret++;
    } 
    
    if (EXFAIL==ret->txstage)
    {
        ret=NULL;
    }
    
    return ret;
}

/**
 * If we got NOP on current stage action for RM, then we lookup
 * this table, to get it's wote for next txstage.
 * @param txstage - current tx stage
 * @param next_rmstatus - RM status (after NOP...)
 * @return NULL or transition descriptor
 */
expublic rmstatus_driver_t* xa_status_get_next_by_new_status(short   txstage, 
                                                    char next_rmstatus)
{
    rmstatus_driver_t *ret=NULL;
    
    OP_TABLE_SWITCH;
    
    while (EXFAIL!=ret->txstage)
    {
        if (ret->txstage == txstage &&
                ret->next_rmstatus == next_rmstatus
            )
        {
            break;
        }
        ret++;
    }
    
    if (EXFAIL==ret->txstage)
    {
        ret=NULL;
    }
    
    return ret;
}

/**
 * Get operation to be done for current RM in given state
 * @param txstage current tx stage
 * @param rmstatus curren RM status
 * @return operation to be done or NOP
 */
expublic int xa_status_get_op(short txstage, char rmstatus)
{
    txaction_driver_t *ret = G_txaction_driver;
    
    while (EXFAIL!=ret->txstage)
    {
        if (ret->txstage == txstage &&
                ret->rmstatus == rmstatus
            )
        {
            break;
        }
        ret++;
    }
    
    if (EXFAIL!=ret->txstage)
    {
        return ret->op;
    }
    else
    {
        return XA_OP_NOP;
    }
}

/**
 * Get stage descriptor
 * @param txstage
 * @return NULL or stage descriptor
 */
expublic txstage_descriptor_t* xa_stage_get_descr(short txstage)
{
    txstage_descriptor_t* ret = G_state_descriptor;
    
    while (EXFAIL!=ret->txstage)
    {
        if (ret->txstage == txstage)
        {
            break;
        }
        ret++;
    }
    
    if (EXFAIL==ret->txstage)
        ret = NULL;
    
    return ret;
}

/**
 * Translate stage to tperrno
 * @param txstage
 * @return 
 */
expublic int xa_txstage2tperrno(short txstage, int master_op)
{
    txstate2tperrno_t* ret = G_txstage2tperrno;
    
    while (EXFAIL!=ret->txstage)
    {
        if (ret->txstage == txstage &&
                ret->master_op == master_op
                )
        {
            break;
        }
        ret++;
    }
    
    if (EXFAIL==ret->txstage)
        return TPESYSTEM;
    
    return ret->tpe;
}

/* vim: set ts=4 sw=4 et smartindent: */
