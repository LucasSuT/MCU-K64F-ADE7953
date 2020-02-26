#include "ntp-client/NTPClient.h"
#include "mbed.h"

//-----------------------------------------------------------------
// feature
//-----------------------------------------------------------------
#define _MULTI_NTP_SERVER

//-----------------------------------------------------------------
// constant
//-----------------------------------------------------------------
#ifdef _MULTI_NTP_SERVER
static const char ntpserver0[] = "pool.ntp.org";
static const char ntpserver1[] = "asia.pool.ntp.org";
static const char ntpserver2[] = "watch.stdtime.gov.tw";
static const char ntpserver3[] = "tick.stdtime.gov.tw";
static const char ntpserver4[] = "time1.google.com";
static const char ntpserver5[] = "time2.google.com";
static const char ntpserver6[] = "time.windows.com";
static const char ntpserver7[] = "ntps.sparktech.io";
static const char ntpserver8[] = "13.76.83.89";
//static const char ntpserver4[] = "time.stdtime.gov.tw";
//static const char ntpserver5[] = "tock.stdtime.gov.tw";
//static const char ntpserver6[] = "clock.stdtime.gov.tw";
//static const char ntpserver7[] = "jp.pool.ntp.org";


//const char *ntpserver_array[] = {ntpserver0, ntpserver1, ntpserver2, ntpserver3, ntpserver4, 
//								ntpserver5, ntpserver6, ntpserver7, ntpserver8, NULL};
const char *ntpserver_array[] = {ntpserver0, ntpserver1, ntpserver2, ntpserver3, ntpserver4, ntpserver5, ntpserver6, ntpserver7, ntpserver8, NULL};
#else
const char* NTPClient::NIST_SERVER_ADDRESS = "pool.ntp.org";
#endif
const int NTPClient::NIST_SERVER_PORT = 123;

NTPClient::NTPClient(NetworkInterface *iface) {
    this->iface = iface;    
}

time_t NTPClient::get_timestamp(int timeout) {
    const time_t TIME1970 = 2208988800L;
    int ntp_send_values[12] = {0};
    int ntp_recv_values[12] = {0};

    SocketAddress nist;

	#ifdef _MULTI_NTP_SERVER // test
	int idx = 0;
	int ret_gethostbyname;
	int ret;

	printf("\n==============================\n");
	while(1)
	{
		if(ntpserver_array[idx] == NULL)
		{
			printf("no more ntp server is ok, return\n");
			return ret;
		}

		printf("----- %d\n", idx);
    	printf("gethostbyname: %s\n", ntpserver_array[idx]);
    	ret_gethostbyname = iface->gethostbyname(ntpserver_array[idx], &nist);
    	if (ret_gethostbyname < 0) {
			printf("gethostbyname fail: %d, try next ntp sever\n", ret_gethostbyname);
			ret = ret_gethostbyname;
    	    idx++;
			continue;
    	}
	    
	    nist.set_port(NIST_SERVER_PORT);
	    
	    memset(ntp_send_values, 0x00, sizeof(ntp_send_values));
	    ntp_send_values[0] = '\x1b';

	    memset(ntp_recv_values, 0x00, sizeof(ntp_recv_values));
	    
	    UDPSocket sock;
	    sock.open(iface);
	    sock.set_timeout(timeout);

	    int ret_send = sock.sendto(nist, (void*)ntp_send_values, sizeof(ntp_send_values));

	    SocketAddress source;
	    const int n = sock.recvfrom(&source, (void*)ntp_recv_values, sizeof(ntp_recv_values));
	    
	    if (n > 10) {
	        return ntohl(ntp_recv_values[10]) - TIME1970;
	    } else {
	    	printf("fail to get time, try next ntp sever\n");
	        if (n < 0) {
	            // Network error
	            ret = n;
	        } else {
	            // No or partial data returned
	            ret = -1;
	        }
	        idx++;
			continue;
	    }
	}

	#else // #ifdef _MULTI_NTP_SERVER // test
	
	printf("gethostbyname: %s\n", NIST_SERVER_ADDRESS);
	int ret_gethostbyname = iface->gethostbyname(NIST_SERVER_ADDRESS, &nist);
	if (ret_gethostbyname < 0) {
		// Network error on DNS lookup
		return ret_gethostbyname;
	}
	
	nist.set_port(NIST_SERVER_PORT);
	
	memset(ntp_send_values, 0x00, sizeof(ntp_send_values));
	ntp_send_values[0] = '\x1b';
	
	memset(ntp_recv_values, 0x00, sizeof(ntp_recv_values));
	
	UDPSocket sock;
	sock.open(iface);
	sock.set_timeout(timeout);
	
	int ret_send = sock.sendto(nist, (void*)ntp_send_values, sizeof(ntp_send_values));
	
	SocketAddress source;
	const int n = sock.recvfrom(&source, (void*)ntp_recv_values, sizeof(ntp_recv_values));
	
	if (n > 10) {
		return ntohl(ntp_recv_values[10]) - TIME1970;
	} else {
		if (n < 0) {
			// Network error
			return n;
		} else {
			// No or partial data returned
			return -1;
		}
	}

	#endif
}

uint32_t NTPClient::ntohl(uint32_t x) {
    uint32_t ret = (x & 0xff) << 24;
    ret |= (x & 0xff00) << 8;
    ret |= (x & 0xff0000UL) >> 8;
    ret |= (x & 0xff000000UL) >> 24;
    return ret;
}
