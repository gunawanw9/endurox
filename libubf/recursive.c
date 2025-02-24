/**
 * @brief Recursive UBF API
 *
 * @file recursive.c
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

/*---------------------------Includes-----------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <ubf.h>
#include <ubf_int.h>	/* Internal headers for UBF... */
#include <fdatatype.h>
#include <ferror.h>
#include <fieldtable.h>
#include <ndrstandard.h>
#include <ndebug.h>
#include <cf.h>

#include "xatmi.h"
/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
#if BBADFLDOCC<0
#define R_IS_NEXT_EOS(X) (BBADFLDOCC==*(X) || BBADFLDOCC==*(X+1) || BBADFLDOCC==*(X+2))
#else
#define R_IS_NEXT_EOS(X) (BBADFLDOCC==*(X) || BBADFLDOCC==*(X+2))
#endif

#define DEBUG_STR_MAX   512 /**< Max debug string len                   */

#define VIEW_FLD_FOUND  1
#define VIEW_FLD_CNAME_PARSED  2

#define STATE_NONE  1
#define STATE_OCC  2
#define STATE_OCCANY  3
#define STATE_VIEW  4

#define IS_VALID_ID(X)  ( X>='a' && X<='z' || X>='0' && X<='9' || X>='A' && X<='Z' || X=='_')
#define IS_VALID_NUM(X)  ( X>='0' && X<='9' )

#define RESOLVE_FIELD   \
if (0==j)\
{\
    UBF_LOG(log_error, "Missing field name at position %d", i);\
    ndrx_Bset_error_fmt(BSYNTAX, "Missing field name at position %d", i);\
    EXFAIL_OUT(ret);\
}\
tmp[j]=EXEOS;\
if (is_view)\
{\
    if (VIEW_FLD_CNAME_PARSED==is_view)\
    {\
        UBF_LOG(log_error, "Sub-fields of view sub-field are not allowed at position %d", i);\
        ndrx_Bset_error_fmt(BEBADOP, "Sub-fields of view sub-field are not allowed at position %d", i);\
        EXFAIL_OUT(ret);\
    }\
    rfldid->cname=NDRX_STRDUP(tmp);\
    if (NULL==rfldid->cname)\
    {\
        int err;\
        err=errno;\
        UBF_LOG(log_error, "Failed to malloc: %s", strerror(errno));\
        ndrx_Bset_error_fmt(BEUNIX, "Failed to malloc: %s", strerror(errno));\
        EXFAIL_OUT(ret);\
    }\
    is_view=VIEW_FLD_CNAME_PARSED;\
    UBF_LOG(log_debug, "Parsed view field [%s] is_view=%d", rfldid->cname, is_view);\
}\
else \
{\
    if (BBADFLDID==(parsedid=Bfldid (tmp)))\
    {\
        UBF_LOG(log_error, "Failed to resolve [%s] nrfld=%d", tmp, nrflds);\
        EXFAIL_OUT(ret);\
    }\
    if (BFLD_VIEW==Bfldtype(parsedid))\
    {\
        is_view=VIEW_FLD_FOUND;\
    } /* cache last field */\
    UBF_LOG(log_debug, "Resolved field [%s] to [%d] is_view=%d", tmp, parsedid, is_view);\
    rfldid->bfldid=parsedid;\
}\
j=0;\
nrflds++;

/** 
 * Add field to ndrx_ubf_rfldid_t, so lets build full version in any case 
 * Also needs to test the case if we parsed the view field, then 
 * nothing to add to growlist
 */
#define RESOLVE_ADD   if (VIEW_FLD_CNAME_PARSED!=is_view)\
{\
    ndrx_growlist_append(&(rfldid->fldidocc), &parsedid);\
    ndrx_growlist_append(&(rfldid->fldidocc), &parsedocc);\
    rfldid->bfldid=parsedid;\
    rfldid->occ=parsedocc;\
}\
else \
{\
    rfldid->cname_occ=parsedocc;\
}\
j=0;\
added=EXTRUE;

/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
exprivate int validate_rfield(ndrx_ubf_rfldid_t *rfldid);

