#pragma once

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

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
/* (not working on MacOS but debian is fine) */
struct ping_pkt
{
	struct icmphdr	hdr;
	char			msg[64 - sizeof(struct icmphdr)];
};

typedef struct s_times {
	double				elapsed_ms;
	struct s_times	*next;
} Times;

typedef struct s_ping {

	/* --- Important --- */
	str					hostname;
	str					ip_addr;
	struct sockaddr_in	target_addr;
	int					socket;

	/* --- Options --- */
	bool	verbose;
	bool	quiet;
	bool	debug;
	bool	broadcast;
	int 	interval;
	int		ttl;

	/* --- Statistics --- */
	int		send_count;	/* Number of packets sent */
	int		recv_count; /* Number of packets received */

	double	rtt_min;	/* Best response time (lowest) */
	double	rtt_avg;	/* Average response time */
	double	rtt_max;	/* Worst response time (biggest) */
	double	rtt_mdev;	/* Mean deviation */

	struct timeval	start_time;
	struct timeval	end_time;

	Times			*times;

} Ping;

/* Global variable */
extern Ping	g_ping;

/* Functions */
void	parseOption(ull ac, char **av);
void	usage(const char * potentialError);
void	add_back_time(double new_elapsed_time);
void	compute_rtt();

