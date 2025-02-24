/**
 * @brief Contains routines for testing find kind functions
 *
 * @file test_find.c
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
#include <cgreen/cgreen.h>
#include <ubf.h>
#include <ndrstandard.h>
#include <string.h>
#include "test.fd.h"
#include "ubfunit1.h"


void load_find_test_data(UBFH *p_ub)
{
    short s = 88;
    long l = -1021;
    char c = 'c';
    float f = 17.31;
    double d = 12312.1111;
    char carr[] = "CARRAY1 TEST STRING DATA";
    BFLDLEN len = strlen(carr);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 0, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 0, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 0, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 0, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 0, (char *)"TEST STR VAL", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 0, (char *)carr, len), EXSUCCEED);
    
    gen_load_ubf(p_ub, 0, 0, 0);
    gen_load_view(p_ub, 0, 0, 0);
    gen_load_ptr(p_ub, 0, 0, 0);
    

    /* Make second copy of field data (another for not equal test)*/
    s = 88;
    l = -1021;
    c = '.';
    f = 17.31;
    d = 12312.1111;
    carr[0] = 'Y';
    len = strlen(carr);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 1, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 1, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 1, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 1, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 1, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 1, (char *)"TEST STRING ARRAY2", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 1, (char *)carr, len), EXSUCCEED);
    
    gen_load_ubf(p_ub, 1, 1, 0);
    gen_load_view(p_ub, 1, 1, 0);
    gen_load_ptr(p_ub, 1, 1, 0);
    

    l = 888;
    assert_equal(Bchg(p_ub, T_LONG_FLD, 4, (char *)&l, 0), EXSUCCEED);

    s = 212;
    l = 212;
    c = 'b';
    f = 12127;
    d = 1231232.1;
    carr[0] = 'X';
    assert_equal(Bchg(p_ub, T_SHORT_2_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_2_FLD, 0, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_2_FLD, 0, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_2_FLD, 0, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_2_FLD, 0, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_2_FLD, 0, (char *)"XTEST STR VAL", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_2_FLD, 0, (char *)carr, len), EXSUCCEED);
    
    gen_load_ubf(p_ub, 0, 2, 1);
    gen_load_view(p_ub, 0, 2, 1);
    gen_load_ptr(p_ub, 0, 2, 1);
}

/**
 * Test data used for CBfindocc
 * @param p_ub
 */
void load_bfindocc_test_data(UBFH *p_ub)
{
    short s = 881;
    long l = -1011;
    char c = 'a';
    float f = 1.31;
    double d = 1312.1111;
    char carr[] = "ZARRAY1 TEST STRING DATA";
    BFLDLEN len = strlen(carr);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 0, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 0, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 0, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 0, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 0, (char *)"TEST STR VAL", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 0, (char *)carr, len), EXSUCCEED);

    gen_load_ubf(p_ub, 0, 3, 0);
    gen_load_view(p_ub, 0, 3, 0);
    gen_load_ptr(p_ub, 0, 3, 0);
    
    /* Make second copy of field data (another for not equal test)*/
    s = 88;
    l = -1021;
    c = '.';
    f = 17.31;
    d = 12312.1111;
    carr[0] = '\0';
    len = 24;

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 1, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 1, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 1, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 1, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 1, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 1, (char *)"TEST STRING ARRAY2", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 1, (char *)carr, len), EXSUCCEED);

    gen_load_ubf(p_ub, 1, 4, 0);
    gen_load_view(p_ub, 1, 4, 0);
    gen_load_ptr(p_ub, 1, 4, 0);
    
    l = 888;
    assert_equal(Bchg(p_ub, T_LONG_FLD, 4, (char *)&l, 0), EXSUCCEED);

    s = 212;
    l = 212;
    c = 'b';
    f = 12127;
    d = 1231232.1;
    carr[0] = 'X';
    assert_equal(Bchg(p_ub, T_SHORT_2_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_2_FLD, 0, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_2_FLD, 0, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_2_FLD, 0, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_2_FLD, 0, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_2_FLD, 0, (char *)"XTEST STR VAL", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_2_FLD, 0, (char *)carr, len), EXSUCCEED);
    
    gen_load_ubf(p_ub, 0, 5, 1);
    gen_load_view(p_ub, 0, 5, 1);
    gen_load_ptr(p_ub, 0, 5, 1);
    
}


/**
 * Test data used for Bfindlast
 * @param p_ub
 */
