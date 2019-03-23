#ifndef V1190_HHH
#define V1190_HHH

#include "cvt_V1190.h"

extern int N_V1190;
extern UINT16 * V1190_BASE;
extern UINT16 * V1190_GEO;

#define N_V1190_ENABLE 1

static cvt_V1190_data *V1190_data;
static cvt_V1190_data **V1190_pdata;


//TDC trigger model parameters(in 25ns units)
#define TRIGGER_MATCHING_WINDOW_WIDTH 200//~~
#define TRIGGER_MATCHING_WINDOW_OFFSET (-120)//~~
#define TRIGGER_MATCHING_EXTRA_SEARCH_MARGIN 10
#define TRIGGER_MATCHING_REJECT_MARGIN 5


int init_v1190(int handle);
int init_v1190_single(int handle, UINT16 V1190_BASE_ADDR, UINT16 *geo, cvt_V1190_data* p_data);
int v1190_read_MEB(UINT32 *tdc_buf, UINT32 *nbyte);
int v1190_clear_buf();
int v1190_data_clear();
int v1190_mod_reset();

int init_v1190_test();

#endif
