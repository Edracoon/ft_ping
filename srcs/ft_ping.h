#pragma once

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

/* Ping relatives imports */
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef unsigned long long	ull;
typedef long long			ll;
typedef char *				str;

typedef enum bool {
	false,
	true
} bool;

/* ping packet structure */
struct ping_pkt
{
	struct icmphdr	hdr;
	char			msg[64 - sizeof(struct icmphdr)];
};

typedef struct s_ping {
	str		hostname;
	bool	verbose;
	bool	quiet;
	bool	debug;
	bool	broadcast;
	int		ttl;

	int		socket;
} Ping;

/* Global variable */
extern Ping	g_ping;

/* Functions */
void	parseOption(ull ac, char **av);
void	usage(const char * potentialError);