void load_bfindlast_test_data(UBFH *p_ub)
{
    short s = 88;
    long l = -1021;
    char c = 'c';
    float f = 17.31;
    double d = 12312.1111;
    char carr[] = "CARRAY1 TEST STRING DATA";
    BFLDLEN len = strlen(carr);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 1, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 2, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 3, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 4, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 5, (char *)"TEST STR VAL", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 6, (char *)carr, len), EXSUCCEED);
    
    gen_load_ptr(p_ub, 7, 6, 0);
    gen_load_ubf(p_ub, 8, 6, 0);
    gen_load_view(p_ub, 9, 6, 0);
    
}

/* dded tests for Bfind */
Ensure(test_bfind)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    BFLDLEN len=0;
    char *p;

    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    load_find_test_data(p_ub);
    
    assert_equal(*((short*)Bfind(p_ub, T_SHORT_FLD, 0, &len)), 88);
    assert_equal(len, sizeof(short));
    
    assert_equal(*((long*)Bfind(p_ub, T_LONG_FLD, 0, &len)), -1021);
    assert_equal(len, sizeof(long));
    
    assert_equal(*((char*)Bfind(p_ub, T_CHAR_FLD, 0, &len)), 'c');
    assert_equal(len, sizeof(char));
    
    assert_double_equal(*((float*)Bfind(p_ub, T_FLOAT_FLD, 0, &len)), 17.31);
    assert_equal(len, sizeof(float));

    assert_string_equal(Bfind(p_ub, T_STRING_FLD, 0, &len), "TEST STR VAL");
    assert_equal(len, strlen("TEST STR VAL")+1);

    p=Bfind(p_ub, T_CARRAY_FLD, 0, &len);
    assert_not_equal(p, NULL);
    assert_equal(memcmp(p, "CARRAY1 TEST STRING DATA", 24), 0);
    assert_equal(len, 24);

    p=Bfind(p_ub, T_PTR_FLD, 0, &len);
    assert_not_equal(p, NULL);
    gen_test_ptr_val_dbg(__FILE__, __LINE__, *((ndrx_longptr_t *)p), 0, &len);
    
    p=Bfind(p_ub, T_VIEW_FLD, 0, &len);
    assert_not_equal(p, NULL);
    gen_test_view_val_dbg(__FILE__, __LINE__, (BVIEWFLD *)p, 0, &len);

    /* test ubf... */
    p=Bfind(p_ub, T_UBF_FLD, 0, &len);
    assert_not_equal(p, NULL);
    gen_test_ubf_val_dbg(__FILE__, __LINE__, (UBFH *)p, 0, &len);
    
}

/**
 * This simply reads all field and adds them to another buffer, then do compare
 */
