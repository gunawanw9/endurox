/**
 *
 * @file test_mkfldhdr.c
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
 * Check duplicate field table resolve
 */
Ensure(test_dup_tab_resolve)
{
    setenv("FLDTBLDIR", "./ubftab", 1);
    setenv("FIELDTBLS", "dup.test", 1);
    
    /* resolves to last loaded */
    assert_equal(Bfldid("T_DUP_FLD"), 67112876);
    
    /* resolves to last loaded. */
    assert_string_equal(Bfname(67112875), "T_HELLO_FLD");
}

/**
 * Calls scripts for checing mkfldhdr. Return code says
 * was OK or not OK.
 */
Ensure(test_mkfldhdr)
{
    load_field_table();
    assert_equal(system("./test_mkfldhdr_cmd.sh"), EXSUCCEED);
    assert_equal(system("./test_mkfldhdr_env.sh"), EXSUCCEED);
    assert_equal(system("./test_mkfldhdr_env_multidir.sh"), EXSUCCEED);
    assert_equal(system("./test_mkfldhdr_dup.sh"), EXSUCCEED);
    assert_not_equal(system("./test_mkfldhdr_err_output.sh"), EXSUCCEED);
    assert_not_equal(system("./test_mkfldhdr_no_FLDTBLDIR.sh"), EXSUCCEED);
    assert_not_equal(system("./test_mkfldhdr_no_FIELDTBLS.sh"), EXSUCCEED);
    assert_not_equal(system("./test_mkfldhdr_syntax_err.sh"), EXSUCCEED);

}

TestSuite *ubf_mkfldhdr_tests(void)
{
    TestSuite *suite = create_test_suite();

    add_test(suite, test_mkfldhdr);
    add_test(suite, test_dup_tab_resolve);

    return suite;
}


/* vim: set ts=4 sw=4 et smartindent: */
