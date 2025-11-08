/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/16 16:27:58 by mgama             #+#    #+#             */
/*   Updated: 2025/11/08 17:22:28 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int uid;
int ident;
int options;
int sockfd;
int interval = 1000; // 1 second
char *hostname;
struct sockaddr_in target_addr;
int datalen = 56;
u_char outpackhdr[IP_MAXPACKET], *outpack;

long npackets;
long nmissedmax;

volatile sig_atomic_t finish_up;

double tmin = 999999999.0;
double tmax = 0.0;
double tsum = 0.0;
double tsumsq = 0.0;

struct tv32 {
	u_int32_t tv32_sec;
	u_int32_t tv32_usec;
};

long ntransmitted = 0;
long nreceived = 0;

void
usage(void)
{
	(void)fprintf(stderr, "Usage\n");
	(void)fprintf(stderr, "  ft_ping [options] <destination>");
	(void)fprintf(stderr, "Options:\n");
	(void)fprintf(stderr, "  <destination>      dns name or ip address\n");
	(void)fprintf(stderr, "  -c <count>         stop after <count> replies\n");
	(void)fprintf(stderr, "  -h                 print help and exit\n");
	(void)fprintf(stderr, "  -V                 print version and exit\n");
	(void)fprintf(stderr, "  -v                 verbose output\n");
	exit(64);
}

u_short
in_cksum(u_short *addr, int len)
{
	int nleft, sum;
	u_short *w;
	union {
		u_short	us;
		u_char	uc[2];
	} last;
	u_short answer;

	nleft = len;
	sum = 0;
	w = addr;

	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		last.uc[0] = *(u_char *)w;
		last.uc[1] = 0;
		sum += last.us;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return(answer);
}

void
printrname(struct sockaddr *sa)
{
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	char ip_str[INET_ADDRSTRLEN];
	(void)inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, ip_str, sizeof(ip_str));

#ifdef __APPLE__
	socklen_t sa_len = sa->sa_len;
#else
	socklen_t sa_len = sizeof(struct sockaddr_in);
#endif /* __APPLE__ */

	/**
	 * Grace au rDNS (reverse DNS), on peut essayer de récupérer le nom
	 * de l'hôte à partir de son adresse IP.
	 */
	if (getnameinfo(sa, sa_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NAMEREQD) != 0)
	{
		(void)printf("%s (%s)", ip_str, ip_str);
	}
	else
	{
		(void)printf("%s (%s)", hbuf, ip_str);
	}
	(void)fflush(stdout);
}

