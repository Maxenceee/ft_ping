/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 16:27:58 by mgama             #+#    #+#             */
/*   Updated: 2024/07/20 19:12:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int options;

void	usage()
{
	(void)fprintf(stderr, "Usage\n");
	(void)fprintf(stderr, "  ft_ping [options] <destination>");
	(void)fprintf(stderr, "Options:\n");
	(void)fprintf(stderr, "  <destination>      dns name or ip address\n");
	(void)fprintf(stderr, "  -c <count>         stop after <count> replies\n");
	(void)fprintf(stderr, "  -D                 print timestamps\n");
	(void)fprintf(stderr, "  -d                 use SO_DEBUG socket option\n");
	(void)fprintf(stderr, "  -h                 print help and exit\n");
	(void)fprintf(stderr, "  -r                 use SO_DONTROUTE socket option\n");
	(void)fprintf(stderr, "  -V                 print version and exit\n");
	(void)fprintf(stderr, "  -v                 verbose output\n");
	exit(64);
}

int main(int ac, char **av)
{
	char	v;
	char	*target;

	while ((v = getopt(ac, av, "c:DdhrVv")) !=  -1)
	{
		switch (v)
		{
		case 'c':
			options |= F_COUNT;
			break;
		case 'D':
			options |= F_TIMESTAMP;
			break;
		case 'd':
			options |= F_SO_DEBUG;
			break;
		case 'r':
			options |= F_SO_DONTROUTE;
			break;
		case 'V':
			printf("ft_ping v%.1f\n", PG_VERSION);
			exit(0);
			break;
		case 'v':
			options |= F_VERBOSE;
			break;
		default:
			usage(av);
		}
	}

	if (ac - optind != 1)
		usage();
	target = av[optind];

	printf("target: %s\n", target);

	return (0);
}
