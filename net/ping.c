#include <common.h>
#include <command.h>
#include <clock.h>
#include <net.h>

static ushort PingSeqNo;

int PingSend(void)
{
	static uchar mac[6];
	volatile IP_t *ip;
	volatile ushort *s;
	uchar *pkt;

	/* XXX always send arp request */

	memcpy(mac, NetEtherNullAddr, 6);

#ifdef ET_DEBUG
	printf("sending ARP for %08lx\n", NetPingIP);
#endif

	NetArpWaitPacketIP = NetPingIP;
	NetArpWaitPacketMAC = mac;

	pkt = NetArpWaitTxPacket;
	pkt += NetSetEther(pkt, mac, PROT_IP);

	ip = (volatile IP_t *)pkt;

	/*
	 *	Construct an IP and ICMP header.  (need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE_NO_UDP + 8);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(0x4000);	/* No fragmentation */
	ip->ip_ttl   = 255;
	ip->ip_p     = 0x01;		/* ICMP */
	ip->ip_sum   = 0;
	NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
	NetCopyIP((void*)&ip->ip_dst, &NetPingIP);	   /* - "" - */
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);

	s = &ip->udp_src;		/* XXX ICMP starts here */
	s[0] = htons(0x0800);		/* echo-request, code */
	s[1] = 0;			/* checksum */
	s[2] = 0; 			/* identifier */
	s[3] = htons(PingSeqNo++);	/* sequence number */
	s[1] = ~NetCksum((uchar *)s, 8/2);

	/* size of the waiting packet */
	NetArpWaitTxPacketSize = (pkt - NetArpWaitTxPacket) + IP_HDR_SIZE_NO_UDP + 8;

	/* and do the ARP request */
	NetArpWaitTry = 1;
	NetArpWaitTimerStart = get_time_ns();
	ArpRequest();
	return 1;	/* waiting */
}

static void
PingTimeout (void)
{
	eth_halt();
	NetState = NETLOOP_FAIL;	/* we did not get the reply */
}

static void
PingHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	IPaddr_t tmp;
	volatile IP_t *ip = (volatile IP_t *)pkt;

	tmp = NetReadIP((void *)&ip->ip_src);
	if (tmp != NetPingIP)
		return;

	NetState = NETLOOP_SUCCESS;
}

void PingStart(void)
{
	NetSetTimeout (10 * SECOND, PingTimeout);
	NetSetHandler (PingHandler);

	PingSend();
}

int do_ping (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2)
		return -1;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}

	if (NetLoop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	printf("host %s is alive\n", argv[1]);

	return 0;
}

U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"ping\t- send ICMP ECHO_REQUEST to network host\n",
	"pingAddress\n"
);