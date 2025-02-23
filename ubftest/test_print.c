/**
 *
 * @file test_print.c
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
#include <unistd.h>
#include "test.fd.h"
#include "ubfunit1.h"
#include "ndebug.h"
#include "atmi_int.h"

/**
 * Reference output that should be printed to log file.
 */
char *ref_print[]= {
"T_SHORT_FLD\t88\n",
"T_SHORT_FLD\t-1\n",
"T_SHORT_2_FLD\t0\n",
"T_SHORT_2_FLD\t212\n",
"T_LONG_FLD\t-1021\n",
"T_LONG_FLD\t-2\n",
"T_LONG_FLD\t0\n",
"T_LONG_FLD\t0\n",
"T_LONG_FLD\t-4\n",
"T_LONG_2_FLD\t0\n",
"T_LONG_2_FLD\t0\n",
"T_LONG_2_FLD\t212\n",
"T_CHAR_FLD\t\\0a\n",
"T_CHAR_FLD\t.\n",
"T_CHAR_2_FLD\t\\00\n",
"T_CHAR_2_FLD\t\\00\n",
"T_CHAR_2_FLD\t\\00\n",
"T_CHAR_2_FLD\tb\n",
"T_FLOAT_FLD\t17.31000\n",
"T_FLOAT_FLD\t1.31000\n",
"T_FLOAT_2_FLD\t0.00000\n",
"T_FLOAT_2_FLD\t0.00000\n",
"T_FLOAT_2_FLD\t0.00000\n",
"T_FLOAT_2_FLD\t0.00000\n",
"T_FLOAT_2_FLD\t1227.00000\n",
"T_DOUBLE_FLD\t12312.111100\n",
"T_DOUBLE_FLD\t112.110000\n",
"T_DOUBLE_2_FLD\t0.000000\n",
"T_DOUBLE_2_FLD\t0.000000\n",
"T_DOUBLE_2_FLD\t0.000000\n",
"T_DOUBLE_2_FLD\t0.000000\n",
"T_DOUBLE_2_FLD\t0.000000\n",
"T_DOUBLE_2_FLD\t1232.100000\n",
"T_STRING_FLD\tTEST STR VAL\n",
"T_STRING_FLD\tTEST STRING ARRAY2\n",
"T_STRING_2_FLD\t\n",
"T_STRING_2_FLD\t\n",
"T_STRING_2_FLD\t\n",
"T_STRING_2_FLD\t\n",
"T_STRING_2_FLD\t\n",
"T_STRING_2_FLD\t\n",
"T_STRING_2_FLD\tMEGA STR\\\\ING \\0a \\09 \\0d\n",
"T_CARRAY_FLD\t\n",
"T_CARRAY_FLD\tY\\01\\02\\03AY1 TEST \\\\STRING DATA\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\n",
"T_CARRAY_2_FLD\t\\00\\01\\02\\03AY1 TEST \\\\STRING DATA\n",
"T_PTR_FLD\t0x2329\n",
"T_PTR_FLD\t0x232a\n",
"T_PTR_FLD\t0x0\n",
"T_PTR_FLD\t0x0\n",
"T_PTR_FLD\t0x0\n",
"T_PTR_FLD\t0x0\n",
"T_PTR_FLD\t0x0\n",
"T_PTR_FLD\t0x232b\n",
"T_UBF_FLD\t\n",
"\tT_STRING_2_FLD\tT\n",
"T_UBF_FLD\t\n",
"\tT_STRING_3_FLD\tU\n",
"T_UBF_FLD\t\n",
"T_UBF_FLD\t\n",
"T_UBF_FLD\t\n",
"T_UBF_FLD\t\n",
"T_UBF_FLD\t\n",
"T_UBF_FLD\t\n",
"T_UBF_FLD\t\n",
"\tT_STRING_4_FLD\tV\n",
"T_VIEW_FLD\tUBTESTVIEW2\n",
"\ttshort1\t2\n",
"\ttlong1\t3\n",
"\ttchar1\t4\n",
"\ttfloat1\t5.00000\n",
"\ttdouble1\t6.000000\n",
"\ttstring1\tB\n",
"\ttcarray1\tD\\00\\09\\09\\09\\09\\09\\09\\09\\09\n",
"T_VIEW_FLD\tUBTESTVIEW2\n",
"\ttshort1\t3\n",
"\ttlong1\t4\n",
"\ttchar1\t5\n",
"\ttfloat1\t6.00000\n",
"\ttdouble1\t7.000000\n",
"\ttstring1\tC\n",
"\ttcarray1\tE\\00\\09\\09\\09\\09\\09\\09\\09\\09\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\t\n",
"T_VIEW_FLD\tUBTESTVIEW2\n",
"\ttshort1\t4\n",
"\ttlong1\t5\n",
"\ttchar1\t6\n",
"\ttfloat1\t7.00000\n",
"\ttdouble1\t8.000000\n",
"\ttstring1\tD\n",
"\ttcarray1\tF\\00\\09\\09\\09\\09\\09\\09\\09\\09\n",
NULL
};

