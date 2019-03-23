#ifndef  __MADC_32_INCLUDE_H__
#define  __MADC_32_INCLUDE_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <mvmestd.h>


extern unsigned int * MADC_BASE;
extern int N_MADC;

// #define MADC_BASE0  0x00000000
// #define MADC_BASE1  0x00010000
// #define MADC_BASE2  0x00020000
// #define MADC_BASE3  0x00030000//~~
// #define MADC_BASE4  0x00040000//~~
// #define MADC_BASE5  0x00050000//~~
// #define MADC_BASE6  0x00060000//~~
// #define MADC_BASE7  0x00070000
// #define MADC_BASE8  0x00080000//~~
// #define MADC_BASE9  0x00090000//~~
// #define MADC_BASEA  0x000a0000//~~
// #define MADC_BASEB  0x000b0000//~~

#define CBLT_BASE   0xAA000000
#define MCST_BASE   0xBB000000
//#define MCST_BASE 0x00010000
#define MADC_IRQ_VEC    0x00
#define IRQ_LEVEL   1
#define IRQ_THRESHOLD 1

#define MADC_MAX_CHANNELS           32
#define MADC_THRES_BASE             (0x4000)    //RW threshold address 0x4000 to 0x403f
#define MADC_ADDRESS_SOURCE         (0x6000)    //RW
#define MADC_ADDRESS_REG            (0x6002)    //RW
#define MADC_SOFT_RESET             (0x6008)    //W
#define MADC_FIRMWARE_REVISION      (0x600E)    //R
#define MADC_IRQ_LEVEL              (0x6010)    //RW IRQ priority 1..7,0=IRQ off
#define MADC_IRQ_VECTOR             (0x6012)    //RW IRQ return value
#define MADC_IRQ_RESET              (0x6016)    //RW reset IRQ(for test)
#define MADC_IRQ_THRESHOLD          (0x6018)    //RW default 1 
#define MADC_MAX_TRANSFER_DATA      (0x601A) //RW 0:unlimited transfer,1:maxinum number is 2048
#define MADC_BUFFER_DATA_LENGTH     (0x6030)    //R
#define MADC_DATA_LEN_FORMAT        (0x6032)    //RW
#define MADC_READOUT_RESET          (0x6034)    //W
#define MADC_MULTIEVENT             (0x6036)    //RW
#define MADC_MARKING_TYPE           (0x6038)    //RW
#define MADC_START_ACQ              (0X603A)    //RW
#define MADC_FIFO_RESET             (0x603C)    //W
#define MADC_DATA_READY             (0x603E)    //R
#define MADC_BANK_OPERATION         (0x6040)    //RW
#define MADC_ADC_RESOLUTION         (0x6042)    //RW 0->2k,1->4k,3->8k
#define MADC_OUTPUT_FORMAT          (0x6044)    //RW
#define MADC_ADC_OVERRIDE           (0x6046)    //RW
#define MADC_SLC_OFF                (0x6048)    //RW
#define MADC_SKIP_OORANGE           (0x604A)    //RW
#define MADC_INPUT_RANGE            (0x6060)    //RW 0->4v.1->10v,2->8v
#define MADC_ECL_TERM               (0x6062)    //RW
#define MADC_ECL_GAT1_OSC           (0x6064)    //RW
#define MADC_ECL_FC_RES             (0x6066)    //RW
#define MADC_ECL_BUSY               (0x6068)    //RW
#define MADC_NIM_GAT1_OSC           (0x606A)    //RW
#define MADC_NIM_FC_RESET           (0x606C)    //RW
#define MADC_NIM_BUSY               (0x606E)    //RW
#define MADC_PULSET_STATUS          (0x6070)    //RW
#define MADC_RESET_CTR_AB           (0x6090)    //RW
#define MADC_TS_SOURCE              (0x6096)
#define MADC_ADC_BUSY_TIME_LO       (0x60A0)
#define MADC_ADC_BUSY_TIME_HI       (0x60A2)

int  madc_get_buffer_length(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *buf_len);
int  madc_isDataReady(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *ready);
int madc_setInput_range(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short range);
int madc_setAdc_resolution(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short resolution);
int madc_setFifo_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_getFirmware_revision(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *fm_ver);
int madc_setThreshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, WORD *threshold);
int madc_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nentry, int nByte);
int madc_Single_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, DWORD *pdest, int *nentry);
int madc_start_acq(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_stop_acq(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_set_multievent(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_timestamp(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_setIrq_level(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short setvalue);
int madc_setIrq_vector(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short setvalue);
int madc_readout_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_readfifo_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_read_register(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_set_max_transfer_data(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_soft_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr);
int madc_setIrq_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_bank_operation(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_ECL_gate1_osc(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_ECL_fc_res(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_NIM_gate1_osc(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_NIM_fc_res(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_reset_ctr_ab(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_ts_source(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_CBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);
int madc_set_skip_oorange(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_set_data_len_format(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value);
int madc_check_irq(MVME_INTERFACE *vme, char irq_levle);
int madc_CMBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD);
int init_madc(MVME_INTERFACE *vme);
int madc_write_register(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short data);
int madc_read_dword(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned int *data);
int madc_get_data_len(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *data);

#endif
