/* system includes */
#include <stdio.h>
#include <termios.h> /* tcgetattr(), tcsetattr() */
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include "cJSON.h"

/* vme includes */
#include "mvmestd.h"
/* root includes */
#include <TFile.h>
#include <TTree.h>
#include "v1190.h"
#include "madc.h"
#include "v830.h"

#define MADC_ENABLE
#define V830_ENABLE
#define V1190_ENABLE


#define ADC_ONLY 1
#define EVT_ID_ADC 0
#define EVT_ID_TDC 1
#define EVT_ID_SCL 2
#define MAX_EVT_LEN 8192*12 /* 10 MB */
#define RB_SIZE 400000000


// Modules
int N_MADC = 0;
int N_V830 = 0;
int N_V1190 = 0;
unsigned int* MADC_BASE;
unsigned int V830_BASE;
UINT16 *V1190_BASE;
UINT16 *V1190_GEO;

///////


unsigned int adc_buf[8192 * 8];
/* function declears */
int try_stop_run();
int try_start_run();
int rb_check_space(unsigned long n_byte);
void rb_round_write_pt();
void rb_round_read_pt();
void do_rb_write(unsigned int n_byte, int evt_id, unsigned int *buf);
void rb_write(unsigned int n_byte, int evt_id, unsigned int *buf);
void cooked(void);
void raw(void);
void Sleep(int t);
int getch(void);
int kbhit();
unsigned int rb_read_dword();
void rb_proc_evt();
void rb_init();

int init_serv_sock(int port);
int cmd_message_recv();
void check_cmd_sock_conn();

void* cmd_thread(void *);

void start_cmd_thread();

void stop_cmd_thread();

/* global mutex */
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

/* event struct, the address of the bank_buf is set to be the tree
 * branch address*/
struct Event_Struct {
    int nDWORD;
    char bank_buf[MAX_EVT_LEN];
};
struct Event_Struct evt_struc_adc;
struct Event_Struct evt_struc_tdc;
struct Event_Struct evt_struc_sca;

/* the address of scaler_buf is set to be the address of the tree
 * branch address */
//unsigned int scaler_buf[SCALER_EVENT_LENGTH];

/* ring buffer, for the data transfer between father thread (main thread)
 * and data logger thread */
struct RingBuf {
    unsigned long read_pt;
    unsigned long write_pt;
    unsigned long end_point;
    unsigned long start_point;
    unsigned char buf[RB_SIZE];
};
struct RingBuf ring_buf;
struct timeval tv_start;
struct timeval tv_end;
//~~

/* vme interface  */
MVME_INTERFACE *vme;

struct termios g_old_kbd_mode;

/* socket connections */
int listen_sock_fd, connect_socket_fd;

/* cmd socket */
int cmd_socket_serv_fd, cmd_connect_fd;
int cmd_stop_flag = 0;
int init_flag = 0;
int verbose = 1;

double AcqRate = 0;
double DataRate = 0;
double FileSize = 0;
int AcqDur = 0;
/* shared memory objects, for data transfer between the frontend and
 * the online analysis */
void *shm_adc;
void *shm_tdc;
void *shm_scl;

TFile *root_file;
TTree *tree_trig;
TTree *tree_scal;
char file[100];
char file_name[100];
char logfile_name[100];
int run_num = 0;
long long Nb_sum = 0;
int PRINT_INTERNAL = 1000000;
pthread_t logger_thread_id;
pthread_t cmd_thread_id;

/* Run status */
int AcqRun = 0;
int Quit;

void ClearEventStruct(struct Event_Struct *evt) {
    // printf("%d\n", evt->nDWORD);
    for (int i = 0; i < evt->nDWORD; ++i)
    {
        evt->bank_buf[i] = 0;
    }
    evt->nDWORD = 0;
}

void PrintInterface() {
//  system("clear");
    struct timeval tv_this;

    gettimeofday(&tv_this, NULL);
    printf("\n\n STATUS: %s\t  FileName:%s\t  run: %05d\t",
           AcqRun ? "STARTED" : "STOPED", file, run_num);
    printf("%d min %d s since started\n",
           (int)((tv_this.tv_sec - tv_start.tv_sec) / 60), (int)((tv_this.tv_sec - tv_start.tv_sec) % 60));
    AcqDur = tv_this.tv_sec - tv_start.tv_sec;
    printf("s ) Start acquisition\n");
    printf("S ) Stop acquisition\n");
    printf("c ) change file name and run number\n");
    printf("q ) Quit\n");
    // if (AcqRun && (tv_this.tv_sec - tv_start.tv_sec) > 3600) {try_stop_run(); try_start_run();}

}

int book_TTree()
{
    char file_name[100], raw_file_name[100];

    sprintf(file_name, "/data/2019Lanzhou/raw/%s%05d.root", file , run_num);
//sprintf(file_name, "/home/junadaq/daq/daq/data1/%s%05d.root", file , run_num);


    /* open root file */
    root_file = new TFile(file_name, "CREATE");
    //printf("file_name is %s\n",file_name);
    if (root_file->IsZombie())
    {
        PRINT_INTERNAL = 1000000000; //> 100 s
        printf("****** Pls type \"S\" and use \"c\" to change ...\n");
        delete root_file;
        root_file = NULL;
//      exit (-1);
        return -1;
    }

    /* create TTree and branches for adc tdc*/
    tree_trig = new TTree("Trigger", "Trigger");
    tree_trig->Branch("ADC0", &evt_struc_adc, "nADC0/I:ADC0[nADC0]/i");
    tree_trig->Branch("TDC0", &evt_struc_tdc, "nTDC0/I:TDC0[nTDC0]/i");

    /* create TTree and branches for scaler */
    tree_scal = new TTree("Scaler", "scal");
    tree_scal->Branch("SCAL", &evt_struc_sca, "nSCA0/I:SCA0[nSCA0]/i");




    return 0;
}

