#include <logging/log.h>
LOG_MODULE_REGISTER(net_telnet_sample, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <linker/sections.h>
#include <errno.h>
#include <stdio.h>

#include <net/net_core.h>
#include <net/net_if.h>
#include <net/net_mgmt.h>


#include <stdlib.h>
#include <net/socket.h>
#include <kernel.h>

#if defined(CONFIG_NET_DHCPV4)
static struct net_mgmt_event_callback mgmt_cb;
bool volatile connected;
static void ipv4_addr_add_handler(struct net_mgmt_event_callback *cb,
				  uint32_t mgmt_event,
				  struct net_if *iface)
{
	char hr_addr[NET_IPV4_ADDR_LEN];
	int i = 0;

	if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
		/* Spurious callback. */
		return;
	}

	for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
		struct net_if_addr *if_addr =
			&iface->config.ip.ipv4->unicast[i];

		if (if_addr->addr_type != NET_ADDR_DHCP || !if_addr->is_used) {
			continue;
		}

		printf("IPv4 address: %s",
			net_addr_ntop(AF_INET,
					       &if_addr->address.in_addr,
					       hr_addr, NET_IPV4_ADDR_LEN));
		LOG_INF("Lease time: %u seconds",
			 iface->config.dhcpv4.lease_time);
		LOG_INF("Subnet: %s",
			log_strdup(net_addr_ntop(AF_INET,
					       &iface->config.ip.ipv4->netmask,
					       hr_addr, NET_IPV4_ADDR_LEN)));
		LOG_INF("Router: %s",
			log_strdup(net_addr_ntop(AF_INET,
						 &iface->config.ip.ipv4->gw,
						 hr_addr, NET_IPV4_ADDR_LEN)));
		connected = true;
		break;
	}
	
}

static void setup_dhcpv4(struct net_if *iface)
{
	LOG_INF("Running dhcpv4 client...");

	net_mgmt_init_event_callback(&mgmt_cb, ipv4_addr_add_handler,
				     NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&mgmt_cb);

	net_dhcpv4_start(iface);
}

#else
#define setup_dhcpv4(...)
#endif /* CONFIG_NET_DHCPV4 */

#if defined(CONFIG_NET_IPV4) && !defined(CONFIG_NET_DHCPV4)

#if !defined(CONFIG_NET_CONFIG_MY_IPV4_ADDR)
#error "You need to define an IPv4 Address or enable DHCPv4!"
#endif

static void setup_ipv4(struct net_if *iface)
{
	char hr_addr[NET_IPV4_ADDR_LEN];
	struct in_addr addr;

	if (net_addr_pton(AF_INET, CONFIG_NET_CONFIG_MY_IPV4_ADDR, &addr)) {
		LOG_ERR("Invalid address: %s", CONFIG_NET_CONFIG_MY_IPV4_ADDR);
		return;
	}

	net_if_ipv4_addr_add(iface, &addr, NET_ADDR_MANUAL, 0);

	LOG_INF("IPv4 address: %s",
		log_strdup(net_addr_ntop(AF_INET, &addr, hr_addr,
					 NET_IPV4_ADDR_LEN)));
}

#else
#define setup_ipv4(...)
#endif /* CONFIG_NET_IPV4 && !CONFIG_NET_DHCPV4 */

void sendData(){
	int serv;
	struct sockaddr_in sa;
	//static int counter;
	serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//serv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (serv < 0) {
		printf("error: socket: %d\n", errno);
		return;
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons(13000);
	int result = inet_pton(sa.sin_family, "172.16.0.195", &sa.sin_addr);
	if (!result){
		printf("Error converting address!\n");
		return;
	}
	

	uint8_t buffer[90] = {0x01, 0xAB, 0xCD, 0xEF, 0xAB, 0x02, 0xCD, 0xEF,
                        0xAB, 0xCD, 0x03, 0xEF, 0xAB, 0xCD, 0xEF, 0x04,
                        0xAB, 0xCD, 0xEF, 0xAB, 0x05, 0xCD, 0xEF, 0xAB,
                        0xCD, 0x06, 0xEF, 0xAB, 0xCD, 0xEF, 0x07, 0xAB,
                        0xCD, 0xEF, 0xAB, 0x08, 0xCD, 0xEF, 0xAB, 0xCD,
                        0x09, 0xEF, 0xAB, 0xCD, 0xEF, 0x10, 0xAB, 0xCD, 
                        0xEF, 0xAB, 0x11, 0xCD, 0xEF, 0xAB, 0xCD, 0x12,
                        0xEF, 0xAB, 0xCD, 0xEF, 0x13, 0xAB, 0xCD, 0xEF, 
                        0xAB, 0x14, 0xCD, 0xEF, 0xAB, 0xCD, 0x15, 0xEF, 
                        0xAB, 0xCD, 0xEF, 0x16, 0xAB, 0xCD, 0xEF, 0xAB,
                        0x17, 0xCD, 0xEF, 0xAB, 0xCD, 0x18, 0xEF, 0xAB,
                        0xCD, 0xEF};

	while (1) {
		//char addr_str[32];
		int result = connect(serv, (struct sockaddr *)&sa, sizeof sa);
		if (result ==0){
			printf("Connected!\n");
		} else{
			printf("Error connecting: %d\n", errno);
			return;
		}
		while (1) {
			char *p;
			int out_len;
			int len=90;
			p = buffer;
			do {
				out_len = send(serv, buffer, len, 0);
				if (out_len < 0) {
					printf("error: send: %d\n", errno);
					goto error;
				}
				p += out_len;
				len -= out_len;
			} while (len);
			k_usleep(2200);
		}

error:
		close(serv);
		printf("Connection from closed\n");
	}
}


void main(void)
{
	struct net_if *iface = net_if_get_default();

	LOG_INF("Starting Telnet sample");

	setup_ipv4(iface);

	setup_dhcpv4(iface);
/*
	while (!connected){
		k_msleep(1000);
	}
*/
for (int c=0; c<500; c++){
		k_msleep(20);
		if (connected){
			break;
		}
	}
	printf("out of loop\n");
	sendData();
	while (1){
		k_msleep(2000);
	}
}