/**
 * Free up the rfield parsed data
 * @param rfldid recursive field id
 */
expublic void ndrx_ubf_rfldid_free(ndrx_ubf_rfldid_t *rfldid)
{
    if (NULL!=rfldid->cname)
    {
        NDRX_FREE(rfldid->cname);
        rfldid->cname=NULL;
    }
    
    if (NULL!=rfldid->fldnm)
    {
        NDRX_FREE(rfldid->fldnm);
        rfldid->fldnm=NULL;
    }
    
    ndrx_growlist_free(&(rfldid->fldidocc));
}

/**
 * Check the rfield before adding
 * @return EXSUCCEED/EXFAIL
 */
exprivate int validate_rfield(ndrx_ubf_rfldid_t *rfldid)
{
    int ret = EXSUCCEED;    
    BFLDID *grow_fields = (BFLDID *)rfldid->fldidocc.mem;

    /* if we are here, then previous must be BFLD_VIEW or BFLD_UBF (if any)
     * other sub-fields are not supported!
     */
    if (rfldid->fldidocc.maxindexused>-1)
    {
        /* first is field, and then occ follows, thus check fieldid */
        int typ=Bfldtype(grow_fields[rfldid->fldidocc.maxindexused-1]);

        if (BFLD_UBF!=typ && BFLD_VIEW!=typ)
        {
            ndrx_Bset_error_fmt(BEBADOP, "Subfield only allowed for ubf or view types, "
                    "but got type %s at field id position %d",
                    G_dtype_str_map[typ].fldname, rfldid->fldidocc.maxindexused);

            UBF_LOG(log_error, "Subfield only allowed for ubf or view types, "
                    "but got type %s at field id position %d",
                    G_dtype_str_map[typ].fldname, rfldid->fldidocc.maxindexused);
            EXFAIL_OUT(ret);
        }

    }
out:
    return ret;
}

/**
 * Parse the field reference
 * TODO: Add check for invalid sub-field -> it must be UBF, otherwise
 * give BEBADOP
 * @param rfldidstr field reference: fld[occ].fld[occ].fld.fld[occ]
 * @param rfldid parsed reference
 * @param bfldid leaf field id
 * @param occ leaf occurrence
 * @return EXFAIL (error), EXSUCCEED (ok)
 */
