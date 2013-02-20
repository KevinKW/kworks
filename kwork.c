/* vim: set expandtab ts=4 sw=4:                               */

/*-===========================================================-*/
/*  Author:                                                    */
/*        KevinKW                                              */
/*  Date:                                                      */
/*        2013-02-20                                           */
/*                                                             */
/*                                                             */
/*-===========================================================-*/

#include <kwtp.h>
#include <kworks.h>

static LIST_HEAD(kworks);
static DEFINE_SPINLOCK(kworks_lock);
static DECLARE_WAIT_QUEUE_HEAD(kwork_event);

void kwork_init(struct kwork *work,
                void (*func)(void *data),
                void (*begin)(struct kwork *work),
                void (*end)(struct kwork *work),
                void *data)
{
    INIT_LIST_HEAD(&work->link);
    work->func = func;
    work->begin = begin;
    work->end = end;
    work->data = data;
}
EXPORT_SYMBOL(kwork_init);

void kwork_dispatch(struct kwork *work)
{
    unsigned long flags;
    spin_lock_irqsave(&kworks_lock, flags);
    list_add_tail(&work->link, &kworks);
    spin_unlock_irqrestore(&kworks_lock, flags);
    wake_up(&kwork_event);
}
EXPORT_SYMBOL(kwork_dispatch);

int kworker(void *data)
{
    struct kwork *work;
    struct kw_thread *thread = (struct kw_thread *)data;
    unsigned long flags;

    if (!(thread->task->flags & PF_MEMALLOC)) {
        thread->task->flags |= PF_MEMALLOC;
    }

    while (!kthread_should_stop()) {
        wait_event_interruptible(kwork_event,
                kthread_should_stop() || !list_empty(&kworks));

        if (kthread_should_stop()) {
            goto out;
        }

        spin_lock_irqsave(&kworks_lock, flags);
        while (!list_empty(&kworks)) {
            // Get one work from list and execute it
            work = list_entry(kworks.next, struct kwork, link);
            list_del_init(&work->link);
            spin_unlock_irqrestore(&kworks_lock, flags);
            work->begin(work);
            work->func(work->data);
            work->end(work);
            if (unlikely(kthread_should_stop())) {
                goto out;
            }
            spin_lock_irqsave(&kworks_lock, flags);
        }
        spin_unlock_irqrestore(&kworks_lock, flags);
    }
  out:
    return 0;
}
