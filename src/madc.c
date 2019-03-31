#include <stdio.h>
#include <string.h>
#include <mvmestd.h>
#include "madc.h"
#include <CAENVMEtypes.h>

// #define N_MADC 12
// #define N_MADC_ENABLED 12

int max_try = 5;

/* madc initial, return 0 if success */
int init_madc(MVME_INTERFACE *vme)
{
  // unsigned int MADC_BASE[N_MADC] = {
  //   0x00000000,
  //   0x00010000,
  //   0x00020000,
  //   0x00030000,
  //   0x00040000,
  //   0x00050000,
  //   0x00060000,
  //   0x00070000,
  //   0x00080000,
  //   0x00090000,
  //   0x000a0000,
  //   0x000b0000
  // };
  int ret, i, firmware;
  WORD madc_thres[32];
  ret = 0;

  printf("MADC-32 Initializing...\n");

  // /* soft reset all adcs */
  for (int i = 0; i < N_MADC; ++i)
  {
    printf("--> MADC-32 0x%04x Initializing...\r", MADC_BASE[i] >> 16);
    ret |= madc_soft_reset(vme, MADC_BASE[i]);
    unsigned short CMBLT_MARK = 0x96;
    if (i == 0)
    {
      CMBLT_MARK = 0xa6;
    }
    else if (i == N_MADC - 1)
    {
      CMBLT_MARK = 0x9a;
    }

    ret |= madc_write_register(vme, MADC_BASE[i] + 0x6020, CMBLT_MARK);
    ret |= madc_write_register(vme, MADC_BASE[i] + 0x6004, i + 1);
    if (ret != 0)
    {
      max_try--;
      printf("--> MADC-32 0x%04x Initial failed, try %d.   \n", MADC_BASE[i] >> 16, max_try);

      if (max_try <= 0)
      {
        return ret;
      }

      i -= 1;
      ret = 0;
      continue;
    }
    else
      printf("--> MADC-32 0x%04x Initialized.   \n", MADC_BASE[i] >> 16);
  }

  // ret |= madc_getFirmware_revision(vme, MADC_BASE0, &firmware);
  // printf("%x\n", firmware);

  // ret |= madc_write_register(vme, MCST_BASE + 0x6070, 0b111); //the first board in chain

  /* set input range, 0=4v, 1=10v, 2=8v */
  ret |= madc_setInput_range(vme, MCST_BASE, 0x1);
  //set tdc bank0&1  channel trigger on negative edge for diff input
  //  ret |= madc_setInput_range(vme, MADC_BASE5, 0x3);

  /* set resolution, 0=2k, 1=4k, 3=8k */
  ret |= madc_setAdc_resolution(vme, MCST_BASE, 0x1);

  for (i = 0; i < 32; i++)
  {
    madc_thres[i] = 0;
  }
  for (int i = 0; i < N_MADC; ++i)
  {
    ret |= madc_setThreshold(vme, MADC_BASE[i], madc_thres);
  }

  // ret |= madc_write_register(vme, MCST_BASE + MADC_PULSET_STATUS, 0x7);

  /* set bank operation, 0->banks connected */
  /* set bank operation, 1->banks independent */
  ret |= madc_set_bank_operation(vme, MCST_BASE, 0);
  //  ret |= madc_set_bank_operation(vme, MADC_BASE5,1);

  //TDC

  /* set resolution, 6=62.5ps , 2=3.9ps for tdc */
  //  ret |= madc_setAdc_resolution(vme, MADC_BASE5, 0x6);
  //  ret |= madc_write_register(vme, MADC_BASE2+0x6042, 9);
  //set trigger source : gate 0
  //  ret |= madc_write_register(vme, MADC_BASE5+0x6058, 1);
  //set trigger source : gate 0 NIM input
  //  ret |= madc_write_register(vme, MADC_BASE5+0x6068, 0);
  //set start of window~ -1100 ns
  //  ret |= madc_write_register(vme, MADC_BASE5+0x6050, 15584);
  //set width of window~ 800 ns
  //  ret |= madc_write_register(vme, MADC_BASE5+0x6054, 10000);

  ret |= madc_setIrq_level(vme, MCST_BASE, IRQ_LEVEL);
  ret |= madc_setIrq_threshold(vme, MCST_BASE, IRQ_THRESHOLD);

  /* set data length to 64bit, a fill word may be added to the buffer
   * to get even number of 32bit words. */
  ret |= madc_set_data_len_format(vme, MCST_BASE, 3);

  /* set marking type: timestamp */
  ret |= madc_set_timestamp(vme, MCST_BASE, 3);

  /* use multi event buffer */
  ret |= madc_set_multievent(vme, MCST_BASE, 3);

  /* set max transfer data, 0->unlimited transfer */
  ret |= madc_set_max_transfer_data(vme, MCST_BASE, 0);

  /* */
  // ret |= madc_set_ts_source(vme, MCST_BASE, 0x3); //use external oscillator input, enable external reset;
  ret |= madc_set_ts_source(vme, MCST_BASE, 0x3); //use internal oscillator input, internal reset;

  ret |= madc_set_reset_ctr_ab(vme, MCST_BASE, 0xC); //allow single shot reset for CTRA;
  ret |= madc_set_NIM_gate1_osc(vme, MCST_BASE, 1);  //set NIM_gate1_osc as oscillator input;
                                                     //  ret |= madc_set_NIM_gate1_osc(vme, MADC_BASE1, 1); //set NIM_gate1_osc as oscillator input;
                                                     //  ret |= madc_set_NIM_gate1_osc(vme, MADC_BASE2, 1); //set NIM_gate1_osc as oscillator input;
                                                     //  ret |= madc_set_NIM_gate1_osc(vme, MADC_BASE3, 1); //set NIM_gate1_osc as oscillator input;
                                                     //  ret |= madc_set_NIM_gate1_osc(vme, MADC_BASE4, 1); //set NIM_gate1_osc as oscillator input;
                                                     //  ret |= madc_set_ECL_gate1_osc(vme, MADC_BASE5, 1); //set ECL_gate1_osc as gate1 input;
  ret |= madc_set_NIM_fc_res(vme, MCST_BASE, 1);     //set NIM_fc_res as reset time stamp oscillator input;
                                                     //  ret |= madc_set_NIM_fc_res(vme, MADC_BASE1, 1); //set NIM_fc_res as reset time stamp oscillator input;
                                                     //  ret |= madc_set_NIM_fc_res(vme, MADC_BASE2, 1); //set NIM_fc_res as reset time stamp oscillator input;
                                                     //  ret |= madc_set_NIM_fc_res(vme, MADC_BASE3, 1); //set NIM_fc_res as reset time stamp oscillator input;
                                                     //  ret |= madc_set_NIM_fc_res(vme, MADC_BASE4, 1); //set NIM_fc_res as reset time stamp oscillator input;
                                                     //  ret |= madc_set_NIM_fc_res(vme, MADC_BASE5, 1); //set NIM_fc_res as reset time stamp oscillator input;
  //ret |= madc_set_NIM_fc_res(vme, MCST_BASE, 1); //set NIM_fc_res as reset time stamp oscillator input;
  ret |= madc_set_skip_oorange(vme, MCST_BASE, 0); //set do not skip out of range values;

  /* initialization occurs only once at the beginning of frontend,
   * so we don't check the return values of each vme commands one
   * by one, we check only at the end of all vme commands to see
   * if all vme commands succeed, i.e. if the return value do NOT
   * equal 0, error must have occured */

  if (ret == 0)
  {
    printf("--> MADC-32 Initialized.\n");
  }
  return ret;
}
/*****************************************************************/
int madc_isDataReady(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *ready)
{

  int reValue;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  reValue = mvme_read_value(vme, MADC_BASE[2] + MADC_DATA_READY, ready);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return reValue;
}