Ensure(test_cbfind)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    short *s;
    long *l;
    char *c;
    float *f;
    double *d;
    char *str;
    char *carr;
    BFLDLEN len=0;

    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    load_find_test_data(p_ub);

    /* Test as short */
    assert_equal(*(s=(short *)CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_SHORT)), 88);
    assert_equal(*(s=(short *)CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_SHORT)), -1021);
    assert_equal(*(s=(short *)CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_SHORT)), 99);
    assert_equal(*(s=(short *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_SHORT)), 17);
    assert_equal(*(s=(short *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_SHORT)), 12312);
    assert_equal(*(s=(short *)CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_SHORT)), 0);
    assert_equal(*(s=(short *)CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_SHORT)), 0);
    
    assert_equal(*(s=(short *)CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_SHORT)), 9000);
    
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_SHORT), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_SHORT), NULL);
    assert_equal(Berror, BEBADOP);
    
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_SHORT)), 88);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_SHORT)), -1021);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_SHORT)), 99);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_SHORT)), 17);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_SHORT)), 12312);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_SHORT)), 0);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_SHORT)), 0);
    assert_equal(len, sizeof(short));
    
    
    len=0;
    assert_equal(*(s=(short *)CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_SHORT)), 9000);
    assert_equal(len, sizeof(short));
    
    len=0;
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, &len, BFLD_SHORT), NULL);
    assert_equal(Berror, BEBADOP);
    
    len=0;
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, &len, BFLD_SHORT), NULL);
    assert_equal(Berror, BEBADOP);
    

    /* Test as long */
    assert_equal(*(l=(long *)CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_LONG)), 88);
    assert_equal(*(l=(long *)CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_LONG)), -1021);
    assert_equal(*(l=(long *)CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_LONG)), 99);
    assert_equal(*(l=(long *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_LONG)), 17);
    assert_equal(*(l=(long *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_LONG)), 12312);
    assert_equal(*(l=(long *)CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_LONG)), 0);
    assert_equal(*(l=(long *)CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_LONG)), 0);
    
    assert_equal(*(l=(long *)CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_LONG)), 9000);
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_LONG), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_LONG), NULL);
    assert_equal(Berror, BEBADOP);
    
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_LONG)), 88);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_LONG)), -1021);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_LONG)), 99);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_LONG)), 17);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_LONG)), 12312);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_LONG)), 0);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_LONG)), 0);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(*(l=(long *)CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_LONG)), 9000);
    assert_equal(len, sizeof(long));
    
    len=0;
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, &len, BFLD_LONG), NULL);
    assert_equal(Berror, BEBADOP);
    
    len=0;
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, &len, BFLD_LONG), NULL);
    assert_equal(Berror, BEBADOP);
    
    
    /* Test as char */
    assert_equal(*(c=(char *)CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_CHAR)), 'X');
    assert_equal(*(c=(char *)CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_CHAR)), 3); /* may be incorrect due to data size*/
    assert_equal(*(c=(char *)CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_CHAR)), 'c');
    assert_equal(*(c=(char *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_CHAR)), 17);
    assert_equal(*(c=(char *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_CHAR)), 24); /* May be incorrect dute to data size*/
    assert_equal(*(c=(char *)CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_CHAR)), 'T');
    assert_equal(*(c=(char *)CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_CHAR)), 'C');
    assert_equal(*(c=(char *)CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_CHAR)), 40);
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_CHAR), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_CHAR), NULL);
    assert_equal(Berror, BEBADOP);
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_CHAR)), 'X');
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_CHAR)), 3); /* may be incorrect due to data size*/
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_CHAR)), 'c');
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_CHAR)), 17);
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_CHAR)), 24); /* May be incorrect dute to data size*/
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_CHAR)), 'T');
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_CHAR)), 'C');
    assert_equal(len, sizeof(char));
    
    len=0;
    assert_equal(*(c=(char *)CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_CHAR)), 40);
    assert_equal(len, sizeof(char));
    

    /* Test as float */
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_FLOAT)), 88);
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_FLOAT)), -1021);
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_FLOAT)), 99);
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_FLOAT)), 17.31);
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_FLOAT)), 12312.1111);
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_FLOAT)), 0);
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_FLOAT)), 0);
    
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_FLOAT)), 9000);
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_FLOAT), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_FLOAT), NULL);
    assert_equal(Berror, BEBADOP);
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_FLOAT)), 88);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_FLOAT)), -1021);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_FLOAT)), 99);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_FLOAT)), 17.31);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_FLOAT)), 12312.1111);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_FLOAT)), 0);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_FLOAT)), 0);
    assert_equal(len, sizeof(float));
    
    len=0;
    assert_double_equal(*(f=(float *)CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_FLOAT)), 9000);
    assert_equal(len, sizeof(float));
    
    /* Test as double */
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_DOUBLE)), 88);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_DOUBLE)), -1021);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_DOUBLE)), 99);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_DOUBLE)), 17.31);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_DOUBLE)), 12312.1111);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_DOUBLE)), 0);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_DOUBLE)), 0);
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_DOUBLE)), 9000);
    
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_DOUBLE), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_DOUBLE), NULL);
    assert_equal(Berror, BEBADOP);
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_DOUBLE)), 88);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_DOUBLE)), -1021);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_DOUBLE)), 99);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_DOUBLE)), 17.31);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_DOUBLE)), 12312.1111);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_DOUBLE)), 0);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_DOUBLE)), 0);
    assert_equal(len, sizeof(double));
    
    len=0;
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_DOUBLE)), 9000);
    assert_equal(len, sizeof(double));
    
    /* no need for ubf/vew -> they will fail */

    /* Test as string */
    assert_string_equal(CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_STRING), "88");
    assert_string_equal(CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_STRING), "-1021");
    assert_string_equal(CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_STRING), "c");
    assert_equal(strncmp(CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_STRING), "17.3", 4), 0);
    assert_equal(strncmp(CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_STRING), "12312.1111", 10), 0);
    assert_string_equal(CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_STRING), "TEST STR VAL");
    assert_string_equal(CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_STRING), "CARRAY1 TEST STRING DATA");
    /* hex value */
    assert_string_equal(CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_STRING), "0x2328");
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_STRING), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_STRING), NULL);
    assert_equal(Berror, BEBADOP);

    len=0;
    assert_string_equal(CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_STRING), "88");
    assert_equal(len, 3);
    
    len=0;
    assert_string_equal(CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_STRING), "-1021");
    assert_equal(len, 6);
    
    len=0;
    assert_string_equal(CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_STRING), "c");
    assert_equal(len, 2);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_STRING), "17.3", 4), 0);
    assert_equal(len, 9);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_STRING), "12312.1111", 10), 0);
    assert_equal(len, 13);
    
    
    len=0;
    assert_string_equal(CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_STRING), "TEST STR VAL");
    assert_equal(len, 13);
    
    len=0;
    assert_string_equal(CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_STRING), "CARRAY1 TEST STRING DATA");
    assert_equal(len, 25);
    
    len=0;
    assert_string_equal(CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_STRING), "0x2328");
    assert_equal(len, 7);

    
    /* Test as carray */
    assert_equal(strncmp(CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_CARRAY), "88", 2), 0);
    assert_equal(strncmp(CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_CARRAY), "-1021", 5), 0);
    assert_equal(strncmp(CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_CARRAY), "c", 1), 0);
    assert_equal(strncmp(CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_CARRAY), "17.3", 4), 0);
    assert_equal(strncmp(CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_CARRAY), "12312.1111", 10), 0);
    assert_equal(strncmp(CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_CARRAY), "TEST STR VAL", 12), 0);
    assert_equal(strncmp(CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_CARRAY), "CARRAY1 TEST STRING DATA", 24), 0);
    
    assert_equal(strncmp(CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_CARRAY), "0x2328", 6), 0);
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_CARRAY), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_CARRAY), NULL);
    assert_equal(Berror, BEBADOP);
    
    /* Bug #330 */
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_CARRAY), "88", 2), 0);
    assert_equal(len, 2);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_CARRAY), "-1021", 5), 0);
    assert_equal(len, 5);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_CARRAY), "c", 1), 0);
    assert_equal(len, 1);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_CARRAY), "17.3", 4), 0);
    /* This is due to 5 zeros... after dot */
    assert_equal(len, 8);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_CARRAY), "12312.1111", 10), 0);
    /* this is due to 6 zeros after dot */
    assert_equal(len, 12);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_CARRAY), "TEST STR VAL", 12), 0);
    assert_equal(len, 12);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_CARRAY), "CARRAY1 TEST STRING DATA", 24), 0);
    assert_equal(len, 24);
    
    assert_equal(CBfind(p_ub, T_CARRAY_3_FLD, 0, &len, BFLD_CARRAY), NULL);
    assert_equal(Berror, BNOTPRES);
    
    len=0;
    assert_equal(strncmp(CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_CARRAY), "0x2328", 6), 0);
    assert_equal(len, 6);
    

    /* Test as ptr */
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_SHORT_FLD, 0, 0, BFLD_PTR)), 88);

    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_LONG_FLD, 0, 0, BFLD_PTR)), -1021);
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_CHAR_FLD, 0, 0, BFLD_PTR)), 99);
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_PTR)), 17);
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_PTR)), 12312);
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_STRING_FLD, 0, 0, BFLD_PTR)), 0);
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_PTR)), 0);

    
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_PTR_FLD, 0, 0, BFLD_PTR)), 9000);
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, 0, BFLD_PTR), NULL);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, 0, BFLD_PTR), NULL);
    assert_equal(Berror, BEBADOP);
    
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_SHORT_FLD, 0, &len, BFLD_PTR)), 88);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_LONG_FLD, 0, &len, BFLD_PTR)), -1021);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_CHAR_FLD, 0, &len, BFLD_PTR)), 99);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_FLOAT_FLD, 0, &len, BFLD_PTR)), 17);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_DOUBLE_FLD, 0, &len, BFLD_PTR)), 12312);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_PTR)), 0);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_CARRAY_FLD, 0, &len, BFLD_PTR)), 0);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(*((ndrx_longptr_t *)CBfind(p_ub, T_PTR_FLD, 0, &len, BFLD_PTR)), 9000);
    assert_equal(len, sizeof(ndrx_longptr_t));
    
    len=0;
    assert_equal(CBfind(p_ub, T_UBF_FLD, 0, &len, BFLD_PTR), NULL);
    assert_equal(Berror, BEBADOP);
    
    len=0;
    assert_equal(CBfind(p_ub, T_VIEW_FLD, 0, &len, BFLD_PTR), NULL);
    assert_equal(Berror, BEBADOP);
    
    len = 0;
    
    /* Now test the thing that we have different pointers for each of the data type
     * also fields will corss match their types.
     */
    assert_not_equal((s=(short *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_SHORT)), NULL);
    assert_not_equal((l=(long *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_LONG)), NULL);
    assert_not_equal((c=(char *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_CHAR)), NULL);
    assert_not_equal((f=(float *)CBfind(p_ub, T_DOUBLE_FLD, 0, 0, BFLD_FLOAT)), NULL);
    assert_not_equal((d=(double *)CBfind(p_ub, T_FLOAT_FLD, 0, 0, BFLD_DOUBLE)), NULL);
    assert_not_equal((str=CBfind(p_ub, T_CARRAY_FLD, 0, 0, BFLD_STRING)), NULL);
    assert_not_equal((carr=CBfind(p_ub, T_STRING_FLD, 0, &len, BFLD_CARRAY)), NULL);
    
    /* ? above is already tested.. */
    
    /* Now compare the values */
    assert_equal(*s, 17);
    assert_equal(*l, 12312);
    assert_equal(*c, 17);
    assert_double_equal(*f, 12312.1111);
    assert_double_equal(*d, 17.31);
    assert_string_equal(str, "CARRAY1 TEST STRING DATA");
    assert_equal(len, 12);
    assert_equal(strncmp(carr, "TEST STR VAL", len), 0);

    /* Now test the error case, when field is not found? */
    assert_equal(CBfind(p_ub, T_FLOAT_FLD, 10, 0, BFLD_SHORT), NULL);
    assert_equal(Berror, BNOTPRES);

    /* try to get data from other occurrance */
    assert_double_equal(*(d=(double *)CBfind(p_ub, T_LONG_FLD, 4, 0, BFLD_DOUBLE)), 888);

    /* Now play with big buffer data */
    assert_equal(Bchg(p_ub, T_CARRAY_2_FLD, 4, BIG_TEST_STRING, strlen(BIG_TEST_STRING)),EXSUCCEED);
    /* now match the string */
    assert_string_equal((str=CBfind(p_ub, T_CARRAY_2_FLD, 4, 0, BFLD_STRING)), BIG_TEST_STRING);

}

