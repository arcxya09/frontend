
/*-- Include files -------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

/* midas includes */
#include "midas.h"
//#include "rmidas.h"
#include "experim.h"
#include "analyzer.h"

/* root includes */
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

#define MADC_modules 2
#define MADC_ch      32

#define V785_CH      32
#define V785_MODULES 3

#define V775_CH      32
#define V775_MODULES 3
#define V775_GEO_STA 12

#define V1190_modules 1
#define V1190_ch     32
//#define USE_V1290

#define ADC_N_BINS  2048
#define ADC_X_LOW   1
#define ADC_X_HIGH  8192
#define ADC_GAMMA_HIGH  2000
#define ADC_SI_HIGH  60000

#define TDC_N_BINS  24000
#define TDC_X_LOW   -12000
#define TDC_X_HIGH  12000

#define ADC_threshold   200
#define TDC_GATE_INPUT_CH 16

// for calibration


/*-- Parameters ----------------------------------------------------*/

ADC_CALIBRATION_PARAM adccalib_param;
extern EXP_PARAM exp_param;
extern RUNINFO runinfo;

/*-- Module declaration --------------------------------------------*/

INT adc_calib(EVENT_HEADER *, void *);
INT adc_calib_init(void);
INT adc_calib_bor(INT run_number);
INT adc_calib_eor(INT run_number);
float get_adcCalib(unsigned int adc_data, int module, int channel);
float get_tdcCalib(unsigned int tdc_data, int module);


ADC_CALIBRATION_PARAM_STR(adc_calibration_param_str);

ANA_MODULE adc_calib_module = {
    "ADC calibration",           /* module name           */
    "imp",               	/* author                */
    adc_calib,                   /* event routine         */
    adc_calib_bor,               /* BOR routine           */
    adc_calib_eor,               /* EOR routine           */
    adc_calib_init,              /* init routine          */
    NULL,                        /* exit routine          */
    &adccalib_param,             /* parameter structure   */
    sizeof(adccalib_param),      /* structure size        */
    adc_calibration_param_str,   /* initial parameters    */
};

/*-- module-local variables ----------------------------------------*/

static TH1F *hAdcHists[V785_MODULES*V785_CH];
static TH1F *hTdcHists[V775_MODULES*V775_CH];

static TH1F *h1_calSi_T2[2];
static TH1F *h1_tof_diff;
static TH2F *h2_de_tof;

unsigned short ADC_tmp[V785_CH]; 
unsigned int TDC_tmp[V775_CH]; 

float ADC_calib_par[V785_MODULES][V785_CH][2];
float TDC_calib_par[V775_MODULES][2];



/*-- init routine --------------------------------------------------*/
INT adc_calib_init(void)
{
    char name[256];
    int i,j;
    /* book CADC histos */
    for (i = 0; i <V785_MODULES; i++) 
    {
        for(j=0;j<V785_CH;j++)
        {
            sprintf(name, "adc%02d_ch%02d",i+1,j+1);
            hAdcHists[i*MADC_ch+j] = H1_BOOK(name, name, ADC_N_BINS, ADC_X_LOW, ADC_X_HIGH);
        }
    }
    for(i=0; i<2; i++) {
		sprintf(name, "calSi_T2_%d",i+1);
		h1_calSi_T2[i] = H1_BOOK(name, name, ADC_N_BINS, ADC_X_LOW, ADC_X_HIGH);
    }

    //book for TDC
    for (i = 0; i <V775_MODULES; i++)
    {
        for(j=0;j<V775_CH;j++)
        {
            sprintf(name, "tdc%02d_ch%03d",i+1,j+1);
            hTdcHists[i*V775_MODULES+j] = H1_BOOK(name, name, TDC_N_BINS, TDC_X_LOW+12000, TDC_X_HIGH+12000);
        }
    }
    h1_tof_diff = H1_BOOK("tof_diff", "tof2 - tof1", 500, 0, 500);
    h2_de_tof = H2_BOOK("de_tof", "de_tof", 500, 50, 200, 500, 0, 500); 


    //Get calibration parameters:
    FILE* fp;
    char adcFile_name[256]="./adc_calib.dat";
    char str_line[512];
    int calib_module=0,calib_channel=0;
    float calib_par1=0,calib_par2=0;
    int tmp;
	for(i=0; i<V785_MODULES; i++)
		for(j=0;j<V785_CH;j++)
		{
			ADC_calib_par[i][j][0]=0.0;
			ADC_calib_par[i][j][1]=1.0;
		}
    if((fp = fopen(adcFile_name, "r"))==NULL)
    {
        printf("open %s error\n",adcFile_name);
        return SUCCESS;  
    }
    fgets(str_line,512,fp); 
    fgets(str_line,512,fp); // skip the first two comment lines
    strcpy(str_line,"\0");
    while((fgets(str_line,512,fp))!=NULL)
    {
	calib_par1=0;
	calib_par2=0;
        calib_module=0;
        calib_channel=0;
        sscanf(str_line,"%d %d %f %f",&cal_module,&cal_channel, &cal_par1, &cal_par2);
	//printf("par:%d %d %f %f\n", cal_module,cal_channel,cal_par1,cal_par2);
        if(calib_module>MADC_modules || calib_channel>=MADC_ch)
            continue;
	else
	{
		ADC_calib_par[calib_module-1][calib_channel][0]=calib_par1;
		ADC_calib_par[calib_module-1][calib_channel][1]=calib_par2;
	}
	strcpy(str_line,"\0");
    }
    fclose(fp);

    char tdcFile_name[256]="./tdc_calib.dat";
    for(i=0; i<V775_MODULES; i++) {
	TDC_calib_par[i][0]=0.0;
	TDC_calib_par[i][1]=0.0;
    }
    if((fp = fopen(tdcFile_name, "r"))==NULL)
    {
        printf("open %s error\n",tdcFile_name);
        return SUCCESS;  
    }
    fgets(str_line,512,fp); 
    fgets(str_line,512,fp); // skip the first two comment lines
    strcpy(str_line,"\0");
    while((fgets(str_line,512,fp))!=NULL)
    {
	calib_par1=0;
	calib_par2=0;
        calib_module=0;
        sscanf(str_line,"%d %f %f",&cal_module, &cal_par1, &cal_par2);
        if( calib_module>=(V775_MODULES-V775_GEO_STA) )
            continue;
	else
	{
		TDC_calib_par[calib_module-V775_GEO_STA][0]=calib_par1;
		TDC_calib_par[calib_module-V775_GEO_STA][1]=calib_par2;
	}
	strcpy(str_line,"\0");
    }
    fclose(fp);
    //end caliboration

    return SUCCESS;
}