int madc_read_dword(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned int *data)
{

  int reValue;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD32);

  reValue = mvme_read_value(vme, MADC_BASE[2], data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return reValue;
}

int madc_get_data_len(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *data)
{

  int reValue;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  reValue = mvme_read_value(vme, vme_addr + 0x6030, data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return reValue;
}

int madc_write_register(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short data)
{
  int reValue;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  reValue = mvme_write_value(vme, vme_addr, &data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return reValue;
}
/*************************************************************/
//set input range of the adc:
//range=0->4V;
//range=1->10V;
//range=2->8V;
int madc_setInput_range(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short range)

{

  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_INPUT_RANGE, &range);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/******************************************************************/
//set resolution of the adc:
//resolution=0->2k;
//resolution=1->4k,1.6us conversion time;
//resolution=2->4k,hires(3.2us conversion time);
//resolution=3->8k,6,4us conversion time;
//resolution=4->8k,12.5us conversion time;
int madc_setAdc_resolution(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short resolution)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_ADC_RESOLUTION, &resolution);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/*****************************************************************************/

//initialise fifo
int madc_setFifo_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr)

{

  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short dum;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  dum = 0;
  ret = mvme_write_value(vme, vme_addr + MADC_FIFO_RESET, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/******************************************************************************/

//get firmware revision
int madc_getFirmware_revision(MVME_INTERFACE *vme, mvme_addr_t vme_addr, int *fm_ver)

{

  int ret;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_read_value(vme, vme_addr + MADC_FIRMWARE_REVISION, fm_ver);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/***********************************************************************************/
//Set threshold of each channel;
//Maximum of the threshold is 8120;
//[in] param threshold: a array of threshold values;
int madc_setThreshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, WORD *threshold)
{
  int k;
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD16);

  for (k = 0; k < MADC_MAX_CHANNELS; k++)
  {
    ret = mvme_write_value(vme, vme_addr + MADC_THRES_BASE + k * 2, threshold + k);
    if (ret != cvSuccess)
      break;
  }

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/***********************************************************/
//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int madc_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int error_code;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  *nbyte = 0;

  mvme_set_am(vme, cvA32_S_BLT);
  mvme_set_dmode(vme, cvD32);

  error_code = mvme_BLT_read(vme, pdest, vme_addr, nDWORD * 4, nbyte);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return error_code;
}

/***********************************************************/
//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int madc_CBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int error_code;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  *nbyte = 0;

  mvme_set_am(vme, cvA32_S_BLT);
  mvme_set_dmode(vme, cvD32);

  error_code = mvme_BLT_read(vme, pdest, vme_addr, nDWORD * 4, nbyte);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return error_code;
}