void close_TTree()
{
    tree_trig->Write();
    tree_scal->Write();
    root_file->Close();
    delete root_file;
}


void quit()
{
    shm_unlink("/CAEN_DGTZ_SHM0");
    shm_unlink("/CAEN_DGTZ_SHM1");
    close(listen_sock_fd);
    if (connect_socket_fd != -1)
        close(connect_socket_fd);
}


/* v2718 initial, return 0 if success */
int init_v2718()
{
    int ret;

    ret = 0;
    /* set out0 and out 1 and out2 source and polarity */
    ret |= CAENVME_WriteRegister(vme->handle, cvOutMuxRegSet, 0x2F);

    /* Set Pulser B register */
    ret |= CAENVME_SetPulserConf(vme->handle, cvPulserB, 50, 1,
                                 cvUnit104ms, 0, cvManualSW, cvManualSW);
    ret |= CAENVME_StartPulser(vme->handle, cvPulserB);

    /* clear the OUT0 and OUT1 level */
    ret |= CAENVME_WriteRegister(vme->handle, cvOutRegClear, 0xC0);

    return ret;
}

int reset_busy()
{
    int ret;
    /* send a NIM pulse from OUT0 port of v2718 */
    ret = CAENVME_WriteRegister(vme->handle, cvOutRegSet, 0x40);
    if (ret != cvSuccess)
        return ret;
    ret = CAENVME_WriteRegister(vme->handle, cvOutRegClear, 0x40);
    if (ret != cvSuccess)
        return ret;

    return 0;
}

int read_vme_config() {
    FILE* fp = fopen("vme.conf", "r");
    if (fp == NULL)
    {
        fputs("VME config file load error code 1!", stderr);
        exit(-1);
    }

    fseek(fp, 0, SEEK_END);
    int fs = ftell(fp);
    rewind(fp);

    char* vmejson = (char*)malloc(sizeof(char) * fs);
    // fread(vmejson, 1, fs, fp);
    if (fread(vmejson, 1, fs, fp) != fs)
    {
        fputs("VME config file load error code 2!", stderr);
        exit(-1);
    }

    cJSON * monitor = NULL;
    monitor = cJSON_Parse(vmejson);
    cJSON * tmp = NULL;

    if (monitor == NULL)
    {
        return 0;
    }

    // read madc config
#ifdef MADC_ENABLE
    cJSON *madcs = NULL, *madc_bases = NULL;
    madcs = cJSON_GetObjectItem(monitor, "madc");
    madc_bases = cJSON_GetObjectItem(madcs, "base");
    N_MADC = cJSON_GetArraySize(madc_bases);
    if (madcs != NULL && madc_bases != NULL && N_MADC > 0)
    {
        MADC_BASE = (unsigned int *)malloc(sizeof(unsigned int) * N_MADC);
        for (int i = 0; i < N_MADC; ++i)
        {
            MADC_BASE[i] = strtol(cJSON_GetArrayItem(madc_bases, i)->valuestring, NULL, 0);
            // printf("0x%08x\n", MADC_BASE[i]);
        }
    }

#endif

#ifdef V830_ENABLE
    cJSON * v830s = NULL, *v830_base = NULL;
    v830s  = cJSON_GetObjectItem(monitor, "v830");
    v830_base = cJSON_GetObjectItem(v830s, "base");
    if (v830s != NULL && v830_base != NULL)
    {
        V830_BASE = strtol(v830_base->valuestring, NULL, 0);
        N_V830 = 1;
    }
#endif

#ifdef V1190_ENABLE
    cJSON *v1190s = NULL, *v1190_bases = NULL, *v1190_geos = NULL;
    v1190s = cJSON_GetObjectItem(monitor, "v1190");
    v1190_bases = cJSON_GetObjectItem(v1190s, "base");
    v1190_geos = cJSON_GetObjectItem(v1190s, "geo");
    N_V1190 = cJSON_GetArraySize(v1190_bases);

    if (v1190s != NULL && v1190_bases != NULL && v1190_geos != NULL && N_V1190 > 0)
    {
        if (N_V1190 > 0)
        {
            V1190_BASE = (UINT16 *)malloc(sizeof(UINT16) * N_V1190);
            V1190_GEO = (UINT16 *)malloc(sizeof(UINT16) * N_V1190);
            for (int i = 0; i < N_V1190; ++i)
            {
                V1190_BASE[i] = strtol(cJSON_GetArrayItem(v1190_bases, i)->valuestring, NULL, 0);
                V1190_GEO[i] = cJSON_GetArrayItem(v1190_geos, i)->valueint;
                // printf("0x%04x %d\n", V1190_BASE[i], V1190_GEO[i]);
            }
        }
    }
#endif

    return 1;
}

