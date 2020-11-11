/*
  Still working on this protocol.

   */
// Write seperate function for UDP 

//Form different functions for coap client commands post, put, get, delete 



#include "coap_protocol.h"
#include "stdlib.h"
#include "stdio.h"
#include <cstdint>
#include <string>
#include "mbed.h"
#include "ip6string.h"



#define buff_str1    "fd00:db8:0:0:0:ff:fe00:c000"

#define buff_str    "fd00:db8:0:0:92cf:7276:ce01:3271"
using namespace std;
UDPSocket udpsock;           // Socket to talk CoAP over
struct coap_s* coapHandle;//=sn_coap_protocol_init(&coap_malloc, &coap_free, &coap_tx_cb, &coap_rx_cb);
coap_version_e coapVersion = COAP_VERSION_1;
//extern SocketAddress sockAddr;
extern MeshInterface *mesh;
uint8_t destbuff[16] = {0};
//coapHandle = 

void* coap_malloc(uint16_t size) {
    return malloc(size);
}

void coap_free(void* addr) {
    free(addr);
}

// tx_cb and rx_cb are not used in this program
uint8_t coap_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d) {
    printf("coap tx cb\n");
    return 0;
}

int8_t coap_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c) {
    printf("coap rx cb\n");
    return 0;
}


void coap_init(uint8_t msg_Code, uint8_t msg_type,uint8_t *payload, uint16_t payload_len)
{
    uint8_t *packet;
    uint8_t packet_len;
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    udpsock.open(interface);
    udpsock.bind(5683); //coap dedicated port
    coapHandle = sn_coap_protocol_init(&coap_malloc, &coap_free, &coap_tx_cb, &coap_rx_cb);
    stoip6(buff_str, sizeof(buff_str),destbuff);
    SocketAddress addr1(destbuff,NSAPI_IPv6,5683);
    packet_len = Packet_builder(addr1,packet,msg_Code,msg_type,payload,payload_len);
}

uint8_t Packet_builder(SocketAddress addr1,uint8_t *packet,uint8_t msg_code,uint8_t msg_type,uint8_t *payload, uint16_t payload_len)
{
     uint8_t packet_len;
     const char* coap_uri_path = "/test";
    sn_coap_content_format_ msgcode;
    
    sn_coap_hdr_s *coap_res_ptr = (sn_coap_hdr_s*)calloc(sizeof(sn_coap_hdr_s), 1);
    coap_res_ptr->uri_path_ptr = (uint8_t*)coap_uri_path;       // Path
    coap_res_ptr->uri_path_len = strlen(coap_uri_path);
    switch(msg_code)
    {
        case 1:coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_GET;         // CoAP method
            printf("get\n"); //test purpose
        break;
        case 2:coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_POST;
            printf("post\n");//test purpose
        break;
        case 3:coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_PUT;
            printf("put\n");//test purpose
        break;
        case 4:coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_DELETE;
            printf("delete\n");//test purpose
        break;
        default:
        break;
    }   
    coap_res_ptr->content_format = COAP_CT_TEXT_PLAIN;          // CoAP content type
    coap_res_ptr->payload_len = payload_len;                              // Body length
    coap_res_ptr->payload_ptr = payload;                              // Body pointer
    coap_res_ptr->options_list_ptr = 0;                         // Optional: options list
    coap_res_ptr->msg_type = COAP_MSG_TYPE_CONFIRMABLE;//sn_coap_msg_type_e(msg_type);
    // Message ID is used to track request->response patterns, because we're using UDP (so everything is unconfirmed).
    // See the receive code to verify that we get the same message ID back
    coap_res_ptr->msg_id = 7;
    // Calculate the CoAP message size, allocate the memory and build the message
    packet_len = sn_coap_builder_calc_needed_packet_data_size(coap_res_ptr);
    printf("Calculated message length: %d bytes\n", packet_len);
     packet = (uint8_t*)malloc(packet_len);
    sn_coap_builder(packet, coap_res_ptr); //build structure into packets
    int scount = udpsock.sendto(addr1, packet, packet_len); //sending through UDP to the server
    printf("Sent %d bytes on UDP\n", scount);
    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    nsapi_size_or_error_t ret = udpsock.recvfrom(&addr1, recv_buffer, 1280); //reading back from server
    Packet_parser(ret,recv_buffer,coap_res_ptr->msg_code); //parsing received buffer data
  //  free(recv_buffer);
 //   free(coap_res_ptr);
 //     udpsock.close();
    return ret; //may need to future implemetations
    
}
void Packet_parser(uint8_t ret,uint8_t *receive_buff_data,uint8_t method)
{
    if (ret > 0) {

        printf("Received a message of length '%d'\n", ret);
        sn_coap_hdr_s* parsed = sn_coap_parser(coapHandle, ret, receive_buff_data, &coapVersion);
        // payload is a string
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);
        printf("\tmsg_id:   %d\n", parsed->msg_id);
        if(parsed->msg_code) // to print 2.xx,4.xx format responses
        {
            int decimal = parsed->msg_code/32;
            int fraction = parsed->msg_code%32;
            printf("msg_code: %d.0%d\n",decimal,fraction);
        }
        if(method == 1) //if get method then only print payload and length of payload
        {
                printf("payload_len:  %d\n", parsed->payload_len);
                printf("payload:  %s\n", payload.c_str());
        }
        if(parsed->msg_type == 0x20)    // coap msg type ack
            printf("Response : ACK\n");
     //   printf("\toptions_list_ptr: %p\n", parsed->options_list_ptr);
        if(parsed->options_list_ptr)
        {
            printf("location_path_ptr: %s\n", parsed->options_list_ptr->location_path_ptr);
            printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
            printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
        }
        
    }
    else {
        printf("Failed to receive message (%d)\n", ret);
    }
    free(receive_buff_data);
}



