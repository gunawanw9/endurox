/* 
** Environment management, commands: `set', `unset', printenv (pe)
** TODO: Improve error handling (print more accurate error from ndrxd instead of 0)
**
** @file cmd_env.c
** 
** -----------------------------------------------------------------------------
** Enduro/X Middleware Platform for Distributed Transaction Processing
** Copyright (C) 2015, Mavimax, Ltd. All Rights Reserved.
** This software is released under one of the following licenses:
** GPL or Mavimax's license for commercial use.
** -----------------------------------------------------------------------------
** GPL license:
** 
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the Free Software
** Foundation; either version 2 of the License, or (at your option) any later
** version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
** PARTICULAR PURPOSE. See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 59 Temple
** Place, Suite 330, Boston, MA 02111-1307 USA
**
** -----------------------------------------------------------------------------
** A commercial use license is available from Mavimax, Ltd
** contact@mavimax.com
** -----------------------------------------------------------------------------
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/param.h>

#include <ndrstandard.h>
#include <ndebug.h>

#include <ndrx.h>
#include <ndrxdcmn.h>
#include <atmi_int.h>
#include <gencall.h>

#include "ntimer.h"
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/
/**
 * Process response back.
 * @param reply
 * @param reply_len
 * @return
 */
public int pe_rsp_process(command_reply_t *reply, size_t reply_len)
{
    if (NDRXD_CALL_TYPE_PE==reply->msg_type)
    {
        command_reply_pe_t * pe_info = (command_reply_pe_t*)reply;
        fprintf(stdout, "%s\n", pe_info->env);
    }
    
    return SUCCEED;
}

/**
 * Get service listings
 * @param p_cmd_map
 * @param argc
 * @param argv
 * @return SUCCEED
 */
public int cmd_pe(cmd_mapping_t *p_cmd_map, int argc, char **argv, int *p_have_next)
{
    command_call_t call;
    memset(&call, 0, sizeof(call));

    /* Then get listing... */
    return cmd_generic_listcall(p_cmd_map->ndrxd_cmd, NDRXD_SRC_ADMIN,
                        NDRXD_CALL_TYPE_GENERIC,
                        &call, sizeof(call),
                        G_config.reply_queue_str,
                        G_config.reply_queue,
                        G_config.ndrxd_q,
                        G_config.ndrxd_q_str,
                        argc, argv,
                        p_have_next,
                        G_call_args,
                        FALSE);
}

/**
 * Set env variable value
 * @param p_cmd_map
 * @param argc
 * @param argv
 * @param p_have_next
 * @return 
 */
public int cmd_set(cmd_mapping_t *p_cmd_map, int argc, char **argv, int *p_have_next)
{
    int ret=SUCCEED;
    command_setenv_t call;
    int i=1;
    
    memset(&call, 0, sizeof(call));
    
    if (argc>=2)
    {
        while (i<argc && strlen(call.env)+strlen(argv[i]) < EX_ENV_MAX-2)
        {
            if (i>1)
            {
                strcat(call.env, " ");
            }
            strcat(call.env, argv[i]);
            i++;
        }
    }
    else
    {
        fprintf(stderr, "Invalid value, format set 'ENV_VAR=VALUE'\n");
        FAIL_OUT(ret);
    }
    
    ret=cmd_generic_listcall(p_cmd_map->ndrxd_cmd, NDRXD_SRC_ADMIN,
                        NDRXD_CALL_TYPE_GENERIC,
                        (command_call_t *)&call, sizeof(call),
                        G_config.reply_queue_str,
                        G_config.reply_queue,
                        G_config.ndrxd_q,
                        G_config.ndrxd_q_str,
                        argc, argv,
                        p_have_next,
                        G_call_args,
                        FALSE);
out:
    return ret;
}


/**
 * Unset env
 * @param p_cmd_map
 * @param argc
 * @param argv
 * @param p_have_next
 * @return 
 */
public int cmd_unset(cmd_mapping_t *p_cmd_map, int argc, char **argv, int *p_have_next)
{
    int ret=SUCCEED;
    command_setenv_t call;
    
    memset(&call, 0, sizeof(call));
    
    if (argc>=2)
    {
        strncpy(call.env, argv[1], EX_ENV_MAX-1);
        call.env[EX_ENV_MAX-1]=EOS;
        if (NULL!=strchr(call.env, '='))
        {
                fprintf(stderr, "Invalid format\n");
                FAIL_OUT(ret);
        }
        strcat(call.env,"=");
    }
    else
    {
        fprintf(stderr, "Missing ENV name\n");
        FAIL_OUT(ret);
    }
    
    ret=cmd_generic_listcall(p_cmd_map->ndrxd_cmd, NDRXD_SRC_ADMIN,
                        NDRXD_CALL_TYPE_GENERIC,
                        (command_call_t *)&call, sizeof(call),
                        G_config.reply_queue_str,
                        G_config.reply_queue,
                        G_config.ndrxd_q,
                        G_config.ndrxd_q_str,
                        argc, argv,
                        p_have_next,
                        G_call_args,
                        FALSE);
out:
    return ret;
}

