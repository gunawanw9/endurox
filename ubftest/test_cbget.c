/**
 *
 * @file test_cbget.c
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




/**
 * Original field is BFLD_SHORT => T_SHORT_FLD
 */
Ensure(test_CBget_short_org)
{
    char buf[640];
    UBFH *p_ub = (UBFH *)buf;
    short test_val=4564;
    int len = 0;
    short small_val=44;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    char test_char = 0;
    float test_float = 0.0;
    double test_double = 0.0;
    char test_string[128];
    char test_carray[128];
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;

    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = sizeof(test_val);
    assert_equal(Badd(p_ub, T_SHORT_FLD, (char *)&test_val, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 4564);
    assert_equal(len, sizeof(test_short));
    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 4564);
    assert_equal(len, sizeof(test_long));
    /* char out */
    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&small_val, 0), EXSUCCEED);
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&test_char, 0, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, ',');
    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&test_val, 0), EXSUCCEED); /* restore back */
    /* test float */
    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 4564);
    assert_equal(len, sizeof(test_float));
    /* test double */
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&test_double, 0, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 4564);
    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "4564");
    assert_equal(len, 5);
    /* test carray */
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)test_carray, 0, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "4564", 4), 0);
    
    /* may be cast to PTR */
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 4564);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_SHORT_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

/**
 * Original field is BFLD_LONG => T_LONG_FLD
 */
Ensure(test_CBget_long_org)
{
    char buf[640];
    UBFH *p_ub = (UBFH *)buf;
    long test_val=456412;
    int len = 0;
    long small_val=44;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    char test_char = 0;
    float test_float = 0.0;
    double test_double = 0.0;
    char test_string[128];
    char test_carray[128];
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = sizeof(test_val);
    assert_equal(Badd(p_ub, T_LONG_FLD, (char *)&small_val, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 44);
    assert_equal(len, sizeof(test_short));
    /* char out */
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, ',');
    assert_equal(Bchg(p_ub, T_LONG_FLD, 0, (char *)&test_val, 0), EXSUCCEED); /* restore back */
    assert_equal(len, sizeof(test_char));

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 456412);
    assert_equal(len, sizeof(test_long));
    /* test float */
    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 456412);
    assert_equal(len, sizeof(test_float));
    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 456412);
    assert_equal(len, sizeof(test_double));
    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "456412");
    assert_equal(len, 7);
    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "456412", 6), 0);
    assert_equal(len, 6);
    
    /* may be cast to PTR */
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 456412);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_LONG_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

/**
 * Original field is BFLD_CHAR => T_CHAR_FLD
 */
Ensure(test_CBget_char_org)
{
    char buf[640];
    UBFH *p_ub = (UBFH *)buf;
    long test_val=456412;
    int len = 0;
    long small_val=0;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    char test_char = 'r';
    float test_float = 0.0;
    double test_double = 0.0;
    char test_string[128];
    char test_carray[128];
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;

    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = sizeof(test_val);
    assert_equal(Badd(p_ub, T_CHAR_FLD, (char *)&test_char, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 114);
    assert_equal(len, sizeof(test_short));

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 114);
    assert_equal(len, sizeof(test_long));

    /* char out */
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, 'r');
    assert_equal(len, sizeof(test_char));

    /* test float */
    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 114);
    assert_equal(len, sizeof(test_float));

    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 114);
    assert_equal(len, sizeof(test_double));

    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "r");
    assert_equal(len, 2);

    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "r", 1), 0);
    assert_equal(len, 1);   
    
    
    /* may be cast to PTR */
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 114);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_CHAR_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

/**
 * Original field is BFLD_FLOAT => T_FLOAT_FLD
 */
Ensure(test_CBget_float_org)
{
    char buf[640];
    UBFH *p_ub = (UBFH *)buf;
    long test_val=0;
    int len = 0;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    unsigned char test_char = 0;
    float test_float = 16822.5;
    double test_double = 0.0;
    char test_string[128];
    char test_carray[128];
    
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = sizeof(test_float);
    assert_equal(Badd(p_ub, T_FLOAT_FLD, (char *)&test_float, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 16822);
    assert_equal(len, sizeof(test_short));

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 16822);
    assert_equal(len, sizeof(test_long));

    /* char out */

    len = sizeof(test_float);
    test_float = 168.5;
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 0, (char *)&test_float, len), EXSUCCEED);
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, 168);
    assert_equal(len, sizeof(test_char));

    /* test float */
    /* continue as normal */
    len = sizeof(test_float);
    test_float = 16822.5;
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 0, (char *)&test_float, len), EXSUCCEED);

    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_float));

    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_double));

    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "16822.50000");
    assert_equal(len, 12);

    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "16822.50000", 10), 0);
    assert_equal(len, 11);
    
    /* may be cast to PTR */
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 16822);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_FLOAT_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

/**
 * Original field is BFLD_DOUBLE => T_DOUBLE_FLD
 */
