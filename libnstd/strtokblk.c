/**
 * @brief block token string, i.e. keep quoted strings together. strip quotes
 *
 * @file strtokblk.c
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
#include <ndrx_config.h>
#include <ndrstandard.h>
#include <string.h>
#include <stdio.h>
#include <nstdutil.h>
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

/**
 * Unescape the string
 * \\ becomes \
 * \<symb> becomes <symb>
 * @param input input string to process
 * @param symbs escaped symbol
 */
expublic void ndrx_str_unescape(char *input, char *symbs)
{
    char *p=input;
    char *output_p=input;
    int has_escape=0;

    
    /*printf("UNSstr [%s]\n", input);*/
    
    while ( *p != '\0')
    {
        /*printf("UNS [%c]\n", *p);*/
        
        if ('\\'==*p)
        {
            has_escape++;

            if (has_escape>1)
            {
                *output_p='\\';
                output_p++;
                has_escape=0;
            }

        }
        else if (1==has_escape)
        {
            char *pp=strchr(symbs, *p);
            
            if (NULL!=pp)
            {
                *output_p=*pp;
                output_p++;
            }
            else
            {
                *output_p='\\';
                output_p++;

                *output_p=*p;
                output_p++;
            }
            has_escape=0;
        }
        else
        {
            *output_p=*p;
            output_p++;
        }

        p++;
    }
    /* terminate the string... */
    *output_p=0x0;
	
}

/**
 * Remove char from left
 * @param input input string to process
 * @param symb symbol to remove from left
 */
expublic void ndrx_str_trim_left_single(char *input, char symb)
{
    int len=strlen(input);
    char *p = strchr(input, symb);
    memmove(p, p+1, len-(p-input)); /* includes eos... */
}

/**
 * Tokenize string, keep blocks (e.g. quotes to gether)
 * This returns only valid data, and not empty strings.
 * @param input input string to process
 * @param delimit token delimiter
 * @param qotesymbs list of quote symbols 
 * @return returns token or NULL
 */
expublic char *ndrx_strtokblk ( char *input, char *delimit, char *qotesymbs)
{
    /* do stuff per thread, not supported with going up to golang or java 
     * with steps
     */
    static __thread char *p = NULL;
    char *token = NULL;
    char *block_sym = NULL;
    int in_block = 0;
    int block_sym_index = -1;
    int consecutive_escapes=0;
     
    /* do not return empty strings... */
    do
    {
        if ( input != NULL)
        {
            p = input;
            token = input;
        }
        else
        {
            token = p;
            if ( *p == '\0')
            {
                token = NULL;
            }
        }

        /* escape: \ */
        while ( *p != '\0')
        {	
            /* printf("Symb: [%c] ESCAPES: %d INBLOCK: %d\n", *p, consecutive_escapes, in_block); */
            if ('\\'==*p)
            {
                consecutive_escapes++;
            }
            else if (in_block)
            {
                int do_inc = EXTRUE;
                
                /*no close if previous is \ of token */
                if (qotesymbs[block_sym_index] == *p)
                {

                    /* terminate only if not escaped.. */
                    if (consecutive_escapes%2==0)
                    {   
                        /* p++; - remove symbol on the fly... */
                        ndrx_str_trim_left_single(p, qotesymbs[block_sym_index]);
                        in_block = 0;
                        do_inc = EXFALSE;
                    }
                }
                consecutive_escapes=0;
                
                if (do_inc)
                {
                    p++;
                }
                
                continue;
            }

            /*no open if previous is \, then replace escaped quotes to single*/
            else if (( block_sym = strchr ( qotesymbs, *p)) != NULL)
            {
                if (consecutive_escapes%2==0)
                {
                    in_block = 1;
                    block_sym_index = block_sym - qotesymbs;
                    
                    /* p++; - remove symbol on the fly... */
                    ndrx_str_trim_left_single(p, qotesymbs[block_sym_index]);
                    
                    continue;
                }
                /* escape is spent... */
                consecutive_escapes=0;
            }

            if ( strchr ( delimit, *p) != NULL)
            {
                *p = '\0';
                p++;
                break;
            }

            p++;
        }

        if (block_sym_index>-1)
        {
            char escp_symb[2]={'\0', '\0'};

            escp_symb[0]=qotesymbs[block_sym_index];

            ndrx_str_unescape(token, escp_symb);
        }
        else if (NULL!=token)
        {
            /* just unescape any stuff ... */
            ndrx_str_unescape(token, qotesymbs);
        }
        
        input = NULL;
        
    } while (NULL!=token && EXEOS==token[0] && EXFAIL==block_sym_index);
   
    return token;
}

/* vim: set ts=4 sw=4 et smartindent: */