/**
 * Prepare test data for print test.
 * @param p_ub
 */
void load_print_test_data(UBFH *p_ub)
{
    short s = 88;
    long l = -1021;
    char c = '\n';
    float f = 17.31;
    double d = 12312.1111;
    char carr[] = "CARRAY1 TEST \\STRING DATA";
    char string2[] = "MEGA STR\\ING \n \t \r";
    carr[0] = 0;
    carr[1] = 1;
    carr[2] = 2;
    carr[3] = 3;
    BFLDLEN len = strlen(carr);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 0, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 0, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 0, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 0, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 0, (char *)"TEST STR VAL", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 0, (char *)carr, len), EXSUCCEED);

    gen_load_ptr(p_ub, 0, 1, 0);
    gen_load_ubf(p_ub, 0, 1, 0);
    gen_load_view(p_ub, 0, 1, 0);
    
    /* Make second copy of field data (another for not equal test)*/
    s = -1;
    l = -2;
    c = '.';
    f = 1.31;
    d = 112.11;
    carr[0] = 'Y';
    len = strlen(carr);

    assert_equal(Bchg(p_ub, T_SHORT_FLD, 1, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_FLD, 1, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_FLD, 1, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_FLD, 1, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_FLD, 1, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_FLD, 1, (char *)"TEST STRING ARRAY2", 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_FLD, 1, (char *)carr, len), EXSUCCEED);

    gen_load_ptr(p_ub, 1, 2, 0);
    gen_load_ubf(p_ub, 1, 2, 0);
    gen_load_view(p_ub, 1, 2, 0);
    
    l = -4;
    assert_equal(Bchg(p_ub, T_LONG_FLD, 4, (char *)&l, 0), EXSUCCEED);

    s = 212;
    l = 212;
    c = 'b';
    f = 1227;
    d = 1232.1;
    carr[0] = 0;
    assert_equal(Bchg(p_ub, T_SHORT_2_FLD, 1, (char *)&s, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_LONG_2_FLD, 2, (char *)&l, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CHAR_2_FLD, 3, (char *)&c, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_FLOAT_2_FLD, 4, (char *)&f, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_DOUBLE_2_FLD, 5, (char *)&d, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_STRING_2_FLD, 6, (char *)string2, 0), EXSUCCEED);
    assert_equal(Bchg(p_ub, T_CARRAY_2_FLD, 7, (char *)carr, len), EXSUCCEED);
    
    gen_load_ptr(p_ub, 7, 3, 0);
    gen_load_ubf(p_ub, 8, 3, 0);
    gen_load_view(p_ub, 9, 3, 0);
    
}

/**
 * Bfprint testing.
 * @return
 */
Ensure(test_bfprint)
{
    char fb[4096];
    UBFH *p_ub = (UBFH *)fb;
    BFLDLEN len=0;
    FILE *f=NULL;
    char filename[]="/tmp/ubf-test-XXXXXX";
    char readbuf[1024];
    int line_counter=0;
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);

    load_print_test_data(p_ub);
    load_field_table();
    assert_not_equal(mkstemp(filename), EXFAIL);
    assert_not_equal((f=fopen(filename, "w")), NULL);
    assert_equal(Bfprint(p_ub, f), EXSUCCEED);
    fclose(f);

    /* Re-open file in read mode end re-compare the buffer. */
    assert_not_equal((f=fopen(filename, "r")), NULL);

    /* compare the buffers */
    while(NULL!=fgets(readbuf, sizeof(readbuf), f))
    {
        if (NULL==ref_print[line_counter])
        {
            /* reached end of our knowledge of the test data */
            assert_equal_with_message(0, 1, "output file too big!");
            break;
        }
        assert_string_equal(ref_print[line_counter], readbuf);
        line_counter++;
    }
    fclose(f);

    /* remove the file */
    assert_equal(unlink(filename), EXSUCCEED);

    /* cannot print on null file */
    assert_equal(Bfprint(p_ub, NULL), EXFAIL);
    assert_equal(Berror, BEINVAL);
    
}

