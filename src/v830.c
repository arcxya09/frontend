#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mvmestd.h>
#include "v830.h"

/****************************************************************************************/
//set acq mode of v830
//value = 0: Trigger disabled
//value = 1: Trigger random (external or from VME)
//value = 2: Periodical trigger
int v830_set_acqmode(MVME_INTERFACE *vme, mvme_addr_t addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short data;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_write_value(vme, addr + V830_BIT_SET, &value);
  if (ret != cvSuccess)
    goto end;
  data = ~(value | 0xFFFC);
  ret = mvme_write_value(vme, addr + V830_BIT_CLR, &data);

end:
  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;

}

/****************************************************************************************/
//check if data ready in MEB
//return 1 if yes, return 0 if no
int v830_isDataReady(MVME_INTERFACE *vme, mvme_addr_t addr, unsigned short *ready)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  ret = mvme_read_value(vme, addr + V830_STATUS, ready);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;

}

/****************************************************************************************/
/*
  enable channels according to the mask param;
  Bit 0 = Channel 0         |
  Bit 1 = Channel 1         |   1 = Channel enabled
  ...                 |   0 = Channel disabled
  Bit 31 = Channel 32         |
*/
int v830_enable_channel(MVME_INTERFACE *vme, mvme_addr_t addr, DWORD mask)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD32);

  ret = mvme_write_value(vme, addr + V830_CHANNEL_ENABLE, &mask);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}



/****************************************************************************************/
//set Dwell time(time interval of periodical trigger)
//Dwell time = time * 400ns
int v830_set_Dwell_time(MVME_INTERFACE* vme, mvme_addr_t addr, DWORD time)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD32);

  ret = mvme_write_value(vme, addr + V830_DWELL_TIME, &time);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/****************************************************************************************/
//reset the scaler(reset counters, temporary buffers, MEB, registers, etc.)
//the following registers are NOT reset: ADER High,ADER Low, ENABLE ADER, MCST/CBLT Address, GEO Address, Dummy 16/32
int v830_soft_reset(MVME_INTERFACE* vme, mvme_addr_t addr)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned int dum;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD16);

  dum = 1;
  ret = mvme_write_value(vme, addr + V830_SOFT_RESET, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/****************************************************************************************/
//reset the counters registers, the MEB, the trigger counter register and the MEB event number register
int v830_clear_register(MVME_INTERFACE* vme, mvme_addr_t addr)
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
  ret = mvme_write_value(vme, addr + V830_CLEAR_REGISTER, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;

}

/****************************************************************************************/
//set the AUTO RESET Bit of the control register
//if the AUTO RESET Bit is set, the counters are reset after a trigger
//value = true:  the AUTO RESET Bit is set
//value = false: the AUTO RESET Bit is cleared
int v830_set_autoreset(MVME_INTERFACE* vme, mvme_addr_t addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short data;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  data = 0x0080;
  if (value)
    ret = mvme_write_value(vme, addr + V830_BIT_SET, &data);
  else
    ret = mvme_write_value(vme, addr + V830_BIT_CLR, &data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;

}

/****************************************************************************************/
//set the DATA FORMAT Bit of the control register
//value = true:  DATA FORMAT Bit is set to 1, the data word in MEB is 26 bit format
//value = false: DATA FORMAT Bit is set to 0, the data word in MEB is 32 bit format
int v830_set_data_format(MVME_INTERFACE* vme, mvme_addr_t addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short data;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  data = 4;
  if (value)
    ret = mvme_write_value(vme, addr + V830_BIT_SET, &data);
  else
    ret = mvme_write_value(vme, addr + V830_BIT_CLR, &data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;

}

/****************************************************************************************/
//set the HEADER ENABLE Bit of the control register
//value = true:  header writting is enabled
//value = false: header writting is disabled
int v830_set_header(MVME_INTERFACE* vme, mvme_addr_t addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short data;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  data = 0x20;
  if (value)
    ret = mvme_write_value(vme, addr + V830_BIT_SET, &data);
  else
    ret = mvme_write_value(vme, addr + V830_BIT_CLR, &data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/****************************************************************************************/
//generate an internal trigger
//a call of this function generates an internal trigger perfectly equivalent to an external one
int v830_soft_trigger(MVME_INTERFACE *vme, mvme_addr_t addr)
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
  ret = mvme_write_value(vme, addr + V830_SOFT_TRIGGER, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}

/*************************************************************/
//read events from MEB, return on BERR
//pdest: output buffer;
//nentry: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int v830_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nentry, int nDWORD)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int count, ret;;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  *nentry = 0;

  mvme_set_am(vme, cvA32_S_BLT);
  mvme_set_dmode(vme, cvD32);

  ret = mvme_BLT_read(vme, pdest, vme_addr, nDWORD * 4, &count);
  *nentry = count;


  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}


/****************************************************************************************/
//set the BERR ENABLE Bit of the control register
//value = true:  bus error is enabled
//value = false: bus error is disabled
int v830_set_BERR(MVME_INTERFACE* vme, mvme_addr_t addr, short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned short data;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);

  mvme_set_am(vme, cvA32_S_DATA);

  mvme_set_dmode(vme, cvD16);

  data = 0x10;
  if (value)
    ret = mvme_write_value(vme, addr + V830_BIT_SET, &data);
  else
    ret = mvme_write_value(vme, addr + V830_BIT_CLR, &data);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}


/* v830 inital, return 0 if success */
int init_v830(MVME_INTERFACE *vme)
{
  int ret;
  printf("--> v830 Initializing...\n");

  ret = 0;
  ret |= v830_soft_reset(vme, V830_BASE);
  /* trigger random */
  ret |= v830_set_acqmode(vme, V830_BASE, 1);

  ret |= v830_enable_channel(vme, V830_BASE, SCALER_ENABLE_CH);

  /* data reset after readout */
  ret |= v830_set_autoreset(vme, V830_BASE, 0);

  /* data format: 0 -> 26 bits, channel id in highest 5 bits
                  1 -> 32 bits, no channel id */
  ret |= v830_set_data_format(vme, V830_BASE, 0);

  /* enable event header */
  ret |= v830_set_header(vme, V830_BASE, 1);

  /* readout mode: bus error enabled */
  ret |= v830_set_BERR(vme, V830_BASE, 0);

  if (ret == 0)
  {
    printf("--> v830 Initialized.\n");
  }
  return ret;
}