/***********************************************************/
//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int madc_MBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int error_code;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  *nbyte = 0;

  mvme_set_am(vme, cvA32_S_MBLT);
  mvme_set_dmode(vme, cvD32);

  error_code = mvme_MBLT_read(vme, pdest, vme_addr, nDWORD * 4, nbyte);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return error_code;
}

/***********************************************************/
//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int madc_CMBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int error_code;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  *nbyte = 0;

  mvme_set_am(vme, cvA32_S_MBLT);
  mvme_set_dmode(vme, cvD32);

  error_code = mvme_MBLT_read(vme, pdest, vme_addr, nDWORD * 4, nbyte);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return error_code;
}

/***********************************************************/
//start acceptting gates
int madc_start_acq(MVME_INTERFACE *vme, mvme_addr_t vme_addr)

{
  int ret;
  CVDataWidth cmode;
  CVAddressModifier cam;
  unsigned short start;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD16);

  start = 1;
  ret = mvme_write_value(vme, vme_addr + MADC_START_ACQ, &start);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  //  madc_readfifo_reset(vme, 0);
  return ret;
}

/*************************************************************/

//stop accepting gates
int madc_stop_acq(MVME_INTERFACE *vme, mvme_addr_t vme_addr)

{
  int ret;
  CVDataWidth cmode;
  CVAddressModifier cam;
  unsigned short start;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  start = 0;
  ret = mvme_write_value(vme, vme_addr + MADC_START_ACQ, &start);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/****************************************************************/

//set interrupt level
int madc_setIrq_level(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short setvalue)

{
  int ret;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_IRQ_LEVEL, &setvalue);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/****************************************************************/

//set interrupt vector
int madc_setIrq_vector(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short setvalue)

{
  int ret;
  CVDataWidth cmode;
  CVAddressModifier cam;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_IRQ_VECTOR, &setvalue);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

int madc_readfifo_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short dum;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  dum = 0;
  ret = mvme_write_value(vme, MADC_BASE[2] + 0x603C, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/***************************************************************/

//At single event mode [multievent(0x6036) = 0], allow new trigger, allow IRQ
//At multievent(0x6036) = 3, clears BERR, allows next readout;
int madc_readout_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short dum;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  dum = 0;
  ret = mvme_write_value(vme, vme_addr + MADC_READOUT_RESET, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set multievent mode;
//value=0->Single event, requires readout_reset;
//value=1->Unlimited transfer, no readout reset required.
//value=3->limited transfer, readout reset required;
int madc_set_multievent(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_MULTIEVENT, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set multievent mode;
//value=0->Single event, requires readout_reset;
//value=1->Unlimited transfer, no readout reset required.
//value=3->limited transfer, readout reset required;
int madc_set_max_transfer_data(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_MAX_TRANSFER_DATA, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set marking type of "End of Event mark"
//value=00->event counter;
//value=01->time stamp;
//value=11->extended time stamp;
int madc_set_timestamp(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_MARKING_TYPE, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//soft reset of the module: breaks all activities, sets critical parameters to default;
int madc_soft_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short dum;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  dum = 1;
  ret = mvme_write_value(vme, vme_addr + MADC_SOFT_RESET, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set irq threshold
int madc_setIrq_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_IRQ_THRESHOLD, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set bank operation
//value = 0 -> banks connected
//value = 1 -> operate banks independent
//value = 3 -> toggle mode for zero dead time (use with internal gate generators enabled)
int madc_set_bank_operation(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_BANK_OPERATION, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set ECL_gate1_osc
//value = 0 -> gate1 input;
//value = 1 -> oscillator input (also set 0x6096)
int madc_set_ECL_gate1_osc(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_ECL_GAT1_OSC, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set ECL_gate1_osc
//value = 0 -> fast clear input;
//value = 1 -> reset time stamp oscillator input;
int madc_set_ECL_fc_res(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_ECL_FC_RES, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set NIM_gate1_osc
//value = 0 -> gate1 input;
//value = 1 -> oscillator input (also set 0x6096)
int madc_set_NIM_gate1_osc(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_NIM_GAT1_OSC, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set NIM_gate1_osc
//value = 0 -> fast clear input;
//value = 1 -> reset time stamp oscillator input;
int madc_set_NIM_fc_res(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_NIM_FC_RESET, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set reset_ctr_ab;
//value = 1 -> reset all counters in CTRA;
//value = 2 -> reset all counters in CTRB;
//value = 0xC -> allows single shot reset for CTRA with first edge of external reset signal. The bit bx1xx is reset with this first edge;
int madc_set_reset_ctr_ab(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_RESET_CTR_AB, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set time stamp sources;
//value: bit0 -> freqency source (VME=0; exteranl=1);
//value: bit1 -> external reset enable = 1;
int madc_set_ts_source(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_TS_SOURCE, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set skip out of range values;
//value=0 -> do not skip out of range values;
//value=1 -> skip out of range values;
int madc_set_skip_oorange(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_SKIP_OORANGE, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/**********************************************************************/
//set data length format;
//value=0 -> 8bit;
//value=1 -> 16bit;
//value=2 -> 32bit;
//value=3 -> 64bit;
//at 3 a fill word may be added to the buffer to get even number of 32 bit words;
int madc_set_data_len_format(MVME_INTERFACE *vme, mvme_addr_t vme_addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, vme_addr + MADC_DATA_LEN_FORMAT, &value);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}
