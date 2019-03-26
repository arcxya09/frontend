#include <v1190.h>
#include <CAENVMElib.h>

// static cvt_V1190_data V1190_data[1];
// static cvt_V1190_data *V1190_pdata = &V1190_data[0];

/* v1190 initial, return 0 if success */
int init_v1190(int handle)
{
    printf("--> v1190 Initializing...\n");
    V1190_data = (cvt_V1190_data *)malloc(sizeof(cvt_V1190_data) * N_V1190);
    V1190_pdata = (cvt_V1190_data **)malloc(sizeof(cvt_V1190_data *) * N_V1190);

    int ret;
    for (int i = 0; i < N_V1190_ENABLE; ++i)
    {
        V1190_pdata[i] = &V1190_data[i];
        printf("----> ADDR 0x%x initializing...\r", V1190_BASE[i]);
        fflush(stdout);

        ret |= init_v1190_single(handle, V1190_BASE[i], &V1190_GEO[i], V1190_pdata[i]);
        printf("----> ADDR 0x%x GEO %d initialized.   \n", V1190_BASE[i], V1190_GEO[i]);
        fflush(stdout);
    }
    if (ret == 0)
    {
        printf("--> v1190 Initialized.\n");
    }
    // UINT16 geo = 1;
    // ret = init_v1190_single(handle, V1190_BASE[0], &geo, &V1190_data[0]);
    // BOOL p_is_data_ready, p_is_term_on, p_is_buffer_full, p_is_buffer_almost_full;
    // CVT_V1190_STATUS_RES p_resolution;
    // UINT8 p_error_bitmask;
    // cvt_V1190_get_status(V1190_pdata, &p_is_data_ready, &p_is_term_on, &p_is_buffer_full, &p_is_buffer_almost_full, &p_resolution, &p_error_bitmask);
    // printf("%x\n", p_error_bitmask);

    return ret;
}

int init_v1190_single(int handle, UINT16 V1190_BASE_ADDR, UINT16 *geo, cvt_V1190_data *p_data)
{
    int i;
    UINT16 enable_0_msk[8];
    BOOL ret;
    UINT16 substract_trigger;

    ret = TRUE;
    if (!cvt_V1190_open(p_data, V1190_BASE_ADDR,
                        handle, CVT_V1190_TYPE_A))
        return -1;

    for (i = 0; i < 8; i++)
    {
        enable_0_msk[i] = 0xFFFF; //enable all channels;
    }

    ret &= cvt_V1190_set_trigger_matching_acquisition_mode(
        p_data,
        TRIGGER_MATCHING_WINDOW_WIDTH,
        TRIGGER_MATCHING_WINDOW_OFFSET,
        TRIGGER_MATCHING_EXTRA_SEARCH_MARGIN,
        TRIGGER_MATCHING_REJECT_MARGIN,
        CVT_V1190_ED_LEADING_ONLY,
        CVT_V1190_PRW_100PS,
        enable_0_msk,
        FALSE, //disable TDC header and trailer.
        TRUE,  //enable empty event
        TRUE); //enable trigger time tag and ETTT

    /* read out mode: BERR enabled, event aligned disabled,
     * BLT event number set to 0.*/
    ret &= cvt_V1190_set_readout_mode(p_data, TRUE, FALSE, 0);

    /* enable substract trigger time from TDC measuments */
    substract_trigger = 1;
    ret &= cvt_V1190_write_2_micro(p_data, CVT_V1190_EN_SUB_TRG_OPCODE, &substract_trigger, 1);

    /* set the TDC GEO(module ID) */
    // geo = 1;
    ret &= (cvSuccess == CAENVME_WriteCycle(
                             handle,
                             (V1190_BASE_ADDR << 16) + 0x100E,
                             geo,
                             cvA32_U_DATA,
                             cvD16));
    return !ret;
}

int v1190_read_MEB(UINT32 *tdc_buf, UINT32 *nbyte)
{
    BOOL ret = 0;
    UINT32 nbyte_tmp = 8192;
    *nbyte = 0;
    for (int i = 0; i < N_V1190_ENABLE; ++i)
    {
        nbyte_tmp = 8192;
        // printf("%u\n", tdc_buf + (*nbyte) / 4);
        ret |= cvt_V1190_read_MEB(&V1190_data[i], tdc_buf + (*nbyte) / 4, &nbyte_tmp);
        *nbyte += nbyte_tmp;
    }
    // printf("%d\/n", *nbyte);
    // printf("\n");
    return ret;

    // int ret = 0;
    // ret = cvt_V1190_read_MEB(&V1190_data[0], tdc_buf, nbyte);
    // printf("%d\n", *nbyte);
    // return ret;
}

/* clear v1190 data buffer by reading all the existing data out,
 * we don't use soft clear here because it also resets the coase counter
 * which is unexpected */
int v1190_clear_buf()
{
    int readout_length = 8 * 8192;
    UINT32 nbyte;
    int error_code;
    unsigned int tdc_buf[8192 * 8];

    nbyte = readout_length;

    for (int i = 0; i < N_V1190_ENABLE; ++i)
    {
        do
        {
            // nbyte = readout_length;
            error_code = cvt_V1190_read_MEB(&V1190_data[i],
                                            tdc_buf, (UINT32 *)&nbyte);
            // printf("%d %u\n", error_code, nbyte);
            // if (error_code != cvSuccess && error_code != cvCommError)
            if (error_code != cvSuccess && error_code != cvBusError)
                return error_code;
        } while (nbyte == readout_length);
    }

    return 0;
}

int v1190_data_clear()
{
    BOOL ret;
    for (int i = 0; i < N_V1190_ENABLE; ++i)
    {
        ret |= cvt_V1190_data_clear(&V1190_data[i]);
    }
    return ret;
}

int v1190_mod_reset()
{
    int ret = 0;
    for (int i = 0; i < N_V1190_ENABLE; ++i)
    {
        ret |= cvt_V1190_module_reset(&V1190_data[i]);
    }
    return ret;
}

// int v1190_clear_buf()
// {
//  int readout_length = 8 * 8192;
//  int nbyte, error_code;
//  unsigned int tdc_buf[8192 * 2];

//  nbyte = readout_length;
//  do {
//      error_code = cvt_V1190_read_MEB(&V1190_data[0],
//                                      tdc_buf, (UINT32 *)&nbyte);
//      if (error_code != cvSuccess && error_code != cvCommError)
//          return error_code;
//  } while (nbyte == readout_length);

//  return 0;
// }

// int v1190_data_clear()
// {
//  return cvt_V1190_data_clear(&V1190_data[0]);
// }
