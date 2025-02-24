/**
 * @brief UBF library - header file for UBF error handling.
 *
 * @file ferror.h
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
#ifndef FERROR_H
#define	FERROR_H

#ifdef	__cplusplus
extern "C" {
#endif

/*---------------------------Includes-----------------------------------*/
#include <regex.h>
#include <ndrstandard.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
#define MAX_ERROR_LEN   1024
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
    
/**
 * UBF error handler save/restore
 */
typedef struct
{
    char ubf_error_msg_buf[MAX_ERROR_LEN+1];
    int ubf_error;
} ndrx_ubf_error_t;

/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/
extern NDRX_API void ndrx_Bset_error(int error_code);
extern NDRX_API void ndrx_Bset_error_msg(int error_code, char *msg);
extern NDRX_API void ndrx_Bset_error_fmt(int error_code, const char *fmt, ...);
/* Is error already set?  */
extern NDRX_API int ndrx_Bis_error(void);
extern NDRX_API void ndrx_Bappend_error_msg(char *msg);

extern NDRX_API void ndrx_Bunset_error(void);

/* This is not related with UBF error, we will handle it here anyway
 * because file is part of error handling
 */
extern NDRX_API void ndrx_report_regexp_error(char *fun_nm, int err, regex_t *rp);

extern NDRX_API void ndrx_Bsave_error(ndrx_ubf_error_t *p_err);
extern NDRX_API void ndrx_Brestore_error(ndrx_ubf_error_t *p_err);

#ifdef	__cplusplus
}
#endif

#endif	/* FERROR_H */

/* vim: set ts=4 sw=4 et smartindent: */
