/*********************************************************************

  Name:         v977.c
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <mvmestd.h>
#include "v977.h"


int v977_SetOutputReg(MVME_INTERFACE *vme, mvme_addr_t vme_addr,unsigned short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);
  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD16);
  ret = mvme_write_value(vme, vme_addr+V977_SET_REG, &value);
  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
  	
}	

/*****************************************************************/
//Set mode of v977
//@param mode: bit 0 = 0 (the module works as I/O register, default setting)
//@param mode: bit 0 = 1 (the module works as MULTIHIT PATTERN UNIT)
//@param mode: bit 1 = 0 (Gate input is enabled)
//@param mode: bit 1 = 1 (Gate input is disabled, default setting)
//@param mode: bit 2 = 0 (OR and /OR outputs are enabled, default setting)
//@param mode: bit 2 = 0 (OR and /OR outputs are disbled)
int v977_SetCtl(MVME_INTERFACE *vme, mvme_addr_t vme_addr,unsigned short value)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);
  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD16);
  ret = mvme_write_value(vme, vme_addr+V977_CTRL, &value);
  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}


int v977_soft_reset(MVME_INTERFACE* vme, mvme_addr_t vme_addr)
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
  ret = mvme_write_value(vme, vme_addr+V977_SOFT_RESET, &dum);

  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;
}


/******************************************************************/
// clears all the channels FLIP-FLOP
int v977_clear_output(MVME_INTERFACE* vme, mvme_addr_t vme_addr)
{
  CVDataWidth cmode;
  CVAddressModifier cam;
  int ret;
  unsigned int data;

  mvme_get_dmode(vme, &cmode);
  mvme_get_am(vme, &cam);
  mvme_set_am(vme, cvA32_S_DATA);
  mvme_set_dmode(vme, cvD16);	

  data = 1;
  ret = mvme_write_value(vme, vme_addr+V977_CLR_OUTPUT, &data);
  
  mvme_set_dmode(vme, cmode);
  mvme_set_am(vme, cam);

  return ret;

}

/* v977 initial, return 0 if success */
int init_v977(MVME_INTERFACE *vme)
{
	int ret;

	ret = 0;
	ret |= v977_soft_reset(vme, V977_BASE);
	/* xxx */

	return ret;
}

