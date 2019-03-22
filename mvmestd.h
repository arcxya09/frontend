#ifndef MVMESTD_H
#define MVMESTD_H

#include "CAENVMEtypes.h"

/*---- types -------------------------------------------------------*/
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int mvme_addr_t;
typedef unsigned int mvme_locaddr_t;
typedef unsigned int mvme_size_t;

/*---- constants ---------------------------------------------------*/

/*---- interface structure -----------------------------------------*/
typedef struct {
   int  initialized;         /**< 1 if VME interface initialized */
   int  handle;              /**< internal handle */
   int  index;               /**< index of interface 0..n */
   void *info;               /**< internal info structure */
   CVAddressModifier  am;                  /**< Address modifier */
   CVDataWidth  dmode;               /**< Data mode (D8,D16,D32,D64) */
   int  blt_mode;            /**< Block transfer mode */
   void *table;              /**< Optional table for some drivers */
} MVME_INTERFACE;

/*---- function declarations ---------------------------------------*/


int mvme_open(MVME_INTERFACE **vme, int idx);

int mvme_close(MVME_INTERFACE *vme);


int mvme_read_value(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *buf);

int mvme_write_value(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *value);

void mvme_set_am(MVME_INTERFACE *vme, CVAddressModifier am);

void mvme_get_am(MVME_INTERFACE *vme, CVAddressModifier *am);

void mvme_set_dmode(MVME_INTERFACE *vme, CVDataWidth dmode);

void mvme_get_dmode(MVME_INTERFACE *vme, CVDataWidth *dmode);

int mvme_set_blt(MVME_INTERFACE *vme, int mode);

int mvme_get_blt(MVME_INTERFACE *vme, int *mode);
int mvme_interrupt_generate(MVME_INTERFACE *mvme, int level, int vector, void *info);
int mvme_interrupt_attach(MVME_INTERFACE *mvme, int level, int vector,
			  void (*isr)(int, void*, void *), void *info);
int mvme_interrupt_detach(MVME_INTERFACE *mvme, int level, int vector, void *info);
int mvme_interrupt_enable(MVME_INTERFACE *mvme, int level, int vector, void *info);
int mvme_interrupt_disable(MVME_INTERFACE *mvme, int level, int vector, void *info);
int mvme_IRQEnable(MVME_INTERFACE *vme, short level);

int mvme_IRQDisable(MVME_INTERFACE *vme, short level);

int mvme_IACKCycle(MVME_INTERFACE *vme, short level, void *vec);
int mvme_IRQCheck(MVME_INTERFACE *mvme, void *mask);
int mvme_set_timeout(MVME_INTERFACE *vme, short flag);

//return 1 if IRQ received, otherwise return 0;
int mvme_IRQWait(MVME_INTERFACE *vme, short level, unsigned long time);

int mvme_set_FIFO_mode(MVME_INTERFACE *mvme, short flag);
int mvme_BLT_read(MVME_INTERFACE *vme, void *dst, mvme_addr_t vme_addr, mvme_size_t n_bytes, int* count);
int mvme_MBLT_read(MVME_INTERFACE *vme, void *dst, mvme_addr_t vme_addr, mvme_size_t n_bytes, int* count);

#endif // MVMESTD_H
