/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 16:27:56 by mgama             #+#    #+#             */
/*   Updated: 2025/11/13 18:00:29 by mgama            ###   ########.fr       */
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
#include <sys/select.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif /* __APPLE__  */
#include <sys/uio.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#ifdef __APPLE__
#include <netinet/ip_var.h>
#endif /* __APPLE__  */
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>

#define F_VERBOSE		0x0001
#define F_COUNT			0x0002
#define F_TIMESTAMP		0x0004
#define	F_SO_DEBUG		0x0010
#define	F_TTL			0x0100
#define	F_TOS			0x0200
#define	F_ONCE			0x0400
#define	F_HDRINCL		0x0800

#define PG_VERSION 1.5

#define MAXTOS 255

#ifndef __APPLE__
#define __unused __attribute__((unused))
#endif

#endif /* FT_PING */