Ensure(test_CBget_double_org)
{
    char buf[640];
    UBFH *p_ub = (UBFH *)buf;
    int len = 0;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    unsigned char test_char = 0;
    float test_float = 0.0;
    double test_double = 16822.5;
    char test_string[128];
    char test_carray[128];
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;

    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = sizeof(test_double);
    assert_equal(Badd(p_ub, T_DOUBLE_FLD, (char *)&test_double, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 16822);
    assert_equal(len, sizeof(test_short));

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 16822);
    assert_equal(len, sizeof(test_long));

    /* char out */
    len = sizeof(test_double);
    test_double = 168.5;
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 0, (char *)&test_double, len), EXSUCCEED);
    
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, 168);
    assert_equal(len, sizeof(test_char));

    /* test float */
    /* continue as normal */
    test_double = 16822.5;
    len = sizeof(test_double);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 0, (char *)&test_double, len), EXSUCCEED);

    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_float));

    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_double));

    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "16822.500000");
    assert_equal(len, 13);

    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "16822.500000", 10), 0);
    assert_equal(len, 12);
    
    /* may be cast to PTR */
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 16822);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_DOUBLE_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}
/**
 * Original field is BFLD_STRING => T_STRING_FLD
 */
Ensure(test_CBget_string_org)
{
    char buf[2048];
    UBFH *p_ub = (UBFH *)buf;
    int len = 0;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    unsigned char test_char = 0;
    float test_float = 0.0;
    double test_double = 16822.5;
    char test_string[1024];
    char test_carray[1024];
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    /*  len 1 - should be ok, because it is ignored!*/
    assert_equal(Badd(p_ub, T_STRING_FLD, "16822.5000000000000", 1), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 16822);
    assert_equal(len, sizeof(test_short));

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 16822);
    assert_equal(len, sizeof(test_long));

    /* char out */
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, '1');
    assert_equal(len, sizeof(test_char));

    /* test float */
    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_float));

    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_double));

    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "16822.5000000000000");
    assert_equal(len, 20);

    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "16822.5000000000000", 17), 0);
    assert_equal(len, 19);
    
    /* Special test for hadling strings & carrays */
    assert_equal(Bchg(p_ub, T_STRING_FLD, 0, BIG_TEST_STRING, 0), EXSUCCEED);
    len=sizeof(test_carray);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, BIG_TEST_STRING, strlen(BIG_TEST_STRING)), 0);
    assert_equal(len, strlen(BIG_TEST_STRING));
    
    
    /* may be cast to PTR -> this will try to parse 0xHEX ptr */
    assert_equal(Bchg(p_ub, T_STRING_FLD, 0, "0xFFAABB", 0L), EXSUCCEED);
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 0xFFAABB);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_STRING_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

/**
 * Original field is BFLD_CARRAY => T_CARRAY_FLD
 */
Ensure(test_CBget_carray_org)
{
    char buf[2048];
    UBFH *p_ub = (UBFH *)buf;
    int len = 0;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    unsigned char test_char = 0;
    float test_float = 0.0;
    double test_double = 16822.5;
    char test_string[1024];
    char test_carray[1024];
    char test_data_str [] = "16822.5000000000000";
    
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = strlen(test_data_str);
    assert_equal(Badd(p_ub, T_CARRAY_FLD, test_data_str, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 16822);
    assert_equal(len, sizeof(test_short));

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 16822);
    assert_equal(len, sizeof(test_long));

    /* char out */
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, '1');
    assert_equal(len, sizeof(test_char));

    /* test float */
    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_float));

    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 16822.5);
    assert_equal(len, sizeof(test_double));

    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, test_data_str);
    assert_equal(len, 20);

    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, test_data_str, 17), 0);
    assert_equal(len, 19);

    /* Special test for hadling strings & carrays */
    len = strlen(BIG_TEST_STRING);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 0, BIG_TEST_STRING, len), EXSUCCEED);
    len=sizeof(test_string);
    memset(test_string, 0xff, sizeof(test_string)); /* For checking EOS terminator */
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_equal(strncmp(test_string, BIG_TEST_STRING, strlen(BIG_TEST_STRING)), 0);
    assert_equal(len, strlen(BIG_TEST_STRING)+1);
    assert_equal(strlen(test_string), strlen(BIG_TEST_STRING));
    

    /* may be cast to PTR -> this will try to parse 0xHEX ptr */
    assert_equal(CBchg(p_ub, T_CARRAY_FLD, 0, "0xFFAABB", 0L, BFLD_STRING), EXSUCCEED);
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 0xFFAABB);
   
    v.data = tmp;
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_CARRAY_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