/**
 * Test data holder for bfprintcb_data
 */
typedef struct bfprintcb_data bfprintcb_data_t;
struct bfprintcb_data
{
    int nrlines;
    char lines[1024][100];
};

/**
 * Write callback, this will fill in passed array
 * @param buffer, we may realloc
 * @param datalen output data len
 * @param dataptr1 custom pointer
 * @param do_write shall the Enduro/X wirte to output log..
 * @param outf output stream currencly used
 * @param fid field it processing
 * @return 
 */
exprivate int test_bfprintcb_writef(char **buffer, long datalen, void *dataptr1, 
        int *do_write, FILE *outf, BFLDID fid)
{
    bfprintcb_data_t *data = (bfprintcb_data_t *)dataptr1;
    
    assert_equal(strlen(*buffer)+1, datalen);
    
    NDRX_STRCPY_SAFE(data->lines[data->nrlines], *buffer);
    data->nrlines++;
    return EXSUCCEED;
}

/**
 * Bfprintcb testing (i.e. callback testing...)
 * @return
 */
Ensure(test_bfprintcb)
{
    char fb[4096];
    UBFH *p_ub = (UBFH *)fb;
    bfprintcb_data_t data;
    int line_counter=0;
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);

    memset(&data, 0, sizeof(data));
    load_print_test_data(p_ub);
    load_field_table();
    
    assert_equal(Bfprintcb(p_ub, test_bfprintcb_writef, (void *)&data), EXSUCCEED);
    UBF_LOG(log_error, "Bfprintcb: %s", Bstrerror(Berror));
    
    /* compare the buffers */
    for (line_counter=0; line_counter<N_DIM(ref_print)-1; line_counter++)
    {
        UBF_LOG(log_debug, "Got line [%s]", data.lines[line_counter]);
        assert_string_equal(data.lines[line_counter], ref_print[line_counter]);
        line_counter++;
    }
    
    assert_equal(data.nrlines, N_DIM(ref_print)-1);
    
    /* cannot print on null file */
    assert_equal(Bfprintcb(p_ub, NULL, NULL), EXFAIL);
    assert_equal(Berror, BEINVAL);
    
}

/**
 * Test bprint
 * There is special note for this!
 * If running in single test mode, then STDOUT will be lost!
 */
Ensure(test_bprint)
{
    char fb[4096];
    UBFH *p_ub = (UBFH *)fb;
    char fb2[4096];
    UBFH *p_ub2 = (UBFH *)fb2;
    BFLDLEN len=0;
    FILE *f;
    int fstdout;
    char filename[]="/tmp/ubf-test-XXXXXX";

    assert_not_equal(mkstemp(filename), EXFAIL);
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Binit(p_ub2, sizeof(fb2)), EXSUCCEED);

    load_print_test_data(p_ub);
    /* nothing much to test here... */
    close(1); /* close stdout */
    assert_not_equal((f=fopen(filename, "w")), NULL);
    fstdout = dup2(fileno(f), 1); /* make file appear as stdout */
    assert_equal(Bprint(p_ub), NULL);
    fclose(f);

    /* OK, if we have that output, try to extread it! */
    assert_not_equal((f=fopen(filename, "r")), NULL);

    assert_equal(Bextread(p_ub2, f), EXSUCCEED);
    /* compare read buffer */
    assert_equal(Bcmp(p_ub, p_ub2), 0);
    /* Remove test file */
    assert_equal(unlink(filename), EXSUCCEED);
}

/**
 * Test function for Bextread, using field IDs (not present in table)
 */