expublic int ndrx_ubf_rfldid_parse(char *rfldidstr, ndrx_ubf_rfldid_t *rfldid)
{
    int ret = EXSUCCEED;
    int i, j, len, state, prev_state, terminator;
    char tmp[PATH_MAX];
    BFLDID parsedid=BBADFLDID;
    BFLDOCC parsedocc;
    int *rfldidseq;
    int nrflds=0;
    int is_view=EXFALSE;
    int added;
    BFLDID *grow_fields;
    
    UBF_LOG(log_debug, "Parsing field id sequence: [%s]", rfldidstr);
    ndrx_growlist_init(&(rfldid->fldidocc), 10, sizeof(int));
    rfldid->cname = NULL;
    rfldid->fldnm = NDRX_STRDUP(rfldidstr);
        
    if (NULL==rfldid->fldnm)
    {
        int err = errno;
        
        ndrx_Bset_error_fmt(BEUNIX, "Failed to malloc rfldidstr: %s", strerror(err));
        UBF_LOG(log_error, "Failed to malloc rfldidstr: %s", strerror(err));
        EXFAIL_OUT(ret);
    }
    
    len=strlen(rfldidstr);
    
    if (len>sizeof(tmp)-1)
    {
        ndrx_Bset_error_fmt(BSYNTAX, "Field id too max len %d max %d: [%s] ",
                len, sizeof(tmp)-1, rfldidstr);
        UBF_LOG(log_error, "Field id too max len %d max %d: [%s] ",
                len, sizeof(tmp)-1, rfldidstr);
        EXFAIL_OUT(ret);
    }
    
    prev_state=state=STATE_NONE;
    j=0;
    added=EXFALSE;
    for (i=0; i<len+1; i++)
    {
        if (STATE_NONE==state)
        {
            /* terminal field comes first... */
            if (0x0 ==rfldidstr[i])
            {
                if (j>0)
                {
                    /* finish field, got 0 occ */
                    tmp[j]=EXEOS;
                    RESOLVE_FIELD;
                    parsedocc=0;
                    RESOLVE_ADD;
                }
                
                /* we are done... */
                break;
            }
            
            /* error! sub-fields of view not supported 
             * this is some leading field and if added, check the types.
             * TODO: might want in future support BFLD_PTR...
             */
            if (added)
            {
                if (VIEW_FLD_CNAME_PARSED==is_view)
                {
                    ndrx_Bset_error_fmt(BEBADOP, "Subfield for view-field "
                            "not expected: [%s] nrfld=%d pos=%d", 
                            rfldidstr, nrflds, i);
                    UBF_LOG(log_error, "Subfield for view-field not expected: "
                            "[%s] nrfld=%d pos=%d", 
                            rfldidstr, nrflds, i);
                    EXFAIL_OUT(ret);
                }
            
                /* validate that we might have sub-fields at given position. */
                if (EXSUCCEED!=validate_rfield(rfldid))
                {
                    EXFAIL_OUT(ret);
                }
                added=EXFALSE;
            }
            
            if (IS_VALID_ID(rfldidstr[i]))
            {
                tmp[j]=rfldidstr[i];
                j++;
            }
            else if ('['==rfldidstr[i])
            {
                /* finish field, count occ */
                tmp[j]=EXEOS;
                RESOLVE_FIELD;
                prev_state=state;
                state=STATE_OCC;
            }
            else if ('.'==rfldidstr[i])
            {
                /* finish field, got 0 occ */
                tmp[j]=EXEOS;
                
                /* do not resolve if previous was occurrence */
                if (STATE_OCC!=prev_state && STATE_OCCANY!=prev_state)
                {
                    if (EXEOS==tmp[0])
                    {
                        /* previous was just dot... (..) thus have a syntax
                         * error
                         */
                        ndrx_Bset_error_fmt(BSYNTAX, "Invalid dot notation (..) at %d", i);
                        UBF_LOG(log_error, "Invalid dot notation (..) at %d", i);
                        EXFAIL_OUT(ret);
                    }

                    RESOLVE_FIELD;
                    parsedocc=0;
                    RESOLVE_ADD;
                }
                else
                {
                    /* reset buffer */
                    j=0;
                    /* reset previous state.. */
                    prev_state=state;
                }
            }
            else
            {
                /* error! invalid character expected C identifier, '.', or '[' */
                ndrx_Bset_error_fmt(BSYNTAX, "Invalid character found [%c] in [%s] pos=%d",
                        rfldidstr[i], rfldidstr, i);
                UBF_LOG(log_error, "Invalid character found [%c] in [%s] pos=%d",
                        rfldidstr[i], rfldidstr, i);
                EXFAIL_OUT(ret);
            }
        }
        else if (STATE_OCC==state)
        {
            if (IS_VALID_NUM(rfldidstr[i]))
            {
                tmp[j]=rfldidstr[i];
                j++;
            }
            else if ('?'==rfldidstr[i])
            {
                
                tmp[j]=rfldidstr[i];
                j++;
                if (j>0)
                {
                    tmp[j]=EXEOS;
                    ndrx_Bset_error_fmt(BSYNTAX, "Invalid occurrence: [%s] at pos=%d", 
                            tmp, i);
                    UBF_LOG(log_error, "Invalid occurrence: [%s] at pos=%d", 
                            tmp, i);
                    EXFAIL_OUT(ret);
                }
                
                parsedocc=-2; /* any occurrence */
                state=STATE_OCCANY;
            }
            else if (']'==rfldidstr[i])
            {
                /* Finish off the field + occ */
                tmp[j]=EXEOS;
                parsedocc=atoi(tmp);
                RESOLVE_ADD;
                prev_state=state;
                state=STATE_NONE;
            }
            else if (0x0 ==rfldidstr[i])
            {
                tmp[j]=rfldidstr[i];
                ndrx_Bset_error_fmt(BSYNTAX, "Unclosed occurrence [%s] at pos=%d", 
                        tmp[j], i);
                UBF_LOG(log_error, "Unclosed occurrence [%s] at pos=%d", 
                        tmp[j], i);
                EXFAIL_OUT(ret);
            }
            else
            {
                /* error! Invalid character, expected number, '?' or ']' */
                tmp[j]=rfldidstr[i];
                j++;
                tmp[j]=EXEOS;
                ndrx_Bset_error_fmt(BSYNTAX, "Invalid occurrence: [%s] at pos=%d", 
                            tmp, i);
                UBF_LOG(log_error, "Invalid occurrence: [%s] at pos=%d", 
                        tmp, i);
                EXFAIL_OUT(ret);
            }
        }
        else if (STATE_OCCANY==state)
        {
            if (']'==rfldidstr[i])
            {
                /* Finish off the field + occ, already parsed.. */
                RESOLVE_ADD;
                prev_state=state;
                state=STATE_NONE;
            }
            else if (0x0 ==rfldidstr[i])
            {
                tmp[j]=rfldidstr[i];
                ndrx_Bset_error_fmt(BSYNTAX, "Unclosed occurrence [%s] at pos=%d", 
                        tmp[j], i);
                UBF_LOG(log_error, "Unclosed occurrence [%s] at pos=%d", 
                        tmp[j], i);
                EXFAIL_OUT(ret);
            }
            else
            {
                /* error! Invalid character, expected [?] */
                tmp[j]=rfldidstr[i];
                tmp[j]=EXEOS;
                ndrx_Bset_error_fmt(BSYNTAX, "Invalid any occurrence: [%s] at pos=%d", 
                            tmp, i);
                UBF_LOG(log_error, "Invalid any occurrence: [%s] at pos=%d", 
                        tmp, i);
                EXFAIL_OUT(ret);
            }
        }
    }
    
    /* Terminate the memory sequence... */
    
    /* Unload the field... */
    terminator=BBADFLDOCC;
    ndrx_growlist_append(&(rfldid->fldidocc), &terminator);
    
    rfldidseq=(int *)rfldid->fldidocc.mem;
    
    /* get the last two identifiers.. */
    if (rfldid->fldidocc.maxindexused<2)
    {
         ndrx_Bset_error_fmt(BSYNTAX, "Empty field name parsed (%d)!",
                 rfldid->fldidocc.maxindexused);
        UBF_LOG(log_error, "Empty field name parsed (%d)!",
                rfldid->fldidocc.maxindexused);
        EXFAIL_OUT(ret);
    }
    
    rfldid->nrflds=nrflds;
    
    grow_fields = (BFLDID *)rfldid->fldidocc.mem;
    /* -1 terminator, -1 occ */
    rfldid->bfldid = grow_fields[rfldid->fldidocc.maxindexused-2];
    
out:

    /* free up un-needed resources */
    if (EXSUCCEED!=ret)
    {
        ndrx_ubf_rfldid_free(rfldid);
    }

    UBF_LOG(log_debug, "returns %d bfldid=%d, occ=%d, nrflds=%d, cname=[%s]", 
        ret, rfldid->bfldid, rfldid->occ, nrflds, rfldid->cname?rfldid->cname:"(null)");
    return ret;
}

