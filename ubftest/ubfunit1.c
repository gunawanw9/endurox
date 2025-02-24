/**
 *
 * @file ubfunit1.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cgreen/cgreen.h>
#include <ubf.h>
#include <ndrstandard.h>
#include <string.h>
#include "test.fd.h"
#include "test1.fd.h"
#include "ubfunit1.h"
#include "ndebug.h"
#include <fdatatype.h>
#include <math.h>
#include <fdatatype.h>
#define DEFAULT_BUFFER  128
UBFH *M_p_ub = NULL;


char M_test_temp_filename[]="/tmp/ubf-test-com-XXXXXX";
FILE *M_test_temp_file=NULL;
int M_has_init = EXFALSE;

/**
 * Open test file for write
 */
void open_test_temp(char *mode)
{
    NDRX_STRCPY_SAFE(M_test_temp_filename, "/tmp/ubf-test-com-XXXXXX");
    assert_not_equal(mkstemp(M_test_temp_filename), EXFAIL);
    assert_not_equal((M_test_temp_file=fopen(M_test_temp_filename, mode)), NULL);
}

/**
 * Open temp file for reading
 */
void open_test_temp_for_read(char *mode)
{
    assert_not_equal((M_test_temp_file=fopen(M_test_temp_filename, mode)), NULL);
}

/**
 * Write to temp file
 * @param data
 */
void write_to_temp(char **data)
{
    int i;
    for (i=0; NULL!=data[i]; i++)
    {
        assert_equal(fwrite (data[i] , 1 , strlen(data[i]) , M_test_temp_file ), strlen(data[i]));
    }
}

/**
 * Close temp file
 */
void close_test_temp(void)
{
    fclose(M_test_temp_file);
}

/**
 * Remove temp file
 */
void remove_test_temp(void)
{
    /* Remove test file */
    assert_equal(unlink(M_test_temp_filename), EXSUCCEED);
}

/**
 * Set up field table environment
 */
void load_field_table(void)
{
    if (!M_has_init)
    {
        setenv("FLDTBLDIR", "./ubftab", 1);
        setenv("FIELDTBLS", "test.fd,Exfields", 1);

        setenv("VIEWDIR", "../libextest", 1);
        setenv("VIEWFILES", "test_view.V", 1);
        M_has_init=EXTRUE;
    }
    
}

/**
 * Basic preparation before the test
 */
void std_basic_setup(void)
{
    /* set view env... */
    load_field_table();

    setenv("NDRX_APIFLAGS", "ubf_ptrparse", 1);
}

/**
 * Fill up buffer with something so that we have more interesting stuff there
 * bigger buffer, etc... (incl ptr, view, ubf)
 * @param p_ub
 */
void set_up_dummy_data(UBFH *p_ub)
{
    load_field_table();
    extest_ubf_set_up_dummy_data(p_ub, EXTEST_PROC_PTR|EXTEST_PROC_UBF|EXTEST_PROC_VIEW);
}

/**
 * Test the dummy data (incl ptr, view, ubf)
 * @param p_ub ubf buffer to validate
 */
void do_dummy_data_test(UBFH *p_ub)
{
    extest_ubf_do_dummy_data_test(p_ub, EXTEST_PROC_PTR|EXTEST_PROC_UBF|EXTEST_PROC_VIEW);
}

/**
 * Load ptr data
 * @param p_ub buffer where to load
 * @param occ occurrence of data
 * @param offset data offset (value to change from base)
 * @param fldoff field offset (different field set)
 */
void gen_load_ptr_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff)
{
    long ptr=9000+offset;   
    UBF_LOG(log_debug, "Asserting %s:%d, start", file, line);
    assert_equal(CBchg(p_ub, T_PTR_FLD+fldoff, occ, (char *)&ptr, 0, BFLD_LONG), EXSUCCEED);
    UBF_LOG(log_debug, "Asserting %s:%d, done", file, line);
}

/**
 * Set view data, load into static..
 * @param file dbg file
 * @param line db line
 * @param vf view field struct for output
 * @param offset data offset for field..
 */