Ensure(test_bextread_bfldid)
{
    char fb[4096];
    UBFH *p_ub = (UBFH *)fb;
    char fb2[4096];
    UBFH *p_ub2 = (UBFH *)fb2;

    BFLDLEN len=0;
    FILE *f;
    char filename[]="/tmp/ubf-test-XXXXXX";

    memset(fb, 6, sizeof(fb));
    memset(fb2, 6, sizeof(fb2));

    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Binit(p_ub2, sizeof(fb2)), EXSUCCEED);

    UBF_LOG(log_debug, "****** load_print_test_data ******");
    load_print_test_data(p_ub);
    UBF_LOG(log_debug, "****** set_up_dummy_data ******");
    set_up_dummy_data(p_ub);

    assert_not_equal(mkstemp(filename), EXFAIL);

    assert_not_equal((f=fopen(filename, "w")), NULL);
    
    UBF_LOG(log_debug, "****** Bfprint ******");
    assert_equal(Bfprint(p_ub, f), EXSUCCEED);
    fclose(f);

    /* read stuff form file */
    assert_not_equal((f=fopen(filename, "r")), NULL);
    UBF_LOG(log_debug, "****** Bextread ******");
    assert_equal(Bextread(p_ub2, f), EXSUCCEED);
    fclose(f);
    
    
    /* compare readed buffer */
    UBF_LOG(log_debug, "****** Bcmp ******");
    assert_equal(Bcmp(p_ub2, p_ub2), 0);
    assert_equal(Bcmp(p_ub, p_ub), 0);
    assert_equal(Bcmp(p_ub, p_ub2), 0);
    /* Remove test file */
    assert_equal(unlink(filename), EXSUCCEED);
}

/**
 * Test function for Bextread, using field names
 */
Ensure(test_bextread_fldnm)
{
    char fb[4096];
    UBFH *p_ub = (UBFH *)fb;
    char fb2[4096];
    UBFH *p_ub2 = (UBFH *)fb2;

    BFLDLEN len=0;
    FILE *f;
    char filename[]="/tmp/ubf-test-XXXXXX";

    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Binit(p_ub2, sizeof(fb2)), EXSUCCEED);

    load_print_test_data(p_ub);
    set_up_dummy_data(p_ub);
    load_field_table();

    assert_not_equal(mkstemp(filename), EXFAIL);
    assert_not_equal((f=fopen(filename, "w")), NULL);
    assert_equal(Bfprint(p_ub, f), EXSUCCEED);
    fclose(f);

    /* read stuff form file */
    assert_not_equal((f=fopen(filename, "r")), NULL);
    assert_equal(Bextread(p_ub2, f), EXSUCCEED);
    fclose(f);

    /* compare readed buffer */
    assert_equal(Bcmp(p_ub, p_ub2), 0);
    /* Remove test file */
    assert_equal(unlink(filename), EXSUCCEED);
}

/**
 * Return the buffer line
 * @param buffer buffer to put in the result. Note that is should go line by line
 * @param bufsz buffer size
 * @param dataptr1 user data ptr
 * @return number of bytes written to buffer
 */
exprivate long bextreadcb_readf(char *buffer, long bufsz, void *dataptr1)
{
    int *idx = (int *)dataptr1;
    
    char *data_buffers[]= {
        "T_SHORT_FLD\t88\n",
        "T_SHORT_FLD\t-1\n",
        "T_SHORT_2_FLD\t0\n",
        "T_SHORT_2_FLD\t212\n",
        "T_LONG_FLD\t-1021\n",
        "T_LONG_FLD\t-2\n",
        "T_LONG_FLD\t0\n",
        "T_LONG_FLD\t0\n", /* <<< error line */
        NULL
    };
    
    if (NULL!=data_buffers[*idx])
    {
        NDRX_STRCPY_SAFE_DST(buffer, data_buffers[*idx], bufsz);
        
        (*idx)++;
        return strlen(buffer)+1;
    }
    else
    {
        return 0;
    }
}

/**
 * Test extread with callbacks
 */
Ensure(test_bextreadcb)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    int idx = 0;
    char *tree;
    
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Bextreadcb(p_ub, bextreadcb_readf, (void *)&idx), EXSUCCEED);
    
    /* test with boolean expression */
    tree = Bboolco("T_SHORT_FLD == 88 && T_SHORT_FLD[1] == -1 && T_SHORT_2_FLD==0 "
            "&& T_SHORT_2_FLD[1]==212 && T_LONG_FLD==-1021 && T_LONG_FLD[1]==-2 "
            "&& T_LONG_FLD[2]==0 && T_LONG_FLD[3]==0");
    
    assert_not_equal(tree, NULL);
    
    assert_equal(Bboolev(p_ub, tree), EXTRUE);
    
    Btreefree(tree);

}

/**
 * Testing extread for errors
 */