/**
 * Get occurrence out from value.
 */
Ensure(test_bfindocc)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    short s;
    long l;
    char c;
    float f;
    double d;
    char *str;
    char *carr;
    BFLDLEN len=0;
    ndrx_longptr_t ptr;

    char buf_tmp[1024];
    UBFH *p_ub_tmp = (UBFH *)buf_tmp;
    BVIEWFLD vf;
    
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    load_bfindocc_test_data(p_ub);

    s = 88;
    l = -1021;
    c = '.';
    f = 17.31;
    d = 12312.1111;
    str = "TEST STRING ARRAY2";
    carr = "\0ARRAY1 TEST STRING DATA";
    ptr = 9004;
    assert_equal(Bfindocc(p_ub, T_SHORT_FLD, (char*) &s, 0),1);
    assert_equal(Bfindocc(p_ub, T_LONG_FLD, (char*) &l, 0),1);
    assert_equal(Bfindocc(p_ub, T_CHAR_FLD, (char*) &c, 0),1);
    assert_equal(Bfindocc(p_ub, T_FLOAT_FLD, (char*) &f, 0),1);
    assert_equal(Bfindocc(p_ub, T_DOUBLE_FLD, (char*) &d, 0),1);
    assert_equal(Bfindocc(p_ub, T_STRING_FLD, (char*) str, 0),1);
    assert_equal(Bfindocc(p_ub, T_CARRAY_FLD, (char*) carr, 24),1);
    
    /* test for PTR, VIEW, UBF */
    assert_equal(Bfindocc(p_ub, T_PTR_FLD, (char*) &ptr, 0),1);
    
    assert_equal(Binit(p_ub_tmp, sizeof(buf_tmp)), EXSUCCEED);
    gen_set_ubf_dbg(__FILE__, __LINE__, p_ub_tmp, 4);
    assert_equal(Bfindocc(p_ub, T_UBF_FLD, (char*)p_ub_tmp, 0),1);
    
    gen_set_view_dbg(__FILE__, __LINE__, &vf, 4);
    assert_equal(Bfindocc(p_ub, T_VIEW_FLD, (char*)&vf, 0),1);
    
    /* Now test first occurrence */
    s = 881;
    l = -1011;
    c = 'a';
    f = 1.31;
    d = 1312.1111;
    str = "TEST STR VAL";
    carr = "ZARRAY1 TEST STRING DATA";

    assert_equal(Bfindocc(p_ub, T_SHORT_FLD, (char*) &s, 0),0);
    assert_equal(Bfindocc(p_ub, T_LONG_FLD, (char*) &l, 0),0);
    assert_equal(Bfindocc(p_ub, T_CHAR_FLD, (char*) &c, 0),0);
    assert_equal(Bfindocc(p_ub, T_FLOAT_FLD, (char*) &f, 0),0);
    assert_equal(Bfindocc(p_ub, T_DOUBLE_FLD, (char*) &d, 0),0);
    assert_equal(Bfindocc(p_ub, T_STRING_FLD, (char*) str, 0),0);
    assert_equal(Bfindocc(p_ub, T_CARRAY_FLD, (char*) carr, 24),0);
    
    /* test for PTR, VIEW, UBF */
    ptr = 9003;
    assert_equal(Bfindocc(p_ub, T_PTR_FLD, (char*) &ptr, 0),0);
    
    assert_equal(Binit(p_ub_tmp, sizeof(buf_tmp)), EXSUCCEED);
    gen_set_ubf_dbg(__FILE__, __LINE__, p_ub_tmp, 3);
    assert_equal(Bfindocc(p_ub, T_UBF_FLD, (char*)p_ub_tmp, 0),0);
    
    gen_set_view_dbg(__FILE__, __LINE__, &vf, 3);
    assert_equal(Bfindocc(p_ub, T_VIEW_FLD, (char*)&vf, 0),0);

    /* Test regexp version */
    assert_equal(Bchg(p_ub, T_STRING_FLD, 10, "A", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 11, "ABC", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 20, "TESTEST", 0), EXSUCCEED);
    assert_equal(Bfindocc(p_ub, T_STRING_FLD, "B|A", 1), 10);
    assert_equal(Bfindocc(p_ub, T_STRING_FLD, "A.C", 1), 11);
    assert_equal(Bfindocc(p_ub, T_STRING_FLD, "TESTEST(|\\(|", 1), -1);
    assert_equal(Bfindocc(p_ub, T_STRING_FLD, "TES....", 1), 20);
}