void gen_set_view_dbg(char *file, int line, BVIEWFLD *vf, int offset)
{
    static struct UBTESTVIEW2 v;
    char str[2];
    
    memset(&v, 9, sizeof(v));
    v.tshort1=1+offset;
    v.tlong1=2+offset;
    v.tchar1='3'+offset;
    v.tfloat1=4+offset;
    v.tdouble1=5+offset;
    str[0]='A'+offset;
    str[1]=EXEOS;
    
    NDRX_STRCPY_SAFE(v.tstring1, str);
    str[0]='C'+offset;
    str[1]=EXEOS;
    
    NDRX_STRCPY_SAFE(v.tcarray1, str);
    memset(vf, 0, sizeof(BVIEWFLD));
    vf->data=(char *)&v;
    vf->vflags=0;
    NDRX_STRCPY_SAFE(vf->vname, "UBTESTVIEW2");
    
}

/**
 * Load test data for view
 * @param p_ub buffer where to load
 * @param occ occurrence of data
 * @param offset data offset (value to change from base)
 * @param fldoff field offset (different field set)
 */
void gen_load_view_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff)
{
    BVIEWFLD vf;
    
    UBF_LOG(log_debug, "Loading %s:%d, start", file, line);
    
    gen_set_view_dbg(file, line, &vf, offset);
    
    assert_equal(Bchg(p_ub, T_VIEW_FLD+fldoff, occ, (char *)&vf, 0L), EXSUCCEED);
    UBF_LOG(log_debug, "Loading %s:%d, end", file, line);
}

/**
 * Setup the UBF buffer for value
 * @param file source file
 * @param line source line
 * @param p_ub_tmp buffer to init
 * @param offset data offset
 */
void gen_set_ubf_dbg(char *file, int line, UBFH *p_ub_tmp, int offset)
{
    char str[2];
    str[0]='S'+offset;
    str[1]=EXEOS;
    assert_equal(Bchg(p_ub_tmp, T_STRING_FLD+offset, 0, str, 0), EXSUCCEED);   
}

/**
 * Load test data for ubf
 * @param p_ub buffer where to load
 * @param occ occurrence of data
 * @param offset data offset (value to change from base)
 * @param fldoff field offset (different field set)
 */
void gen_load_ubf_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff)
{
    char tmp[1024];
    UBFH* p_ub_tmp=(UBFH*)tmp;
    memset(tmp, 17, sizeof(tmp));
    
    
    UBF_LOG(log_debug, "Loading UBF %s:%d, start", file, line);
    
    assert_equal(Binit(p_ub_tmp, sizeof(tmp)), EXSUCCEED);
    
    /* Load some sub-field */
    gen_set_ubf_dbg(file, line, p_ub_tmp, offset);
    assert_equal(Bchg(p_ub, T_UBF_FLD+fldoff, occ, (char *)p_ub_tmp, 0), EXSUCCEED);
    UBF_LOG(log_debug, "Loading UBF %s:%d, end", file, line);
}

/**
 * Validate ptr data
 * @param p_ub
 * @param occ
 * @param offset
 * @param fldoff UBF field offset
 */
void gen_test_ptr_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff)
{
    ndrx_longptr_t ptr;
    BFLDLEN len=sizeof(ptr);
    assert_equal(CBget(p_ub, T_PTR_FLD+fldoff, occ, (char *)&ptr, &len, BFLD_PTR), EXSUCCEED);
    gen_test_ptr_val_dbg(file, line, ptr, offset, &len);
}


/**
 * Validate the data is OK
 * @param p_ub
 * @param occ
 * @param offset
 * @param fldoff UBF field offset
 */
void gen_test_view_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff)
{
    struct UBTESTVIEW2 v;
    BVIEWFLD vf;
    BFLDLEN len;
    
    memset(&v, 13, sizeof(v));

    UBF_LOG(log_debug, "Asserting %s:%d, start", file, line);
    
    /* View test */
    vf.data=(char *)&v;
    len=sizeof(v)-1;
    assert_equal(Bget(p_ub, T_VIEW_FLD+fldoff, occ, (char *)&vf, &len), EXFAIL);
    assert_equal(Berror, BNOSPACE);
    
    len=sizeof(v);
    assert_equal(Bget(p_ub, T_VIEW_FLD+fldoff, occ, (char *)&vf, &len), EXSUCCEED);
    
    /* Check value of view fields... */
    gen_test_view_val_dbg(file, line, &vf, offset, &len);
    
    UBF_LOG(log_debug, "Asserting %s:%d, end", file, line);
}


/**
 * Validate the ubf data is OK
 * @param p_ub
 * @param occ
 * @param offset
 * @param fldoff UBF field offset
 */
