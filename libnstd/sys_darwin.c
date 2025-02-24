/**
 * @brief Darwin Abstraction Layer (DAL)
 *
 * @file sys_darwin.c
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
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <mach/mach_time.h>

#include <ndrstandard.h>
#include <ndebug.h>
#include <nstdutil.h>
#include <limits.h>
#include <sys_unix.h>
#include <utlist.h>


/*---------------------------Externs------------------------------------*/
/*---------------------------Macros-------------------------------------*/
#define MT_NANO (+1.0E-9)
#define MT_GIGA UINT64_C(1000000000)
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/* TODO create a list of timers,*/
static double mt_timebase = 0.0;
static uint64_t mt_timestart = 0;
/*---------------------------Prototypes---------------------------------*/

/**
 * Return list of message queues (actually it is list of named pipes
 * as work around for missing posix queue listing functions.
 */
expublic string_list_t* ndrx_sys_mqueue_list_make_pl(char *qpath, int *return_status)
{
    return ndrx_sys_folder_list(qpath, return_status);
}

#ifndef EX_OS_DARWIN_HAVE_CLOCK
/* TODO be more careful in a multithreaded environement */
expublic int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    kern_return_t retval = KERN_SUCCESS;
    if( clk_id == TIMER_ABSTIME)
    {
        if (!mt_timestart)
        { 
            /* only one timer, initilized on the first call to the TIMER */
            mach_timebase_info_data_t tb = { 0 };
            mach_timebase_info(&tb);
            mt_timebase = tb.numer;
            mt_timebase /= tb.denom;
            mt_timestart = mach_absolute_time();
        }

        double diff = (mach_absolute_time() - mt_timestart) * mt_timebase;
        tp->tv_sec = diff * MT_NANO;
        tp->tv_nsec = diff - (tp->tv_sec * MT_GIGA);
    }
    else
    {
        /* other clk_ids are mapped to the coresponding mach clock_service */
        clock_serv_t cclock;
        mach_timespec_t mts;

        host_get_clock_service(mach_host_self(), clk_id, &cclock);
        retval = clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);

        tp->tv_sec = mts.tv_sec;
        tp->tv_nsec = mts.tv_nsec;
    }

    return retval;
}
#endif

/**
 * Test the pid to contain regexp 
 * @param pid process id to test
 * @param p_re compiled regexp to test against
 * @return -1 failed, 0 - not matched, 1 - matched
 */
expublic int ndrx_sys_env_test(pid_t pid, regex_t *p_re)
{
    return ndrx_sys_cmdout_test("ps eww %d", pid, p_re);
}

/**
 * Pthreads spinlock emulation, init
 * @param lock spinlock
 * @param pshared (not used)
 * @return EXSUCCEED
 */
expublic int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    __asm__ __volatile__ ("" ::: "memory");
    *lock = 0;
    return EXSUCCEED;
}

/**
 * Destroy spinlock (does nothing)
 * @param lock spinlock var
 * @return EXSUCCEED
 */
expublic int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    return EXSUCCEED;
}

/**
 * Acquire spin lock
 * @param lock lock variable
 * @return EXSUCCEED
 */
expublic int pthread_spin_lock(pthread_spinlock_t *lock)
{
    while (1) 
    {
        int i;
        for (i=0; i < 10000; i++) 
        {
            if (__sync_bool_compare_and_swap(lock, 0, 1)) 
            {
                return EXSUCCEED;
            }
        }
        sched_yield();
    }
    
}

/**
 * Try lock to acquire lock
 * @param lock lock variable
 * @return EXSUCCEED/EBUSY
 */
expublic int pthread_spin_trylock(pthread_spinlock_t *lock) 
{
    if (__sync_bool_compare_and_swap(lock, 0, 1)) 
    {
        return EXSUCCEED;
    }
    return EBUSY;
}

/**
 * Unlock variable
 * @param lock spin lock variable
 * @return EXSUCCEED
 */
expublic int pthread_spin_unlock(pthread_spinlock_t *lock) 
{
    __asm__ __volatile__ ("" ::: "memory");
    *lock = 0;
    return EXSUCCEED;
}

/* vim: set ts=4 sw=4 et smartindent: */
