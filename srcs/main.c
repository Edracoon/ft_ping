#include "ft_ping.h"

Ping g_ping;

/**
 * * https://www.alpharithms.com/internet-checksum-calculation-steps-044921/
 * The Internet checksum is used in standard Internet Protocols such as IP, UDP, and TCP.
 * This value is used to verify the integrity of data after transmission across the network.
 * A client includes a checksum value in the segment header so that the
 * receiver can use that value to directly verify data integrity.
 * We need to verify that all bits are 1's.
 */
unsigned short checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	// Flips all the bits. ex.: 1011 -> 0100
	result = ~sum;
	return result;
}

struct sockaddr_in	resolve_address_dns(const char *hostname)
{
	struct addrinfo		*host;
	int gai_result;

	gai_result = getaddrinfo(hostname, NULL, NULL, &host);
	if (gai_result != 0) {
		fprintf(stderr, "getaddrinfo: Couldn't retrieve host '%s' IPv4 address: %s\n", hostname, gai_strerror(gai_result));
		exit(EXIT_FAILURE);
	}
	// Convert addrinfo to sockaddr_in
	struct sockaddr_in	target_addr;
	bzero(&target_addr, sizeof(target_addr));
	target_addr.sin_family = host->ai_family;
	target_addr.sin_addr = ((struct sockaddr_in *)host->ai_addr)->sin_addr;

	if (target_addr.sin_addr.s_addr == INADDR_BROADCAST && g_ping.debug == false) {
		fprintf(stderr, "ft_ping: Error: Broadcast address not allowed. Use -b option to enable broadcast.\n");
		exit(EXIT_FAILURE);
	}
	else if (target_addr.sin_family != AF_INET) {
		fprintf(stderr, "ft_ping: Error: '%s' is not an IPv4 address.\n", hostname);
		exit(EXIT_FAILURE);
	}
	return target_addr;
}

int	setup_socket(void)
{
	/* Init socket */
	int	sock;
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0) {
        printf("ft_ping: Error: Socket failed\n");
        return 0;
    }

	/* set socket options at ip to TTL */
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &g_ping.ttl, sizeof(g_ping.ttl)) == -1) {
        printf("ft_ping: Error: Setting socket options to TTL failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* setting timeout of recv setting */
	struct timeval recv_timeout;
	recv_timeout.tv_sec = 1; // 1 second by default
	recv_timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout)) == -1) {
		fprintf(stderr, "ft_ping: Error: Setting socket options timeout failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Setting socket option with SO_DEBUG if -d is enabled */
	// if (g_ping.debug && setsockopt(sock, SOL_SOCKET, SO_DEBUG, NULL, 0) == -1) {
	// 	fprintf(stderr, "ft_ping: Error: SO_DEBUG setting failed: %s\n", strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }
	return sock;
}

void	ping()
{
	struct ping_pkt		packet; 			/* The packet to send to destination */
	struct sockaddr_in	target_addr;		/* Target sockaddr to ping */
	struct sockaddr_in	recev_addr;			/* Recev sockaddr sent by destination */
	int msg_count = 0, msg_received_count = 0, i, addr_len, flag = true;

	target_addr = resolve_address_dns(g_ping.hostname);
	g_ping.socket = setup_socket();


	/* Send ping packet in an infinite loop */
    while(true)
	{
		/* Set the packet */
		bzero(&packet, sizeof(packet));
		packet.hdr.type = ICMP_ECHO;
		packet.hdr.un.echo.id = getpid();

		/* Fill the packet message */
		for (i = 0; (unsigned long)i < sizeof(packet.msg) - 1; i++)
			packet.msg[i] = i + '0';

		packet.msg[i] = 0;
		packet.hdr.un.echo.sequence = msg_count++;

		/* Fill checksum header */
		packet.hdr.checksum = checksum(&packet, sizeof(packet));
	}
	(void)target_addr;
	(void)recev_addr;
	(void)addr_len;
	(void)flag;
	(void)msg_received_count;
}

int	main(int ac, char **av)
{
	parseOption(ac, av);
	// Debug
	printf("Host: %s\n", g_ping.hostname);
	printf("Verbose: %s\n", g_ping.verbose ? "true" : "false");
	printf("Quiet: %s\n", g_ping.quiet ? "true" : "false");
	printf("Debug: %s\n", g_ping.debug ? "true" : "false");
	printf("Broadcast: %s\n", g_ping.broadcast ? "true" : "false");
	printf("TTL: %i\n", g_ping.ttl);
	ping();
}