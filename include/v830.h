#include <mvmestd.h>

extern unsigned int V830_BASE;
extern int N_V830;
// #define V830_BASE   0x10000000
#define SCALER_ENABLE_CH 0xffffffff
#define SCALER_EVENT_LENGTH 34 //> cha_num+2


#define V830_BIT_SET 0x110A
#define V830_BIT_CLR 0x110C
#define V830_STATUS 0x110E
#define V830_CHANNEL_ENABLE 0x1100
#define V830_DWELL_TIME 0x1104
#define V830_SOFT_RESET 0x1120
#define V830_CLEAR_REGISTER 0x1122
#define V830_SOFT_TRIGGER 0x1124

int v830_set_acqmode(MVME_INTERFACE *vme, mvme_addr_t addr, short value);
int v830_isDataReady(MVME_INTERFACE *vme, mvme_addr_t addr, unsigned short *ready);
int v830_enable_channel(MVME_INTERFACE *vme, mvme_addr_t addr, DWORD mask);
int v830_set_Dwell_time(MVME_INTERFACE* vme, mvme_addr_t addr, DWORD time);
int v830_soft_reset(MVME_INTERFACE* vme, mvme_addr_t addr);
int v830_clear_register(MVME_INTERFACE* vme, mvme_addr_t addr);
int v830_set_autoreset(MVME_INTERFACE* vme, mvme_addr_t addr, short value);
int v830_set_data_format(MVME_INTERFACE* vme, mvme_addr_t addr, short value);
int v830_set_header(MVME_INTERFACE* vme, mvme_addr_t addr, short value);
int v830_read_register(MVME_INTERFACE *vme, mvme_addr_t addr, DWORD* dest);

//void v830_write_register(MVME_INTERFACE *vme, mvme_addr_t addr, DWORD value);
int v830_soft_trigger(MVME_INTERFACE *vme, mvme_addr_t addr);
int v830_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nentry, int nDWORD);
int v830_set_BERR(MVME_INTERFACE* vme, mvme_addr_t addr, short value);
int init_v830(MVME_INTERFACE *vme);
