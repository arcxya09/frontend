#ifndef  __V785_32_INCLUDE_H__
#define  __V785_32_INCLUDE_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <mvmestd.h>

#define V785_BASE1 0x10010000
#define V785_BASE2 0x10020000
#define V785_BASE3 0x10030000
#define V785_BASE4 0x10040000
#define V785_BASE5 0x10050000
#define V785_MCST_BASE 0xAA000000
#define V785_CBLT_BASE 0xAA000000
//#define V785_BASE_ADDR V785_BASE2 //> only one V785 used
#define V785_BASE_ADDR V785_MCST_BASE //> MCST/CBLT used

#define IRQ_LEVEL_V785 1
#define GEO_REG_ADDR 0x1002
#define V785_THRESHOLD_ADDR0 0x1080
#define V785_BIT_SET1_ADDR 0x1006
#define V785_BIT_CLEAR1_ADDR 0x1008
#define V785_BIT_SET2_ADDR 0x1032
#define V785_BIT_CLEAR2_ADDR 0x1034
#define IRQ_LEVEL_ADDR 0x100A
#define IRQ_THRESHOLD_ADDR 0x1020
#define V785_EVTCNT_RESET_ADDR 0x1040

int init_v785(MVME_INTERFACE *vme);
int v785_soft_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int v785_data_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int v785_write_register(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);
int v785_set_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, WORD *value, unsigned short cha_num);
int v785_set_evt_counter(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);
int v785_set_irq_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short irq_threshold);
int v785_set_irq_level(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short irq_level);
int v785_reset_evtCounter(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value);

int v785_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);
int v785_CBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);
int v785_CMBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);

#endif
