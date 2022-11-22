#include "ft_ping.h"

/* The global variable :) */
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

	/* getaddrinfo() will convert an hostname into an addrinfo struct for us */
	gai_result = getaddrinfo(hostname, NULL, NULL, &host);
	if (gai_result != 0) {
		fprintf(stderr, "getaddrinfo: Couldn't retrieve host '%s' IPv4 address: %s\n", hostname, gai_strerror(gai_result));
		exit(EXIT_FAILURE);
	}

	/* Convert addrinfo to sockaddr_in */
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

	/* Get the raw address ip from the sin_addr with inet_ntoa */
	g_ping.ip_addr = inet_ntoa(target_addr.sin_addr);

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

	/* set socket options at ip to TTL (TLL is number of router socket can pass trought~)*/
    if (setsockopt(sock, SOL_IP, IP_TTL, &g_ping.ttl, sizeof(g_ping.ttl)) == -1) {
        printf("ft_ping: Error: Setting socket options to TTL failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* setting timeout of recv */
	struct timeval recv_timeout;
	recv_timeout.tv_sec = 1; // 1 second by default of interval
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

void	ping(struct sockaddr_in target_addr)
{
	struct ping_pkt		packet;				/* The packet to send to destination */
	struct sockaddr_in	recev_addr;			/* Recev sockaddr sent by destination */
	bool				sent = true;		/* Flag to know if we sent a packet or not */
	struct timeval		start_time;			/* Time we sent the packet */
	struct timeval		end_time;			/* Time we received the packet */
	double				elapsed_ms = 0;		/* Time between start_time and end_time */
	int					i, addr_len;		/* Randoms utils */

	/* Setup the packet */
	bzero(&packet, sizeof(packet));
	packet.hdr.type = ICMP_ECHO;
	packet.hdr.un.echo.id = getpid();

	/* Fill the packet message randomly */
	for (i = 0; (unsigned long)i < sizeof(packet.msg) - 1; i++)
		packet.msg[i] = i + '0';
	packet.msg[i] = '\0';
	packet.hdr.un.echo.sequence = g_ping.send_count++;

	/* Fill checksum header to ensure our packet is not corrupted */
	packet.hdr.checksum = checksum(&packet, sizeof(packet));

	/* Get the current time */
	gettimeofday(&start_time, NULL);

	/* Send the packet */
	if (sendto(g_ping.socket, &packet, sizeof(packet), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) == -1)
	{
		if (g_ping.verbose)
			fprintf(stderr, "ft_ping: Error: Sending packet failed: %s\n", strerror(errno));
		sent = false;
	}

	/* Receive the packet */
	addr_len = sizeof(recev_addr);
	if (recvfrom(g_ping.socket, &packet, sizeof(packet), 0, (struct sockaddr *)&recev_addr, (socklen_t *)&addr_len) == -1)
	{
		if (g_ping.verbose)
			fprintf(stderr, "ft_ping: Error: Receiving packet failed: %s\n", strerror(errno));
		return ;
	}

	/* Calculate elapsed time during the process */
	gettimeofday(&end_time, NULL);
	elapsed_ms = (end_time.tv_sec - start_time.tv_sec) * 1000.0; 	/* sec to ms */
	elapsed_ms += (end_time.tv_usec - start_time.tv_usec) / 1000.0;	/* micro to ms */

	/* Check if packet is an echo_reply ( 69 in RFC and manual ;) ), also check that we have sent the packet */
	if (packet.hdr.type == 69 && sent)
	{
		/* Print packet infos as the real ping */
		printf("%ld bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1f ms\n", sizeof(packet), g_ping.hostname, g_ping.ip_addr, g_ping.recv_count, g_ping.ttl, elapsed_ms);
		g_ping.recv_count++;
	}
	else if (g_ping.verbose && sent)
		fprintf(stderr, "ft_ping: Error: Packet received is not an ECHO_REPLY\n");

	/* Add back the current elapsed time to the list of elapsed time */
	add_back_time(elapsed_ms);
}

/* Alarm handler */
void	alarm_handler(int sig)
{
	(void)sig;
	ping(g_ping.target_addr);
	alarm(g_ping.interval);
}

/* Ctrl+C Handler */
void	quit_handler(int sig)
{
	(void)sig;
	/* Get end time */
	gettimeofday(&g_ping.end_time, NULL);
	ull	elapsed_total_time = (g_ping.end_time.tv_sec - g_ping.start_time.tv_sec) * 1000.0;	/* sec to ms */
	elapsed_total_time += (g_ping.end_time.tv_usec - g_ping.start_time.tv_usec) / 1000.0;	/* micro to ms */

	/* Print stats */
	printf("--- %s ping statistics ---\n", g_ping.hostname);
	printf("%d packets transmitted, %d received, %.2f%% packet loss, time %llums\n",
			g_ping.send_count, g_ping.recv_count, (double)(g_ping.send_count - g_ping.recv_count) / g_ping.send_count * 100, elapsed_total_time);

	/* Compute RTT */
	compute_rtt();

	/* Print RTT stats */
	printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", g_ping.rtt_min, g_ping.rtt_avg, g_ping.rtt_max, g_ping.rtt_mdev);
	exit(EXIT_SUCCESS);
}

int	main(int ac, char **av)
{
	/* Parse params ans setup the global struct */
	parseOption(ac, av);

	/* Get the sockaddr_in of the target destination */
	g_ping.target_addr = resolve_address_dns(g_ping.hostname);

	/* Setup our socket */
	g_ping.socket = setup_socket();

	/* Setup signal handlers */
	signal(SIGALRM, alarm_handler);
	signal(SIGINT, quit_handler);

	/* Get the current time for statistics later on */
	gettimeofday(&g_ping.start_time, NULL);

	/* Create an infinite loop using alarm and signal */
	printf("PING %s (%s) 64 bytes of data.\n", g_ping.hostname, g_ping.ip_addr);
	ping(g_ping.target_addr);
	alarm(g_ping.interval);
	while (true) ;
}