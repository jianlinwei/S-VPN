#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "crypt.h"
#include <unistd.h>
#include <netinet/in.h>

#define BUFFER_LEN	4096

int tun_create(char *dev) {
	int fd, err;
	
	if((fd = open(dev, O_RDWR)) < 0) {
		perror("tun_create");
		return fd;
	}

	return fd;
}


int main() {
	int fd = tun_create("/dev/tun0");
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	int alen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	struct CodeTable table;
	unsigned char buffer[BUFFER_LEN], tmp_buffer[BUFFER_LEN];

	BuildTable(&table, "a", 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(33333);
	addr.sin_addr.s_addr = inet_addr("36.54.3.49");

	if(fd < 0) {
		return -1;
	}
	system("ifconfig tun0 up");
	system("ifconfig tun0 192.168.3.3 192.168.3.1");
	while(1) {
		unsigned char src_ip[4];
		unsigned char dst_ip[4];
		int len = read(fd, buffer, BUFFER_LEN);
		printf("I received %d bytes!\n", len);
		if(len <= 0) {
			printf("fuck?\n");
			continue;
		}
		memcpy(src_ip, &buffer[12], 4);
		memcpy(dst_ip, &buffer[16], 4);
		/*
		memcpy(&buffer[12], &buffer[16], 4);
		memcpy(&buffer[16], src_ip, 4);
		buffer[20] = 0;
		*((unsigned short*)&buffer[22]) += 8;
		*/
		printf("from %d.%d.%d.%d -> to %d.%d.%d.%d\n", 
				src_ip[0], src_ip[1], src_ip[2], src_ip[3],
				dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);

		Encrypt(&table, buffer, tmp_buffer, len);
		//memcpy(tmp_buffer, buffer, len);

		sendto(sock, tmp_buffer, len, 0, (struct sockaddr*)&addr, alen);

		recvfrom(sock, tmp_buffer, len, 0, (struct sockaddr*)&addr, &alen);

		//memcpy(buffer, tmp_buffer, len);
		Decrypt(&table, tmp_buffer, buffer, len);

		memcpy(src_ip, &buffer[12], 4);
		memcpy(dst_ip, &buffer[16], 4);
		printf("from %d.%d.%d.%d -> to %d.%d.%d.%d\n", 
				src_ip[0], src_ip[1], src_ip[2], src_ip[3],
				dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);



		len = write(fd, buffer, len);
		printf("I sent %d bytes!\n", len);
	}
	return 0;
}