int init_frontend()
{
    // read modules config from vme.conf
    if (read_vme_config())
    {
        printf("--> VME config file loaded: %d MADC, %d V1190, %d V830 found.\n", N_MADC, N_V1190, N_V830);
    };

    init_flag = 1;
    //init vme interface;
    if (mvme_open(&vme, 0))
    {
        printf("--> board initial failed! %d\n");
        return 1;
    }
    else printf("--> mvme_open success...\n");


    if (init_v2718()) {
        printf("--> v2718 initial failed!\n");
        return 1;
    }
    else printf("--> v2718 initial success ...\n");

#ifdef MADC_ENABLE
    //-Init MADC32
    if (init_madc(vme)) {
        printf("--> madc initial failed!\n");
        return 1;
    }
    // else printf("--> madc initial success ...\n");
#endif

#ifdef V1190_ENABLE
    /* Init V1190 */
    if (init_v1190(vme->handle)) {
        printf("--> v1190 initial failed!\n");
        return 1;
    }
    // else printf("--> v1190 initial success ...\n");
#endif

#ifdef V830_ENABLE
//  init v830
    if (init_v830(vme)) {
        printf("--> v830 initial failed!\n");
        return 1;
    }
    // else printf("--> v830 initial success ...\n");
#endif

    init_flag = 0;
    return 0;
}

int data_ready()
{
    int x;
// test:
//  madc_isDataReady(vme, 0, &x);
//  if (x) return 1;
//  else goto test;
    if (mvme_IRQWait(vme, IRQ_LEVEL, 1)) {
        // printf("--> IRQ_LEVEL created ...\n");
        return 1;
    }
    return 0;
}

/* check if the ring buffer still has enough space for further n_byte bytes */

void evt_handler(int n_byte, unsigned int *buf, int evt_id)
{
    int *p_buf;
    char evt_name[4];
    switch (evt_id) {
    case EVT_ID_ADC:
        p_buf = (int *)shm_adc;
        memcpy(evt_name, "adc0", 4);
        break;
    case EVT_ID_TDC:
        p_buf = (int *)shm_tdc;
        memcpy(evt_name, "tdc0", 4);
        break;
    case EVT_ID_SCL:
        p_buf = (int *)shm_scl;
        memcpy(evt_name, "scal", 4);
        break;
    default:
        printf("unknown event type\n");
    }


    /* fill the shared memory and inform the analyzer */
    *p_buf = n_byte;
    memcpy(p_buf + 1, buf, n_byte);
    if ((connect_socket_fd != -1) &&
            (-1 == send(connect_socket_fd, evt_name, 4, 0))) {
        printf("error sending data\n");
        connect_socket_fd = -1;
        close(connect_socket_fd);
    }

    /* write the event to the ring buffer */
    rb_write(n_byte, evt_id, buf);
}

void vme_error_handler(const char *msg, int error_code)
{
    printf("%s\n", msg);
    printf("error: %d\n", error_code);

    /* In case of error, it is usually nessasory to
     * open the vme interface again. In this case, it
     * is always better to start a new run */
    try_stop_run();
    mvme_close(vme);
    if (mvme_open(&vme, 0)) {
        printf("board initial failed!\n");
        exit(-1);
    }
    if (init_v2718()) {
        printf("v2718 initial failed!\n");
        exit(-1);
    }
    if (try_start_run()) {
        printf("error starting a new run \n");
        exit(-1);
    }
}

/* read madc32, return number of bytes read,
 * return negtive error code in case of error */
int read_madc()
{

    int readout_length = 8192 * 2;
    int nbyte, i = 0;
    int test_num = 0;
    int error_code;
    unsigned int *p = adc_buf;
    static int nn = 0;
//  goto test;
    do {
        error_code = madc_CMBLT_Event_read(vme, CBLT_BASE, adc_buf, &nbyte, readout_length);
//  error_code = madc_BLT_Event_read(vme, MADC_BASE, adc_buf, &nbyte, readout_length);
//   for (int i = 0; i < nbyte / 4; i++) printf("--> madc, nbyte=%d, adc_buf[%d]=%0x\n", nbyte, i, adc_buf[i]);
        if (error_code != cvBusError && error_code != cvSuccess)
            return error_code;
        evt_handler(nbyte, adc_buf, EVT_ID_ADC);
        test_num += nbyte;
        i++;
        //if(i>1)
//  printf("i=%d,nbyte=%d,readout_length=%d\n",i,nbyte,readout_length*4);
    } while (nbyte == readout_length * 4);
    return test_num;

//OBtest:
//  /* be careful! no error checkings are done here.*/
//  nn++;
//  error_code = madc_get_data_len(vme, 0, &nbyte);
//  //printf("nbytre: %d\n", nbyte);
//  nbyte = nbyte << 1; /* assumed to be 64-bit format*/
//  for (i = 0; i < nbyte; i++) {
//      error_code = madc_read_dword(vme, 0, p++);
//      //  printf("data-----> %x\n", *(p-1));
//  }
//  nbyte = nbyte << 2;
//  evt_handler(nbyte, adc_buf, EVT_ID_ADC);
//  //if (nn>=5) exit(-1);
//  return nbyte;
}

/* read v1190, return number of bytes read,
 * return negtive error code (cvCommError here) in case of error */
int read_v1190()
{
    unsigned int tdc_buf[8192 * 4];
    int readout_length = 8192;
    int nbyte;

    nbyte = readout_length;
    if (!v1190_read_MEB(tdc_buf, (UINT32 *)&nbyte))
        /* the error code is not returned from v1190_read_MEB function,
         * so we return the most probobal error code */
        return cvCommError;
    // printf("%d\n", nbyte);
    evt_handler(nbyte, tdc_buf, EVT_ID_TDC);
    return nbyte;
}

/* read v830, return number of bytes read,
 * return negtive error code in case of error */
