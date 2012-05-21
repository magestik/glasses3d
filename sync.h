#define SYNC_WITH_FS
//#define SYNC_WITH_IRQ

#ifndef SYNC_H
#define SYNC_H

#include <linux/interrupt.h>

#ifdef SYNC_WITH_IRQ
	void sync_init_irq(void);
	void sync_stop_irq(void);

	irqreturn_t sync_interrupt(int irq, void *dev_id);
#endif

#ifdef SYNC_WITH_FS
	int sync_write_eye(struct ctl_table *table, int write, void __user *buffer, size_t *lenp, loff_t *ppos);
	void sync_init_fs(void);
	void sync_stop_fs(void);
#endif

void sync_init(void);
void sync_stop(void);

#endif
