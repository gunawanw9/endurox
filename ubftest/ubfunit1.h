/**
 * @brief UBF and standard library unit tests..
 *
 * @file ubfunit1.h
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

#ifndef UBFUNIT1_H
#define	UBFUNIT1_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <cgreen/cgreen.h>
#include <ubf.h>
#include <extest.h>


#define BIG_TEST_STRING \
"as0dfuoij34n5roi32u98ujh;oaot3i2h4;523juh4l8y7af8sdufjk34i5ry7834h532kj4h5rt89"\
"iaudoifua09s8df0a870s882390480923u4uj23hnkjabndf98yhpugb345234234234*/-*//*/-9"\
"adslkflkasdkfjhkjhk3482u3940u23hj4;lhn2l3h4283u49;2j3kln;kjbn.kvnb.fbzxkchvzxc"\
"asdjf;alkjdfkajkdfjklru902034jtp²45kjl34nltn3klmnbkkjlkvnbmxvb.xvsdlfsdfsfdsfs"\
"-809u89wy679856674r5e6dytfvkgjhknjlk;i0-=87p96yotghlj;io876tgyuhj90a7usdyhczcv"\
"sajdfoyuas9ohfbn;23ru7gbv98yzgbpiuzhbncvl/w8e4hr3p289ubhr;.kljfgn;zucbuvlbsdas"\
"ajsdof8w3y80r9h23oij4ne2ljnjjksdghifopasyd8f09u[2o34h;kjrh23;ijrfpdysahpugvasz"\
"adslkflkasdkfjhkjhk3482u3940u23hj4;lhn2l3h4283u49;2j3kln;kjbn.kvnb.fbzxkchvzxc"

/* globals */
extern char M_test_temp_filename[];
extern FILE *M_test_temp_file;

/* Common functions */
extern void load_field_table(void);
extern void set_up_dummy_data(UBFH *p_ub);
extern void do_dummy_data_test(UBFH *p_ub);
extern void randomize_test_data(char *ptr, int size);
extern void load_recursive_data(UBFH *p_ub);
extern void std_basic_setup(void);

extern void gen_test_ubf_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff);
extern void gen_test_view_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff);
extern void gen_test_ptr_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff);


extern void gen_test_ubf_val_dbg(char *file, int line, UBFH *p_ub,  int offset, BFLDLEN *len);
extern void gen_test_ptr_val_dbg(char *file, int line, ndrx_longptr_t ptr,  int offset, BFLDLEN *len);
extern void gen_test_view_val_dbg(char *file, int line, BVIEWFLD *vf, int offset, BFLDLEN *len);


extern void gen_load_ubf_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff);
extern void gen_load_view_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff);
extern void gen_load_ptr_dbg(char *file, int line, UBFH *p_ub, BFLDOCC occ, int offset, BFLDID32 fldoff);

extern void gen_set_ubf_dbg(char *file, int line, UBFH *p_ub_tmp, int offset);
extern void gen_set_view_dbg(char *file, int line, BVIEWFLD *vf, int offset);

#define gen_test_ubf(P_UB, OCC, OFFSET, FLDOFF) gen_test_ubf_dbg(__FILE__, __LINE__, P_UB, OCC, OFFSET, FLDOFF)
#define gen_test_view(P_UB, OCC, OFFSET, FLDOFF) gen_test_view_dbg(__FILE__, __LINE__, P_UB, OCC, OFFSET, FLDOFF)
#define gen_test_ptr(P_UB, OCC, OFFSET, FLDOFF) gen_test_ptr_dbg(__FILE__, __LINE__, P_UB, OCC, OFFSET, FLDOFF)

#define gen_load_ubf(P_UB, OCC, OFFSET, FLDOFF) gen_load_ubf_dbg(__FILE__, __LINE__, P_UB, OCC, OFFSET, FLDOFF)
#define gen_load_view(P_UB, OCC, OFFSET, FLDOFF) gen_load_view_dbg(__FILE__, __LINE__, P_UB, OCC, OFFSET, FLDOFF)
#define gen_load_ptr(P_UB, OCC, OFFSET, FLDOFF) gen_load_ptr_dbg(__FILE__, __LINE__, P_UB, OCC, OFFSET, FLDOFF)

extern void open_test_temp(char *mode);
extern void open_test_temp_for_read(char *mode);
extern void write_to_temp(char **data);
extern void close_test_temp(void);
extern void remove_test_temp(void);

/* Test suites */
extern TestSuite *ubf_Badd_tests(void);
extern TestSuite *ubf_genbuf_tests(void);
extern TestSuite *ubf_cfchg_tests(void);
extern TestSuite *ubf_cfget_tests(void);
extern TestSuite *ubf_expr_tests(void);
extern TestSuite *ubf_fdel_tests(void);
extern TestSuite *ubf_fnext_tests(void);
extern TestSuite *ubf_fproj_tests(void);
extern TestSuite *ubf_mem_tests(void);
extern TestSuite *ubf_fupdate_tests(void);
extern TestSuite *ubf_fconcat_tests(void);
extern TestSuite *ubf_find_tests(void);
extern TestSuite *ubf_get_tests(void);
extern TestSuite *ubf_print_tests(void);
extern TestSuite *ubf_printv_tests(void);
extern TestSuite *ubf_macro_tests(void);
extern TestSuite *ubf_readwrite_tests(void);
extern TestSuite *ubf_mkfldhdr_tests(void);
extern TestSuite *ubf_bcmp_tests(void);
extern TestSuite *ubf_bnum_tests(void);
extern TestSuite *ubf_bjoin_tests(void);
extern TestSuite *ubf_bojoin_tests(void);
extern TestSuite *ubf_embubf_tests(void);

/* Standard library suites */
extern TestSuite *ubf_nstd_crypto(void);
extern TestSuite *ubf_nstd_base64(void);
extern TestSuite *ubf_nstd_growlist(void);

extern TestSuite *ubf_nstd_mtest(void);
extern TestSuite *ubf_nstd_mtest2(void);
extern TestSuite *ubf_nstd_mtest3(void);
extern TestSuite *ubf_nstd_mtest4(void);
extern TestSuite *ubf_nstd_mtest5(void);
extern TestSuite *ubf_nstd_mtest6_dupcursor(void);
extern TestSuite *ubf_nstd_mtest6(void);
extern TestSuite *ubf_nstd_mtest7(void);
extern TestSuite *test_nstd_macros(void);
extern TestSuite *ubf_nstd_fpa(void);
extern TestSuite * ubf_nstd_debug(void);
extern TestSuite * ubf_nstd_standard(void);
extern TestSuite *ubf_nstd_util(void);
extern TestSuite *ubf_nstd_lh(void);
extern TestSuite *ubf_nstd_atomicadd(void);
extern TestSuite *ubf_nstd_fsync(void);
extern TestSuite *ubf_nstd_cid(void);

#ifdef	__cplusplus
}
#endif

#endif	/* UBFUNIT1_H */

/* vim: set ts=4 sw=4 et smartindent: */