#ifdef V830_ENABLE
int read_v830()
{
    DWORD dest[SCALER_EVENT_LENGTH];
    int nbyte, error_code;
    struct timeval tv;

    //-before saving data into buffer, the buffer should be cleared.
    do {
        error_code = v830_BLT_Event_read(vme, V830_BASE, dest, &nbyte, SCALER_EVENT_LENGTH - 1);
        if (error_code != cvSuccess && error_code != cvCommError && error_code != -1) //> "-1" by znt
            return error_code; //> when no data in buffer, cvBussError returned (znt)
    } while (nbyte == SCALER_EVENT_LENGTH - 1);

    //-generate a trigger and save data into buffer
    error_code = v830_soft_trigger(vme, V830_BASE);
    if (error_code != cvSuccess)
        return error_code;
    error_code = v830_BLT_Event_read(vme, V830_BASE, dest, &nbyte, SCALER_EVENT_LENGTH - 1);
    // printf("*********************************\n\n");
    // for (int i = 0; i < nbyte / 4; i++) printf("--> v830, nbyte=%d, dest[%d]=%0x\n", nbyte, i, dest[i]);
    if (error_code != cvSuccess && error_code != cvBusError)
        return error_code;

    gettimeofday(&tv, NULL);
    dest[0] = tv.tv_sec;
    dest[SCALER_EVENT_LENGTH - 1] = tv.tv_usec;
    evt_handler(SCALER_EVENT_LENGTH * 4, dest, EVT_ID_SCL);

    return nbyte;
}
#endif

int print_stat(int *Nr0, int *Nb0)
{
    int elipsed_micro_sec;
    static struct timeval tv_this, tv_last;
    int Nr = *Nr0, Nb = *Nb0;

    gettimeofday(&tv_this, NULL);
    elipsed_micro_sec = (tv_this.tv_sec - tv_last.tv_sec) * 1000000 +
                        (tv_this.tv_usec - tv_last.tv_usec);
    if (elipsed_micro_sec < 0)
    {
        tv_last = tv_this;  /*because tv_last is not initialized */
        return 0;
    }
    //printf("%d\n", elipsed_micro_sec);
//  if (elipsed_micro_sec >= PRINT_INTERNAL)
    if (elipsed_micro_sec >= 1000000)
    {
        tv_last = tv_this;
        Nb_sum += Nb;
        // if (root_file != NULL)
        // {
        //     // tree_trig->FlushBaskets();
        //     // tree_scal->FlushBaskets();
        //     // root_file->Flush();
        //     FileSize = root_file->GetSize() / 1024.0 / 1024;
        //     /* code */
        // }
        // else {
        //     FileSize = Nb_sum / 1024.0 / 1024;
        // }
        FileSize = Nb_sum / 1024.0 / 1024;

        DataRate = 1.0e3 * Nb / elipsed_micro_sec;
        AcqRate = 1.0e3 * Nr / elipsed_micro_sec;
        if (verbose)
        {
            PrintInterface();
            printf("data: %.3f kB/s\t", 1000.0 * Nb / elipsed_micro_sec);
            printf("rate: %.3f kHz\t", 1000.0 * Nr / elipsed_micro_sec);
            printf("total size: %.3f MB\t", FileSize);
        }
        *Nr0 = 0;
        *Nb0 = 0;
        return 1;
    }

    return 0;
}

int reset_ts()
{
    int ret;
    // madc_set_reset_ctr_ab(vme, MCST_BASE, 0x3);

    /* First, we send veto signal to N93B */
    ret = CAENVME_WriteRegister(vme->handle, cvOutRegSet, 0x80); //> out_lemo_1
    if (ret != cvSuccess)
        return ret;

    /* Then send the reset signals */
    ret = CAENVME_WriteRegister(vme->handle, cvOutRegSet, 0x40); //> out_lemo_0
    if (ret != cvSuccess)
        return ret;
    ret = CAENVME_WriteRegister(vme->handle, cvOutRegClear, 0x40);
    if (ret != cvSuccess)
        return ret;

    return 0;
}


/* begin of run routin, return 0 if success, return non zero if error */
int  begin_of_run()
{
    //stop all madc and mtdc
    int i;
    int ret;
    WORD thres[32];

    ret = 0;

    ret = mvme_IRQDisable(vme, IRQ_LEVEL);
#ifdef MADC_ENABLE
    ret = madc_stop_acq(vme, MCST_BASE);

    for (i = 0; i < 32; i++)
//      thres[i] = 150;
        thres[i] = 0;
//  thres[31] = 0;
    ret |= madc_setThreshold(vme, MCST_BASE, thres);
    // for (i = 0; i < 32; i++)
    //  thres[i] = 0;
    // ret |= madc_setThreshold(vme, MADC_BASE4, thres);
    // for (i = 0; i < 32; i++)
    //  thres[i] = 0;
    // ret |= madc_setThreshold(vme, MADC_BASE5, thres);
    // reset timestamp
    // ret |= madc_set_reset_ctr_ab(vme, MCST_BASE, 0x3);
    ret |= madc_set_reset_ctr_ab(vme, MCST_BASE, 0xE);
#endif

#ifdef V1190_ENABLE
    ret |= v1190_clear_buf();
    ret |= v1190_data_clear();
#endif


#ifdef MADC_ENABLE
    //-Start MADC-32;
    ret |= madc_setFifo_reset(vme, MCST_BASE);
    ret |= madc_readout_reset(vme, MCST_BASE);
    ret |= madc_start_acq(vme, MCST_BASE);
#endif

    ret |= reset_ts();


#ifdef V830_ENABLE
    ret = v830_clear_register(vme, V830_BASE);//initial the counter
#endif

    ret = mvme_IRQEnable(vme, IRQ_LEVEL);
    //reset_busy(); //> znt
//printf("ret=%d\n", ret);

    return ret;
}

