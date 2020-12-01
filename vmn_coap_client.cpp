#include "mbed.h"
#include "vmn_coap_client.h"
#include "ip6string.h"
#include "stdlib.h"
#include "stdio.h"
#include <cstdint>
#include <cstdlib>
#include <string>

UDPSocket coap_client_udpsock;           // Socket to talk CoAP over
struct coap_s* coaphandle;//=sn_coap_protocol_init(&coap_client_malloc, &coap_client_free, &coap_client_tx_cb, &coap_client_rx_cb);
coap_version_e coap_version = COAP_VERSION_1;
uint8_t ipdestbuffer[16] = {0};
EventQueue coapclient_eventqueue;
uint8_t requested_temp_scaling = 1;
uint8_t requested_hum_scaling = 1;

void* coap_client_malloc(uint16_t size) {
    return malloc(size);
}

void coap_client_free(void* addr) {
    free(addr);
}
// tx_cb and rx_cb are not used in this program
uint8_t coap_client_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d) {
    printf("coap tx cb\n");
    return 0;
}

int8_t coap_client_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c) {
    printf("coap rx cb\n");
    return 0;
}

//    char constantstring[] = "/server/advertise";
//    char constantstring1[] = "server/advertise";
void client_requestpacket_build(char *host_address, uint8_t *uri_path, uint8_t msg_code,uint8_t msg_type,uint8_t *payload, uint16_t payload_len) //uint8_t *packet, this about this later
{
 //   uint8_t packet_len;
  //  uint8_t *packet;
    uint8_t handle;
    sn_coap_content_format_ msgcode;
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    coap_client_udpsock.open(interface);
    coap_client_udpsock.bind(5683); //coap dedicated port
    coaphandle = sn_coap_protocol_init(&coap_client_malloc, &coap_client_free, &coap_client_tx_cb, &coap_client_rx_cb);
  //  strcat(coap_uri_path, (char *)uri_path);
    stoip6(host_address, strlen(host_address),ipdestbuffer);
    SocketAddress addr1(ipdestbuffer,NSAPI_IPv6,5683);
    sn_coap_hdr_s *coap_res_ptr = (sn_coap_hdr_s*)calloc(sizeof(sn_coap_hdr_s), 1);
    coap_res_ptr->uri_path_ptr = uri_path;       // Path
    coap_res_ptr->uri_path_len = strlen((char *)uri_path);
     printf("uripath:%s",uri_path);
  //  printf("hostadr:%s  %d\n",host_address,strlen(host_address));
    switch(msg_code) {
        case 1:coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_GET;         // CoAP method
            printf("get\n"); //test purpose
        break;
        case 2:coap_res_ptr->msg_code = COAP_MSG_CODE_REQUEST_POST;
        if (strncmp((char *)coap_res_ptr->uri_path_ptr, "/sensor/temp/scaling", coap_res_ptr->uri_path_len) == 0) {//"/sensor/temp/scaling")) { // == 0 
            requested_temp_scaling = atoi((char *)payload);
        }else if (strncmp((char *)coap_res_ptr->uri_path_ptr, "/sensor/hum/scaling", coap_res_ptr->uri_path_len)==0) {
            requested_hum_scaling = atoi((char *)payload);
        }
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
    uint8_t packet_len = sn_coap_builder_calc_needed_packet_data_size(coap_res_ptr);
    printf("Calculated message length: %d bytes\n", packet_len);
    uint8_t *packet = (uint8_t*)malloc(packet_len);
    sn_coap_builder(packet, coap_res_ptr); //build structure into packets
    int scount = coap_client_udpsock.sendto(addr1, packet, packet_len); //sending through UDP to the server
    printf("Sent %d bytes on UDP\n", scount);
    handle = coapclient_eventqueue.call(client_responsepacket_parse,addr1,coap_res_ptr->msg_code);
    coapclient_eventqueue.cancel(handle);
}

void client_responsepacket_parse(SocketAddress addr, uint8_t requestmethod)
{
    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    nsapi_size_or_error_t ret = coap_client_udpsock.recvfrom(&addr, recv_buffer, 1280); //reading back from server
//   Packet_parser(ret,recv_buffer);//,coap_res_ptr->msg_code); //parsing received buffer data
    if (ret > 0) {
        printf("Received a message of length '%d'\n", ret);
        sn_coap_hdr_s* parsed = sn_coap_parser(coaphandle, ret, recv_buffer, &coap_version);
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);// payload is a string
        std::string uri((const char*)parsed->uri_path_ptr, parsed->uri_path_len);// payload is a string
       // std::uint8_t data_payload((const char*)parsed->payload_ptr);
      //  sprintf
        printf("uripath:%s \n",uri.c_str());
        printf("msg_id:   %d\n", parsed->msg_id);
        message_code_responses(parsed->msg_code);
    /*    if (parsed->msg_code) {// to print 2.xx,4.xx format responses
            int decimal = parsed->msg_code/32;
            int fraction = parsed->msg_code%32;
            printf("msg_code: %d.0%d\n",decimal,fraction);
        }*/
        //here look for content format
        if (requestmethod == 1) {//if get method then only print payload and length of payload
           //  uint8_t payload_value[2];
            printf("payload_len:  %d\n", parsed->payload_len);
            printf("payload:  %s\n",  payload.c_str());
            uint8_t payload_value;
            payload_value = atoi( payload.c_str());
            if (strncmp(uri.c_str(),"/sensor/temp/value" ,strlen(uri.c_str()))==0) {  //"/sensor/temp/scaling"
               payload_value = payload_value * requested_temp_scaling;
               printf("RTS: %d   %d", requested_temp_scaling, requested_hum_scaling);
                
            }else if (strncmp(uri.c_str(), "/sensor/hum/value",strlen(uri.c_str()))==0) { //"/sensor/hum/scaling"
             //   sprintf(payload_value, "%d", payload.c_str());
              //  payload_value = (uint8_t)payload.data();
                payload_value =   payload_value * requested_hum_scaling;
            //    printf("payload_len:  %d\n", parsed->payload_len);
            //    printf("payload:  %d\n", payload_value); // payload.c_str());
             }
             printf("payload after change : %d \n",payload_value);          
        }
        if (parsed->msg_type == 0x20) {   // coap msg type ack
            printf("Response : ACK \n");
        } else if(parsed->msg_type == 0x30) {
            printf("Response : Reset Message \n");
        }
     //   printf("\toptions_list_ptr: %p\n", parsed->options_list_ptr);
        if (parsed->options_list_ptr) {
            printf("location_path_ptr: %s\n", parsed->options_list_ptr->location_path_ptr);
            printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
            printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
        }
        printf("uri-path : %s\n", uri.c_str());
        
    } else {
        printf("Failed to receive message (%d)\n", ret);
    }
    free(recv_buffer);
  
    
   // uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
   // nsapi_size_or_error_t ret = coap_client_udpsock.recvfrom(&addr1, recv_buffer, 1280); //reading back from server
  //  Packet_parser(ret,recv_buffer,coap_res_ptr->msg_code); //parsing received buffer data
  //  free(recv_buffer);
 //   free(coap_res_ptr);
 //   coap_client_udpsock.close();
 //   return 1; //may need to future implemetations
    
}
void packet_structure(uint8_t resourcefile_pointer)
{
    
    switch(resourcefile_pointer) {
        case 1:
        break;
        case 2:
        break;
        case 3:
        break;
        default:
        break;
    }
}
//This function to identify the type response message code
void message_code_responses(uint8_t msg_code)
{
    int decimal=0, fraction=0;
    if (msg_code) {// to print 2.xx,4.xx format responses
        decimal = msg_code/32;
        fraction = msg_code%32;
      //  printf("msg_code: %d.0%d\n",decimal,fraction);
    }
    switch(msg_code)
    {
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

/*
void Packet_parser(uint8_t ret,uint8_t *receive_buff_data)//,uint8_t method)
{
    SocketAddress addr1;
   //    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
  //  nsapi_size_or_error_t ret1 = coap_client_udpsock.recvfrom(&addr1, receive_buff_data, 1280); //reading back from server
    if (ret > 0) {
        printf("Received a message of length '%d'\n", ret);
        sn_coap_hdr_s* parsed = sn_coap_parser(coaphandle, ret, receive_buff_data, &coap_version);
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);// payload is a string
        printf("\tmsg_id:   %d\n", parsed->msg_id);
        if (parsed->msg_code) {// to print 2.xx,4.xx format responses
            int decimal = parsed->msg_code/32;
            int fraction = parsed->msg_code%32;
            printf("msg_code: %d.0%d\n",decimal,fraction);
        }
        if (method == 1) {//if get method then only print payload and length of payload
                printf("payload_len:  %d\n", parsed->payload_len);
                printf("payload:  %s\n", payload.c_str());
        }
        if (parsed->msg_type == 0x20)    // coap msg type ack
            printf("Response : ACK\n");
     //   printf("\toptions_list_ptr: %p\n", parsed->options_list_ptr);
        if (parsed->options_list_ptr) {
            printf("location_path_ptr: %s\n", parsed->options_list_ptr->location_path_ptr);
            printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
            printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
        }
        
    } else {
        printf("Failed to receive message (%d)\n", ret);
    }
    free(receive_buff_data);
}

*/

//This function implemented for test purpose not neccessary to keep it final code 
uint8_t getfunction(void)
{
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    coap_client_udpsock.open(interface);
    coap_client_udpsock.bind(5683);
     // Initialize the CoAP protocol handle, pointing to local implementations on malloc/free/tx/rx functions
    coaphandle=sn_coap_protocol_init(&coap_client_malloc, &coap_client_free, &coap_client_tx_cb, &coap_client_rx_cb);
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

    stoip6(IPADDRESS, sizeof(IPADDRESS),ipdestbuffer);
    SocketAddress addr1(ipdestbuffer, NSAPI_IPv6, 5683);
    int scount1 = coap_client_udpsock.sendto(addr1, message_ptr, message_len);
    printf("Sent %d bytes on UDP\n", scount1);

    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    printf("posr: %s  %d\n",addr1.get_ip_address(),addr1.get_port());
    nsapi_size_or_error_t ret = coap_client_udpsock.recvfrom(&addr1, recv_buffer, 1280);

    if (ret > 0) 
    {
        printf("Received a message of length '%d'\n", ret);
        sn_coap_hdr_s* parsed = sn_coap_parser(coaphandle, ret, recv_buffer, &coap_version);

        // We know the payload is going to be a string
       // string payload((const char*)parsed->payload_ptr, parsed->payload_len);
        std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);

        printf("msg_id:   %d\n", parsed->msg_id);
        printf("msg_code: %d\n", parsed->msg_code);
        printf("content_format:   %d\n", parsed->content_format);
        printf("payload_len:  %d\n", parsed->payload_len);
        printf("payload:  %s\n", payload.c_str());
        if (parsed->msg_type == 0x20)
            printf("ACK\n");
        printf("\toptions_list_ptr: %p\n", parsed->options_list_ptr);
        if (parsed->options_list_ptr) {
            printf("location_path_ptr: %p\n", parsed->options_list_ptr->location_path_ptr);
            printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
            printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
        }
        
    } else {
        printf("Failed to receive message (%d)\n", ret);
    }
    free(recv_buffer);
    free(coap_res_ptr);
    free(message_ptr);
 //   coap_client_udpsock.close();
    return ret;
}