Ensure(test_CBget_view_org)
{
    char buf[2048];
    UBFH *p_ub = (UBFH *)buf;
    char tmp[1024];
    BVIEWFLD vf;
    struct UBTESTVIEW2 v;
    
    memset(&v, 0, sizeof(v));
    
    vf.data=(char *)&v;
    vf.vflags=0;
    NDRX_STRCPY_SAFE(vf.vname, "UBTESTVIEW2");
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    assert_equal(Badd(p_ub, T_VIEW_FLD, (char *)&vf, 0), EXSUCCEED);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_SHORT), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_LONG), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_CHAR), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_FLOAT), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_DOUBLE), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_STRING), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_CARRAY), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_PTR), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_VIEW_FLD, 0, (char *)tmp, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

Ensure(test_CBget_ubf_org)
{
    char buf[2048];
    UBFH *p_ub = (UBFH *)buf;
    char tmp[1024];
    UBFH *p_tmp_ub = (UBFH *)tmp;    
    
    /* Check adding invalid UBF...? Check atleast the magic..? */
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    
    memset(tmp, 1, sizeof(tmp));
    assert_equal(Badd(p_ub, T_UBF_FLD, (char *)tmp, 0), EXFAIL);
    assert_equal(Berror, BNOTFLD);
    
    assert_equal(Binit(p_tmp_ub, sizeof(tmp)), EXSUCCEED);
    assert_equal(Badd(p_tmp_ub, T_STRING_FLD, "HELLO", 0), EXSUCCEED);
    assert_equal(Badd(p_ub, T_UBF_FLD, (char *)p_ub, 0), EXSUCCEED);

    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_SHORT), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_LONG), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_CHAR), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_FLOAT), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_DOUBLE), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_STRING), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_CARRAY), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_PTR), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_UBF_FLD, 0, (char *)tmp, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
}

Ensure(test_CBget_ptr_org)
{
    char buf[640];
    UBFH *p_ub = (UBFH *)buf;
    long test_val=456412;
    int len = 0;
    char *small_val=(char *)44;
    /* test buffers */
    short test_short = 0;
    long test_long = 0;
    char test_char = 0;
    float test_float = 0.0;
    double test_double = 0.0;
    char test_string[128];
    char test_carray[128];
    char tmp[1024];
    char *ptr;
    BVIEWFLD v;
    UBFH *p_tmp_ub = (UBFH *)tmp;
    
    /* init */
    assert_equal(Binit(p_ub, sizeof(buf)), EXSUCCEED);
    len = sizeof(test_val);
    assert_equal(Badd(p_ub, T_PTR_FLD, (char *)&small_val, len), EXSUCCEED);

    /* Now test all types */
    /* short out */
    len=sizeof(test_short);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&test_short, &len, BFLD_SHORT), EXSUCCEED);
    assert_equal(test_short, 44);
    assert_equal(len, sizeof(test_short));
    /* char out */
    len=sizeof(test_char);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&test_char, &len, BFLD_CHAR), EXSUCCEED);
    assert_equal(test_char, 44);

    /* long out */
    len=sizeof(test_long);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&test_long, &len, BFLD_LONG), EXSUCCEED);
    assert_equal(test_long, 44);
    assert_equal(len, sizeof(test_long));
    /* test float */
    len=sizeof(test_float);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&test_float, &len, BFLD_FLOAT), EXSUCCEED);
    assert_equal(test_float, 44);
    assert_equal(len, sizeof(test_float));
    /* test double */
    len=sizeof(test_double);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&test_double, &len, BFLD_DOUBLE), EXSUCCEED);
    assert_equal(test_float, 44);
    assert_equal(len, sizeof(test_double));
    /* test string */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)test_string, &len, BFLD_STRING), EXSUCCEED);
    assert_string_equal(test_string, "0x2c");
    assert_equal(len, 5);
    /* test carray */
    len=sizeof(test_string);
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)test_carray, &len, BFLD_CARRAY), EXSUCCEED);
    assert_equal(strncmp(test_carray, "0x2c", 4), 0);
    assert_equal(len, 4);
    
    /* may be cast to PTR */
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&ptr, 0, BFLD_PTR), EXSUCCEED);
    assert_equal((long)ptr, 44);
    
    v.data = tmp;
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)&v, 0, BFLD_VIEW), EXFAIL);
    assert_equal(Berror, BEBADOP);
    
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, (char *)p_tmp_ub, 0, BFLD_UBF), EXFAIL);
    assert_equal(Berror, BEBADOP);
}

TestSuite *ubf_cfget_tests(void)
{
    TestSuite *suite = create_test_suite();
    
    std_basic_setup();
    
    add_test(suite, test_CBget_short_org);
    add_test(suite, test_CBget_long_org);
    add_test(suite, test_CBget_char_org);
    add_test(suite, test_CBget_float_org);
    add_test(suite, test_CBget_double_org);
    add_test(suite, test_CBget_string_org);
    add_test(suite, test_CBget_carray_org);
    add_test(suite, test_CBget_view_org);
    add_test(suite, test_CBget_ubf_org);
    add_test(suite, test_CBget_ptr_org);
    
    /* string & carray */

    return suite;
}

/* vim: set ts=4 sw=4 et smartindent: */