/**
 * Find the buffer for the final FLDID/OCC
 * i.e. fld1,occ1,fld2,occ2,fld3,occ3.
 * Thus our function is to return the UBF buffer presented by fld2.
 * During the search ensure that sub-buffers are UBFs actually
 * @param p_ub parent UBF buffer to search for field
 * @param fldidocc sequence of fld1,occ1,fld2,occ2,fld3,occ3.
 * @param fldid_leaf terminating field e.g. fld3
 * @param occ_leaf terminating field e.g. occ3
 * @return PTR to UBF buffer found
 */
exprivate UBFH * ndrx_ubf_R_find(UBFH *p_ub, BFLDID *fldidocc, 
        BFLDID *fldid_leaf, BFLDOCC *occ_leaf, BFLDLEN *len)
{
    int ret = EXSUCCEED;
    BFLDID bfldid;
    BFLDID occ;
    int pos=0;
    int typ;
    
    /* lookup ahead do we have EOF */
    while (!R_IS_NEXT_EOS(fldidocc))
    {
        /* first is fld id */
        bfldid=*fldidocc;
        
        if (BBADFLDOCC==*fldidocc)
        {
            UBF_LOG(log_error, "Invalid recursive field identifier sequence, "
                    "expected BFLDID, got BBADFLDOCC(%d) at pos %d", BBADFLDOCC, pos);
            ndrx_Bset_error_fmt(BBADFLD, "Invalid recursive field identifier sequence, "
                    "expected BFLDID, got BBADFLDOCC(%d) at pos %d", BBADFLDOCC, pos);
            p_ub=NULL;
            goto out;
        }

        /* second is occurrence */
        fldidocc++;
        pos++;
        
/* can check if <0, as >=0 is valid occ */
#if BBADFLDOCC<0
        if (BBADFLDOCC==*fldidocc)
        {
            UBF_LOG(log_error, "Invalid recursive occurrence sequence, "
                    "expected occ, got BBADFLDOCC(%d) at pos %d", BBADFLDOCC, pos);
            ndrx_Bset_error_fmt(BBADFLD, "Invalid recursive field identifier sequence, "
                    "expected occ, got BBADFLDOCC(%d) at pos %d", BBADFLDOCC, pos);
            p_ub=NULL;
            goto out;
        }
#endif

        occ=*fldidocc;
        /* find the buffer */
        typ = Bfldtype(bfldid);
        if (BFLD_UBF!=typ)
        {
            UBF_LOG(log_error, "Expected BFLD_UBF (%d) at position %d in "
                    "sequence but got: %d type", BFLD_UBF, pos, typ);
            ndrx_Bset_error_fmt(BEBADOP, "Expected BFLD_UBF (%d) at "
                    "position %d in sequence but got: %d type", BFLD_UBF, pos, typ);
            p_ub=NULL;
            goto out;
        }
        
        p_ub = (UBFH *)ndrx_Bfind(p_ub, bfldid, occ, len, NULL);
        
        if (NULL==p_ub)
        {
            UBF_LOG(log_error, "Buffer not found at position of field sequence %d", pos);
            p_ub=NULL;
            goto out;
        }

        /* step to next pair */
        fldidocc++;
        pos++;
        
    }
    
    if (NULL!=p_ub)
    {
        if(BBADFLDOCC==*fldidocc)
        {
            UBF_LOG(log_error, "Field ID not present at position %d in sequence (BBADFLDOCC (%d) found)",
                    pos, BBADFLDOCC);
            ndrx_Bset_error_fmt(BBADFLD, "Field ID not present at position %d in sequence (BBADFLDOCC (%d) found)",
                    pos, BBADFLDOCC);
            p_ub=NULL;
            goto out;
        }
        
        *fldid_leaf=*fldidocc;
        
        fldidocc++;
        pos++;
        
#if BBADFLDOCC<0
        if (BBADFLDOCC==*fldidocc)
        {
            UBF_LOG(log_error, "Occurrence not present at position %d in sequence (BBADFLDID (%d) found)",
                    pos, BBADFLDOCC);
            ndrx_Bset_error_fmt(BBADFLD, "Occurrence not present at position %d in sequence (BBADFLDID (%d) found)",
                    pos, BBADFLDOCC);
            p_ub=NULL;
            goto out;
        }
#endif
        
        *occ_leaf=*fldidocc;    
        
        
        fldidocc++;
        pos++;
    }
    
    UBF_LOG(log_debug, "Leaf fldid=%d occ=%d", *fldid_leaf, *occ_leaf);
    
out:
    
    UBF_LOG(log_debug, "Returning status=%d p_ub=%p", ret, p_ub);
    
    return p_ub;
}