static void
tvsub(struct timeval *out, const struct timeval *in)
{

	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

static void
finish(void)
{

	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGALRM, SIG_IGN);
	(void)putchar('\n');
	(void)fflush(stdout);
	(void)printf("--- %s ping statistics ---\n", hostname);
	(void)printf("%ld packets transmitted, ", ntransmitted);
	(void)printf("%ld packets received, ", nreceived);
	if (ntransmitted) {
		if (nreceived > ntransmitted)
			(void)printf("-- somebody's printing up packets!");
		else
			(void)printf("%.1f%% packet loss",
			    ((ntransmitted - nreceived) * 100.0) /
			    ntransmitted);
	}
	(void)putchar('\n');
	if (nreceived) {
		double n = nreceived;
		double avg = tsum / n;
		double vari = tsumsq / n - avg * avg;
		(void)printf(
		    "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		    tmin, avg, tmax, sqrt(vari));
	}

	if (nreceived)
		exit(0);
	else
		exit(2);
}

void
pinger()
{
	struct timeval now;
	struct tv32 tv32;
	int cc;
	struct ip *ip;
	struct icmp *icp;
	u_char *packet;

	packet = outpack;
	icp = (struct icmp *)outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = htons(ntransmitted);
	icp->icmp_id = ident;

	(void)gettimeofday(&now, NULL);

	tv32.tv32_sec = htonl(now.tv_sec);
	tv32.tv32_usec = htonl(now.tv_usec);

	bcopy((void *)&tv32, (void *)&outpack[ICMP_MINLEN], sizeof(tv32));

	cc = ICMP_MINLEN + datalen;

	icp->icmp_cksum = in_cksum((u_short *)icp, cc);

	if (options & F_HDRINCL) {
		cc += sizeof(struct ip);
		ip = (struct ip *)outpackhdr;
		ip->ip_len = cc;
		ip->ip_sum = in_cksum((u_short *)outpackhdr, cc);
		packet = outpackhdr;
	}

	if (sendto(sockfd, (char *)packet, cc, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
		perror("sendto");
		return;
	}

	ntransmitted++;
}

void
receiver(char *buff, int rcv, struct sockaddr_in *from, struct timeval *tv)
{
	struct ip *ip;
	struct icmp *icmp;
	double rtt;
	const void *tp;
	int hlen, seq;
	uint32_t recv_len;

	ip = (struct ip *)buff;
	hlen = ip->ip_hl << 2;
	recv_len = rcv;
	if (rcv < hlen + ICMP_MINLEN) {
		if (options & F_VERBOSE)
			printf("packet too short (%d bytes) from %s\n", rcv, inet_ntoa(from->sin_addr));
		return;
	}

	rcv -= hlen;
	icmp = (struct icmp *)(buff + hlen);
	if (icmp->icmp_type == ICMP_ECHOREPLY) {
		if (icmp->icmp_id != ident) {
			if (options & F_VERBOSE)
				printf("wrong identification %d from %s\n", ntohs(icmp->icmp_id), inet_ntoa(from->sin_addr));
			return;
		}
		++nreceived;
		rtt = 0.0;

		struct timeval tv1;
		struct tv32 tv32;

#ifndef icmp_data
		tp = &icmp->icmp_ip;
#else
		tp = icmp->icmp_data;
#endif
		tp = (const char *)tp;

		if (recv_len - ICMP_MINLEN >= sizeof(tv1))
		{
			memcpy(&tv32, tp, sizeof(tv32));
			tv1.tv_sec = ntohl(tv32.tv32_sec);
			tv1.tv_usec = ntohl(tv32.tv32_usec);

			tvsub(tv, &tv1);
			rtt = ((double)tv->tv_sec) * 1000.0 + ((double)tv->tv_usec) / 1000.0;

			tsum += rtt;
			tsumsq += rtt * rtt;
			if (rtt < tmin)
				tmin = rtt;
			if (rtt > tmax)
				tmax = rtt;
		}

		seq = ntohs(icmp->icmp_seq);
		if (seq < ntransmitted - 1) {
			if (options & F_VERBOSE)
				printf("duplication %d from %s\n", seq, inet_ntoa(from->sin_addr));
			return;
		}

		printf("%d bytes from ", rcv);
		if (options & F_VERBOSE)
			printrname((struct sockaddr *)from);
		else
			printf("%s", inet_ntoa(from->sin_addr));
		printf(": icmp_seq=%u", seq);
		printf(" ttl=%d", ip->ip_ttl);
		printf(" time=%.3f ms\n", rtt);
	}
}

void
stop(int sig __unused)
{
	if (finish_up)
		_exit(nreceived ? 0 : 2);
	finish_up = 1;
}

int
main(int ac, char **av)
{
	char	v;
	char	*target, *ep;
	int		hold, ttl, tos;
#ifdef __APPLE__
	int		mib[4];
#endif /* __APPLE__ */
	struct iovec iov;
	struct sockaddr_in *to;
	struct sockaddr_in from;
	struct ip *ip;
	struct hostent *hosts;
	struct sigaction si_sa;
	struct timeval last, intvl;
	struct msghdr msg;
	u_long ultmp;
	u_char packet[IP_MAXPACKET] __attribute__((aligned(4)));

	outpack = outpackhdr + sizeof(struct ip);
	while ((v = getopt(ac, av, "c:dhom:Vvz:")) !=  -1)
	{
		switch (v)
		{
		case 'c':
			ultmp = strtoul(optarg, &ep, 0);
			if (*ep || ep == optarg || ultmp > LONG_MAX || !ultmp)
				errx(EX_USAGE,
				    "invalid count of packets to transmit: `%s'",
				    optarg);
			npackets = ultmp;
			break;
		case 'd':
			options |= F_SO_DEBUG;
			break;
		case 'o':
			options |= F_ONCE;
			break;
		case 'm':
			ultmp = strtoul(optarg, &ep, 0);
			if (*ep || ep == optarg || ultmp > MAXTTL)
				errx(EX_USAGE, "invalid TTL: `%s'", optarg);
			ttl = ultmp;
			options |= F_TTL;
			break;
		case 'V':
			printf("ft_ping v%.1f\n", PG_VERSION);
			exit(0);
			break;
		case 'v':
			options |= F_VERBOSE;
			break;
		case 'z':
			options |= F_HDRINCL;
			ultmp = strtoul(optarg, &ep, 0);
			if (*ep || ep == optarg || ultmp > MAXTOS)
				errx(EX_USAGE, "invalid TOS: `%s'", optarg);
			tos = ultmp;
			break;
		default:
			usage();
		}
	}

	if (ac - optind != 1)
		usage();
	target = av[optind];

	if (getuid())
		sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	else
		sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket");
		exit(1);
	}

	if (setuid(getuid()) != 0)
	{
		perror("setuid");
		exit(1);
	}
	uid = getuid();

	bzero(&target_addr, sizeof(target_addr));
	to = &target_addr;
	to->sin_family = AF_INET;
#ifdef __APPLE__
	to->sin_len = sizeof(*to);
#endif /* __APPLE__ */
	if (inet_aton(target, &to->sin_addr) != 0)
	{
		hostname = target;
	}
	else
	{
		hosts = gethostbyname2(target, AF_INET);
		if (hosts == NULL)
		{
			perror("gethostbyname2");
			exit(1);
		}
		if ((unsigned)hosts->h_length > sizeof(to->sin_addr))
		{
			fprintf(stderr, "gethostbyname2: address too long\n");
			exit(1);
		}
		memcpy(&to->sin_addr, hosts->h_addr, sizeof(to->sin_addr));
		hostname = hosts->h_name;
	}

	hold = IP_MAXPACKET + 128;
	(void)setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &hold, sizeof(hold));
	if (uid == 0)
		(void)setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&hold, sizeof(hold));

	if (options & F_SO_DEBUG)
		(void)setsockopt(sockfd, SOL_SOCKET, SO_DEBUG, (char *)&hold,
		    sizeof(hold));

	if (options & F_TTL) {
		if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl,
		    sizeof(ttl)) < 0) {
			err(EX_OSERR, "setsockopt IP_TTL");
		}
	}

	if (options & F_HDRINCL) {
		ip = (struct ip*)outpackhdr;
		if (!(options & (F_TTL))) {
#ifdef __APPLE__
			size_t sz = sizeof(ttl);
			mib[0] = CTL_NET;
			mib[1] = PF_INET;
			mib[2] = IPPROTO_IP;
			mib[3] = IPCTL_DEFTTL;

			if (sysctl(mib, 4, &ttl, &sz, NULL, 0) == -1)
				err(1, "sysctl(net.inet.ip.ttl)");
#else
			FILE *f = fopen("/proc/sys/net/ipv4/ip_default_ttl", "r");
			if (!f)
				err(1, "fopen(/proc/sys/net/ipv4/ip_default_ttl)");

			if (fscanf(f, "%d", &ttl) != 1) {
				fclose(f);
				err(1, "fscanf(/proc/sys/net/ipv4/ip_default_ttl)");
			}

			fclose(f);		
#endif /* __APPLE__ */
		}
		setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &hold, sizeof(hold));
		ip->ip_v = IPVERSION;
		ip->ip_hl = sizeof(struct ip) >> 2;
		ip->ip_tos = tos;
		ip->ip_id = 0;
		ip->ip_off = IP_DF;
		ip->ip_ttl = ttl;
		ip->ip_p = IPPROTO_ICMP;
		ip->ip_src.s_addr = INADDR_ANY;
		ip->ip_dst = to->sin_addr;
    }

	ident = getpid() & 0xFFFF;
	
	printf("FT_PING %s (%s): %d data bytes\n", hostname, inet_ntoa(target_addr.sin_addr), datalen);

	sigemptyset(&si_sa.sa_mask);
	si_sa.sa_flags = 0;

	si_sa.sa_handler = stop;
	if (sigaction(SIGINT, &si_sa, NULL) < 0)
	{
		perror("sigaction");
		exit(1);
	}

	si_sa.sa_handler = stop;
	if (sigaction(SIGQUIT, &si_sa, NULL) < 0)
	{
		perror("sigaction");
		exit(1);
	}

	si_sa.sa_handler = stop;
	if (sigaction(SIGALRM, &si_sa, NULL) < 0)
	{
		perror("sigaction");
		exit(1);
	}

	bzero(&msg, sizeof(msg));
	msg.msg_name = (caddr_t)&from;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_base = packet;
	iov.iov_len = IP_MAXPACKET;

	pinger();
	(void)gettimeofday(&last, NULL);

	intvl.tv_sec = interval / 1000;
	intvl.tv_usec = interval % 1000 * 1000;

	while (!finish_up)
	{
		fd_set readfds;
		struct timeval now, timeout;
		int n, rcv;

		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		(void)gettimeofday(&now, NULL);
		timeout.tv_sec = last.tv_sec + intvl.tv_sec - now.tv_sec;
		timeout.tv_usec = last.tv_usec + intvl.tv_usec - now.tv_usec;
		while (timeout.tv_usec < 0) {
			timeout.tv_usec += 1000000;
			timeout.tv_sec--;
		}
		while (timeout.tv_usec >= 1000000) {
			timeout.tv_usec -= 1000000;
			timeout.tv_sec++;
		}
		if (timeout.tv_sec < 0)
			timeout.tv_sec = timeout.tv_usec = 0;
		n = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
		if (n < 0)
			continue;
		if (n == 1)
		{
			struct timeval *tv = NULL;

			msg.msg_namelen = sizeof(from);
			if ((rcv = recvmsg(sockfd, &msg, 0)) < 0)
			{
				if (errno == EINTR)
					continue;
				perror("recvmsg");
				continue;
			}
			if (tv == NULL) {
				(void)gettimeofday(&now, NULL);
				tv = &now;
			}
			receiver((char *)packet, rcv, &from, tv);
			if ((options & F_ONCE && nreceived) || (npackets && nreceived >= npackets))
				break;
		}
		if (n == 0)
		{
			pinger();

			(void)gettimeofday(&last, NULL);
			if (ntransmitted - nreceived - 1 > nmissedmax) {
				nmissedmax = ntransmitted - nreceived - 1;
				printf("Request timeout for icmp_seq %ld\n", ntransmitted - 2);
			}
		}
	}

	finish();
	return (0);
}
