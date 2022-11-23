#include "ft_ping.h"

double	absolute(double n)
{
	return (n < 0 ? -n : n);
}

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

		count++;
		tmp = tmp->next;
	}
	avg = avg / count;

	/* mdev */
	tmp = g_ping.times;
	while (tmp)
	{
		mdev += absolute(tmp->elapsed_ms - avg);
		tmp = tmp->next;
	}
	mdev = mdev / count;

	free_list_time();

	/* Store the stats */
	g_ping.rtt_min = min;
	g_ping.rtt_max = max;
	g_ping.rtt_avg = avg;
	g_ping.rtt_mdev = mdev;
}

void	free_list_time()
{
	Times	*tmp = g_ping.times;
	Times	*next;

	while (tmp)
	{
		next = tmp->next;
		free(tmp);
		tmp = next;
	}
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