/**
 * Construct the full debug string as the path 
 * FIELD1[OCC1].FIELD2[OCC2]
 * @param fldidocc
 * @param debug_buf
 * @param debug_buf_len
 * @return 
 */
exprivate void ndrx_ubf_sequence_str(BFLDID *fldidocc, 
        char *debug_buf, size_t debug_buf_len)
{
    int pos=0;
    char *nam;
    char tmp[128];
    int err=Berror;
    debug_buf[0]=EXEOS;
    
    while (BBADFLDOCC!=*fldidocc)
    {
        /* field id: */
        nam=Bfname(*fldidocc);

        if (pos>0)
        {
            NDRX_STRCAT_S(debug_buf, debug_buf_len, ".");
        }

        NDRX_STRCAT_S(debug_buf, debug_buf_len, nam);
        
        /* step to occ */
        fldidocc++;
        pos++;
        
#if BBADFLDOCC < 0
        if (*fldidocc==BBADFLDOCC)
        {
            break;
        }
#endif

        NDRX_STRCAT_S(debug_buf, debug_buf_len, "[");
        snprintf(tmp, sizeof(tmp), "%d", *fldidocc);
        NDRX_STRCAT_S(debug_buf, debug_buf_len, tmp);
        NDRX_STRCAT_S(debug_buf, debug_buf_len, "]");
        
        /* step to next */
        fldidocc++;
        pos++;
        
    }
    Berror=err;
}