//This function implemented for test purpose not neccessary to keep it final code 
uint8_t getfunction(void)
{
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    udpsock.open(interface);
    udpsock.bind(5683);
     // Initialize the CoAP protocol handle, pointing to local implementations on malloc/free/tx/rx functions
    coapHandle=sn_coap_protocol_init(&coap_malloc, &coap_free, &coap_tx_cb, &coap_rx_cb);
    const char* coap_uri_path = "/test";
    sn_coap_content_format_ msgcode;
    // See ns_coap_header.h
    sn_coap_hdr_s *coap_res_ptr = (sn_coap_hdr_s*)calloc(sizeof(sn_coap_hdr_s), 1);
    coap_res_ptr->uri_path_ptr = (uint8_t*)coap_uri_path;       // Path
    coap_res_ptr->uri_path_len = strlen(coap_uri_path);
    coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_GET;         // CoAP method
    coap_res_ptr->content_format = COAP_CT_TEXT_PLAIN;          // CoAP content type
    coap_res_ptr->payload_len = 0;                              // payload length
    coap_res_ptr->payload_ptr = 0;                              // payload pointer
    coap_res_ptr->options_list_ptr = 0;                         // Optional: options list
    coap_res_ptr->msg_type = COAP_MSG_TYPE_CONFIRMABLE;
    // Message ID is used to track request->response patterns, because we're using UDP (so everything is unconfirmed).
    // See the receive code to verify that we get the same message ID back
    coap_res_ptr->msg_id = 7;
    // Calculate the CoAP message size, allocate the memory and build the message
    uint16_t message_len = sn_coap_builder_calc_needed_packet_data_size(coap_res_ptr);
    printf("Calculated message length: %d bytes\n", message_len);
    uint8_t* message_ptr = (uint8_t*)malloc(message_len);
    sn_coap_builder(message_ptr, coap_res_ptr);

    stoip6(buff_str, sizeof(buff_str),destbuff);
    SocketAddress addr1(destbuff, NSAPI_IPv6, 5683);
    int scount1 = udpsock.sendto(addr1, message_ptr, message_len);
    printf("Sent %d bytes on UDP\n", scount1);

    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    printf("posr: %s  %d\n",addr1.get_ip_address(),addr1.get_port());
    nsapi_size_or_error_t ret = udpsock.recvfrom(&addr1, recv_buffer, 1280);

    if (ret > 0) 
    {
        printf("Received a message of length '%d'\n", ret);
        sn_coap_hdr_s* parsed = sn_coap_parser(coapHandle, ret, recv_buffer, &coapVersion);

        // We know the payload is going to be a string
       // string payload((const char*)parsed->payload_ptr, parsed->payload_len);
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);

        printf("msg_id:   %d\n", parsed->msg_id);
        printf("msg_code: %d\n", parsed->msg_code);
        printf("content_format:   %d\n", parsed->content_format);
        printf("payload_len:  %d\n", parsed->payload_len);
        printf("payload:  %s\n", payload.c_str());
        if(parsed->msg_type == 0x20)
            printf("ACK\n");
        printf("\toptions_list_ptr: %p\n", parsed->options_list_ptr);
        if(parsed->options_list_ptr)
        {
            printf("location_path_ptr: %p\n", parsed->options_list_ptr->location_path_ptr);
            printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
            printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
        }
        
    }
    else {
        printf("Failed to receive message (%d)\n", ret);
    }
    free(recv_buffer);
    free(coap_res_ptr);
    free(message_ptr);
 //   udpsock.close();
    return ret;
}


