#include "ft_ping.h"

// Make a atoull function to convert a string to an unsigned long long
// https://www.geeksforgeeks.org/write-your-own-atoi/

ull		strLen(str s) {
	ull		i = 0;
	while (s[i++] != '\0');
	return (i);
}

void	usage(const char * potentialError)
{
	fprintf(stderr, "Description:\n\tPing send an ICMP ECHO_REQUEST to <destination>\n \
	and waits for an ICMP ECHO_RESPONSE from the <destination>.\n \
	The ping command is used to verify the connectivity at IP level.\n \
	It ensure that the data packet sent has reached its destination point without any loss.\n\n \
Usage: \n\t ./ft_ping <destination> [-h] [-v] [-d] [-q] [-b] [-t <ttl>] [-4]\n\n \
Option:\n \
	-h\t\t( Show this help )\n \
	-v\t\t( Enable verbose mode )\n \
	-d\t\t( Enable debug mode using SO_DEBUG )\n \
	-q\t\t( Enable quiet mode )\n \
	-b\t\t( Enable broadcast, ex.: 255.255.255.255 )\n \
	-t <ttl>\t( Time To Live = Maximum number of IP routers that the packet can go through )\n \
	-4 <ipv4>\t( IPv4 address )\n");
	if (potentialError)
		fprintf(stderr, "\e[0;31mft_ping: Error: %s\n\e[0m", potentialError);
	exit(EXIT_FAILURE);
}

void	parseOption(ull ac, char **av) {
	g_ping.hostname = NULL;
	g_ping.verbose = false;
	g_ping.quiet = false;
	g_ping.debug = false;
	g_ping.broadcast = false;
	g_ping.ttl = 64; // Avoid infinite loop if destination can't be found
	g_ping.interval = 1; // 1 second by default of interval

	for (ull i = 1 ; i < ac ; i++)
	{
		if (av[i][0] == '-')
		{
			if (av[i][1] == 'h')
				usage(NULL);
			else if (av[i][1] == 'v')
				g_ping.verbose = true;
			else if (av[i][1] == 'd')
				g_ping.debug = true;
			else if (av[i][1] == 'q')
				g_ping.quiet = true;
			else if (av[i][1] == 'b')
				g_ping.broadcast = true;
			else if (av[i][1] == 't')
			{
				if (i + 1 < ac)
				{
					ull		ttl = 0;
					for (ull j = 0 ; av[i + 1][j] ; j++)
					{
						if (av[i + 1][j] < '0' || av[i + 1][j] > '9')
							usage("invalid argument for -t option");
						ttl = ttl * 10 + (av[i + 1][j] - '0');
						if (ttl > 255)
							usage("invalid argument: '266': out of range: 0 <= value <= 255");
						if (ttl == 0)
							usage("cannot set unicast time-to-live: Invalid argument");
					}
					g_ping.ttl = ttl;
					i++;
				}
				else
					usage("Missing argument for -t option");
			}
			else if (av[i][1] == '4') {} // Nice bonus thanks you
		}
		else
			g_ping.hostname = av[i];
	}
	if (!g_ping.hostname)
		usage("Destination address required");
}