/**
 * Recursive field field get
 * @param p_ub parent UBF buffer
 * @param fldidocc array of: <field_id1>,<occ1>,..<field_idN><occN>,BBADFLDOCC
 * @param buf buffer where to return the value
 * @param len in input buffer len, on output bytes written
 * @return EXSUCCEED/EXFAIL
 */
expublic int ndrx_Bgetr (UBFH * p_ub, BFLDID *fldidocc,
                            char * buf, BFLDLEN * buflen)
{
    
    int ret = EXSUCCEED;
    
    BFLDID bfldid;
    BFLDOCC occ;
    BFLDLEN len_data;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &occ, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        EXFAIL_OUT(ret);
    }
    
    ret=Bget(p_ub, bfldid, occ, buf, buflen);
    
out:
    return ret;
}


/**
 * Recursive field field get, with convert
 * @param p_ub parent UBF buffer
 * @param fldidocc array of: <field_id1>,<occ1>,..<field_idN><occN>,BBADFLDOCC
 * @param buf buffer where to return the value
 * @param len in input buffer len, on output bytes written
 * @param usrtype user type to convert data to
 * @return EXSUCCEED/EXFAIL
 */
expublic int ndrx_CBgetr (UBFH * p_ub, BFLDID *fldidocc,
                            char * buf, BFLDLEN * buflen, int usrtype)
{
    int ret = EXSUCCEED;
    BFLDID bfldid;
    BFLDOCC occ;
    BFLDLEN len_data;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &occ, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        EXFAIL_OUT(ret);
    }
    
    ret=CBget(p_ub, bfldid, occ, buf, buflen, usrtype);
    
out:
    return ret;
}

/**
 * Recursive get string field and alloc
 * @param p_ub parent UBF buffer
 * @param fldidocc array of: <field_id1>,<occ1>,..<field_idN><occN>,BBADFLDOCC
 * @param usrtype user type to cast to
 * @param extralen on input number of bytes to allocate extra
 *  on output number of bytes copied to data block
 * @return EXSUCCEED/EXFAIL
 */
expublic char * ndrx_CBgetallocr (UBFH *p_ub, BFLDID *fldidocc, int usrtype, BFLDLEN *extralen)
{
    char *ret = NULL;
    BFLDID bfldid;
    BFLDOCC occ;
    BFLDLEN len_data;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &occ, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        goto out;
    }
    
    /* read the field and allocate */
    ret=CBgetalloc(p_ub, bfldid, occ, usrtype, extralen);
    
out:
    return ret;
}

/**
 * Recursive find implementation
 * @param p_ub root UBF buffer
 * @param fldidocc field sequence
 * @param p_len data len
 * @return ptr to data
 */
