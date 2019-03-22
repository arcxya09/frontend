/*Driver for CAEN V2718 controller;
	Created by Gao @ IMP;
 Here the variable vme->dmode is used as CVDataWidth DW;
*/

/*---------------------------------------------------------------*/
#include <string.h>
#include <mvmestd.h>
#include <CAENVMElib.h>

//open board, must be called before any operation on vme;
int mvme_open(MVME_INTERFACE **vme, int idx)
{
	int32_t Handle;
	CVErrorCodes ErrorCode;
	//alloc memory for INTERFACE;
	*vme = (MVME_INTERFACE *) malloc(sizeof(MVME_INTERFACE));
	if (*vme == NULL)
		return -1;
	memset(*vme, 0, sizeof(MVME_INTERFACE));
	
	//open V2718;
	ErrorCode = CAENVME_Init(cvV2718, 0, idx, &Handle); //initialize the idx controller;
	if(cvSuccess != ErrorCode)
		return ErrorCode;
	(*vme)->handle = Handle;

/* default values */
	(*vme)->initialized = 1;
	(*vme)->am          = cvA32_S_DATA;  //address modifier type
	(*vme)->dmode       = cvD32;  //data mode
	(*vme)->blt_mode    = 1;
	(*vme)->table       = NULL; // not used
		
	return ErrorCode;
}

/*---------------------------------------------------------------*/

int mvme_close(MVME_INTERFACE *vme)
{
	CAENVME_End(vme->handle);
	free(vme);
	return 0;
}

/*--------------------------------------------------------------*/

void mvme_get_dmode(MVME_INTERFACE *vme, CVDataWidth *dmode)
{
   *dmode = vme->dmode;
}

/*--------------------------------------------------------------*/

void mvme_set_dmode(MVME_INTERFACE *vme, CVDataWidth dmode)
{
   vme->dmode = dmode;
}

/*--------------------------------------------------------------*/

void mvme_set_am(MVME_INTERFACE *vme, CVAddressModifier am)
{
   vme->am = am;
}

/*------------------------------------------------------------------*/

void mvme_get_am(MVME_INTERFACE *vme, CVAddressModifier *am)
{
   *am = vme->am;
}


/*------------------------------------------------------------------*/
//write single value to vme module.
//param: value-> the value to be written into vme module.
int mvme_write_value(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *value)
{
	return CAENVME_WriteCycle(vme->handle, vme_addr, 
			value, vme->am, vme->dmode);
}

/*------------------------------------------------------------------*/
//BLT write to vme module.
//param: src-> pointer to the data to be written into vme module;
//param: n_bytes-> [in] number of bytes to be written into vme module;
//param: n_bytes-> [out] number of bytes actually written into vme module;
int mvme_BLT_write(MVME_INTERFACE *vme, mvme_addr_t vme_addr,char* src, mvme_size_t* n_bytes)
{
	CVErrorCodes ErrorCode;
	int count = 0;
	ErrorCode = CAENVME_BLTWriteCycle(vme->handle, vme_addr, 
			src, *n_bytes, vme->am, vme->dmode, &count);
	*n_bytes = count;
	return ErrorCode;
}

/*------------------------------------------------------------------*/
//BLT write to vme module.
//param: src-> pointer to the data to be written into vme module;
//param: n_bytes-> [in] number of bytes to be written into vme module;
//param: n_bytes-> [out] number of bytes actually written into vme module;
int mvme_MBLT_write(MVME_INTERFACE *vme, mvme_addr_t vme_addr,char* src, mvme_size_t* n_bytes)
{
	CVErrorCodes ErrorCode;
	int count = 0;
	ErrorCode = CAENVME_MBLTWriteCycle(vme->handle, vme_addr, 
			src, *n_bytes, vme->am, &count);
	*n_bytes = count;
	return ErrorCode;
}

/*-------------------------------------------------------------*/
//read single value from vme module
//return: the value read out from vme module if success, otherwise return error code;
int mvme_read_value(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *value)
{
	return CAENVME_ReadCycle(vme->handle, vme_addr, value, vme->am, vme->dmode);
}