/* keeping checking the ring buffer, if new data available, store it */
void logger_cycle()
{
    do {
        /* if the read poingter equals the write pointer,
         * then there is no new data, we only *read* the pointers
         * here, so we don't have to lock the mutex */
        if (ring_buf.read_pt == ring_buf.write_pt) {
            usleep(100000);
            continue;
        }


        /* now we have new data, process the new event
         * (read and save it) */
        rb_proc_evt();

    } while (1);
}

/* entry point of the logger thread */
void *logger_thread(void *)
{
    if (book_TTree()) {
        printf("error creating TTree\n");
        logger_thread_id = -1;
        pthread_exit(NULL);
    }
    logger_cycle();
    return NULL;
}

void start_logger_thread()
{
    int ret;
    ret = pthread_create(&logger_thread_id, NULL, logger_thread, NULL);
    if (ret) {
        perror("clone");
        exit(-1);
    }
}


/* start run routin, return 0 if success, non zero if error */
int start_run()
{

    DataRate = 0;
    AcqRate = 0;
    FileSize = 0;
    run_num++;
    rb_init();
    start_logger_thread();
    if (begin_of_run())
        return -1;
    gettimeofday(&tv_start, NULL);
    Nb_sum = 0;

    //Writing log file for time
    sprintf(logfile_name, "../log/%s%05d.log", file, run_num);
    std::ofstream LogFile(logfile_name);
//  struct tm* stm;
    LogFile << "Run: " << run_num << std::endl;
    LogFile << "Start time: " << tv_start.tv_sec << " second " << tv_start.tv_usec << " usecond " << std::endl;
    LogFile.close();

    return 0;
}

/* try start run routin, return 0 if success, non zero if error */
int try_start_run()
{
    if (AcqRun) {
        printf("Acquisition already started...\n");
        return 0;
    }
    if (start_run())
        return -1;

    AcqRun = 1;
    return 0;
}

void stop_logger_thread()
{
    /* Just write a large event length (0xFFFFFFFF) to the ring buffer,
     * when the logger thread sees it, it will exit */
    if (logger_thread_id != -1)
    {
        rb_write(0xFFFFFFFF, 0, NULL);
        pthread_join(logger_thread_id, NULL);
    } else {
        // printf("logger stopped\n");
    }
}

/* stop run routin, return 0 if success, non zero if error */
int stop_run()
{
//    close_TTree();
    PRINT_INTERNAL = 1000000; //> update every 1s
    // v1190_clear_buf();
    stop_logger_thread();
    //Writing log file for time
    sprintf(logfile_name, "../log/%s%05d.log", file, run_num);
    std::ofstream LogFile(logfile_name, std::ios::app);
    gettimeofday(&tv_end, NULL);
//  std::cout<<"End time: "<<tv_end.tv_sec<<" second "<<tv_end.tv_usec<<" usecond "<<std::endl;
//  LogFile.open(logfile_name, std::ios::app);
//  struct tm* stm;
    LogFile << "Run: " << run_num << std::endl;
    LogFile << "End time: " << tv_end.tv_sec << " second " << tv_end.tv_usec << " usecond " << std::endl;
    LogFile.close();

    return 0;
}

int try_stop_run()
{

    if (!AcqRun) {
        printf("Acquisition already stoped...\n");
        return 0;
    }
    // printf("Try stopping...\n");
    if (stop_run())
        return -1;

    AcqRun = 0;
    // printf("Stopped. AcqRun status: %d\n", AcqRun);
    return 0;
}

int try_change_file_name()
{
    //system("clear");
    int k = 0;
    char runNum_str[50];

    if (AcqRun) {
        printf("Acquisition must be stoped first...\n");
        // printf("Acquisition must be stoped first...\n");
        return 0;
    }

    printf("Input file name : \n");
    do
    {
        file[k] = getch();
        printf("%c", file[k]);
        k++;
        fflush(stdout);
    }   while (file[k - 1] != '\n');
    file[k - 1] = '\0';
    printf("File name is set as: %s \n", file);

    printf("Input run number : \n");
    k = 0;
    do
    {
        runNum_str[k] = getch();
        printf("%c", runNum_str[k]);
        fflush(stdout);
        k++;
    } while (runNum_str[k - 1] != '\n');
    run_num = atoi(runNum_str);
    printf("Run number is set as: %d\n", run_num);
    run_num--;

    return 0;
}
int try_quit()
{
    stop_cmd_thread();
    Quit = 1;
    return try_stop_run();
}
int exec_com(char c)
{
    if (c == 'q')
        return try_quit();

    /* start run */
    if (c == 's')
        return try_start_run();

    /* stop run */
    if (c == 'S')
        return try_stop_run();
    /* change file name */
    if (c == 'c')
        return try_change_file_name();

    return 0;
}

int check_kb()
{
    char c;
    if (kbhit()) {
        c = getch();
        return exec_com(c);
    }

    return 0;
}

int socket_readable(int sock)
{
    int ret;
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    ret = select(sock + 1, &readfds, NULL, NULL, &timeout);
    if (ret == -1) {
        printf("error select syscall\n");
        return 0;
    }
    else
        return ret;
}