/*-- BOR routine ---------------------------------------------------*/

INT adc_calib_bor(INT run_number)
{

    return SUCCESS;
}

/*-- eor routine ---------------------------------------------------*/

INT adc_calib_eor(INT run_number)
{
    return SUCCESS;
}

/*-- adc caliboration-----------------------------------------------*/
float calib(int module, int channel,float* position)
{
    return SUCCESS;
}
/*-- event routine -------------------------------------------------*/

INT adc_calib(EVENT_HEADER * pheader, void *pevent)
{
    int i,j;
    short bank_size;

    unsigned short adc_sig, adc_geo, adc_ch;
    unsigned short adc_rawVal;
    unsigned int adc_evtCounter;
    unsigned short tdc_sig, tdc_geo, tdc_ch;
    unsigned short tdc_rawVal;
    unsigned int tdc_evtCounter;

    DWORD *pdata;
    unsigned int data;

    memset(TDC_tmp,0,sizeof(unsigned int)*V1190_ch);
    memset(ADC_tmp,0,sizeof(unsigned short)*MADC_ch);

    /*******fill adc hist*************************************/
    /* look for ADC0 bank, return if not present */
    if (!bk_locate(pevent, "ADC0", &pdata))
        return 1;
    //Get bank size:
    bank_size = *((WORD*)pdata - 1);
    for(i=0; i<bank_size; i+=4)
    {
        //Get 32-bit data;
        data = (*((DWORD*)((char*)pdata+i)));
	adc_sig = (data&0x07000000)>>24;
	adc_geo = (data&0xf8000000)>>27;
        if(adc_sig==0x0 && adc_geo!=18) //check if it is adc data, 18-> SCAL_GEO;
        {
        //    if((data & 0x00004000)==0 && module_id>0) //true data,overflow is discard
		if(adc_geo==9) //for V785N
			adc_ch = (data&0x001E0000)>>17;
		else
			adc_ch = (data&0x001E0000)>>17;
                adc_rawVal = (unsigned short)(data & 0x0fff);
                hAdcHists[(adc_geo-1)*V785_CH+adc_ch]->Fill(adc_rawVal);
                //tmp, should be updated
                ADC_tmp[adc_ch] = adc_rawVal;
		if(adc_geo==8 && adc_ch<2)
		{
			h1_calSi_T2[adc_ch]->Fill(get_adcCalib(adc_rawVal, adc_geo, adc_ch));
		}
        }
        else if(adc_sig==0x4 && adc_geo!=18) //check if it is EOB
        {
	    adc_evtCounter = (data&0x00ffffff);
            memset(ADC_tmp,0,sizeof(unsigned short)*V785_CH);
        }
    }
    /*******fill adc hist*************************************/

    /*******file tdc hist************************************/
    module_id=0;
    channel=0;
    if(!bk_locate(pevent, "TDC0", &pdata))
        return 1;
    //Get bank size;
    tdc_gate_input=0;
    bank_size = *((WORD*)pdata - 1);
    for(i=0; i<bank_size; i+=4)
    {
        //Get the data
        data = (*((DWORD*)((char*)pdata+i)));
	tdc_sig = (data&0x07000000)>>24;
	tdc_geo = (data&0xf8000000)>>27;
        if(tdc_sig==0x0 && tdc_geo!=18) //check if it is adc data, 18-> SCAL_GEO;
        {
        //    if((data & 0x00004000)==0 && module_id>0) //true data,overflow is discard
		if(tdc_geo==16) //for V785N
			tdc_ch = (data&0x001E0000)>>17;
		else
			tdc_ch = (data&0x001E0000)>>17;
                tdc_rawVal = (unsigned short)(data & 0x0fff);
                hTdcHists[(tdc_geo-V775_GEO_STA)*V775_CH+tdc_ch]->Fill(tdc_rawVal);
                //tmp, should be updated
                TDC_tmp[tdc_ch] = tdc_rawVal;
        }
        else if(tdc_sig==0x4 && tdc_geo!=18) //check if it is EOB
        {
	    tdc_evtCounter = (data&0x00ffffff);
	    if(tdc_geo==15) // hist for tof1-tof2
	      h1_tof_diff->Fill(get_tdcCalib(TDC_tmp[0], tdc_geo-V775_GEO_STA, tdc_ch), get_tdcCalib(TDC_tmp[1], tdc_geo-V775_GEO_STA, tdc_ch));
            memset(TDC_tmp,0,sizeof(unsigned short)*V775_CH);
        }
    }
    /*******file tdc hist************************************/


    return SUCCESS;
}

float get_adcCalib(unsigned int val, int module, int cha)
{
	return val*ADC_calib_par[module][cha][1] + ADC_calib_par[module][channel][0];

}
float get_tdcCalib(unsigned int data, int module)
{
	return data*TDC_calib_par[module][1] + TDC_calib_par[module][0];
}