/*-----------------------------------------------------------------*/
//performs a vme block transfer read cycle;
//param: dst-> [out] The data read from the VME bus;
//param: n_bytes [in] The size of transfer in bytes;
//param: count [out] The number of bytes actually transferred;
//return: MVME_SUCCESS on success, otherwise returns error code;
int mvme_BLT_read(MVME_INTERFACE *vme, void *dst, mvme_addr_t vme_addr, mvme_size_t n_bytes, int* count)
{
	return CAENVME_FIFOBLTReadCycle(vme->handle, vme_addr, dst, 
			n_bytes, vme->am, vme->dmode, count);
}

/*--------------------------------------------------*/
//performs a vme multiplexed block transfer read cycle(64 bits);
//param: dst-> [out] The data read from the VME bus;
//param: n_bytes [in] The size of transfer in bytes;
//param: count [out] The number of bytes actually transferred;
//return: MVME_SUCCESS on success, otherwise returns error code;
int mvme_MBLT_read(MVME_INTERFACE *vme, void *dst, mvme_addr_t vme_addr, mvme_size_t n_bytes, int* count)
{
	return CAENVME_FIFOMBLTReadCycle(vme->handle, vme_addr, 
			dst, n_bytes, vme->am, count);
}

/*-------------------------------------------------*/
//Enables the IRQ lines specified by level;
int mvme_IRQEnable(MVME_INTERFACE *mvme, short level)
{
	short mask=1<<(level-1);
	CVErrorCodes error_code;
	error_code = CAENVME_WriteRegister(mvme->handle,cvVMEIRQEnaReg,mask);
	if (error_code != cvSuccess) 
		return error_code;
        error_code = CAENVME_IRQEnable(mvme->handle, mask);
	
	return error_code;
}

/*------------------------------------------------*/
//Disables the IRQ lines specified by level;
int mvme_IRQDisable(MVME_INTERFACE *mvme,short level)
{
	short mask=~(1<<(level-1)); 
	return CAENVME_WriteRegister(mvme->handle,cvVMEIRQEnaReg,mask);
}

/*------------------------------------------------*/
//performs a VME  interrupt acknowledge cycle;
//param: level-> [in]IRQ level;
//param: vec-> [out]IRQ vector;
int mvme_IACKCycle(MVME_INTERFACE *mvme,short level,void *vec)
{
	return CAENVME_IACKCycle(mvme->handle,(CVIRQLevels)level,vec,cvD16 );
}

/*----------------------------------------------------------------*/
//wait for IRQ lines specified by level until the IRQ line raises or time expires.
//param: level-> [in]IRQ level;
//param: time-> [in]timeout in milliseconds;
int mvme_IRQWait(MVME_INTERFACE *vme, short level, unsigned long time)
{
	if(cvSuccess == CAENVME_IRQWait(vme->handle, 1<<(level-1), time))
		return 1;
	else
		return 0;
}

/*---------------------------------------------------------------*/
//Enable/disable the FIFO mode
int mvme_set_FIFO_mode(MVME_INTERFACE *mvme, short flag)
{
	return CAENVME_SetFIFOMode(mvme->handle, flag);
}

/*----------------------------------------------------------------*/
//sets the specified VME bus timeout on the module;
//param: flag-> [in]zero:timeout=50us; nonzero:timeout=400us.
int mvme_set_timeout(MVME_INTERFACE *mvme, short flag)
{
	if(flag)
		return CAENVME_SetTimeout(mvme->handle, cvTimeout400us);
	else
		return CAENVME_SetTimeout(mvme->handle, cvTimeout50us);
}

/*----------------------------------------------------------------*/
//check if any irq lines are active;
//param: mask-> [out]A bit-mask indicating the active IRQ lines;
int mvme_IRQCheck(MVME_INTERFACE *mvme, void *mask)
{
	return CAENVME_IRQCheck(mvme->handle, (unsigned char*)mask);
}
/*----------------------------------------------------------------*/