expublic char* ndrx_Bfindr (UBFH *p_ub, BFLDID *fldidocc, BFLDLEN *p_len)
{
    char* ret = NULL;
    BFLDID bfldid;
    BFLDOCC occ;
    BFLDLEN len_data;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &occ, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        goto out;
    }
    
    ret=Bfind (p_ub, bfldid, occ, p_len);
    
out:
    return ret;
}

/**
 * Recursive find implementation
 * @param p_ub root UBF buffer
 * @param fldidocc field sequence
 * @param p_len data len
 * @param usrtype user type
 * @return ptr to data
 */
expublic char* ndrx_CBfindr (UBFH *p_ub, BFLDID *fldidocc, BFLDLEN *p_len, int usrtype)
{
    char* ret = NULL;
    BFLDID bfldid;
    BFLDOCC occ;
    BFLDLEN len_data;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &occ, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        goto out;
    }
    
    ret=CBfind (p_ub, bfldid, occ, p_len, usrtype);
    
out:
    return ret;
}

/**
 * Test field presence in recursive fldidocc sequence
 * @param p_ub UBF buffer
 * @param fldidocc fldid,occ,fldocc,...,BBADFLDID
 * @return EXFALSE/EXTRUE
 */ 
expublic int ndrx_Bpresr (UBFH *p_ub, BFLDID *fldidocc)
{
    char ret = EXSUCCEED;
    BFLDID bfldid;
    BFLDOCC occ;
    BFLDLEN len_data;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &occ, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        ret=EXFALSE;
        goto out;
    }
    
    ret=Bpres(p_ub, bfldid, occ);
    
out:
    return ret;
}

/**
 * Retrieve field from view which is set in UBF buffer
 * @param p_ub UBF buffer
 * @param fldidocc fldid,occ,fldid,occ (last if BFLD_VIEW field)
 * @param cname field name in view
 * @param occ occurrence of view field
 * @param buf buffer where to unload the view data
 * @param len buffer length 
 * @param usrtype user type of buf to unload to
 * @param flags optional BVACCESS_NOTNULL
 * @return EXSUCCEED/EXFAIL
 */
expublic int ndrx_CBvgetr(UBFH *p_ub, BFLDID *fldidocc, char *cname, BFLDOCC occ, 
             char *buf, BFLDLEN *len, int usrtype, long flags)
{
    int ret = EXSUCCEED;
    BFLDID bfldid;
    BFLDOCC iocc;
    BFLDLEN len_data;
    BVIEWFLD *vdata;
    int typ;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &iocc, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        goto out;
    }
    
    /* check the field type, must be view */
    typ = Bfldtype(bfldid);
    if (BFLD_VIEW!=typ)
    {
        ndrx_Bset_error_fmt(BEBADOP, "Expected BFLD_VIEW(%d) got %d",
                BFLD_VIEW, typ);
        UBF_LOG(log_error, "Expected BFLD_VIEW(%d) got %d",
                BFLD_VIEW, typ);
        EXFAIL_OUT(ret);
    }
    
    /* retrieve the VIEW */
    vdata = (BVIEWFLD *)Bfind(p_ub, bfldid, iocc, &len_data);
    
    if (NULL==vdata)
    {
        UBF_LOG(log_error, "Failed to find %d fld occ %d", bfldid, iocc);
        EXFAIL_OUT(ret);
    }
    
    UBF_LOG(log_debug, "Reading view field [%s] field [%s] occ [%d] dataptr=%p",
            vdata->vname, cname, occ, vdata->data);
    
    ret = CBvget(vdata->data, vdata->vname, cname, occ, buf, len, usrtype, flags);
    
out:
            
    UBF_LOG(log_debug, "returns %d", ret);

    return ret;
}

/**
 * Retrieve field from view which is set in UBF buffer
 * @param p_ub UBF buffer
 * @param fldidocc fldid,occ,fldid,occ (last if BFLD_VIEW field)
 * @param cname field name in view
 * @param occ occurrence of view field
 * @param usrtype user type of buf to unload to
 * @param flags optional BVACCESS_NOTNULL
 * @param extralen extra len to alloc and output len
 * @return NULL on error, ptr to data
 */