void gen_test_ubf_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff)
{
    char tmp[1024];
    UBFH* p_ub_tmp=(UBFH*)tmp;
    BFLDLEN len;
    
    UBF_LOG(log_debug, "Asserting %s:%d, start", file, line);
    
    /* UBF test */
    len=1;
    assert_equal(Bget(p_ub, T_UBF_FLD+fldoff, occ, (char *)p_ub_tmp, &len), EXFAIL);
    assert_equal(Berror, BNOSPACE);
    
    len=sizeof(tmp);
    assert_equal(Bget(p_ub, T_UBF_FLD+fldoff, occ, (char *)p_ub_tmp, &len), EXSUCCEED);
    
    
    gen_test_ubf_val_dbg(file, line, p_ub_tmp, offset, &len);
    
    
    UBF_LOG(log_debug, "Asserting %s:%d, end", file, line);
}

/**
 * Test ptr value
 * @param file src file
 * @param line src line
 * @param ptr data ptr
 * @param offset data offset to test
 * @param len optional len
 */
void gen_test_ptr_val_dbg(char *file, int line, ndrx_longptr_t ptr,  int offset, BFLDLEN *len)
{
    UBF_LOG(log_debug, "Asserting %s:%d, start", file, line);
    
    if (NULL!=len)
    {
        assert_equal(*len, sizeof(ndrx_longptr_t));
    }
    
    assert_equal(ptr, 9000+offset);
    UBF_LOG(log_debug, "Asserting %s:%d, end", file, line);
}

/**
 * Validate the data is OK
 * @param p_ub
 * @param occ
 * @param offset
 * @param fldoff UBF field offset
 * @param len ptr to optional len
 */
void gen_test_view_val_dbg(char *file, int line, BVIEWFLD *vf, int offset, BFLDLEN *len)
{
    struct UBTESTVIEW2 *v=(struct UBTESTVIEW2 *)vf->data;
    char str[2];
    
    UBF_LOG(log_debug, "Asserting %s:%d, start", file, line);
    
    if (NULL!=len)
    {
        assert_equal(*len, sizeof(struct UBTESTVIEW2));
    }

    /* Check value of view fields... */
    
    assert_equal(v->tshort1, 1+offset);
    assert_equal(v->tlong1, 2+offset);
    assert_equal(v->tchar1, '3'+offset);
    assert_equal(v->tfloat1, 4+offset);
    assert_equal(v->tdouble1, 5+offset);
    
    str[0]='A'+offset;
    str[1]=EXEOS;
    assert_string_equal(v->tstring1,str);
    
    str[0]='C'+offset;
    str[1]=EXEOS;
    assert_string_equal(v->tcarray1, str);
    assert_string_equal(vf->vname, "UBTESTVIEW2");
    
    UBF_LOG(log_debug, "Asserting %s:%d, end", file, line);
}


/**
 * Check ubf value
 * @param file src file
 * @param line src line
 * @param p_ub UBF value to test
 * @param offset value offset
 * @param len ptr to len (optional)
 */
void gen_test_ubf_val_dbg(char *file, int line, UBFH *p_ub, int offset, BFLDLEN *len)
{
    char str[2];
    
    UBF_LOG(log_debug, "Asserting %s:%d, start", file, line);

    /* check minimum UBF size */
    if (NULL!=len)
    {
        assert_equal( !!(*len >= sizeof(UBF_header_t)), EXTRUE);
    }

    /* UBF test */
    str[0]='S'+offset;
    str[1]=EXEOS;
    assert_string_equal(Bfind(p_ub, T_STRING_FLD+offset, 0, 0L), str);
    
    UBF_LOG(log_debug, "Asserting %s:%d, end", file, line);
}

/**
 * Basic preparation before the test
 */
void basic_setup(void)
{
    /*printf("basic_setup\n");*/
    
    if (NULL!=M_p_ub)
    {
        NDRX_FREE(M_p_ub);
    }
    M_p_ub = NDRX_MALLOC(DEFAULT_BUFFER);
    memset(M_p_ub, 255, DEFAULT_BUFFER);
    if (EXFAIL==Binit(M_p_ub, DEFAULT_BUFFER))
    {
        fprintf(stderr, "Binit failed!\n");
    }

    load_field_table();
    
}

void basic_teardown(void)
{
    /*printf("basic_teardown\n"); */
    
    if (NULL!=M_p_ub)
    {
        NDRX_FREE(M_p_ub);
    }
}

