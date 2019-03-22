#ifndef SOCKET_SERVER_HHH
#define SOCKET_SERVER_HHH

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

/* vme includes */
#include "mvmestd.h"
#include "madc.h"
#include "v830.h"
#include "v1190.h"


int cmd_socket_serv_fd, cmd_connect_fd;
int cmd_stop_flag = 0;
pthread_t cmd_thread_id;

int cmd_socket_readable(int sock);

int cmd_message_recv();

void check_cmd_sock_conn();
void* cmd_thread(void *);

void start_cmd_thread();

void stop_cmd_thread();
int init_cmd_serv_sock(int port);

#endif
