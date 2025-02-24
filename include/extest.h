/**
 * @brief Shared test functions
 *
 * @file extest.h
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

#ifndef EXTEST_H
#define	EXTEST_H

#ifdef	__cplusplus
extern "C" {
#endif

/*---------------------------Includes-----------------------------------*/
#include <test_view.h>
#include <ubf.h>
/*---------------------------Externs------------------------------------*/

/**
 * Flags used for setting test modes
 * @defgroup testflags 
 * @{
 */
#define EXTEST_PROC_PTR             0x00000001  /**< Process PTR        */
#define EXTEST_PROC_UBF             0x00000002  /**< Process UBF        */
#define EXTEST_PROC_VIEW            0x00000004  /**< Process VIEW       */

/** @} */ /* end of testflags */

/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

extern void extest_init_UBTESTVIEW1(struct UBTESTVIEW1 *v);
void extest_ubf_set_up_dummy_data(UBFH *p_ub, long flags);
void extest_ubf_do_dummy_data_test(UBFH *p_ub, long flags);

#ifdef	__cplusplus
}
#endif

#endif	/* EXTEST_H */

/* vim: set ts=4 sw=4 et smartindent: */