/**
 * Base Binit test
 */
Ensure(test_Binit) {
    char tmpbuf[1024];
    UBFH * p_ub =  (UBFH *) tmpbuf;
    /* Check basic Binit */
    assert_equal(Binit(p_ub, 1024), EXSUCCEED);
    /* Check the size of Binit */
    assert_equal(Bsizeof(p_ub), 1024);
}

/**
 * Basic Field table tests
 */
Ensure(test_fld_table)
{
    assert_equal(strcmp(Bfname(T_STRING_FLD), "T_STRING_FLD"), 0);
    assert_equal(Bfldid("T_STRING_FLD"), T_STRING_FLD);
    
    assert_equal(Bfname(-1), NULL);
    
}

/**
 * NOTE: These are running in 16 bit mode (currently, unless we recompiled the
 * header files for fields.
 *
 * Or replace with generic numbers?
 */
Ensure(test_Bmkfldid)
{
    /* Short */
    assert_equal(Bmkfldid(BFLD_SHORT, 1021), T_SHORT_FLD);
    /* Long */
    assert_equal(Bmkfldid(BFLD_LONG, 1031), T_LONG_FLD);
    /* Char */
    assert_equal(Bmkfldid(BFLD_CHAR, 1011), T_CHAR_FLD);
    /* Float */
    assert_equal(Bmkfldid(BFLD_FLOAT, 1041), T_FLOAT_FLD);
    /* Double */
    assert_equal(Bmkfldid(BFLD_DOUBLE, 1051), T_DOUBLE_FLD);
    /* String */
    assert_equal(Bmkfldid(BFLD_STRING, 1061), T_STRING_FLD);
    /* Carray */
    assert_equal(Bmkfldid(BFLD_CARRAY, 1081), T_CARRAY_FLD);
}

Ensure(test_Bmkfldid_multidir)
{
    /* load_field_table(); */
    setenv("FLDTBLDIR", "./ubftab_test:./ubftab", 1);
    setenv("FIELDTBLS", "test.fd,test1.fd,Exfields", 1);
    
    /* from test.fd */
    assert_equal(strcmp(Bfname(T_STRING_FLD), "T_STRING_FLD"), 0);
    /* from test1.fd */
    /* Char */
    assert_equal(Bmkfldid(BFLD_CHAR, 1111), T1_CHAR_FLD);
    assert_equal(strcmp(Bfname(T1_CHAR_FLD), "T1_CHAR_FLD"), 0);
    /* Long */
    assert_equal(Bmkfldid(BFLD_LONG, 1118), T1_LONG_2_FLD);
    assert_equal(strcmp(Bfname(T1_LONG_2_FLD), "T1_LONG_2_FLD"), 0);
    /* Short */
    assert_equal(Bmkfldid(BFLD_SHORT, 1116), T1_SHORT_3_FLD);
    assert_equal(strcmp(Bfname(T1_SHORT_3_FLD), "T1_SHORT_3_FLD"), 0);
    /* Float */
    assert_equal(Bmkfldid(BFLD_FLOAT, 1122), T1_FLOAT_3_FLD);
    assert_equal(strcmp(Bfname(T1_FLOAT_3_FLD), "T1_FLOAT_3_FLD"), 0);
    /* Double */
    assert_equal(Bmkfldid(BFLD_DOUBLE, 1126), T1_DOUBLE_4_FLD);
    assert_equal(strcmp(Bfname(T1_DOUBLE_4_FLD), "T1_DOUBLE_4_FLD"), 0);
    /* String */
    assert_equal(Bmkfldid(BFLD_STRING, 1130), T1_STRING_4_FLD);
    assert_equal(strcmp(Bfname(T1_STRING_4_FLD), "T1_STRING_4_FLD"), 0);
    /* Carray */
    assert_equal(Bmkfldid(BFLD_CARRAY, 1137), T1_CARRAY_FLD);
    assert_equal(strcmp(Bfname(T1_CARRAY_FLD), "T1_CARRAY_FLD"), 0);
}

/**
 * Test function the returns field number out of ID
 */