Ensure(test_bextread_chk_errors)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;

    char *missing_new_line_at_end[]= {
        "T_SHORT_FLD\t88\n",
        "T_SHORT_FLD\t-1\n",
        "T_SHORT_2_FLD\t0\n",
        "T_SHORT_2_FLD\t212\n",
        "T_LONG_FLD\t-1021\n",
        "T_LONG_FLD\t-2\n",
        "T_LONG_FLD\t0\n",
        "T_LONG_FLD\t0", /* <<< error line */
        NULL
    };

    char *no_field_name[]= {
        "T_SHORT_FLD\t-1\n",
        "\t0\n", /* <<< error line */
        "T_SHORT_2_FLD\t212\n",
        "T_LONG_FLD\t-1021\n",
        "T_LONG_FLD\t-2\n",
        "T_LONG_FLD\t0\n",
        "T_LONG_FLD\t0\n",
        NULL
    };
    
    char *no_value_seperator[]= {
        "T_SHORT_FLD\t-1\n",
        "T_SHORT_2_FLD\t212\n",
        "T_LONG_FLD\t-1021\n",
        "T_LONG_FLD -2\n",/* <<< error line */
        "T_LONG_FLD\t0\n",
        "T_LONG_FLD\t0\n",
        NULL
    };

    char *prefix_error_hex[]= {
        "T_SHORT_FLD\t-1\n",
        "T_SHORT_2_FLD\t212\n",
        "T_LONG_FLD\t-1021\n",
        "T_LONG_FLD\t0\n",
        "T_LONG_FLD\t0\n",
        "T_STRING_FLD\t\\\n", /* <<< error on this line, strange prefixing.*/
        NULL
    };

    char *invalid_hex_number[]= {
        "T_SHORT_FLD\t-1\n",
        "T_SHORT_2_FLD\t212\n",
        "T_LONG_FLD\t-1021\n",
        "T_LONG_FLD\t0\n",
        "T_LONG_FLD\t0\n",
        "T_STRING_FLD\tabc\\yu123\n", /* <<< error on this line, strange prefixing.*/
        NULL
    };

    char *empty_line_error[]= {
        "\n", /* <<< fail on empty line */
        NULL
    };

    char *invalid_field_id[]= {
        "T_SHORT_FLD\t-1\n",
        "IVALID_FIELD_NAME\t212\n", /* <<< error on this line */
        NULL
    };

    char *invalid_field_id_syntax[]= {
        "T_SHORT_FLD\t-1\n",
        "((BFLDID32)4294967295)\t212\n", /* <<< error on this line */
        NULL
    };
    
    /* Check view does not exists */
    char *invalid_view[]= {
        "T_VIEW_FLD\tNO_SUCH_VIEW\n",
        NULL
    };
    
    /* Check subfield for non-non ubf type */
    char *invalid_sub_buffer[]= {
        "T_STRING_FLD\t\n",
        "\tT_STRING_FLD\tXXX\n",
        "\tT_STRING_FLD\tYYY\n",
        NULL
    };
        
    /* load field table */
    load_field_table();

    /*--------------------------------------------------------*/
    /* test the newline is missing at the end */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(missing_new_line_at_end);
    close_test_temp();

    /* file not opened - unix error - might cause double free!?!? */
    /*M_test_temp_file=NULL;
    assert_equal(Bextread(p_ub, M_test_temp_file), FAIL);
    assert_equal(Berror, BEUNIX);
     */

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BSYNTAX);
    close_test_temp();

    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* test the newline is missing at the end */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(no_field_name);
    close_test_temp();
    
    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BSYNTAX);
    close_test_temp();

    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* test the newline is missing at the end */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(no_value_seperator);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BSYNTAX);
    close_test_temp();

    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* test bad prefixing */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(prefix_error_hex);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BSYNTAX);
    close_test_temp();

    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* invalid hex number provided */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(invalid_hex_number);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BSYNTAX);
    close_test_temp();

    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* Empty line also is not supported */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(empty_line_error);
    close_test_temp();

    /* syntax error should fail */
    /* to be backwards compatible we just ignore this stuff... */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXSUCCEED);
    /* assert_equal(Berror, BSYNTAX); */
    close_test_temp();
    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* Field id not found */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(invalid_field_id);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BBADNAME);
    close_test_temp();
    /* now open the file */
    remove_test_temp();
    /*--------------------------------------------------------*/
    /* Invalid bfldid syntax */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(invalid_field_id_syntax);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BBADFLD);
    close_test_temp();
    /* now open the file */
    remove_test_temp();

    /*--------------------------------------------------------*/
    /* Invalid view */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(invalid_view);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BBADVIEW);
    close_test_temp();
    /* now open the file */
    remove_test_temp();
    
    
    /*--------------------------------------------------------*/
    /* Invalid sub-buffer */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(invalid_sub_buffer);
    close_test_temp();

    /* syntax error should fail */
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BSYNTAX);
    close_test_temp();
    /* now open the file */
    remove_test_temp();
    
    
}