int cmd_message_recv() {
    char sbuf[100];
    int bufc;
    char* buf, *arg;
    char send_msg[100];

    // printf("check\n");
    if (cmd_connect_fd != -1 && socket_readable(cmd_connect_fd))
    {
        recv(cmd_connect_fd, sbuf, 100, 0);
        // printf("%s\n", buf);
    }

    buf = strtok(sbuf, " ");
    arg = strtok(NULL, " ");


    if (arg != NULL)
    {
        bufc = 2;
    } else {
        bufc = 1;
    }

    // printf("%s %s\n", buf, arg);
    if (buf != NULL)
    {
        if (strcmp(buf, "start") == 0)
        {
            printf("Message: %s\n", buf);
            exec_com('s');
        }
        else if (strcmp(buf, "stop") == 0)
        {
            printf("Message: %s\n", buf);
            exec_com('S');
        }
        else if (strcmp(buf, "quit") == 0)
        {
            printf("Message: %s\n", buf);
            exec_com('q');
        }
        else if (strcmp(buf, "init") == 0) {
            if (!AcqRun)
            {
                printf("Initializing...\n");
                mvme_close(vme);
                init_frontend();
            } else {
                printf("Acquiring, unable to initialize.\n");
            }
        }
        else if (strcmp(buf, "setfilename") == 0 && bufc == 2) {
            if (!AcqRun)
            {
                sprintf(file, arg);
                printf("File name is set to %s\n", file);
            } else {
                printf("Acquiring, unable to set file name.\n");
            }
        }
        else if (strcmp(buf, "setrunnum") == 0 && bufc == 2) {
            if (!AcqRun)
            {
                run_num = atoi(arg);
                printf("Run number is set to %d\n", run_num);
            } else {
                printf("Acquiring, unable to set file name.\n");
            }
        }
        else if (strcmp(buf, "get") == 0 && bufc == 2) {
            if (strcmp(arg, "status") == 0)
            {
                sprintf(send_msg, "%d %.2f %.2f %.0f %d", AcqRun, DataRate, AcqRate, FileSize, AcqDur);
            }
            else if (strcmp(arg, "filename") == 0)
            {
                sprintf(send_msg, "%s", file);
            }
            else if (strcmp(arg, "runnum") == 0)
            {
                sprintf(send_msg, "%d", run_num);
            }
            send(cmd_connect_fd, send_msg, strlen(send_msg), 0);
        }
        else {
            if (strcmp(sbuf, "")) printf("Message received: %s\n", sbuf);
        }
    }

}

void check_cmd_sock_conn() {
    if (socket_readable(cmd_socket_serv_fd))
        cmd_connect_fd = accept(cmd_socket_serv_fd, NULL, NULL);
}

void* cmd_thread(void *) {
    while (!cmd_stop_flag) {
        usleep(10000);
        check_cmd_sock_conn();
        cmd_message_recv();
    }
}

void start_cmd_thread() {
    int ret;

    cmd_socket_serv_fd = init_serv_sock(9999);
    cmd_connect_fd = -1;

    ret = pthread_create(&cmd_thread_id, NULL, cmd_thread, NULL);
    if (ret) {
        perror("clone");
        exit(-1);
    }
}

void stop_cmd_thread()
{
    /* Just write a large event length (0xFFFFFFFF) to the ring buffer,
     * when the logger thread sees it, it will exit */
    cmd_stop_flag = 1;
    pthread_join(cmd_thread_id, NULL);
}


void check_sock_conn()
{
    if ((connect_socket_fd == -1) && socket_readable(listen_sock_fd))
        connect_socket_fd = accept(listen_sock_fd, NULL, NULL);
}

void loop_cycle()
{
    int Nr, Nb, elipsed_micro_sec, ret;
    int Nsca = 0;
    struct timeval tv_last, tv_this;
    //struct timeval tv1, tv0;

    AcqRun = Nr = Nb = Quit = 0;
    gettimeofday(&tv_last, NULL);
    PrintInterface();
    while (!Quit) {
begin_loop:

        gettimeofday(&tv_this, NULL);
        elipsed_micro_sec = (tv_this.tv_sec - tv_last.tv_sec) * 1000000
                            + (tv_this.tv_usec - tv_last.tv_usec);
        if (elipsed_micro_sec >= 1000000)
        {
            tv_last = tv_this;
            /* Check keyboard */
            if (check_kb()) {
                printf("error check kb\n");
                exit(-1);
            }

            /* Check incoming connection requests */
            check_sock_conn();
            // check_cmd_sock_conn();
            // cmd_message_recv();
        }

        /* Print interface */
        if (!AcqRun) {
            if (!init_flag && verbose) PrintInterface();
            Sleep(1);
            continue;
        }

        if (data_ready()) {
            mvme_IRQDisable(vme, IRQ_LEVEL);
#ifdef MADC_ENABLE
            ret = read_madc();
            if (ret < 0) {
                vme_error_handler("madc blt read error", ret);
                goto begin_loop;
            }
            Nb += ret;
            // if (AcqRate > 100000) madc_readfifo_reset(vme, 0);
            madc_readout_reset(vme, MCST_BASE);
#endif

#ifdef V1190_ENABLE
            ret = read_v1190();
            if (ret < 0) {
                vme_error_handler("v1190 blt read error", ret);
                goto begin_loop;
            }
            Nb += ret;
#endif


            mvme_IRQEnable(vme, IRQ_LEVEL);
            Nr++;
        }
        //ret = reset_busy(); // ~ 10us


        //-read scaler
#ifdef V830_ENABLE
        if (elipsed_micro_sec >= 1000000) {
            ret = read_v830();
            if (ret < 0) {
                vme_error_handler("v830 blt read error", ret);
                goto begin_loop;
            }
            Nb += ret;
        }


        // if (elipsed_micro_sec / 100000 > Nsca) {
        //     ret = read_v830();
        //     if (ret < 0) {
        //         vme_error_handler("v830 blt read error", ret);
        //         goto begin_loop;
        //     }
        //     Nb += ret;
        //     Nsca++;
        // }
        // if (elipsed_micro_sec >= 1000000) Nsca = 0;
#endif

        // printf("%d\n", Nr);
        if ((elipsed_micro_sec >= 1000000)) {
            print_stat(&Nr, &Nb);
            Nr = Nb = 0;
        }

//      usleep(100);
    }

}


