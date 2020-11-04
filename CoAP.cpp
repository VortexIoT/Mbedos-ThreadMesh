/*
  Still working on this protocol.

   */




#include "CoAP.h"
#include "stdlib.h"
#include "stdio.h"
//#include <cstring>
#include <cstdint>
//#include <cstdlib>
#include <string>
#include "mbed.h"
#include "ip6string.h"

UDPSocket udpsock;
extern MeshInterface *mesh;
uint8_t UDP_WR_RD(char *hostdomain, uint16_t port,uint8_t *msg, uint16_t msglen)
{
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    udpsock.open(interface);
    udpsock.bind(port);
  //  interface->gethostbyname(hostdomain,&addr);//,NSAPI_IPv6,NULL);
    addr.set_port(port);
    //udpsock.bind(addr);
     int scount = udpsock.sendto(addr, msg, msglen);
    printf("Sent %d bytes on UDP\n", scount);

    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    printf("posr: %s  %d\n",addr.get_ip_address(),addr.get_port());
    nsapi_size_or_error_t ret = udpsock.recvfrom(&addr, recv_buffer, 1280);
    printf("ret:%d\n",ret);
    printf("%s\n", recv_buffer);
     udpsock.close();
    return ret;
}