/**
 * Testing extread for errors
 */
Ensure(test_bextread_comments)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    short s;
    long l;

    char *comment_test[]= {
        "T_SHORT_FLD\t88\n",
        "#T_SHORT_2_FLD\t212\n",
        "#T_STRING_FLD\t-1021\n",
        "#T_STRING_FLD\t-2\n",
        "T_LONG_FLD\t-1\n",
        NULL
    };
    
    /* load field table */
    load_field_table();

    /*--------------------------------------------------------*/
    /* Testing comment. */
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(comment_test);
    close_test_temp();
    
    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXSUCCEED);

    /* Ensure that fields are missing */
    assert_equal(Bpres(p_ub, T_SHORT_2_FLD, 0), EXFALSE);
    assert_equal(Bpres(p_ub, T_STRING_FLD, 0), EXFALSE);
    assert_equal(Bpres(p_ub, T_STRING_FLD, 1), EXFALSE);

    /* Ensure that we have what we expect */
    assert_equal(Bget(p_ub, T_SHORT_FLD, 0, (char *)&s, 0), EXSUCCEED);
    assert_equal(s, 88);
    assert_equal(Bget(p_ub, T_LONG_FLD, 0, (char *)&l, 0), EXSUCCEED);
    assert_equal(l, -1);
    close_test_temp();
    
    /* now open the file */
    remove_test_temp();
}

/**
 * Test - flag for buffer delete
 */
Ensure(test_bextread_minus)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    char fb2[2048];
    UBFH *p_ub2 = (UBFH *)fb2;
    short s;
    long l;

    char *test_minus[]= {
        "T_SHORT_FLD\t123\n",
        "T_DOUBLE_FLD\t0.1\n",
        "T_CARRAY_FLD\tABCDE\n",
        "T_STRING_FLD\tTEST_STRING\n",
        "T_FLOAT_FLD\t1\n",
        "T_PTR_FLD\t0x1111\n",
        "T_UBF_FLD\t\n",
        "\tT_STRING_FLD\tHELLO\n",
        "T_VIEW_FLD\t\n",
        "- T_CARRAY_FLD\tABCDE\n",
        "- T_STRING_FLD\tTEST_STRING\n",
        "- T_PTR_FLD\t0x0\n",
        "- T_UBF_FLD\t\n",
        "- T_VIEW_FLD\t\n",
        NULL
    };

    /* load field table */
    load_field_table();
    
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Binit(p_ub2, sizeof(fb2)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(test_minus);
    close_test_temp();

    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXSUCCEED);
    close_test_temp();

    /* now open the file */
    remove_test_temp();

    /* Load reference data into buffer 2 */
    assert_equal(CBchg(p_ub2, T_SHORT_FLD, 0, "123", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_DOUBLE_FLD, 0, "0.1", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_FLOAT_FLD, 0, "1", 0, BFLD_STRING), EXSUCCEED);

    /* Compare buffers now should be equal */
    assert_equal(Bcmp(p_ub, p_ub2), 0);
}

/**
 * Test - flag for buffer delete
 */
