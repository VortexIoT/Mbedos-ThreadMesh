/*
  Still work needs be impemente on this protocol. At the moment It's just implemented for test purpose 

//not handling token ptr 

   */

#include "mbed.h"
#include "stdlib.h"
#include "stdio.h"
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>
#include "ip6string.h"
#include <cstring>
#include "vmn_coap_server.h"
#include "ThreadInterface.h"
#include "cli_cmd.h"
#include "temp_humidity_sensor.h"
#include "mesh_config.h"
#include "vmn_coap_client.h"

DigitalOut led(LED1); //for green led
EventQueue coapserver_eventqueue;
UDPSocket udpsock;           // Socket to talk CoAP over
struct coap_s* coapHandle;//=sn_coap_protocol_init(&coap_malloc, &coap_free, &coap_tx_cb, &coap_rx_cb);
coap_version_e coapVersion = COAP_VERSION_1;
extern nwk_interface_id id;
uint8_t ipaddress_buffer[16] = {0};
uint16_t coapserver_eventqueue_handle=0;
uint16_t portnumber;
uint8_t requested_temp_scaling = 1;
uint8_t requested_hum_scaling = 1;
uint8_t coapserver_state_control = 1;
uint8_t requested_msg_type = 0;
uint8_t *server_recv_coappayload;
uint8_t  server_recv_coappayload_length;
//uint16_t coap_msg_payload_len;
//uint8_t coap_msg_payload[200];
//4-bit MSB of this byte is the Class Version and the 4-bit LSB defines the Class type
uint8_t classid = 0; //MSB version and LSB for class type
uint16_t messageid = 0;
extern uint8_t RH_percentage;
extern uint8_t Temp_centigrade;

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
    udpsock.bind(portnumber);
    if (recv_return_data_len > 0) {
        parsed = sn_coap_parser(coapHandle, recv_return_data_len, recv_buffer, &coapVersion);
        if(parsed->msg_code != COAP_MSG_CODE_EMPTY) {
            std::string payload((const char*)parsed->payload_ptr, parsed->payload_len);// payload is a string
            parsed->uri_path_ptr[parsed->uri_path_len] = '\0';
            std::string uri_path((const char *)parsed->uri_path_ptr,parsed->uri_path_len);  // uri is a string
            std::string token((const char*)parsed->token_ptr, parsed->token_len);   // token is a string
            //packet information 
       //     printf("msg_id: %d\n", parsed->msg_id);
            requested_message_code(parsed->msg_code); //here, get,post,put or delete are the message codes.
            requested_message_type(parsed->msg_type); //con or non
            requested_msg_type = parsed->msg_type;  //checking this when buliding the response packet
            printf("Uri-path: %s\n",parsed->uri_path_ptr);
         //   printf("token-ID: %s\n",parsed->token_ptr);
            if (parsed->options_list_ptr) {
                printf("location_path_ptr: %s\n", parsed->options_list_ptr->location_path_ptr);
                printf("location_path_len: %d\n", parsed->options_list_ptr->location_path_len);
                printf("uri_host_ptr: %p\n", parsed->options_list_ptr->uri_host_ptr);
            }
            //handling request from here
            if (parsed->msg_code == COAP_MSG_CODE_REQUEST_POST) {
                uint8_t posted_payload = atoi(payload.c_str());
                printf("payload_len: %d\n", parsed->payload_len);
                printf("payload: %d\n", posted_payload);
                if (strncmp(uri_path.c_str(), "sensor/temp/scaling", parsed->uri_path_len) == 0) { //checking for uri path
                //here handling only upto 255 if it is more than 255 scaling res would rollback to value - 256 ,so scaling must be 1 to 5 if possible
                    if(posted_payload > 0 && posted_payload <= 5) //setting the range of scaling factor
                        requested_temp_scaling = posted_payload;
                } else if (strncmp(uri_path.c_str(), "sensor/hum/scaling", parsed->uri_path_len) == 0) {  //checking for uri path
                //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 
                    if(posted_payload > 0 && posted_payload <= 5)
                        requested_hum_scaling = posted_payload;
                } else if (strncmp(uri_path.c_str(), "control/state", parsed->uri_path_len) == 0) {           //This controls the state of led as well as server responses   
                    if((posted_payload == 0) || (posted_payload == 1)) {
                        coapserver_state_control = posted_payload;  //changing the state byte here this will be handle on server side
                        led = posted_payload;
                    }       
                }else {
                    server_recv_coappayload = (uint8_t *)(payload.c_str());
                    server_recv_coappayload_length = parsed->payload_len;
                   // vmn_dpd_extract();
                }
            } else if (parsed->msg_code == COAP_MSG_CODE_REQUEST_PUT) { //not hadling at the moment, just prints the payload requested by client
                server_recv_coappayload = (uint8_t *)payload.c_str();
                server_recv_coappayload_length = parsed->payload_len;
             //   vmn_dpd_extract();
            //     for( int i = 0; i < coap_msg_payload_len ; i++ )
            //        printf("coapmsg %x\n", coap_msg_payload[i]);
            //    printf("payload_len:  %d\n", server_recv_coappayload_length);
            //    printf("payload:  %s\n", server_recv_coappayload);
            } else if (parsed->msg_code == COAP_MSG_CODE_REQUEST_DELETE) { //not hadling at the moment, just prints the payload requested by client
                server_recv_coappayload = (uint8_t *)payload.c_str();
                server_recv_coappayload_length = parsed->payload_len;
             //   printf("payload_len:  %d\n", parsed->payload_len);
            //    printf("payload:  %s\n", payload.c_str());
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
          //  printf("get method temp''''\n");
            if ( coapserver_state_control == 1 ) {
                if (strncmp(uri_path.c_str(), "sensor/temp/value", strlen(uri_path.c_str())) == 0) {//this for temperature
                    coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
                //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 
                    if (requested_temp_scaling > 0) {
                        Temp_hum_data = Temp_centigrade * requested_temp_scaling; //if scaling
                    } else {
                        Temp_hum_data = Temp_centigrade; 
                    }
                    if( requested_msg_type == COAP_MSG_TYPE_CONFIRMABLE ) {
                        coap_res_ptr->payload_ptr =  &(Temp_hum_data);  //current updated temperature reading ack_vmn_coapmsg_build(Temp_hum_data);//
                        coap_res_ptr->payload_len =  sizeof(Temp_hum_data); //sizeof(coap_res_ptr->payload_ptr);//
                    } else if( requested_msg_type == COAP_MSG_TYPE_NON_CONFIRMABLE ) {
                        coap_res_ptr->payload_ptr = &(Temp_hum_data);//nonack_vmn_coapmsg_build(Temp_hum_data);//&(Temp_hum_data);  //current updated temperature reading
                        coap_res_ptr->payload_len =  sizeof(Temp_hum_data);//sizeof(Temp_hum_data);
                    }
                    send_response_packet(addr, coap_res_ptr);
                } else if (strncmp(uri_path.c_str(), "sensor/hum/value", strlen(uri_path.c_str())) == 0) {//this for humidity
                    coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_CONTENT;
                    //here handling only upto 255 if it is more than after scaling res would rollback to value - 256 
                    if ( requested_hum_scaling > 0 ) {
                        Temp_hum_data = RH_percentage * requested_hum_scaling;   //current updated humidity reading
                    } else {
                        Temp_hum_data = RH_percentage; 
                    }
                    if( requested_msg_type == COAP_MSG_TYPE_CONFIRMABLE ) {
                        coap_res_ptr->payload_ptr =  &Temp_hum_data;// &(Temp_hum_data);  //humidity
                        coap_res_ptr->payload_len =  sizeof(Temp_hum_data);//sizeof(coap_res_ptr->payload_ptr);//sizeof(Temp_hum_data);
                    }
                    if( requested_msg_type == COAP_MSG_TYPE_NON_CONFIRMABLE ) {
                        coap_res_ptr->payload_ptr =  &Temp_hum_data;//nonack_vmn_coapmsg_build(Temp_hum_data);//&(Temp_hum_data);  //current updated temperature reading
                        coap_res_ptr->payload_len =  sizeof(Temp_hum_data);//sizeof(coap_res_ptr->payload_ptr);//sizeof(Temp_hum_data);
                    }
                    send_response_packet(addr, coap_res_ptr);
                } else if ( strncmp((char *)coap_res_ptr->uri_path_ptr, "control/state", coap_res_ptr->uri_path_len ) == 0 ) {
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
            coap_res_ptr->msg_type = COAP_MSG_TYPE_ACKNOWLEDGEMENT;
            coap_res_ptr->msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; //if it is created but at the moment we are not creating anything so checking for validity
            vmn_dpd_extract();
          //   ack_frm_server_to_client();
            coap_res_ptr->payload_ptr =  coap_msg_payload;//nonack_vmn_coapmsg_build(Temp_hum_data);//&(Temp_hum_data);  //current updated temperature reading
            coap_res_ptr->payload_len =  coap_msg_payload_len;
            send_response_packet(addr, coap_res_ptr);
        break;
        case COAP_MSG_CODE_REQUEST_PUT:
            if (coapserver_state_control == 1) {
                coap_res_ptr->msg_type = COAP_MSG_TYPE_ACKNOWLEDGEMENT;
                coap_res_ptr->msg_code =  COAP_MSG_CODE_RESPONSE_VALID;//COAP_MSG_CODE_RESPONSE_VALID;
             //   ack_frm_server_to_client();
                vmn_dpd_extract();
                coap_res_ptr->payload_ptr =  coap_msg_payload;//nonack_vmn_coapmsg_build(Temp_hum_data);//&(Temp_hum_data);  //current updated temperature reading
                coap_res_ptr->payload_len =  coap_msg_payload_len;
              //  for( int i = 0; i < coap_msg_payload_len ; i++ )
             //   printf("%x", *coap_res_ptr->payload_ptr++);
              // printf("pp:%x len:%d", *coap_res_ptr->payload_ptr, coap_res_ptr->payload_len);
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
  //  printf("addr%s port:%d",addr.get_ip_address(),addr.get_port());
    printf("Response Sent %d bytes on UDP\n\n", scount);
//    printf("%s\n",coap_build_res_ptr->payload_ptr);
    free(message_ptr);
}

void vmn_dpd_extract(void) {
    //total payload bytes count if payload length is 0 then byte count 9
    uint8_t sync1 = 0,sync2 = 0; //to identify the start of the packet
    uint8_t version = 0, messagetype = 0;
    uint16_t payloadlength;
    char local_buffer[2] = {0};
    char messageid_upperbyte = 0, messageid_lowerbyte = 0;
    uint16_t coap_messagechecksum;
    uint16_t cal_checksum = 0;
    //56d703050100002609  or V*.....&.
    local_buffer[0] = *server_recv_coappayload++; //ascii 0th position 
    local_buffer[1] = *server_recv_coappayload++; //ascii 1th position
    sync1 = strtohex8(local_buffer);  //sync1
    local_buffer[0] = *server_recv_coappayload++; //ascii 2nd position
    local_buffer[1] = *server_recv_coappayload++; //ascii 3rd position
    sync2 = strtohex8(local_buffer);
    /*********  02/02/2021 to check for valid payload ****************/
    if(  ( sync1 == 0x56 )  &&  ( sync2 == 0xd7 ) ) { //think about non ascii characters 
        local_buffer[0] = *server_recv_coappayload++; //ascii 4 position
        local_buffer[1] = *server_recv_coappayload++; //ascii 5 position
        classid = strtohex8(local_buffer);//*server_recv_coappayload+2; //3rd byte
        version = ( classid & 0xf0 ); //MSB version
        messagetype = ( classid & 0x0f ); //message type
        local_buffer[0] = *server_recv_coappayload++; //ascii 6 position
        local_buffer[1] = *server_recv_coappayload++; //ascii 7 position
        messageid_upperbyte = strtohex8(local_buffer);//*server_recv_coappayload+3; //4th
        local_buffer[0] = *server_recv_coappayload++; //ascii 8 position
        local_buffer[1] = *server_recv_coappayload++; //ascii 9 position
        messageid_lowerbyte = strtohex8(local_buffer);//*server_recv_coappayload+4;
        messageid = ( ( messageid_upperbyte << 8 ) | ( messageid_lowerbyte ) );
        uint8_t payld1,payld2;
        local_buffer[0] = *server_recv_coappayload++; //ascii 10 position
        local_buffer[1] = *server_recv_coappayload++; //ascii 11 position
        payld1 = strtohex8(local_buffer); //This may change later because length is 2byte wide by coap msg format it's represented by 1 byte check it later
        local_buffer[0] = *server_recv_coappayload++; //ascii 12 position
        local_buffer[1] = *server_recv_coappayload++; //ascii 13 position
        payld2 = strtohex8(local_buffer);//*server_recv_coappayload+4;
        payloadlength = ( ( payld1 << 8 ) | ( payld2 ) );
    //    printf("payloadlen : %d",payloadlength);
    ///   payloadlength = atoi(local_buffer); //converting to int
        uint8_t payload[payloadlength];
        for(uint16_t i=0; i< payloadlength; i++) {
        //    local_buffer[0] = *server_recv_coappayload++; //ascii 14 position
            if( (*server_recv_coappayload <='9') && ( *server_recv_coappayload >='0' ))
            {
                local_buffer[0] = *server_recv_coappayload++;
            }
            else if(isalpha(*server_recv_coappayload))
            {
                 if((*server_recv_coappayload <='F') || ((*server_recv_coappayload >= 'a') && (*server_recv_coappayload <= 'f')))
                 {
                     local_buffer[0] = *server_recv_coappayload++;
                 }
            } else {
                nonack_frm_server_to_client();
            }
               
            if( (*server_recv_coappayload <='9') && ( *server_recv_coappayload >='0' )){
                local_buffer[1] = *server_recv_coappayload++;
            }
            else if(isalpha(*server_recv_coappayload)) {
                 if( ( *server_recv_coappayload <= 'F' ) || ( ( *server_recv_coappayload >= 'a' ) && ( *server_recv_coappayload <= 'f' ) ) )
                 {
                     local_buffer[1] = *server_recv_coappayload++;
                 }
            } else {
                nonack_frm_server_to_client(); }

        //    local_buffer[1] = *server_recv_coappayload++; //ascii 15 position
            payload[i] = strtohex8(local_buffer);//*server_recv_coappayload++;
            cal_checksum += payload[i];
        }
        payload[payloadlength] = '\0';
    /*   local_buffer[0] = *server_recv_coappayload+(6 + payloadlength); //first byte after payload
        local_buffer[1] = *server_recv_coappayload+(7 + payloadlength); //last byte
        coap_messagechecksum = atoi(local_buffer);
        cal_checksum += (classid + messageid_lowerbyte + messageid_upperbyte + payloadlength);
        if(cal_checksum == coap_messagechecksum) {
            printf("checksum matched\n");
        } else {
            printf("checksum doesn't matched\n");
        }*/

       // printf("sync1:%x\n",sync1);
      //  printf("sync2:%x\n",sync2);
      //  printf("msgid:%x\n",messageid);
      //  printf("paylen:%x\n",payloadlength);
     //   printf("payload:%s\n",payload);
        switch(messagetype) {
            //version 0
            case 0x01: //ACK messages
                ack_nonack_generic_msg(messageid,payloadlength,payload);
            break;
            case 0x02: //SET configuration message
                configuration_commands(messageid,payloadlength,payload);
            break;
            case 0x03: //operational commands
            ack_frm_server_to_client();
        //    operational_commands(messageid);
            break;
            case 0x04: //sensor measurements
            nonack_frm_server_to_client();
    //     sensordata_measure_commands(messageid);
            break;
            case 0x05: //monitoring data
            break;
            case 0x06: //information message
            break;
            case 0x07: //Firmware update message
            break;
        }
    }

}
void ack_si7021_response(uint8_t actualdata) {
 uint8_t checksum1 = 0,checksum2 = 0;
 uint16_t checksum = 0;
 checksum = (0x01 + 0x00 + 0x01 + 0x04 + 0x01 + 0x00 + 0x01 + actualdata);
 checksum2 = (checksum & 0xff);
 checksum1 = (checksum & 0xff00) >> 8;
 uint8_t vmn_coapmsg[] = {0x56, 0xd7, 0x01, 0x00, 0x01, 0x04, classid, (uint8_t) ((messageid &0xff00) >> 8), (uint8_t)(messageid & 0xff), actualdata, checksum1, checksum2};
 strcpy((char *)coap_msg_payload, (char *)vmn_coapmsg);
 coap_msg_payload_len = sizeof(vmn_coapmsg);
}

void nonack_si7021_response(uint8_t actualdata) {
    uint8_t checksum1 = 0,checksum2 = 0;
    uint16_t checksum = 0;
    checksum = (0x01 + 0x00 + 0x01 + 0x04 + 0x01 + 0x00 + 0x01 + actualdata);
    checksum2 = (checksum & 0xff);
    checksum1 = (checksum & 0xff00) >> 8;
    uint8_t vmn_coapmsg[12] = {0x56, 0xd7, 0x01, 0x00, 0x00, 0x04 , classid, (uint8_t) ((messageid &0xff00) >> 8), (uint8_t)(messageid & 0xff), actualdata, checksum1, checksum2};
    strcpy((char *)coap_msg_payload, (char *)vmn_coapmsg);
    coap_msg_payload_len = sizeof(vmn_coapmsg);
}


void ack_nonack_generic_msg(uint16_t messageid,uint16_t payloadlength, uint8_t *ack_nonack_payload) {  //0x01
    switch(messageid)
    {
        case 0x0000: //Non ACK 
           // if(payload[0] == *server_recv_coappayload+)
                printf("clsid : %x", classid);
                printf("messageid : %x", messageid);
         //   if(payload[1] == *server_recv_coappayload+3)
            //    printf("msid : %x", payload[1]);
         //   if(payload[2] == *server_recv_coappayload+4)
             //   printf("%x", payload[2]);
        break;
        case 0x0001: //ACK
            printf("clsid : %x", classid);
            printf("messageid : %x", messageid);
          /*  if(payload[0] == *server_recv_coappayload+2)
                printf("clsid : %x", payload[0]);
            if(payload[1] == *server_recv_coappayload+3)
                printf("msid : %x", payload[1]);
            if(payload[2] == *server_recv_coappayload+4)
                printf("%x", payload[2]);*/
        break;
        default:
        break;
    }
}
//To indicate the strcture of the requested command is valid
void ack_frm_server_to_client(void) {  //here forming the payload for ack
    coap_msg_payload[0] = 0x56; //sync
    coap_msg_payload[1] = 0xd7; //sync
    coap_msg_payload[2] = 0x01; //cls
    coap_msg_payload[3] = 0x00; //id
    coap_msg_payload[4] = 0x01; //id
    coap_msg_payload[5] = 0x00; //2bytes of length
    coap_msg_payload[6] = 0x03; //general ack to inform client about the previous 
    coap_msg_payload[7] = classid; //previous command class id which we have to send ack
    coap_msg_payload[8] = (messageid & 0xff00) >> 8; //message id upper byte
    coap_msg_payload[9] = (messageid & 0xff); //message id upper byte
    coap_msg_payload[10] = '\0';
    coap_msg_payload_len = 10;
  //  strcpy((char *)coap_msg_payload, (char *)masterkey);
  //  coap_msg_payload_len = 16;
}
//To indicate the strcture of the requested command is not within the format or somedata is missing
void nonack_frm_server_to_client(void) {
   // uint8_t *nonack_payload;
    coap_msg_payload[0] = 0x56; //sync
    coap_msg_payload[1] = 0xd7; //sync
    coap_msg_payload[2] = 0x01; //cls
    coap_msg_payload[3] = 0x00; //id
    coap_msg_payload[4] = 0x00; //id
    coap_msg_payload[5] = 0x00; //2bytes of length
    coap_msg_payload[6] = 0x03; //general ack to inform client about the previous 
    coap_msg_payload[7] = classid; //previous command class id which we have to send ack
    coap_msg_payload[8] = (messageid & 0xff00) >> 8; //message id upper byte
    coap_msg_payload[9] = (messageid & 0xff); //message id upper byte
    coap_msg_payload[10] = '\0';
    coap_msg_payload_len = 10;
  /*  *nonack_payload++ = 0x56; //sync
    *nonack_payload++ = 0xd7; //sync
    *nonack_payload++ = 0x01; //cls
    *nonack_payload++ = 0x00; //id
    *nonack_payload++ = 0x00; //id
    *nonack_payload++ = 0x00; //2bytes of length
    *nonack_payload++ = 0x03; //general ack to inform client about the previous 
    *nonack_payload++ = classid; //previous command class id which we have to send ack
    *nonack_payload++ = (messageid & 0xff00) >> 8; //message id upper byte
    *nonack_payload = (messageid & 0xff); //message id upper byte*/
    
 //   return nonack_payload; 
}

void ack_frm_client_to_server(uint8_t ack_payload[])
{
  //  ack_payload[0] = 
}

void nonack_frm_client_to_server(uint8_t ack_payload[])
{
  //  ack_payload[0] = 
}

void operational_commands(uint16_t messageid, uint8_t *commandsdata) {  //0x03
    switch(messageid)
    {
        case 0x0000: //CMD-LDPPR-START 
        break;
        case 0x0001: //CMD-LDPPR-START 
        break;
        case 0x0100: //CMD-AQM-START 
        break;
        case 0x0101: //CMD-AQM-STOP  
        break;
        case 0x0102: //CMD-AQM-INTVL  
        break;
        case 0x0103: //CMD-AQM-PURGE
        break;
        case 0x0104: //CMD-AQM-LATCH
        break;
        case 0x00105: //CMD-AQM-VER-LMT 
        break;
        case 0x0106: //CMD-AQM-PURGE-LMT
        break;
        case 0x0107: //CMD-GET-PURGE-LMT  
        break;
        case 0x0108: //CMD-GET-VERIFICATION-LMT
        break;
        case 0x0109: //CMD-GET-AQM-INTVL 
        break;
        case 0x0200: //CMD-SI7021-START
        break;
        case 0x0201: //CMD-SI7021-STOP
            stop_si7021_datacapture();
        break;
        case 0x0202: //CMD-SI7021-SINGLE
            stop_si7021_datacapture(); //stop if continuous measurement is active
            humidity_temp_read();
        break;
        case 0x0203: //CMD-SI7021-INTVL
            set_si7021_datacapture_intervel((int )commandsdata);
        break;
        case 0x0400: //CMD-GAS-START 
        break;
        case 0x0401: //CMD-GAS-STOP
        break;
        case 0x0402: //CMD-GAS-SINGLE  
        break;
        case 0x0403: //CMD-GAS-GET-INTVL 
        break;
        case 0x0404: //CMD-GAS-SET-INTVL 
        break;
        case 0x0405: //CMD-GAS-GET-ADC-GAIN
        break;

        default:
        break;
    }

}

uint8_t device_node_Type_flag = 1;
thread_device_type_e thread_devicetype(uint8_t devicetype) {
    thread_device_type_e node_type;;
    switch(devicetype) {
        case 1: node_type = THREAD_DEVICE_REED; device_node_Type_flag = 1; break;
        case 2: node_type = THREAD_DEVICE_FED; device_node_Type_flag = 2; break;
        case 3: node_type = THREAD_DEVICE_MED; device_node_Type_flag = 3; break;
        case 4: node_type = THREAD_DEVICE_SED; device_node_Type_flag = 4; break;
        default: node_type = THREAD_DEVICE_REED; device_node_Type_flag = 1; break;
    }
    return node_type;
}
void configuration_commands(uint16_t messageid,uint16_t recvpayloadlen, uint8_t recvpayload[]) {  //0x02
uint8_t *ptr;
 //   uint16_t recvpayloadlen = *server_recv_coappayload+5;
 //   uint8_t recvpayload[recvpayloadlen];
 //   for(uint16_t i = 0;i < recvpayloadlen; i++)
 //       recvpayload[i] = *server_recv_coappayload+(6 + i);
    switch(messageid) {
        case 0x0000: //Poll LiDAR Control Setting Request SET-LDCS 
        break;
        case 0x0001: //SET-LDCS-RPLY 
        break;
        case 0x0002: //SET-EUI64 
            static_cast<ThreadInterface*>(mesh)->device_eui64_set(recvpayload);
            //send ack here
            ack_frm_server_to_client();
        break;
        case 0x0003: //SET-NET-RESET
        break;
        case 0x0004: //SET-SOFT-RESET
        break;
        case 0x0005: //GET-EUID
            thread_eui64_trace();
        break;
        case 0x0006: //GET-RADIO-POWER 
        break;
        case 0x0007: //SET-HEARTBEAT-INTVL
        break;
        case 0x0008: //GET-TXPWR-QUERY 
        break;
        case 0x0009: //GET-HEARTBEAT
        break;
        case 0x000a: //SET-NODE-TYPE 
            thread_management_device_type_set(id, thread_devicetype(recvpayload[recvpayloadlen]));
        //     ack_frm_server_to_client();
        break;
        case 0x000b: //GET-NODE-TYPE 
        switch(device_node_Type_flag) {
            case 1: coap_msg_payload_len = 1; coap_msg_payload[0] = device_node_Type_flag; break;
            case 2: coap_msg_payload_len = 1; coap_msg_payload[0] = device_node_Type_flag; break;
            case 3: coap_msg_payload_len = 1; coap_msg_payload[0] = device_node_Type_flag; break;
            case 4: coap_msg_payload_len = 1; coap_msg_payload[0] = device_node_Type_flag; break;
            default:coap_msg_payload_len = 1; coap_msg_payload[0] = device_node_Type_flag; break;
        }
        //    thread_device_type_e node_type;
        //    payload = thread_devicetype(payload);
        break;
        case 0x000c: //SET-NET-PARAM 
        //I am thinking that because of changing the network details ip address will change so that gateway is looking for the ack from the address which it sent
        //after changing the network details ip will change so gateway not handling the ack.
        for(uint8_t i = 0; i < 16; i++ ) {
            masterkey[i] = recvpayload[i];
        } for(uint8_t i = 0; i < 8; i++ ) {
            meshprefix[i] = recvpayload[16 + i];
        }
        panid = ( ( recvpayload[24] << 8 ) | ( recvpayload[25] ) );
        channel = recvpayload[26];
    //    ack_frm_server_to_client();
        datasetcommit_active();
        //send ACK here
     //    ack_frm_server_to_client();
        break;
        case 0x000d: //SET-MASTER-KEY
        for(uint8_t i = 0; i < 16; i++ ) {
            masterkey[i] = recvpayload[i];
        }
    //    ack_frm_server_to_client();
        datasetcommit_active();
        //send ACK here
        break;
        case 0x000e: //SET-NET-SWAP
        break;
        case 0x000f: //SET-AQM-POWER-MODE
        break;
        case 0x0010: //GET-AQM-POWER-MODE 
        break;
        case 0x0011: //SET-RTCC-DATE-TIME 
        break;
        case 0x0012: //GET-RTCC-DATE-TIME  
        break;
        default:
        break;
    }
}
/*
void sensors_data_measure_commands(uint16_t messageid) { //0x04
    switch(messageid)
    {
        case 0x0000: //DAT-LDPVT 
        break;
        case 0x0001: //DAT-SDS011
        break;
        case 0x0002: //DAT-SDS198 
        break;
        case 0x0005: //DAT-AQM-PURGE 
        break;
        case 0x0006: //DAT-AQM-COMS-FAIL 
        break;
        case 0x0007: //DAT-AQM-PURGE-LMT
        break;
        case 0x0008: //DAT-AQM-VERIFICATION-LMT 
        break;
        case 0x0009: //DAT-AQM-INTVL 
        break;
        case 0x0010: //DAT-GAS 
        break;
        case 0x0011: //DAT-GAS-INTVL 
        break;
        case 0x0012: //DAT-AQM-MASTER-INTERVAL
        break;
        case 0x0013: //DAT-AQ-MASTER-IS-RUNNING  
        break;
        case 0x0014: //DAT-ADC-GAIN  
        break;
        case 0x0015: //DAT-RODIO-DATA 
        break;
        case 0x0016: //DAT-GAS-DUAL-ADC 
        break;
        case 0x0017: //DAT-GAS-FAN-SPEED  
        break;
        case 0x0018: //DAT-GAS-PURGE-DURATION 
        break;
        case 0x0019: //DAT-GAS-POST-PURGE
        break;
    //this function has few more cases not included now
        default:
        break;
    }
}*/

void devicedata_monitor_commands(uint16_t messageid) { //0x05
    switch(messageid) {
        case 0x0000: //Non ACK 
        break;
        case 0x0001: //ACK
        break;
        default:
        break;
    }
}
void errorcode_commands(uint16_t messageid) { //0x06
    switch(messageid) {
        case 0x0000: //Non ACK 
        break;
        case 0x0001: //ACK
        break;
        default:
        break;
    }
}
void firmwareupdate_commands(uint16_t messageid) { //0x07
    switch(messageid) {
        case 0x0000: //Non ACK 
        break;
        case 0x0001: //ACK
        break;
        default:
        break;
    }
}

/*classid = *coap_payload+2; //3rd byte
messageid_upperbyte = *coap_payload+3; //4th
messageid_lowerbyte = *coap_payload+4; //5th

messageid = ((messageid_upperbyte << 8) | (messageid_lowerbyte));
local_buffer[0] = *coap_payload+5; //6th
local_buffer[1] =  '\0';// *coap_payload++; //7th byte //This may change later because length is 2byte wide by coap msg format it's represented by 1 byte check it later
//at the moment consider it as 1byte
payloadlength = atoi(local_buffer); //converting to int
uint8_t payload[payloadlength];
for(uint16_t i=0; i< payloadlength; i++) {
    payload[i] = *coap_payload+6;
    cal_checksum += payload[i];
}
local_buffer[0] = *coap_payload+(6 + payloadlength); //first byte after payload
local_buffer[1] = *coap_payload+(7 + payloadlength); //last byte
coap_messagechecksum = atoi(local_buffer);
cal_checksum += (classid + messageid_lowerbyte + messageid_upperbyte + payloadlength);
if(cal_checksum == coap_messagechecksum)
    printf("checksum matched\n");
else {
    printf("checksum doesn't matched\n");
}
*/
//printf("sync1:%x\n",sync1);
 //   printf("sync2:%x\n",sync2);
    //|| ( sync1 == ASCIISYNC1 ) || ( sync1 == DECSYNC1 ) )  //|| ( sync1 == DECSYNC2 )) 