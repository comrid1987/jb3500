
//-------------------------------------------------------------------------
//Network Protocal Stack Modules
//-------------------------------------------------------------------------
#if TCPPS_TYPE == TCPPS_T_KEILTCP
#include "Net_Config.c"
#if TELNET_SERVER_ENABLE
#include "Telnet_uif.c"
#endif
#if DEBUG_NET_ENABLE
#include <net/rtxip/Retarget.c>
#endif
#include <net/rtxip/rtxip_eth.c>
#include <net/rtxip/rtxip.c>
#include <net/rtxip/sockets.c>

#endif












#if TCPPS_TYPE == TCPPS_T_LWIP
#include <net/lwip/arch/sys_arch.c>

#include <net/lwip/core/autoip.c>
#include <net/lwip/core/dhcp.c>
#include <net/lwip/core/dns.c>
#include <net/lwip/core/icmp.c>
#include <net/lwip/core/igmp.c>
#include <net/lwip/core/inet.c>
#include <net/lwip/core/inet_chksum.c>
#include <net/lwip/core/ip.c>
#include <net/lwip/core/ip_addr.c>
#include <net/lwip/core/ip_frag.c>
#include <net/lwip/core/mem.c>
#include <net/lwip/core/memp.c>
#include <net/lwip/core/netif.c>
#include <net/lwip/core/pbuf.c>
#include <net/lwip/core/raw.c>
#include <net/lwip/core/stats.c>
#include <net/lwip/core/sys.c>
#include <net/lwip/core/tcp.c>
#include <net/lwip/core/tcp_in.c>
#include <net/lwip/core/tcp_out.c>
#include <net/lwip/core/timers.c>
#include <net/lwip/core/udp.c>

#include <net/lwip/core/init.c>

#include <net/lwip/api/api_lib.c>
#include <net/lwip/api/api_msg.c>
#include <net/lwip/api/err.c>
#include <net/lwip/api/netbuf.c>
#include <net/lwip/api/netdb.c>
#include <net/lwip/api/netifapi.c>
#include <net/lwip/api/sockets.c>
#include <net/lwip/api/tcpip.c>

#if NETIO_SERVER_ENABLE
#include <net/apps/netio.c>
#endif

#if FTP_SERVER_ENABLE
#include <net/apps/ftpd.c>
#endif

#if RPC_ENABLE
#include <cp/rpc/pmap.c>
#include <cp/rpc/auth_none.c>
#include <cp/rpc/clnt_generic.c>
#include <cp/rpc/clnt_udp.c>
#include <cp/rpc/rpc_prot.c>
#include <cp/rpc/xdr.c>
#include <cp/rpc/xdr_mem.c>
#endif

#if TCPPS_ETH_ENABLE
#include <net/lwip/netif/etharp.c>
#include <net/lwip/netif/ethernetif.c>
#if STM32_ETH_ENABLE
#include <net/lwip/netif/netif_stm32f10x.c>
#endif
#if LM3S_ETH_ENABLE
#include <net/lwip/netif/netif_lm3s.c>
#endif
#if LPC176X_ETH_ENABLE
#include <net/lwip/netif/netif_lpc17xx.c>
#endif
#endif

#if TCPPS_PPP_ENABLE
#include <net/lwip/netif/ppp/auth.c>
#include <net/lwip/netif/ppp/chap.c>
#include <net/lwip/netif/ppp/chpms.c>
#include <net/lwip/netif/ppp/fsm.c>
#include <net/lwip/netif/ppp/ipcp.c>
#include <net/lwip/netif/ppp/lcp.c>
#include <net/lwip/netif/ppp/magic.c>
#include <net/lwip/netif/ppp/md5.c>
#include <net/lwip/netif/ppp/pap.c>
#include <net/lwip/netif/ppp/ppp.c>
#include <net/lwip/netif/ppp/ppp_oe.c>
#include <net/lwip/netif/ppp/randm.c>
#include <net/lwip/netif/ppp/vj.c>
#include <net/lwip/arch/sio.c>
#endif


#if TCPPS_DEBUG_ENABLE
void ppp_trace(int level, const char *format,...)
{
	va_list args;

	if (level < 5) {
		va_start(args, format);
		rt_kprintf(format, args);
		va_end(args);
	}
}
#endif


void net_Init()
{
	struct ip_addr ipaddr, netmask, gw;
#if LWIP_DHCP
 	rt_uint32_t mscnt = 0;
#endif

	tcpip_init(RT_NULL, RT_NULL);

#if TCPPS_ETH_ENABLE
	eth_system_device_init();
#if REGISTER_ENABLE
	///////////////Unfinished////////////////
	reg_Get(TERMINAL, 0x0161, &ipaddr);
	reg_Get(TERMINAL, 0x0162, &netmask);
	reg_Get(TERMINAL, 0x0163, &gw);
	///////////////////////////////////////
#else
	IP4_ADDR(&ipaddr, 192, 168, 18, 177);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 18, 1);
#endif
	netif_set_addr(netif_default, &ipaddr, &netmask, &gw);
	netif_set_up(netif_default);
#endif

#if TCPPS_PPP_ENABLE
	pppInit();
#endif

//#if LWIP_DHCP
#if 0
	/* use DHCP client */
	dhcp_start(netif_default);

    while (netif_default->ip_addr.addr == 0)
	{
        rt_thread_delay(DHCP_FINE_TIMER_MSECS * RT_TICK_PER_SECOND / 1000);

        dhcp_fine_tmr();
        mscnt += DHCP_FINE_TIMER_MSECS;
        if (mscnt >= DHCP_COARSE_TIMER_SECS*1000)
        {
            dhcp_coarse_tmr();
            mscnt = 0;
        }
    }
	
	rt_kprintf("Acquired IP address from DHCP server:");
	rt_kprintf("%d.%d.%d.%d\n", netif_default->ip_addr.addr & 0xff,
		(netif_default->ip_addr.addr>>8) & 0xff,
		(netif_default->ip_addr.addr>>16) & 0xff, 
		(netif_default->ip_addr.addr>>24) & 0xff);
#endif

#if defined(RT_USING_FINSH) && (LWIP_STATS_DISPLAY)
    finsh_syscall_append("lwip_info", (syscall_func)stats_display);
#endif
}

#endif


void net_Handler()
{

#if TCPPS_TYPE == TCPPS_T_KEILTCP
	rtxip_Handler(NULL);
#endif
}