int init_serv_sock(int port)
{
    int listen_sock_fd;
    int ret;
    struct sockaddr_in servaddr;
    int on = 1;

    listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock_fd == -1) {
        printf("error creating socket\n");
        exit(-1);
    }
    setsockopt(listen_sock_fd, SOL_SOCKET,
               SO_REUSEADDR, &on, sizeof(on));

    /* Init servaddr */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    /* Bind */
    ret = bind(listen_sock_fd, (struct sockaddr*)&servaddr,
               sizeof(servaddr));
    if (ret == -1) {
        printf("error binding");
        exit(-1);
    }

    /* Listen */
    ret = listen(listen_sock_fd, 128);
    if (ret == -1) {
        printf("error listening\n");
        exit(-1);
    }

    return listen_sock_fd;
}


void *open_shared(const char *name, int len)
{
    int shm_id;
    void *addr;
    shm_id = shm_open(name, O_RDWR | O_CREAT, 0644);
    if (shm_id == -1) {
        printf("open shared memory error\n");
        return NULL;
    }
    ftruncate(shm_id, len);
    addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if (addr == MAP_FAILED)
        return NULL;
    else
        return addr;
}


static void sig_handler(int signum)
{
    /* stop the current run to save data */
    try_stop_run();
    printf("sig handler called\n");
    exit(-1);
}

int main(int argc, char* argv[])
{
    struct sigaction sa;

    /* Install signal handler for SIGSEGV */
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        perror("sigaction");

    /* Init frontend */
    printf("--> Init...\n");
    if (init_frontend()) return -1;
    printf("--> Finished...\n");

    listen_sock_fd = init_serv_sock(9093);
    connect_socket_fd = -1;


    /* Create shared memory object */
    shm_adc = open_shared("/MYDAQ_ADC0", 1024 * 1024 * 10);
    shm_tdc = open_shared("/MYDAQ_TDC0", 1024 * 1024 * 10);
    shm_scl = open_shared("/MYDAQ_SCAL", 4096);

    // start_cmd_thread();


    // printf("Input file name:\n");
    // scanf("%s", file);

    // // input run number
    // printf("Input run number: \n");
    // scanf("%d", &run_num); run_num--;

    sprintf(file, "test");
    run_num = 0;


    // // printf("File name: %s\n", file);
    // printf("Run number: %d\n", run_num);

    // scanf("%s", file);

    //// input run number
    // printf("Input run number: \n");
    // scanf("%d", &run_num); run_num--;
    //-------------------------------------
    sprintf(file_name, "../data/%s%05d.root", file, run_num + 1);
    /*
    //- open root file
    root_file = new TFile(file_name, "CREATE");
    while(!(root_file->IsOpen())) {
        printf("Input run number: \n");
        scanf("%d", &run_num); run_num--;
    sprintf(file_name, "../data/%s%05d.root",file,run_num+1);
        root_file = new TFile(file_name, "CREATE");
    }
    run_num_0 = run_num+1;
    */
    //-------------------------------------
    //-main loop
    loop_cycle();

    /* quit */
    quit();

    return 0;
}

/* ring buffer operation functions, it is better to put them in a seprate file,
 * here we just put them at the end of the current file, which is a lazy way
 * -:)) */
void rb_init()
{
    ring_buf.read_pt = (unsigned long)(ring_buf.buf);
    ring_buf.write_pt = (unsigned long)(ring_buf.buf);
    ring_buf.end_point = (unsigned long)(ring_buf.buf) +
                         (unsigned long)RB_SIZE - 1;
    ring_buf.start_point = (unsigned long)(ring_buf.buf);
}

/* read a DWORD from ring buffer */
unsigned int rb_read_dword()
{
    unsigned int ret;
    ret = *((unsigned int *)(ring_buf.read_pt));
    ring_buf.read_pt += 4;
    rb_round_read_pt();

    return ret;
}

