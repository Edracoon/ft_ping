#include "ft_ping.h"

/* Calculate and Print Stats */
void	compute_rtt()
{
	Times	*tmp = g_ping.times;
	double	min = tmp->elapsed_ms;
	double	max = tmp->elapsed_ms;
	double	avg = 0;
	double	mdev = 0;
	int		count = 0;

	while (tmp)
	{
		/* min */
		if (tmp->elapsed_ms < min)
			min = tmp->elapsed_ms;

		/* max */
		if (tmp->elapsed_ms > max)
			max = tmp->elapsed_ms;

		/* avg */
		avg += tmp->elapsed_ms;

		/* mdev */
		mdev += pow(tmp->elapsed_ms - avg, 2);

		count++;
		tmp = tmp->next;
	}
	avg /= count;
	mdev = sqrt(mdev / count);

	/* Store the stats */
	g_ping.rtt_min = min;
	g_ping.rtt_max = max;
	g_ping.rtt_avg = avg;
	g_ping.rtt_mdev = mdev;
}

void	add_back_time(double new_elapsed_time)
{
	/* Init new */
	Times *new = malloc(sizeof(Times));
	if (new == NULL) usage("Malloc failed, no space left on device");
	new->elapsed_ms = new_elapsed_time;
	new->next = NULL;

	/* Add to linked list */
	Times *tmp = g_ping.times;
	if (tmp == NULL)
	{
		g_ping.times = new;
		return ;
	}
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = new;
}
