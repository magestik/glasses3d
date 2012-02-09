#include <linux/interrupt.h>

#ifdef SYNC_WITH_IRQ
	void sync_init_irq(void);
	void sync_stop_irq(void);

	irqreturn_t sync_interrupt(int irq, void *dev_id);
#endif

#ifdef SYNC_WITH_FS
	void sync_init_fs(void);
	void sync_stop_fs(void);
#endif

void sync_init(void);
void sync_swap(void);
void sync_stop(void);
