#include "socket_server.h"

int cmd_socket_readable(int sock)
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
	char buf[100];
	// printf("check\n");
	if (cmd_connect_fd != -1 && cmd_socket_readable(cmd_connect_fd))
	{
		recv(cmd_connect_fd, buf, 100, 0);
		// printf("%s\n", buf);
	}
	if (strcmp(buf, "start") == 0)
	{
		printf("%s\n", buf);
		exec_com('s');
	}
	else if (strcmp(buf, "stop") == 0)
	{
		printf("%s\n", buf);
		exec_com('S');
	}
	else if (strcmp(buf, "quit") == 0)
	{
		printf("%s\n", buf);
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
}

void check_cmd_sock_conn() {
	if (cmd_socket_readable(cmd_socket_serv_fd))
		cmd_connect_fd = accept(cmd_socket_serv_fd, NULL, NULL);
}

void* cmd_thread(void *) {
	while (!cmd_stop_flag) {
		usleep(1 * 1e6);
		check_cmd_sock_conn();
		cmd_message_recv();
	}
}

void start_cmd_thread() {
	int ret;

	cmd_socket_serv_fd = init_cmd_serv_sock(9999);
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


int init_cmd_serv_sock(int port)
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
