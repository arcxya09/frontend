#ifndef  __V977_INCLUDE_H__
#define  __V977_INCLUDE_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <mvmestd.h>


#define V977_BASE	0x0E000000



#define V977_SET_REG        (0x000A)
#define V977_CTRL           (0x0028) /* Control register */
#define V977_INPUT_READ    (0x0004)
#define V977_OUTPUT_MASK   (0x000C)
#define V977_IRQ_LEVEL	(0x0020)
#define V977_IRQ_VECTOR	(0x0022)
#define V977_SOFT_RESET	(0x002E)
#define V977_IRQ_MASK	(0x000E)
#define V977_INPUT_MASK	(0x0002)
#define V977_CLR_OUTPUT	(0x0010)



int v977_SetOutputReg(MVME_INTERFACE *vme, mvme_addr_t vme_addr,unsigned short var);
int v977_SetCtl(MVME_INTERFACE *vme, mvme_addr_t vme_addr,unsigned short mode);
int v977_soft_reset(MVME_INTERFACE* vme, mvme_addr_t vme_addr);
int v977_clear_output(MVME_INTERFACE* vme, mvme_addr_t vme_addr);
int init_v977(MVME_INTERFACE *vme);


#endif
