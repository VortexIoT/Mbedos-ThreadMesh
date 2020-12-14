/*
  Still work needs be impemente on this protocol. At the moment It's just implemented for test purpose 

//not handling token ptr 

   */

#include "mbed.h"
#include "stdlib.h"
#include "stdio.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include "ip6string.h"
#include <cstring>
#include "vmn_coap_server.h"


//#define IPADDRESS2    "fd00:db8:0:0:0:ff:fe00:2000"
//#define IPADDRESS1    "fd00:db8:0:0:2e5e:ccdc:af4:87e3"//"fd00:db8:0:0:92cf:7276:ce01:3271" dongle cli
//#define IPADDRESS     "fd00:db8:0:0:a58a:2805:667e:9cef" //gateway

EventQueue coapserver_eventqueue;
UDPSocket udpsock;           // Socket to talk CoAP over
struct coap_s* coapHandle;//=sn_coap_protocol_init(&coap_malloc, &coap_free, &coap_tx_cb, &coap_rx_cb);
coap_version_e coapVersion = COAP_VERSION_1;
uint8_t ipaddress_buffer[16] = {0};
uint16_t coapserver_eventqueue_handle=0;
uint16_t portnumber;
static uint8_t requested_temp_scaling = 1;
static uint8_t requested_hum_scaling = 1;
static uint8_t coapserver_state_control = 1;

DigitalOut led(LED1); //for green led

void* coap_server_malloc(uint16_t size) {
    return malloc(size);
}

void coap_server_free(void* addr) {
    free(addr);
}

// tx_cb and rx_cb are not used in this program
uint8_t coap_server_tx_cb(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d) {
    printf("coap tx cb\n");
    return 0;
}

int8_t coap_server_rx_cb(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c) {
    printf("coap rx cb\n");
    return 0;
}

