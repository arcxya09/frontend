#ifndef  __V775_32_INCLUDE_H__
#define  __V775_32_INCLUDE_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <mvmestd.h>

#define V775_BASE1 0x10100000
#define V775_BASE2 0x10110000
#define V775_BASE3 0x10110000
#define V775_MCST_BASE 0xBB000000
#define V775_CBLT_BASE 0xBB000000
#define V775_BASE_ADDR V775_BASE1 //> only one V775 used
//#define V775_BASE_ADDR V775_MCST_BASE //> MCST/CBLT used

#define IRQ_LEVEL_TDC 5
#define GEO_REG_ADDR 0x1002
#define V775_MCST_ADDR 0x1004
#define V775_THRESHOLD_ADDR0 0x1080
#define V775_BIT_SET1_ADDR 0x1006
#define V775_BIT_CLEAR1_ADDR 0x1008
#define V775_BIT_SET2_ADDR 0x1032
#define V775_BIT_CLEAR2_ADDR 0x1034
#define IRQ_LEVEL_ADDR 0x100A
#define IRQ_THRESHOLD_ADDR 0x1020
#define V775_EVTCNT_RESET_ADDR 0x1040
#define V775_FSR_ADDR  0x1060

int init_v775(MVME_INTERFACE *vme);
int v775_soft_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int v775_data_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int v775_set_scale_range(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);
int v775_set_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, WORD *value, unsigned short cha_num);
int v775_set_commonStart_mode(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short i);
int v775_set_evt_counter(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);
int v775_set_irq_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short irq_threshold);
int v775_set_irq_level(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short irq_level);
int v775_reset_evtCounter(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);
int v775_set_MCST_ADDR(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);

int v775_write_register(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);
int v775_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);
int v775_CBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);
int v775_CMBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);

#endif