/* process the event pending in the ring buffer */
void rb_proc_evt()
{
    unsigned int data_len;
    unsigned int evt_id;
    unsigned int *pDWORD;
    int *p_nDWORD;
    unsigned int i;
    static int cnt = 0;

    ClearEventStruct(&evt_struc_adc);
    ClearEventStruct(&evt_struc_tdc);
    ClearEventStruct(&evt_struc_sca);

    pthread_mutex_lock(&mutex1);
    /* first, read the data length */
    data_len = rb_read_dword();
    if (data_len == 0xFFFFFFFF) {
        /* this abnormally large event length indidates that the
         * father thread requires this logger thread should quit*/
        close_TTree();
        pthread_mutex_unlock(&mutex1);
        pthread_exit(0);
    }

    // printf("%x\n", data_len);

    /* then, read the event id */
    evt_id = rb_read_dword();


    /* lastly, read event data */
    p_nDWORD = NULL;
    switch (evt_id) {
    case EVT_ID_ADC:
        pDWORD = (unsigned int *)evt_struc_adc.bank_buf;
        p_nDWORD = &evt_struc_adc.nDWORD;
        break;
    case EVT_ID_TDC:
        pDWORD = (unsigned int *)evt_struc_tdc.bank_buf;
        p_nDWORD = &evt_struc_tdc.nDWORD;
        break;
    case EVT_ID_SCL:
        pDWORD = (unsigned int *)evt_struc_sca.bank_buf;
        p_nDWORD = &evt_struc_sca.nDWORD;
    }
//  printf("->EVTID: %d\n", evt_id);
    for (i = 0; i < data_len / 4; i++) {
        pDWORD[i] = rb_read_dword();
//      printf("----> %x\n",pDWORD[i]);
//      fflush(stdout);
    }
    cnt++;
//  if(cnt>1) exit(0);
    if (p_nDWORD)
        *p_nDWORD = data_len / 4;

    /* Now we have already have a copy of the new data, release the mutex as
     * soon as possible, especially before filling the tree which may take a
     * long time */
    pthread_mutex_unlock(&mutex1);

    /* we fill the tree only if the event id is scaler or tdc (not adc)*/
    switch (evt_id) {
    case EVT_ID_ADC:
        tree_trig->Fill();
        break;
    case EVT_ID_TDC:
        tree_trig->Fill();
        break;
    case EVT_ID_SCL:
        tree_scal->Fill();
    }
}

int rb_check_space(unsigned long n_byte)
{
    unsigned long remain_size;
    if (ring_buf.write_pt >= ring_buf.read_pt)
        remain_size = ring_buf.write_pt - ring_buf.read_pt + RB_SIZE;
    else
        remain_size = ring_buf.read_pt - ring_buf.write_pt;
//printf("remain_size: %d\n", remain_size);
//printf("nbyte: %d\n", n_byte);

    return remain_size <= (n_byte + 100);
}

/* round the write pointer back to the start point if nessasory */
void rb_round_write_pt()
{
    if (ring_buf.write_pt > ring_buf.end_point)
        ring_buf.write_pt = ring_buf.start_point;
}

/* round the write pointer back to the start point if nessasory */
void rb_round_read_pt()
{
    if (ring_buf.read_pt > ring_buf.end_point)
        ring_buf.read_pt = ring_buf.start_point;
}

/* write number of bytes, event id and the event data to the ring buffer */
void do_rb_write(unsigned int n_byte, int evt_id, unsigned int *buf)
{
    unsigned int i;

    /* write event size */
    *((int *)(ring_buf.write_pt)) = n_byte;
    ring_buf.write_pt += 4;
    rb_round_write_pt();
    /* if the event length (n_byte) is abnormally large, this is not
     * a real event, but only a signal for the daughter thread to quit,
     * so we don't need to write the event id and data */
    if (n_byte == 0xFFFFFFFF) return;

    /* write event id */
    *((int *)(ring_buf.write_pt)) = evt_id;
    ring_buf.write_pt += 4;
    rb_round_write_pt();

    /* write data */
    for (i = 0; i < n_byte / 4; i++) {
        *((unsigned int *)(ring_buf.write_pt)) = buf[i];
        ring_buf.write_pt += 4;
        rb_round_write_pt();
    }
}

/* write number of bytes, event id and the event data to the ring buffer,
 * we check if enough space is available before writting  */
void rb_write(unsigned int n_byte, int evt_id, unsigned int *buf)
{
    unsigned int n_byte1;
    if (n_byte == 0xFFFFFFFF)
        n_byte1 = 4;
    else
        n_byte1 = n_byte;
loop:
    pthread_mutex_lock(&mutex1);
    if (rb_check_space(n_byte1)) {
        pthread_mutex_unlock(&mutex1);
        usleep(1);
        goto loop;
    }
    do_rb_write(n_byte, evt_id, buf);
    pthread_mutex_unlock(&mutex1);
}


/* input mode operation functions, it is better to put them in a seprate file,
 * here we just put them at the end of the current file, which is, again, a
 * lazy way  -:)) */
void cooked(void)
{
    tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}

void raw(void)
{
    static char init;
    /**/
    struct termios new_kbd_mode;

    if (init)
        return;
    /* put keyboard (stdin, actually) in raw, unbuffered mode */
    tcgetattr(0, &g_old_kbd_mode);
    memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_kbd_mode);
    /* when we exit, go back to normal, "cooked" mode */
    atexit(cooked);

    init = 1;
}

void Sleep(int t) {
    usleep( t * 1000000 );
}

int getch(void)
{
    unsigned char temp;

    raw();
    /* stdin = fd 0 */
    if (read(0, &temp, 1) != 1)
        return 0;
    return temp;

}


int kbhit()
{

    struct timeval timeout;
    fd_set read_handles;
    int status;

    raw();
    /* check stdin (fd 0) for activity */
    FD_ZERO(&read_handles);
    FD_SET(0, &read_handles);
    timeout.tv_sec = timeout.tv_usec = 0;
    status = select(0 + 1, &read_handles, NULL, NULL, &timeout);
    if (status < 0)
    {
        printf("select() failed in kbhit()\n");
        exit(1);
    }
    return (status);
}