Ensure(test_Bfldno)
{
       /* Short */
    assert_equal(Bfldno(T_SHORT_FLD), 1021);
    /* Long */
    assert_equal(Bfldno(T_LONG_FLD), 1031);
    /* Char */
    assert_equal(Bfldno(T_CHAR_FLD), 1011);
    /* Float */
    assert_equal(Bfldno(T_FLOAT_FLD), 1041);
    /* Double */
    assert_equal(Bfldno(T_DOUBLE_FLD), 1051);
    /* String */
    assert_equal(Bfldno(T_STRING_FLD), 1061);
    /* Carray */
    assert_equal(Bfldno(T_CARRAY_FLD), 1081);
}

Ensure(test_Btype)
{
    assert_string_equal(Btype(T_SHORT_FLD), "short");
    assert_string_equal(Btype(T_LONG_FLD), "long");
    assert_string_equal(Btype(T_CHAR_FLD), "char");
    assert_string_equal(Btype(T_FLOAT_FLD), "float");
    assert_string_equal(Btype(T_DOUBLE_FLD), "double");
    assert_string_equal(Btype(T_STRING_FLD), "string");
    assert_string_equal(Btype(T_CARRAY_FLD), "carray");
    
    assert_string_equal(Btype(T_PTR_FLD), "ptr");
    assert_string_equal(Btype(T_UBF_FLD), "ubf");
    assert_string_equal(Btype(T_VIEW_FLD), "view");
    
    assert_string_equal(Btype(0xffffffff), NULL);
    assert_equal(Berror, BTYPERR);
}

/**
 * Test Bisubf function
 */
Ensure(test_Bisubf)
{
    char tmpbuf[72];
    UBFH * p_ub =  (UBFH *) tmpbuf;
    /* Check basic Binit */
    assert_equal(Binit(p_ub, sizeof(tmpbuf)), EXSUCCEED);
    assert_equal(Bisubf(p_ub), EXTRUE);
    memset(p_ub, 0, sizeof(tmpbuf));
    assert_equal(Bisubf(p_ub), EXFALSE);
    /* Error should not be set */
    assert_equal(Berror, BMINVAL);
}

/**
 * Test Bsizeof
 */
Ensure(test_Bsizeof)
{
    char tmpbuf[72];
    UBFH * p_ub =  (UBFH *) tmpbuf;

    assert_equal(Binit(p_ub, sizeof(tmpbuf)), EXSUCCEED);
    assert_equal(Bsizeof(p_ub), sizeof(tmpbuf));

}

/**
 * Test buffer usage.
 */
Ensure(test_Bunused)
{
    char tmpbuf[76]; /* +2 for short align, +4 for string dlen */
    short s;
    UBFH * p_ub =  (UBFH *) tmpbuf;

    /* Check basic Binit */
    assert_equal(Binit(p_ub, sizeof(tmpbuf)), EXSUCCEED);
#if EX_ALIGNMENT_BYTES == 8
    assert_equal(Bunused(p_ub), sizeof(tmpbuf) - sizeof(UBF_header_t) + sizeof(BFLDID)*2);
#else
    assert_equal(Bunused(p_ub), sizeof(tmpbuf) - sizeof(UBF_header_t) + sizeof(BFLDID));
#endif
    
#if EX_ALIGNMENT_BYTES != 8
    /* Add some field and then see what happens */
    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bunused(p_ub), sizeof(tmpbuf) - sizeof(UBF_header_t)-sizeof(s)-2/* align of short */);
    /* fill up to zero */
    assert_equal(Bchg(p_ub, T_STRING_FLD, 0, "abc", 0), EXSUCCEED);
    assert_equal(Bunused(p_ub), 0);
#endif
}


/**
 * Simple Blen test.
 */
Ensure(test_Blen)
{
    char fb[1024];
    UBFH *p_ub = (UBFH *)fb;

    short s = 88;
    long l = -1021;
    char c = 'c';
    float f = 17.31;
    double d = 12312.1111;
    char carr[] = "CARRAY1 TEST STRING DATA";
    BFLDLEN len = strlen(carr);
    char *str="TEST STR VAL";
    
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 1, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 1, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 1, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 1, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 1, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 1, (char *)str, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 1, (char *)carr, len), EXSUCCEED);
    
    
    assert_equal(Blen(p_ub, T_SHORT_FLD, 1), sizeof(s));
    assert_equal(Blen(p_ub, T_LONG_FLD, 1), sizeof(l));
    assert_equal(Blen(p_ub, T_CHAR_FLD, 1), sizeof(c));
    assert_equal(Blen(p_ub, T_FLOAT_FLD, 1), sizeof(f));
    assert_equal(Blen(p_ub, T_DOUBLE_FLD, 1), sizeof(d));
    assert_equal(Blen(p_ub, T_STRING_FLD, 1), strlen(str)+1);
    assert_equal(Blen(p_ub, T_CARRAY_FLD, 1), len);
    
    assert_equal(Blen(p_ub, T_CARRAY_FLD, 2), EXFAIL);
    assert_equal(Berror, BNOTPRES);
}

