/* vim: set expandtab ts=4 sw=4:                               */

/*-===========================================================-*/
/*  Author:                                                    */
/*        KevinKW                                              */
/*  Date:                                                      */
/*        2013-02-20                                           */
/*                                                             */
/*                                                             */
/*-===========================================================-*/

#ifndef __KWORKS_H__
#define __KWORKS_H__

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/version.h>
#include <linux/module.h>
#include <asm/bug.h>

struct kwork {
    struct list_head link;
    void (*func)(void *data);
    void (*begin)(struct kwork *work);
    void (*end)(struct kwork *work);
    void *data;
};

extern void kwork_init(struct kwork *work,
                       void (*func)(void *data),
                       void (*begin)(struct kwork *work),
                       void (*end)(struct kwork *work),
                       void *data);
extern void kwork_dispatch(struct kwork *work);
#endif //__KWORKS_H__