/**
 * Get occurrence out from value (from user converted value)
 */
Ensure(test_cbfindocc)
{
    char fb[8192];
    UBFH *p_ub = (UBFH *)fb;
    short s;
    long l;
    char c;
    float f;
    double d;
    char *str;
    char *carr;
    BFLDLEN len=0;
    ndrx_longptr_t ptr;
    
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    load_find_test_data(p_ub);
    
    /* Override some values for common test */
    assert_equal(CBchg(p_ub, T_LONG_FLD, 100, "251", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_FLOAT_FLD, 12, "102", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_DOUBLE_FLD, 13, "99", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_STRING_FLD, 14, "55", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_CARRAY_FLD, 15, "66", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_PTR_FLD, 16, "0x77", 0, BFLD_STRING), EXSUCCEED);
    
    gen_load_ubf(p_ub, 17, 10, 2);
    gen_load_view(p_ub, 18, 10, 2);
    
    
    /* Test as short */
    s=88;
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&s, 0, BFLD_SHORT), 0);
    s=251;
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&s, 0, BFLD_SHORT), 100);
    s=99;
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&s, 0, BFLD_SHORT), 0);
    s=102;
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&s, 0, BFLD_SHORT), 12);
    s=99;
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&s, 0, BFLD_SHORT), 13);
    s=55;
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&s, 0, BFLD_SHORT), 14);
    s=66;
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&s, 0, BFLD_SHORT), 15);
    s=0x77;
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&s, 0, BFLD_SHORT), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&s, 0, BFLD_SHORT), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&s, 0, BFLD_SHORT), -1);
    assert_equal(Berror, BEBADOP);


    /* Test as long */
    l=88;
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&l, 0, BFLD_LONG), 0);
    l=251;
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&l, 0, BFLD_LONG), 100);
    l=99;
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&l, 0, BFLD_LONG), 0);
    l=102;
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&l, 0, BFLD_LONG), 12);
    l=99;
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&l, 0, BFLD_LONG), 13);
    l=55;
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&l, 0, BFLD_LONG), 14);
    l=66;
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&l, 0, BFLD_LONG), 15);
    l=0x77;
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&l, 0, BFLD_LONG), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&l, 0, BFLD_LONG), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&l, 0, BFLD_LONG), -1);
    assert_equal(Berror, BEBADOP);
    

    /* Test as char */
    c=88;
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&c, 0, BFLD_CHAR), 0);
    c=(char)251;
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&c, 0, BFLD_CHAR), 100);
    c=99;
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&c, 0, BFLD_CHAR), 0);
    c=102;
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&c, 0, BFLD_CHAR), 12);
    c=99;
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&c, 0, BFLD_CHAR), 13);

    assert_equal(CBchg(p_ub, T_STRING_FLD, 14, "7", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_CARRAY_FLD, 15, "A", 0, BFLD_STRING), EXSUCCEED);

    c=55;
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&c, 0, BFLD_CHAR), 14);
    c=65;
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&c, 0, BFLD_CHAR), 15);
    c=0x77;
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&c, 0, BFLD_CHAR), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&c, 0, BFLD_CHAR), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&c, 0, BFLD_CHAR), -1);
    assert_equal(Berror, BEBADOP);
    

    assert_equal(CBchg(p_ub, T_STRING_FLD, 14, "55", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub, T_CARRAY_FLD, 15, "66", 0, BFLD_STRING), EXSUCCEED);

    /* Test as float */
    f=88;
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&f, 0, BFLD_FLOAT), 0);
    f=251;
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&f, 0, BFLD_FLOAT), 100);
    f=99;
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&f, 0, BFLD_FLOAT), 0);
    f=102;
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&f, 0, BFLD_FLOAT), 12);
    f=99;
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&f, 0, BFLD_FLOAT), 13);
    
    /* Due to string as float value after the comma, we cannot find such values */
    f=55;
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&f, 0, BFLD_FLOAT), -1);
    f=66;
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&f, 0, BFLD_FLOAT), -1);
    
    f=0x77;
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&f, 0, BFLD_FLOAT), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&f, 0, BFLD_FLOAT), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&f, 0, BFLD_FLOAT), -1);
    assert_equal(Berror, BEBADOP);


    /* Test as double */
    d=88;
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&d, 0, BFLD_DOUBLE), 0);
    d=251;
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&d, 0, BFLD_DOUBLE), 100);
    d=99;
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&d, 0, BFLD_DOUBLE), 0);
    d=102;
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&d, 0, BFLD_DOUBLE), 12);
    d=99;
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&d, 0, BFLD_DOUBLE), 13);
    
    /* Due to string as float value after the comma, we cannot find such values */
    d=55;
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&d, 0, BFLD_DOUBLE), -1);
    d=66;
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&d, 0, BFLD_DOUBLE), -1);
    
    d=0x77;
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&d, 0, BFLD_DOUBLE), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&d, 0, BFLD_DOUBLE), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&d, 0, BFLD_DOUBLE), -1);
    assert_equal(Berror, BEBADOP);
    
    
    /* Test as string */
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, "88", 0, BFLD_STRING), 0);
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, "251", 0, BFLD_STRING), 100);
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, "c", 0, BFLD_STRING), 0);
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, "102", 0, BFLD_STRING), 12);
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, "99", 0, BFLD_STRING), 13);
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, "55", 0, BFLD_STRING), 14);
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, "66", 0, BFLD_STRING), 15);
    
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, "0x77", 0, BFLD_STRING), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, "0x77", 0, BFLD_STRING), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, "0x77", 0, BFLD_STRING), -1);
    assert_equal(Berror, BEBADOP);
    
    /* Test as carray */
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, "88", 2, BFLD_CARRAY), 0);
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, "251", 3, BFLD_CARRAY), 100);
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, "c", 1, BFLD_CARRAY), 0);
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, "102", 3, BFLD_CARRAY), 12);
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, "99", 2, BFLD_CARRAY), 13);
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, "55", 2, BFLD_CARRAY), 14);
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, "66", 2, BFLD_CARRAY), 15);
    
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, "0x77", 4, BFLD_CARRAY), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, "0x77", 4, BFLD_CARRAY), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, "0x77", 4, BFLD_CARRAY), -1);
    assert_equal(Berror, BEBADOP);
    
    /* Test as ptr */
    ptr=88;
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&ptr, 0, BFLD_PTR), 0);
    ptr=251;
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&ptr, 0, BFLD_PTR), 100);
    ptr=99;
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&ptr, 0, BFLD_PTR), 0);
    ptr=102;
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&ptr, 0, BFLD_PTR), 12);
    ptr=99;
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&ptr, 0, BFLD_PTR), 13);
    ptr=55;
    /* not found as ptr is built as 0x... */
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&ptr, 0, BFLD_PTR), -1);
    ptr=66;
    /* not found as ptr is built as 0x... */
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&ptr, 0, BFLD_PTR), -1);
    ptr=0x77;
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&ptr, 0, BFLD_PTR), 16);
    
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&ptr, 0, BFLD_PTR), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&ptr, 0, BFLD_PTR), -1);
    assert_equal(Berror, BEBADOP);
    
    /* test as UBF (use some ptr on input data, as there will be error anyway) */
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    /* not found as ptr is built as 0x... */
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    /* not found as ptr is built as 0x... */
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&ptr, 0, BFLD_UBF), -1);
    assert_equal(Berror, BEBADOP);
    
    /* test as view */
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_LONG_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_CHAR_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_FLOAT_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_DOUBLE_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    /* not found as ptr is built as 0x... */
    assert_equal(CBfindocc(p_ub, T_STRING_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    /* not found as ptr is built as 0x... */
    assert_equal(CBfindocc(p_ub, T_CARRAY_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_PTR_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_UBF_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    assert_equal(CBfindocc(p_ub, T_VIEW_FLD, (char *)&ptr, 0, BFLD_VIEW), -1);
    assert_equal(Berror, BEBADOP);
    
    /* Now test invalid argument */
    assert_equal(CBfindocc(p_ub, T_SHORT_FLD, "88", 2, 100), -1);
    assert_equal(Berror, BTYPERR);
}

/**
 * Test Bfindlast
 */
Ensure(test_bfindlast)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    short *s1;
    long *l1;
    char *c1;
    float *f1;
    double *d1;
    char *str1;
    char *carr1;
    BFLDLEN len=0;
    BFLDOCC occ=-1;
    ndrx_longptr_t *ptr;
    UBFH *p_ub_tmp = NULL;
    BVIEWFLD *vf;
    
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    load_bfindlast_test_data(p_ub);

    /* Test as short */
    occ=-1;
    assert_not_equal((s1=(short *)Bfindlast(p_ub, T_SHORT_FLD, &occ, 0)), NULL);
    assert_equal(*s1, 88);
    assert_equal(occ, 0);

    /* Test as long */
    occ=-1;
    assert_not_equal((l1=(long *)Bfindlast(p_ub, T_LONG_FLD, &occ, 0)), NULL);
    assert_equal(*l1, -1021);
    assert_equal(occ, 1);

     /* Test as char */
    occ=-1;
    assert_not_equal((c1=(char *)Bfindlast(p_ub, T_CHAR_FLD, &occ, 0)), NULL);
    assert_equal(*c1, 'c');
    assert_equal(occ, 2);

    /* Test as float */
    occ=-1;
    assert_not_equal((f1=(float *)Bfindlast(p_ub, T_FLOAT_FLD, &occ, 0)), NULL);
    assert_double_equal(*f1, 17.31);
    assert_equal(occ, 3);

    /* Test as double */
    occ=-1;
    assert_not_equal((d1=(double *)Bfindlast(p_ub, T_DOUBLE_FLD, &occ, 0)), NULL);
    assert_double_equal(*d1, 12312.1111);
    assert_equal(occ, 4);

    /* Test as string */
    occ=-1;
    assert_not_equal((str1=Bfindlast(p_ub, T_STRING_FLD, &occ, 0)), NULL);
    assert_string_equal(str1, "TEST STR VAL");
    assert_equal(occ, 5);

    /* Test as carray */
    len = 1000;
    occ=-1;
    assert_not_equal((carr1=Bfindlast(p_ub, T_CARRAY_FLD, &occ, &len)), NULL);
    assert_equal(len, 24);
    assert_equal(strncmp(carr1, "CARRAY1 TEST STRING DATA", 24), 0);
    assert_equal(occ, 6);
    
    /* Test as PTR */
    len = 1000;
    occ=-1;
    assert_not_equal((ptr=(ndrx_longptr_t*)Bfindlast(p_ub, T_PTR_FLD, &occ, &len)), NULL);
    assert_equal(len, sizeof(ndrx_longptr_t));
    assert_equal(occ, 7);
    assert_equal(*ptr, 0x232e);
    
    /* Test as UBF */
    len = 1000;
    occ=-1;    
    assert_not_equal((p_ub_tmp=(UBFH*)Bfindlast(p_ub, T_UBF_FLD, &occ, &len)), NULL);
    assert_equal(occ, 8);
    gen_test_ubf_val_dbg(__FILE__, __LINE__, p_ub_tmp, 6, &len);
    
    /* Test as VIEW */
    len = 1000;
    occ=-1;    
    assert_not_equal((vf=(BVIEWFLD*)Bfindlast(p_ub, T_VIEW_FLD, &occ, &len)), NULL);
    assert_equal(occ, 9);
    gen_test_view_val_dbg(__FILE__, __LINE__, vf, 6, &len);

    /* Test the case when data is not found! */
    occ=-1;
    assert_equal((str1=Bfindlast(p_ub, T_STRING_2_FLD, &occ, 0)), NULL);
    assert_equal(Berror, BNOTPRES);
    assert_equal(occ, -1);
}

TestSuite *ubf_find_tests(void)
{
    TestSuite *suite = create_test_suite();

    
    add_test(suite, test_bfind);
    add_test(suite, test_cbfind);
    add_test(suite, test_bfindocc);
    add_test(suite, test_cbfindocc);
    add_test(suite, test_bfindlast);
    
    return suite;
}
/* vim: set ts=4 sw=4 et smartindent: */
