/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#ifndef __ZEPHYR__

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#else

#include <net/socket.h>
#include <kernel.h>
#include <net/net_app.h>

#endif

const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;

int main(void)
{
	int serv;
#if 0
	struct sockaddr_in bind_addr;
#endif
	struct sockaddr_in6 bind_addr;
	static int counter;

	serv = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

#if 0
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(4242);
#endif

	bind_addr.sin6_family = AF_INET6;
	bind_addr.sin6_addr = in6addr_any;
	bind_addr.sin6_port = htons(4242);
	bind(serv, (struct sockaddr *)&bind_addr, sizeof(bind_addr));

	listen(serv, 5);

	while (1) {
		struct sockaddr client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char addr_str[32];
		void *addr;
		int client = accept(serv, &client_addr, &client_addr_len);

		if (client_addr.sa_family == AF_INET) {
			addr = &((struct sockaddr_in *)&client_addr)->sin_addr;
		} else if (client_addr.sa_family == AF_INET6) {
			addr = &((struct sockaddr_in6 *)&client_addr)->sin6_addr;
		} else {
			continue;
		}

		inet_ntop(client_addr.sa_family, addr, addr_str, sizeof(addr_str));

		printf("Connection #%d from %s\n", counter++, addr_str);

		while (1) {
			char buf[128];
			printk("before recv\n");
			int len = recv(client, buf, sizeof(buf), 0);
			printk("after recv\n");

			if (len == 0) {
				break;
			}

			if (memcmp(buf, "bye", 3) == 0) {
				printf("Active close connection !!\n");
				close(client);
				// close(serv);
				continue;
			}

			// send(client, buf, len, 0);
		}

		close(client);
		printf("Connection from %s closed\n", addr_str);
		printf("CLOSE CONN\n");
	}
}
