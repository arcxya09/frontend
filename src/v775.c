//-------------------------------
// Created on Sep. 10, 2016, NAG
//  -grammar similar to MIDAS
//  -
//-------------------------------
#include <stdio.h>
#include <string.h>
#include <mvmestd.h>
#include <CAENVMEtypes.h>
#include "v775.h"

int init_v775(MVME_INTERFACE *vme)
{
	int ret, i;
	WORD cha_thres[32]={0};
	ret = 0;

	//-set GEO address, pls set GEO: 10(0xA), 11, 12 ...
	v775_write_register(vme, V775_BASE1 + GEO_REG_ADDR, 0x0A);
	//v775_write_register(vme, V775_BASE2 + GEO_REG_ADDR, 0x0B);
	//v775_write_register(vme, V775_BASE3 + GEO_REG_ADDR, 0x0C);
	
	//-set CBLT mode
	ret = v775_set_MCST_ADDR(vme, V775_BASE1, 0xBB);
	//ret = v775_set_MCST_ADDR(vme, V775_BASE2, 0xBB);
	//ret = v775_set_MCST_ADDR(vme, V775_BASE3, 0xBB);
	ret = v775_write_register(vme, V775_BASE1 + 0x101A, 0x02);//the first board in chain
	//ret = v775_write_register(vme, V775_BASE2 + 0x101A, 0x03);//the active board in chain
	//ret = v775_write_register(vme, V775_BASE3 + 0x101A, 0x01);//the last board in chain
	ret = v775_soft_reset(vme, V775_BASE_ADDR); //soft reset 

	//-enable Bus Error
	ret = v775_write_register(vme, V775_BASE_ADDR + 0x1010, 0x0024); //set control register 1
	//-set Common Start mode
	ret = v775_set_commonStart_mode(vme, V775_BASE_ADDR, 0); //0-> common stop
	//
	//-set Full Scale Range (FSR) 
	//--range=LSB*4074, LSB=8.9ns/N (N scope: 0xFF->0x1E)
	ret = v775_set_scale_range(vme, V775_BASE_ADDR, 0x24); //0x24-> FSR~1us(LSB~247ps)
	//-set Threshold Resolution
	ret = v775_write_register(vme, V775_BASE_ADDR + V775_BIT_SET2_ADDR, 0x0100); //THR*2
	//-set Threshold for each channel
	for(i=0; i<32; i++) cha_thres[i] = 1;
	ret = v775_set_threshold(vme, V775_BASE_ADDR, cha_thres,32); //4th_par: channel-num to be open

	//-set irq only when no adc is used
	//ret = v775_set_irq_level(vme, V775_BASE1, IRQ_LEVEL);
	//-set mode for Event Eounter, 3rd_par: all triggers->1,accepted triggers->0
	ret = v775_set_evt_counter(vme, V775_BASE_ADDR, 1); 

	return ret;
}

// soft reset after writing GEO to boards
int v775_soft_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	unsigned short value;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	value = 0x80;
	ret = mvme_write_value(vme, vme_addr+V775_BIT_SET1_ADDR, &value);
	ret = mvme_write_value(vme, vme_addr+V775_BIT_CLEAR1_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

//DataReset of V775, do this before each readout
int v775_data_reset(MVME_INTERFACE *vme, mvme_addr_t vme_addr)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	unsigned short value;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	value = 0x4;
	ret = mvme_write_value(vme, vme_addr+V775_BIT_SET2_ADDR, &value);
	ret = mvme_write_value(vme, vme_addr+V775_BIT_CLEAR2_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set full scale range (FSR)
int v775_set_scale_range(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+V775_FSR_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set common start/stop mode
int v775_set_commonStart_mode(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short i)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	unsigned short value;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	if(i==1) {
		value = 0x0400;
		ret = mvme_write_value(vme, vme_addr+V775_BIT_CLEAR2_ADDR, &value);
	}
	else if(i==0) {
		value = 0x0400;
		ret = mvme_write_value(vme, vme_addr+V775_BIT_SET2_ADDR, &value);
	}
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// BIT SET 1 register
int v775_set_bit_set1(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+V775_BIT_SET1_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// BIT CLEAR 1 register
int v775_set_bit_clear1(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+V775_BIT_CLEAR1_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set irq level
int v775_set_irq_level(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short irq_level)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+IRQ_LEVEL_ADDR, &irq_level);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set irq threshold / event trigger
int v775_set_irq_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short irq_threshold)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+IRQ_THRESHOLD_ADDR, &irq_threshold);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set mode for the event counter register by setting BIT SET2 register
int v775_set_evt_counter(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	unsigned short bit_set2_val;
	int ret;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	bit_set2_val = 0x4000;
	if(value==1) 
		ret = mvme_write_value(vme, vme_addr+V775_BIT_SET2_ADDR, &bit_set2_val);
	else if(value==0)
		ret = mvme_write_value(vme, vme_addr+V775_BIT_CLEAR2_ADDR, &bit_set2_val);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set the low threshold for each channel
int v775_set_threshold(MVME_INTERFACE *vme, mvme_addr_t vme_addr, WORD *value, unsigned short cha_num)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret, k;
	unsigned short val=0x100;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	for(k=0; k<cha_num; k++) {
		ret = mvme_write_value(vme, vme_addr+V775_THRESHOLD_ADDR0+2*k, value);
		if(ret != cvSuccess) break;
	}
	for(k=cha_num; k<32; k++) {
		ret = mvme_write_value(vme, vme_addr+V775_THRESHOLD_ADDR0+2*k, &val);
		if(ret != cvSuccess) break;
	}
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Reset the event counter
int v775_reset_evtCounter(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret, k;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+V775_EVTCNT_RESET_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

// Set the MSCT/CBLT Address Register
// -default val: 0xAA
int v775_set_MCST_ADDR(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short value)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int ret, k;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_DATA);
	mvme_set_dmode(vme, cvD16);
	
	ret = mvme_write_value(vme, vme_addr+V775_MCST_ADDR, &value);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return ret;
}

int v775_write_register(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned short data)
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

//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int v775_BLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int error_code;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_BLT);
	mvme_set_dmode(vme, cvD32);
	
	*nbyte = 0;
	error_code = mvme_BLT_read(vme, pdest, vme_addr, nDWORD*4, nbyte);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return error_code;
}

//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int v775_CBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int error_code;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	mvme_set_am(vme, cvA32_S_BLT);
	mvme_set_dmode(vme, cvD32);
	
	*nbyte = 0;
	error_code = mvme_BLT_read(vme, pdest, vme_addr, nDWORD*4, nbyte);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return error_code;
}

/***********************************************************/
//read events from MEB, return on BERR
//pdest: output buffer;
//nbyte: transfered number of bytes;
//nDWORD: requested transfer number of DWORD in each BLT read;
int v775_CMBLT_Event_read(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *pdest, int *nbyte, int nDWORD)
{
	CVDataWidth cmode;
	CVAddressModifier cam;
	int error_code;
	
	mvme_get_dmode(vme, &cmode);
	mvme_get_am(vme, &cam);
	
	*nbyte = 0;
	
	mvme_set_am(vme, cvA32_S_MBLT);
	mvme_set_dmode(vme, cvD32);
	
	error_code = mvme_MBLT_read(vme, pdest, vme_addr, nDWORD*4, nbyte);
	
	mvme_set_dmode(vme, cmode);
	mvme_set_am(vme, cam);
	
	return error_code;
}

//0000000oooooooooooOOOOOOOOOOOOOOooooooooooooooo000000000oooooooo
