//#include "mbed.h"
#include "vmn_coap_client.h"
#include "ip6string.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "cli_cmd.h"
#include "ThreadInterface.h"
#include "InternetSocket.h"
#include "InternetDatagramSocket.h"


/*

#define SN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE  --------sn_config.h
#define DEFAULT_RESPONSE_TIMEOUT                        10  //< Default re-sending timeout as seconds /

*/

UDPSocket coap_client_udpsock;           // Socket to talk CoAP over
struct coap_s* coaphandle;//=sn_coap_protocol_init(&coap_client_malloc, &coap_client_free, &coap_client_tx_cb, &coap_client_rx_cb);
coap_version_e coap_version = COAP_VERSION_1;
uint8_t ipdestbuffer[16] = {0};
EventQueue coapclient_eventqueue;
uint32_t payload_length = 0;
extern nwk_interface_id id;
uint8_t *coap_payload = 0;
uint8_t *sendpacket;
uint8_t event_cancelhandle;
uint8_t retransmit_count=0;
uint16_t packet_len;
void* coap_client_malloc(uint16_t size) {
    return malloc(size);
}

void coap_client_free(void* addr) {
    free(addr);
}
// tx_cb and rx_cb are not used in this program
uint8_t coap_client_tx_cb( uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d ) {
    printf("coap tx cb\n");
    return 0;
}

int8_t coap_client_rx_cb( sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c ) {
    printf("coap rx cb\n");
    return 0;
}

void client_requestpacket_build( char *host_address, uint8_t *uri_path, uint8_t msg_code, uint8_t msg_type, uint8_t *payload, uint16_t payload_len ) {//uint8_t *packet, this about this later
    sn_coap_content_format_ msgcode;
    sn_coap_options_list_s *options;
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    coap_client_udpsock.open(interface); //open udp on mesh network
    coap_client_udpsock.bind(5683); //binding udp to coap
    coaphandle = sn_coap_protocol_init(&coap_client_malloc, &coap_client_free, &coap_client_tx_cb, &coap_client_rx_cb); //creating handler to access the packets
    stoip6(host_address, strlen(host_address),ipdestbuffer);
    SocketAddress sendaddr(ipdestbuffer, NSAPI_IPv6, 5683);
    sn_coap_hdr_s *coap_res_ptr = (sn_coap_hdr_s*)calloc(sizeof(sn_coap_hdr_s), 1); //allocate memory for packet
    coap_res_ptr->uri_path_ptr = uri_path;       // resource path
    coap_res_ptr->uri_path_len = strlen((char *)uri_path);
   // printf("uripath: %s\n",uri_path); 
    switch(msg_code) { //checking coap request method entered through terminal to form a packet
        case 1:
            coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_GET;         // if CoAP get method
           // printf("get\n"); //test purpose
        break;
        case 2:
            coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_POST;
            //printf("post\n");//test purpose
        break;
        case 3:
            coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_PUT;
           // printf("put\n");//test purpose
        break;
        case 4:
            coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_DELETE;
           // printf("delete\n");//test purpose
        break;
        default:
        break;
    }   
    coap_res_ptr->content_format = COAP_CT_TEXT_PLAIN;          // CoAP content type
    coap_res_ptr->payload_len = payload_len;                    // Payload length
    coap_res_ptr->payload_ptr = payload;                        // Payload pointer
    coap_res_ptr->options_list_ptr->block1 = options->block1;                         // Optional: options list
    coap_res_ptr->msg_type = COAP_MSG_TYPE_CONFIRMABLE;//sn_coap_msg_type_e(msg_type);
    // Message ID is used to track request->response patterns, because we're using UDP (so everything is unconfirmed).
    // See the receive code to verify that we get the same message ID back
    coap_res_ptr->msg_id = rand();
    // Calculate the CoAP message size, allocate the memory and build the message
    packet_len = sn_coap_builder_calc_needed_packet_data_size(coap_res_ptr);
    printf("Calculated message length: %d bytes\n", packet_len);
    sendpacket = (uint8_t*)malloc(packet_len);
    sn_coap_builder(sendpacket, coap_res_ptr); //build structure into packets
    int scount = coap_client_udpsock.sendto(sendaddr, sendpacket, packet_len); //sending through UDP to the server
    printf("Sent %d bytes on UDP\n", scount); //for user information
    event_cancelhandle = coapclient_eventqueue.call(client_responsepacket_parse, sendaddr, coap_res_ptr->msg_code);
    coapclient_eventqueue.cancel(event_cancelhandle);
}