Ensure(test_bextread_plus)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    char fb2[2048];
    UBFH *p_ub2 = (UBFH *)fb2;
    short s;
    long l;
    
    /* Check with sub-fields */
    char *test_plus[]= {
        "T_SHORT_FLD\t999\n",
        "T_DOUBLE_FLD\t888\n",
        "T_FLOAT_FLD\t777.11\n",
        "T_STRING_FLD\tABC\n",
        "T_PTR_FLD\t0x0\n",
        "T_UBF_FLD\t\n",
        "T_VIEW_FLD\t\n",
        "+ T_SHORT_FLD\t123\n",
        "+ T_DOUBLE_FLD\t0.1\n",
        "+ T_FLOAT_FLD\t1\n",
        "+ T_STRING_FLD\tCDE\n",
        "+ T_PTR_FLD\t0x2329\n",
        /* Set empty, override with real values */
        "+ T_UBF_FLD\t\n",
	"\tT_STRING_2_FLD\tZ\n",
        "\t+ T_STRING_2_FLD\tT\n", /* so root level shall get U */
        "+ T_VIEW_FLD\tUBTESTVIEW2\n",
        "\ttshort1\t1\n",
	"\ttlong1\t2\n",
	"\ttchar1\t3\n",
	"\ttfloat1\t4.00000\n",
	"\ttdouble1\t5.000000\n",
	"\ttstring1\tg\n",
	"\ttcarray1\tg\\00\\00\\00\\00\\00\\00\\00\\00\\00\n",
	"\t+ tshort1\t2\n",
	"\t+ tlong1\t3\n",
	"\t+ tchar1\t4\n",
	"\t+ tfloat1\t5.00000\n",
	"\t+ tdouble1\t6.000000\n",
	"\t+ tstring1\tB\n",
	"\t+ tcarray1\tD\\00\\09\\09\\09\\09\\09\\09\\09\\09\n",
        NULL
    };

    /* load field table */
    load_field_table();

    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Binit(p_ub2, sizeof(fb2)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(test_plus);
    close_test_temp();

    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXSUCCEED);
    close_test_temp();
    /* now open the file */
    remove_test_temp();
    
    /* Load reference data into buffer 2 */
    assert_equal(CBchg(p_ub2, T_SHORT_FLD, 0, "123", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_DOUBLE_FLD, 0, "0.1", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_FLOAT_FLD, 0, "1", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_STRING_FLD, 0, "CDE", 0, BFLD_STRING), EXSUCCEED);
    
    /* load occ1 ptr/ubf/view */
    gen_load_ptr(p_ub2, 0, 1, 0);
    gen_load_ubf(p_ub2, 0, 1, 0);
    gen_load_view(p_ub2, 0, 1, 0);

    /* Compare buffers now should be equal */
    assert_equal(Bcmp(p_ub, p_ub2), 0);
}


/**
 * Test - Test buffer EQ op
 */
Ensure(test_bextread_eq)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    char fb2[2048];
    UBFH *p_ub2 = (UBFH *)fb2;
    short s;
    long l;
    
    /* TODO: CHeck with sub-fields */
    char *test_eq[]= {
        "T_SHORT_FLD\t999\n",
        "T_LONG_FLD\t124545\n",
        "T_CHAR_FLD\ta\n",
        "T_DOUBLE_FLD\t888\n",
        "T_FLOAT_FLD\t777.11\n",
        "T_STRING_FLD\tABC\n",
        "T_CARRAY_FLD\tEFGH\n",
        "T_PTR_FLD\t0x2329\n",
        "T_UBF_FLD\t\n",
        "\tT_STRING_2_FLD\tT\n",
        "T_VIEW_FLD\tUBTESTVIEW2\n",
        "\ttshort1\t2\n",
        "\ttlong1\t3\n",
        "\ttchar1\t4\n",
        "\ttfloat1\t5.00000\n",
        "\ttdouble1\t6.000000\n",
        "\ttstring1\tB\n",
        "\ttcarray1\tD\\00\\09\\09\\09\\09\\09\\09\\09\\09\n",
        /* Value will be copied to FLD2 */
        "= T_SHORT_2_FLD\tT_SHORT_FLD\n",
        "= T_LONG_2_FLD\tT_LONG_FLD\n",
        "= T_CHAR_2_FLD\tT_CHAR_FLD\n",
        "= T_DOUBLE_2_FLD\tT_DOUBLE_FLD\n",
        "= T_FLOAT_2_FLD\tT_FLOAT_FLD\n",
        "= T_STRING_2_FLD\tT_STRING_FLD\n",
        "= T_CARRAY_2_FLD\tT_CARRAY_FLD\n",
        "= T_PTR_2_FLD\tT_PTR_FLD\n",
        /* well in this case we shall not search for view, but just take value of  */
        "= T_VIEW_2_FLD\tT_VIEW_FLD\n",
        "= T_UBF_2_FLD\tT_UBF_FLD\n",
        NULL
    };

    /* load field table */
    load_field_table();

    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    assert_equal(Binit(p_ub2, sizeof(fb2)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(test_eq);
    close_test_temp();

    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXSUCCEED);
    close_test_temp();
    /* now open the file */
    remove_test_temp();

    /* Load reference data into buffer 2 */
    assert_equal(CBchg(p_ub2, T_SHORT_FLD, 0, "999", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_LONG_FLD, 0, "124545", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_CHAR_FLD, 0, "a", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_FLOAT_FLD, 0, "777.11", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_DOUBLE_FLD, 0, "888", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_STRING_FLD, 0, "ABC", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_CARRAY_FLD, 0, "EFGH", 0, BFLD_STRING), EXSUCCEED);

    assert_equal(CBchg(p_ub2, T_SHORT_2_FLD, 0, "999", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_LONG_2_FLD, 0, "124545", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_CHAR_2_FLD, 0, "a", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_FLOAT_2_FLD, 0, "777.11", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_DOUBLE_2_FLD, 0, "888", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_STRING_2_FLD, 0, "ABC", 0, BFLD_STRING), EXSUCCEED);
    assert_equal(CBchg(p_ub2, T_CARRAY_2_FLD, 0, "EFGH", 0, BFLD_STRING), EXSUCCEED);
    
    /* prepare data */
    gen_load_ptr(p_ub2, 0, 1, 0);
    gen_load_ubf(p_ub2, 0, 1, 0);
    gen_load_view(p_ub2, 0, 1, 0);
    
    gen_load_ptr(p_ub2, 0, 1, 1);
    gen_load_ubf(p_ub2, 0, 1, 1);
    gen_load_view(p_ub2, 0, 1, 1);
    
    assert_equal(Bcmp(p_ub, p_ub2), 0);
    
}