//This method initializes mbed board as a server
void coap_server_init(void) { 
    uint8_t *packet;
    uint8_t packet_len;
    SocketAddress addr;
    NetworkInterface * interface = (NetworkInterface *)mesh;
    interface->get_ip_address(&addr);
    udpsock.open(interface);
    udpsock.bind(5683); //coap dedicated port
 //   coapHandle = sn_coap_protocol_init(&coap_server_malloc, &coap_server_free, &coap_server_tx_cb, &coap_server_rx_cb);
  //  stoip6(ipaddr, sizeof(ipaddr),ipaddress_buffer);
 //   SocketAddress addr1(ipaddress_buffer,NSAPI_IPv6,5683);
    coapserver_eventqueue.call(receive_msg); //Thread calls this conitnuously without interrupting other 
}
//This method handles the received message from the client
//parses the packet and prints the information passed by client
//depends on resorces it perform actions
void receive_msg(void){
    SocketAddress addr;
    sn_coap_hdr_s* parsed;
    uint8_t* recv_buffer = (uint8_t*)malloc(1280); // Suggested is to keep packet size under 1280 bytes
    nsapi_size_or_error_t recv_return_data_len = udpsock.recvfrom(&addr, recv_buffer, 1280); //reading back from server
    coapHandle = sn_coap_protocol_init(&coap_server_malloc, &coap_server_free, &coap_server_tx_cb, &coap_server_rx_cb);
    portnumber = addr.get_port();
    printf("IP : %s    Port: %d\n", addr.get_ip_address(), portnumber);
    if (recv_return_data_len > 0) {
        parsed = sn_coap_parser(coapHandle, recv_return_data_len, recv_buffer, &coapVersion);
        if(parsed->msg_code != COAP_MSG_CODE_EMPTY) {
            std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);// payload is a string
            parsed->uri_path_ptr[parsed->uri_path_len] = '\0';
            std::string uri_path((const char *)parsed->uri_path_ptr,parsed->uri_path_len);  // uri is a string
            std::string token((const char*)parsed->token_ptr, parsed->token_len);   // token is a string
            //packet information 
            printf("msg_id: %d\n", parsed->msg_id);
            requested_message_code(parsed->msg_code); //here, get,post,put or delete are the message codes.
            requested_message_type(parsed->msg_type); //con or non
            printf("Uri-path: %s\n",parsed->uri_path_ptr);
         //   printf("token-ID: %s\n",parsed->token_ptr);
            if (parsed->options_list_ptr) {
                printf("location_path_ptr: %s\n", parsed->options_list_ptr->location_path_ptr);
                printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
                printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
            }
            //hadnling request from here
            if (parsed->msg_code == COAP_MSG_CODE_REQUEST_POST) {
                uint8_t posted_payload = atoi(payload.c_str());
                printf("payload_len: %d\n", parsed->payload_len);
                printf("payload: %d\n", posted_payload);
                if (strncmp(uri_path.c_str(), "sensor/temp/scaling", parsed->uri_path_len) == 0) { //checking for uri path
                //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 ,so scaling must be 1 to 5 if possible
                    if(posted_payload > 0 && posted_payload <= 5) //setting the range of scaling factor
                        requested_temp_scaling = posted_payload;
                } else if (strncmp(uri_path.c_str(), "sensor/hum/scaling", parsed->uri_path_len) == 0) {  //checking for uri path
                //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 
                    if(posted_payload > 0 && posted_payload <= 5)
                        requested_hum_scaling = posted_payload;
                } else if (strncmp(uri_path.c_str(), "control/state", parsed->uri_path_len) == 0) {           //this to control the state of led as well as server responses   
                    if((posted_payload == 0) || (posted_payload == 1)) {
                        coapserver_state_control = posted_payload;  //changing the state byte here this will be handle on server side
                        led = posted_payload;
                    }       
                }
            } else if (parsed->msg_code == COAP_MSG_CODE_REQUEST_PUT) { //not hadling at the moment, just prints the payload requested by client
                printf("payload_len:  %d\n", parsed->payload_len);
                printf("payload:  %s\n", payload.c_str());
            } else if (parsed->msg_code == COAP_MSG_CODE_REQUEST_DELETE) { //not hadling at the moment, just prints the payload requested by client
                printf("payload_len:  %d\n", parsed->payload_len);
                printf("payload:  %s\n", payload.c_str());
            }
            coapserver_eventqueue.cancel(coapserver_eventqueue_handle); //cancel the server event
            coapserver_response_build(addr, parsed); //this function builds the response    
        }
        else {
            printf("got ACK from client\n");
        }
    } else {
        printf("Failed to receive message (%d)\n", recv_return_data_len);
    }
    free(recv_buffer);
}

// This function to prints instead of numbers actual request method of the client 
void requested_message_code(uint8_t msg_code) {
    switch(msg_code) {
        case COAP_MSG_CODE_REQUEST_GET:
         printf("Request method : GET\n");
         break;
        case COAP_MSG_CODE_REQUEST_POST:
         printf("Request method : POST\n");
        break;
        case COAP_MSG_CODE_REQUEST_PUT:
         printf("Request method : PUT\n");
        break;
        case COAP_MSG_CODE_REQUEST_DELETE:
         printf("Request method : DELETE\n");
        break;
        default:
        //    printf("ACK from client\n");
        break;
    }
}
// This function to prints instead of numbers actual request message type of the client 
void requested_message_type(uint8_t msg_type) {
    switch(msg_type) {
        case COAP_MSG_TYPE_CONFIRMABLE:
        printf("message type : con\n");
        break;
        case COAP_MSG_TYPE_NON_CONFIRMABLE:
        printf("message type : non\n");
        break;
        case COAP_MSG_TYPE_ACKNOWLEDGEMENT:
        printf("message type : ack\n");
        break;
        case COAP_MSG_TYPE_RESET:
        printf("message type : reset\n");
        break;
        default: printf("message type : con\n");
        break;
    }
}

