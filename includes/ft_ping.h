/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 16:27:56 by mgama             #+#    #+#             */
/*   Updated: 2025/10/20 14:20:49 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING
#define FT_PING

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <err.h>
#include <math.h>
#include <sysexits.h>
#include <ifaddrs.h>

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>

#define F_VERBOSE		0x001
#define F_COUNT			0x002
#define F_TIMESTAMP		0x004

#define F_SO_DEBUG		0x100
#define F_SO_DONTROUTE	0x200

#define PG_VERSION 1.0

#endif /* FT_PING */