/**
 * This tests so that buffer terminats with BBADFLDID
 * and then pre-last item is the data.
 */
Ensure(test_buffer_align_fadd)
{
#if 0
    /* Cannot test this directly because of align... */
    char buf[1024];
    UBFH *p_ub = (UBFH *)buf;
    short data=0xffff;
    int *err;
    char *p = buf+1024-sizeof(BFLDID);
    BFLDID *check = (BFLDID *)p;
    short *short_check;
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    while (EXSUCCEED==Badd(p_ub, T_SHORT_FLD, (char *)&data, 0)){}
    /* check that buffer is full */
    err = ndrx_Bget_Ferror_addr();
    assert_equal(*err, BNOSPACE);
    /* Check last element */
    assert_equal(*check, BBADFLDID);
    /* Pre last must data! */
    p = buf+1024-sizeof(BFLDID)-sizeof(short)-4;
    short_check = (short *)p;
    assert_equal(*short_check, data);
#endif
}

/**
 * Test is not actual anymore - we do not end with BBADFLD
 * - we operation with actual length of the buffer to find the EOF
 * ---------------------------------------------------------------
 * This tests so that buffer terminats with BBADFLDID
 * and then pre-last item is the data.
 * Do test with Bchg
 * Tests Bpres and Boccur
 */
Ensure(test_buffer_align_fchg_and_fpresocc)
{
    char buf[1024];
    UBFH *p_ub = (UBFH *)buf;
    short data=0xffff;
    int *err;
    char *p = buf+1024-sizeof(BFLDID);
    BFLDID *check = (BFLDID *)p;
    short *short_check;
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    BFLDOCC occ=0;
    int i;

    while (EXSUCCEED==Bchg(p_ub, T_SHORT_FLD, occ, (char *)&data, 0)){occ++;}
    /* check that buffer is full */
    err = ndrx_Bget_Ferror_addr();
    assert_equal(*err, BNOSPACE);
    /* Check last element */
    assert_equal(*check, BBADFLDID);
    /* Pre last must data! */
#if 0
    /* Due to align it is hard to check */
    p = buf+1024-sizeof(BFLDID)-sizeof(short)-4;
    short_check = (short *)p;
    assert_equal(*short_check, data);
#endif
    /* test the Boccur */
    assert_equal(Boccur(p_ub, T_SHORT_FLD), occ);
    /* If not found, then 0 */
    assert_equal(Boccur(p_ub, T_LONG_FLD), 0);

    /* Check that every field is present */
    for (i=0; i<occ; i++)
    {
        assert_equal(Bpres(p_ub, T_SHORT_FLD, i), EXTRUE);
    }
    /* check for non existing, should fail */
    assert_equal(Bpres(p_ub, T_SHORT_FLD, occ), EXFALSE);
}

/**
 * Basically we should test all API functions here which operate with FB!
 * This also seems to be not valid... We do not end with BADFLDID anymore.
 */
Ensure(test_buffer_alignity)
{
    char buf[1024];
    int short_v;
    UBFH *p_ub = (UBFH *)buf;
    BFLDID bfldid=BBADFLDID;
    BFLDOCC occ;
    
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    memset(buf+sizeof(UBF_header_t)-sizeof(BFLDID), 0xff,
            sizeof(buf)-sizeof(UBF_header_t)+sizeof(BFLDID));

    assert_equal(Bget(p_ub, T_SHORT_FLD, 0, (char *)&short_v, 0), EXFAIL);
    assert_equal(Berror, BALIGNERR);
    assert_equal(Badd(p_ub, T_SHORT_FLD, (char *)&short_v, 0), EXFAIL);
    assert_equal(Berror, BALIGNERR);
    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&short_v, 0), EXFAIL);
    assert_equal(Berror, BALIGNERR);
    assert_equal(Bdel(p_ub, T_SHORT_FLD, 0), EXFAIL);
    assert_equal(Berror, BALIGNERR);
    assert_equal(Bnext(p_ub, &bfldid, &occ, NULL, 0), EXFAIL);
    assert_equal(Berror, BALIGNERR);
}

