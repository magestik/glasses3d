#include "sync.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysctl.h>

#include "glasses3d.h"

/*** Fonctions de synchro ***/

void sync_init() {
	#ifdef SYNC_WITH_IRQ
		sync_init_irq();
	#endif
	
	#ifdef SYNC_WITH_FS
		sync_init_fs();
	#endif
}

void sync_stop() {
	#ifdef SYNC_WITH_IRQ
		sync_stop_irq();
	#endif
	
	#ifdef SYNC_WITH_FS
		sync_stop_fs();
	#endif
}

/*** Synchronisation manuelle depuis /proc/sys/glasses3d ***/

#ifdef SYNC_WITH_FS

static struct ctl_table_header * test_sysctl_header;

int min = 0;
int max = 1;

static ctl_table test_table[] = {
	{
		.procname	= "eye",
		.data		= &current_eye,
		.maxlen		= sizeof(int),
		.mode		= 0666,
		.proc_handler	= &sync_write_eye,
		.extra1		= &min,
		.extra2		= &max
	},
	{
		.procname	= "inversed",
		.data		= &inversed,
		.maxlen		= sizeof(int),
		.mode		= 0666,
		.proc_handler	= &proc_dointvec_minmax,
		.extra1     = &min,
		.extra2     = &max
	},
	{ }
};

static ctl_table test_root_table[] = {
	{
		.procname	= "glasses3d",
		.mode		= 0555,
		.child		= test_table
	},
	{ }
};

void sync_init_fs(void){
	test_sysctl_header = register_sysctl_table(test_root_table);
}

void sync_stop_fs(void){
	unregister_sysctl_table(test_sysctl_header);
}

int sync_write_eye(struct ctl_table *table, int write, void __user *buffer, size_t *lenp, loff_t *ppos) {

	int r = proc_dointvec_minmax(table, write, buffer, lenp, ppos); // on met à jour proprement
	
	if(write) {
		glasses3d_swap();
	}
	
	return r;
}

#endif

/*** Synchronisation automatique grace à l'IRQ de la carte graphique ***/

#ifdef SYNC_WITH_IRQ

#define NVCARD_IRQ 19 /* cat /proc/driver/nvidia/cards/0 */

irqreturn_t sync_interrupt(int irq, void *dev_id) {

	// TODO :: check if interrupt signal a vblank
	printk(KERN_INFO "IRQ\n");
	sync_swap();
	
	return IRQ_HANDLED;
}

void sync_init_irq(){
	/* Request the video card IRQ */
	retval = 0;
	retval = request_irq(NVCARD_IRQ, &sync_interrupt, IRQF_SHARED, DRIVER_NAME, &sync_interrupt);
	if (retval) {
		printk(KERN_ALERT "Error: unable to get Graphic Card IRQ %d (errno=%d).\n", NVCARD_IRQ, retval);
		return retval;
	}
	
	enable_irq(NVCARD_IRQ);	
}

void sync_stop_irq(){
	free_irq(NVCARD_IRQ, &sync_interrupt);
}

#endif