expublic char *ndrx_CBvgetallocr(UBFH *p_ub, BFLDID *fldidocc, char *cname, BFLDOCC occ, 
             int usrtype, long flags, BFLDLEN *extralen)
{
    char * ret = NULL;
    BFLDID bfldid;
    BFLDOCC iocc;
    BFLDLEN len_data;
    BVIEWFLD *vdata;
    int typ;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &iocc, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        goto out;
    }
    
    /* check the field type, must be view */
    typ = Bfldtype(bfldid);
    if (BFLD_VIEW!=typ)
    {
        ndrx_Bset_error_fmt(BEBADOP, "Expected BFLD_VIEW(%d) got %d",
                BFLD_VIEW, typ);
        UBF_LOG(log_error, "Expected BFLD_VIEW(%d) got %d",
                BFLD_VIEW, typ);
        goto out;
    }
    
    /* retrieve the VIEW */
    vdata = (BVIEWFLD *)Bfind(p_ub, bfldid, iocc, &len_data);
    
    if (NULL==vdata)
    {
        UBF_LOG(log_error, "Failed to find %d fld occ %d", bfldid, iocc);
        goto out;
    }
    
    UBF_LOG(log_debug, "Reading view field [%s] field [%s] occ [%d] dataptr=%p",
            vdata->vname, cname, occ, vdata->data);
    
    ret = CBvgetalloc(vdata->data, vdata->vname, cname, occ, usrtype, flags, extralen);
    
out:
            
    UBF_LOG(log_debug, "returns %p", ret);

    return ret;
}

/**
 * Recursive UBF buffer extract view and test for view field presence
 * @param p_ub UBF buffer to search for sub-view
 * @param fldidocc fldid,occ,fldid,occ,BBADFLDID sequence the last fldid shall match
 *  the view.
 * @param cname View field name to test
 * @param occ occurrence to test
 * @return EXFAIL (on error), EXFALSE (field 
 */
expublic int ndrx_Bvnullr(UBFH *p_ub, BFLDID *fldidocc, char *cname, BFLDOCC occ)
{
    int ret = EXSUCCEED;
    BFLDID bfldid;
    BFLDOCC iocc;
    BFLDLEN len_data;
    BVIEWFLD *vdata;
    int typ;
    char debugbuf[DEBUG_STR_MAX]="";
    
    p_ub=ndrx_ubf_R_find(p_ub, fldidocc, &bfldid, &iocc, &len_data);
    
    if (NULL==p_ub)
    {
        if (debug_get_ubf_level() > log_info)
        {
            ndrx_ubf_sequence_str(fldidocc, debugbuf, sizeof(debugbuf));
            UBF_LOG(log_info, "Field not found, sequence: %s", debugbuf);
        }
        
        goto out;
    }
    
    /* check the field type, must be view */
    typ = Bfldtype(bfldid);
    if (BFLD_VIEW!=typ)
    {
        ndrx_Bset_error_fmt(BEBADOP, "Expected BFLD_VIEW(%d) got %d",
                BFLD_VIEW, typ);
        UBF_LOG(log_error, "Expected BFLD_VIEW(%d) got %d",
                BFLD_VIEW, typ);
        EXFAIL_OUT(ret);
    }
    
    /* retrieve the VIEW */
    vdata = (BVIEWFLD *)Bfind(p_ub, bfldid, iocc, &len_data);
    
    if (NULL==vdata)
    {
        UBF_LOG(log_error, "Failed to find %d fld occ %d", bfldid, iocc);
        EXFAIL_OUT(ret);
    }
    
    UBF_LOG(log_debug, "Reading view field [%s] field [%s] occ [%d] dataptr=%p",
            vdata->vname, cname, occ, vdata->data);
    
    ret = Bvnull(vdata->data,  cname, occ, vdata->vname);
    
out:
            
    UBF_LOG(log_debug, "returns %d", ret);

    return ret;
}

/* vim: set ts=4 sw=4 et smartindent: */