//This function builds the response packet for client's request
//It accepts socket address and requested packet.
void coapserver_response_build( SocketAddress addr, sn_coap_hdr_s* coap_res_ptr ) {
    uint16_t messageid = coap_res_ptr->msg_id;
    uint8_t msg_code = coap_res_ptr->msg_code;
    std::string uri_path((const char *)coap_res_ptr->uri_path_ptr,coap_res_ptr->uri_path_len);
    std::string payload((const char*)coap_res_ptr->payload_ptr, coap_res_ptr->payload_len);// payload is a string
    coap_res_ptr = sn_coap_build_response(coapHandle,coap_res_ptr,coap_res_ptr->msg_code);  //build basic response
    coap_res_ptr->msg_id = messageid;   //response message id
    switch(msg_code) {
        case COAP_MSG_CODE_REQUEST_GET:           
            uint8_t Temp_hum_data;
            if ( coapserver_state_control == 1 ) {
                if (strncmp(uri_path.c_str(), "sensor/temp/value", strlen(uri_path.c_str())) == 0) {//this for temperature
                    coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
                //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 
                    if (requested_temp_scaling > 0) {
                        Temp_hum_data = Temp_centigrade * requested_temp_scaling;
                    } else {
                        Temp_hum_data = Temp_centigrade; 
                    }
                    coap_res_ptr->payload_ptr =  &(Temp_hum_data);  //current updated temperature reading
                    coap_res_ptr->payload_len =  sizeof(Temp_hum_data);
                    send_response_packet(addr, coap_res_ptr);
                } else if (strncmp(uri_path.c_str(), "sensor/hum/value", strlen(uri_path.c_str())) == 0) {//this for humidity
                    coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
                    //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 
                    if (requested_hum_scaling > 0) {
                        Temp_hum_data = RH_percentage * requested_hum_scaling;   //current updated humidity reading
                    } else {
                        Temp_hum_data = RH_percentage; 
                    }
                    coap_res_ptr->payload_ptr =  &(Temp_hum_data);  //humidity
                    coap_res_ptr->payload_len =  sizeof(Temp_hum_data);
                    send_response_packet(addr, coap_res_ptr);
                } else if (strncmp((char *)coap_res_ptr->uri_path_ptr, "control/state", coap_res_ptr->uri_path_len)==0) {
                    coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_SERVICE_UNAVAILABLE;//                  = 128,
                    coap_res_ptr->payload_ptr =  0; //no payload 
                    coap_res_ptr->payload_len =  0;
                }
        } else {
             coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_SERVICE_UNAVAILABLE;//                  = 128,
             coap_res_ptr->payload_ptr =  0;//(uint8_t *)payload_buff; //&(var);
             coap_res_ptr->payload_len =  0;//sizeof(var);//sizeof(Temp_centigrade);
        }
        break;
        case COAP_MSG_CODE_REQUEST_POST:
            coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; //if it is created but at the moment we are not creating anything so checking for validity
            send_response_packet(addr, coap_res_ptr);
        break;
        case COAP_MSG_CODE_REQUEST_PUT:
            if (coapserver_state_control == 1) {
                coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_VALID;
            } else {
                coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_NOT_IMPLEMENTED;
            }
            send_response_packet(addr, coap_res_ptr);
        break;
        case COAP_MSG_CODE_REQUEST_DELETE:
            if(coapserver_state_control == 1) {
                coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_VALID;//COAP_MSG_CODE_RESPONSE_DELETED;
            } else {
                coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_NOT_IMPLEMENTED;
            }
            send_response_packet(addr, coap_res_ptr);
        break;
        default: coap_res_ptr->msg_code = COAP_MSG_CODE_EMPTY; //test purpose
        break;
    }
    coapserver_eventqueue_handle = coapserver_eventqueue.call_every(5000, receive_msg); 
}

//This function sends the packet to the client through UDP
void send_response_packet(SocketAddress addr, sn_coap_hdr_s* coap_build_res_ptr) {
    uint16_t message_len = sn_coap_builder_calc_needed_packet_data_size(coap_build_res_ptr);
    uint8_t* message_ptr = (uint8_t*)malloc(message_len);
    sn_coap_builder(message_ptr, coap_build_res_ptr);
    udpsock.bind(portnumber);
    int scount = udpsock.sendto(addr, message_ptr, message_len);
    printf("Response Sent %d bytes on UDP\n\n", scount);
}