uint8_t udp_socketopen_timeout=10;
void client_responsepacket_parse(SocketAddress receive_addr, uint8_t requestmethod) {
    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    coap_client_udpsock.set_timeout(10);
    nsapi_size_or_error_t return_packet_len = coap_client_udpsock.recvfrom(&receive_addr, recv_buffer, 1280); //reading back from server
    if (return_packet_len > 0) {
        printf("Received a message of length '%d'\n", return_packet_len);
        sn_coap_hdr_s* parsed = sn_coap_parser(coaphandle, return_packet_len, recv_buffer, &coap_version);
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);// payload is a string
        std::string uri((const char*)parsed->uri_path_ptr, parsed->uri_path_len);// payload is a string
        printf("uripath: %s\n",(const char*)parsed->uri_path_ptr);
        printf("msg_id: %d\n", parsed->msg_id);
        response_message_code(parsed->msg_code);
        //here look for content format
        /* ##########  Check here whether it's needed or not after the call ~~~~~~~~~~~~~~*/  
        if (requestmethod == 1) {   //if get method then only print payload and length of payload
            uint8_t payload_value;
            printf("payload_len : %d\n", parsed->payload_len);
            payload_value = atoi( payload.c_str());
            if (strcmp(uri.c_str(),"/sensor/temp/value") == 0) {  //"/sensor/temp/scaling"  strlen(uri.c_str()) , parsed->uri_path_len
               printf("Temperature : %d\n",payload_value); 
            } else if (strcmp(uri.c_str(), "/sensor/hum/value") == 0) { //"/sensor/hum/scaling" strlen(uri.c_str())
                printf("Humidity : %d\n",payload_value); 
            }
            else {
                coap_payload = (uint8_t *)payload.c_str();
                payload_length = strlen((char *)coap_payload);
                printf("payload: %s\n", coap_payload);
            }        
        }
        if (parsed->msg_type == 0x20) {   // coap msg type ack
            printf("Response : ACK \n");
        } else if(parsed->msg_type == 0x30) {
            printf("Response : Reset Message \n");
        }
        //check is there any options in the response packet
        if (parsed->options_list_ptr) {
            printf("block1: %d\n", parsed->options_list_ptr->block1);
            printf("block2: %d\n", parsed->options_list_ptr->block2);
            printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
        }
       // printf("uri-path : %s\n", uri.c_str());
    } else {
        sn_coap_protocol_set_retransmission_parameters(coaphandle, 5, udp_socketopen_timeout);  //resending count 5 and intervel for every 5sec
        
        if(retransmit_count < 3) {

         //   sn_coap_protocol_set_retransmission_buffer(coaphandle, 6, packet_len); //changes default message retransmission queue size
            int scount = coap_client_udpsock.sendto(receive_addr, sendpacket, packet_len); //sending through UDP to the server
            printf("Sent %d bytes on UDP\n", scount); //for user information
            event_cancelhandle = coapclient_eventqueue.call(client_responsepacket_parse, receive_addr, requestmethod);
          //  coapclient_eventqueue.cancel(event_cancelhandle);
         //   printf("Failed to receive message (%d)\n", return_packet_len);
            retransmit_count++;
            udp_socketopen_timeout += 15;
        } else {
            retransmit_count = 0;
            udp_socketopen_timeout = 10;
            coapclient_eventqueue.cancel(event_cancelhandle);
        printf("Response Timeout\n");
        }
    }
    free(recv_buffer);
}

//This function to identify the type response message code
void response_message_code(uint8_t msg_code) {
    int decimal=0, fraction=0;
    if (msg_code) {// to print 2.xx,4.xx format responses
        decimal = msg_code/32;
        fraction = msg_code%32;
    }
    switch(msg_code) {
        case 65:  printf("%d.0%d  \"created\"\n",decimal,fraction);
        break;
        case 66:  printf("%d.0%d  \"deleted\"\n",decimal,fraction);
        break;
        case 67:  printf("%d.0%d  \"valid\"\n",decimal,fraction);
        break;
        case 68:  printf("%d.0%d  \"changed\"\n",decimal,fraction);
        break;
        case 69:  printf("%d.0%d  \"content\"\n",decimal,fraction);
        break;
        case 95:  printf("%d.0%d  \"continue\"\n",decimal,fraction);
        break;
        default:
        break;
    }
}

//dpd is the payload of the coap message packet

//the Header includes 2 SYNC bytes, 1 Class byte, 2 ID bytes and 2 Length of the payload, payload of payload length bytes and check sum
//now I am not handling payload at the moment so this function checks for other information present in coap payload