/**
 * Test - Test buffer EQ failure
 */
Ensure(test_bextread_eq_err)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    short s;
    long l;

    /* TODO: CHeck with sub-fields ? + PTR */
    char *test_eq_err[]= {
        "T_SHORT_FLD\t999\n",
        "T_LONG_FLD\t124545\n",
        "T_CHAR_FLD\ta\n",
        "T_DOUBLE_FLD\t888\n",
        "T_FLOAT_FLD\t777.11\n",
        "T_STRING_FLD\tABC\n",
        "T_CARRAY_FLD\tEFGH\n",
        /* Value will be copied to FLD2 */
        "= T_SHORT_2_FLD\tT_SHORT_FLD\n",
        "= T_LONG_2_FLD\tT_LONG_FLD\n",
        "= T_CHAR_2_FLD\tT_CHAR_FLD\n",
        "= T_DOUBLE_2_FLD\tT_DOUBLE_FLD\n",
        "= T_FLOAT_2_FLD\tT_FLOAT_FLD\n",
        "= T_STRING_2_FLD\tFIELD_NOT_PRESENT\n", /* <<< error on this lines */
        "= T_CARRAY_2_FLD\tT_CARRAY_FLD\n",
        NULL
    };
    /* load field table */
    load_field_table();
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(test_eq_err);
    close_test_temp();

    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXFAIL);
    assert_equal(Berror, BBADNAME);
    close_test_temp();
    /* now open the file */
    remove_test_temp();
}

/**
 * Check that by default, pointers are not read
 */
Ensure(test_bextread_no_ptr)
{
    char fb[2048];
    UBFH *p_ub = (UBFH *)fb;
    char buf[128];
    BFLDLEN len;
    
    char *test_ptr[]= {
        "T_STRING_FLD\tABC\n",
        "T_PTR_FLD\t111\n",
        NULL
    };
    
    /* remove settings... */
    unsetenv("NDRX_APIFLAGS");
    
    /* load field table */
    load_field_table();
    assert_equal(Binit(p_ub, sizeof(fb)), EXSUCCEED);
    open_test_temp("w");
    write_to_temp(test_ptr);
    close_test_temp();

    open_test_temp_for_read("r");
    assert_equal(Bextread(p_ub, M_test_temp_file), EXSUCCEED);
    
    /* check buffer... */
    len = sizeof(buf);
    assert_equal(Bget(p_ub, T_STRING_FLD, 0, buf, &len), EXSUCCEED);
    assert_string_equal(buf, "ABC");
    
    assert_equal(CBget(p_ub, T_PTR_FLD, 0, buf, &len, BFLD_STRING), EXFAIL);
    assert_equal(Berror, BNOTPRES);
    
    close_test_temp();
    /* now open the file */
    remove_test_temp();
}

TestSuite *ubf_print_tests(void)
{
    TestSuite *suite = create_test_suite();

    std_basic_setup();
    
    add_test(suite, test_bfprintcb);
     
    add_test(suite, test_bprint);
    add_test(suite, test_bfprint);
    
    add_test(suite, test_bextread_bfldid);
    add_test(suite, test_bextread_fldnm);
    add_test(suite, test_bextread_chk_errors);
    add_test(suite, test_bextread_comments);
    add_test(suite, test_bextread_minus);
    add_test(suite, test_bextread_plus);
    add_test(suite, test_bextread_eq);
    add_test(suite, test_bextread_eq_err);
    add_test(suite, test_bextreadcb);
    add_test(suite, test_bextread_no_ptr);

    return suite;
}
/* vim: set ts=4 sw=4 et smartindent: */