/**
 * Very basic tests of the framework
 * @return
 */
TestSuite *ubf_basic_tests() {
    TestSuite *suite = create_test_suite();
    
    set_setup(suite, basic_setup);
    set_teardown(suite, basic_teardown);

    /*  UBF init test */
    add_test(suite, test_Binit);
    add_test(suite, test_fld_table);
    add_test(suite, test_Bmkfldid);
    add_test(suite, test_Bfldno);
/* no more for new processing priciples of bytes used.
    add_test(suite, test_buffer_align_fadd);
    add_test(suite, test_buffer_align_fchg_and_fpresocc);
*/
/*
    - not valid any more the trailer might non zero
    add_test(suite, test_buffer_alignity);
*/
    add_test(suite, test_Bisubf);
    add_test(suite, test_Bunused);
    add_test(suite, test_Bsizeof);
    add_test(suite, test_Btype);
    add_test(suite, test_Blen);

    return suite;
}

/**
 * Testing bmkfldid with multi directory in FLDTBLDIR environment
 * @return 
 */
TestSuite *ubf_bmkfldid_multidir_tests(void) {
    TestSuite *suite = create_test_suite();
    

    add_test(suite, test_Bmkfldid_multidir);

    return suite;
}

/*
 * Main test entry.
 */
int main(int argc, char** argv)
{    
    TestSuite *suite = create_test_suite();
    int ret;

    /*
     * NSTD Library tests
     */
    add_suite(suite, ubf_nstd_atomicadd());
    add_suite(suite, ubf_nstd_fpa());
    add_suite(suite, ubf_nstd_standard());
    add_suite(suite, ubf_nstd_util());
    add_suite(suite, ubf_nstd_debug());
    add_suite(suite, ubf_nstd_lh());
    add_suite(suite, test_nstd_macros());
    add_suite(suite, ubf_nstd_crypto());
    add_suite(suite, ubf_nstd_base64());
    add_suite(suite, ubf_nstd_growlist());
    
    add_suite(suite, ubf_nstd_mtest());
    add_suite(suite, ubf_nstd_mtest2());
    add_suite(suite, ubf_nstd_mtest3());
    add_suite(suite, ubf_nstd_mtest4());
    add_suite(suite, ubf_nstd_mtest5());
    add_suite(suite, ubf_nstd_mtest6_dupcursor());
    add_suite(suite, ubf_nstd_mtest6());
    add_suite(suite, ubf_nstd_mtest7());
    add_suite(suite, ubf_nstd_fsync());
    add_suite(suite, ubf_nstd_cid());
    
    /*
     * UBF tests
     */
    add_suite(suite, ubf_basic_tests());
    add_suite(suite, ubf_Badd_tests());
    add_suite(suite, ubf_genbuf_tests());
    add_suite(suite, ubf_cfchg_tests());
    add_suite(suite, ubf_cfget_tests());
    add_suite(suite, ubf_fdel_tests());
    add_suite(suite, ubf_expr_tests());
    add_suite(suite, ubf_fnext_tests());
    add_suite(suite, ubf_fproj_tests());
    add_suite(suite, ubf_mem_tests());
    add_suite(suite, ubf_fupdate_tests());
    add_suite(suite, ubf_fconcat_tests());
    add_suite(suite, ubf_find_tests());
    add_suite(suite, ubf_get_tests());
    add_suite(suite, ubf_print_tests());
    add_suite(suite, ubf_printv_tests());
    add_suite(suite, ubf_macro_tests());
    add_suite(suite, ubf_readwrite_tests());
    add_suite(suite, ubf_mkfldhdr_tests());
    add_suite(suite, ubf_bcmp_tests());
    add_suite(suite, ubf_bnum_tests());
    add_suite(suite, ubf_bjoin_tests());
    add_suite(suite, ubf_bojoin_tests());
    add_suite(suite, ubf_bmkfldid_multidir_tests());
    add_suite(suite, ubf_embubf_tests());

    if (argc > 1)
    {
        ret=run_single_test(suite,argv[1],create_text_reporter());
    }
    else
    {
        ret=run_test_suite(suite, create_text_reporter());
    }

    destroy_test_suite(suite);

    return ret;
    
}
/* vim: set ts=4 sw=4 et smartindent: */
