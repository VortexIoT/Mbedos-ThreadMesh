#include "stdio.h"
#include "socket_api.h"
#include "mesh_nvm.h"
#include "mbed.h"
#include "ThreadInterface.h"
#include "string.h"
#include <cstdint>
#include "cli_cmd.h"
#include "sn_coap_header.h"
#include "vmn_coap_client.h"


mbed::RawSerial pc(USBTX, USBRX,115200);
MeshInterface *mesh;
/*********************  Variable Decalrations START **************************/
/*  Network Parameters */
uint8_t extpanid[8] = {0xf1, 0xb5, 0xa1, 0xb2,0xc4, 0xd5, 0xa1, 0xbd };
uint8_t masterkey[16] ={0xa4, 0xb1, 0x14, 0x95, 0xa8, 0x89, 0x5e, 0x6e, 0xc2, 0x93, 0xf5, 0x3d, 0x54, 0x86, 0x18, 0xd6};//{ 0x00, 0xeb, 0x64, 0x37, 0x02, 0x4c, 0x86, 0x8d, 0xdd, 0x2b, 0x18, 0xde, 0x62, 0xc7, 0x98, 0x68};
uint16_t panid= 0x0700;;
uint8_t Network_name[16];// = "Thread Network";
uint16_t channel = 22;
uint8_t meshprefix[8] = {0xfd, 0x0, 0x0d, 0xb8, 0x0, 0x0, 0x0, 0x0};
uint8_t channel_mask[9] = "07fff800";
uint8_t psk[16] = MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PSKC;
uint8_t securitypolicy = 255; 

link_configuration_s *linkcopy;
link_configuration_s *link;
nwk_interface_id id = IF_IPV6;
volatile uint8_t flag=0;
volatile char Rx_buff[256];
uint8_t Receive_buff_length=0;
uint8_t getcmd_count=1;
SocketAddress sockAddr;

/*********************  Variable Decalrations END **************************/

//This function generates the EUI64
void thread_eui64_trace() {
    #define LOWPAN 1
    #define THREAD 2
    #if MBED_CONF_NSAPI_DEFAULT_MESH_TYPE == THREAD && (MBED_VERSION >= MBED_ENCODE_VERSION(5,10,0))
    uint8_t eui64[8] = {0};
    static_cast<ThreadInterface*>(mesh)->device_eui64_get(eui64);
    printf("Device EUI64 address = %02x%02x%02x%02x%02x%02x%02x%02x\n", eui64[0], eui64[1], eui64[2], eui64[3], eui64[4], eui64[5], eui64[6], eui64[7]);
    #endif
}

int j=0;
/* Receive interrupt function.This function will be automatically called when data entered through the serial terminal*/
void isr_rx() {
    char databyte=pc.getc();
    if (databyte != '\n') {
        if(databyte == 0x08)
         j = j-1;
        else
            Rx_buff[j++] = databyte;
    } else {
        Receive_buff_length = j;
        Rx_buff[j] = '\0';
        flag = 1;
        j=0;    
    }
}

//Transmit interrupt function
void  transmit_interrupt(char *Data_buff) {
  pc.puts(Data_buff);
}
/***************  END ******************/

// This function connects the device into the network and also it return the device Connected IP address
uint8_t mesh_connect(void) {
    uint8_t error;
    mesh = MeshInterface::get_default_instance();  //returns pointer to the mesh interface
    if (!mesh) {
        printf("Error! MeshInterface not found!\n");
        return -1;
    }
    thread_eui64_trace();  //This function generates the EUI64
    mesh_nvm_initialize();  //initializes the non-volatile memory
    printf("Connecting...\n");
    error = mesh->connect();  
    if (error) {
        printf("Connection failed! %d\n", error);
       return error;
    } 
    SocketAddress sockAddr;
    while (NSAPI_ERROR_OK != mesh->get_ip_address(&sockAddr)) {//local ip address
        ThisThread::sleep_for(500); //keep this loop on until get the IP address with offering 500ms for each turn  
    }
    printf("Connected IP : %s\n",sockAddr.get_ip_address());
    link = thread_management_configuration_get(id);
    thread_management_set_link_timeout(id,80);
    getcmd_count = 1;
    return 1;
}

 //Disconnects the device from network
void mesh_disconnect(void) {
    mesh->disconnect();
}

// Call this function whenever want to read the IP address of the network device
void read_ipaddr(void) {
    SocketAddress sockAddr;
    while (NSAPI_ERROR_OK != mesh->get_ip_address(&sockAddr)) {//local ip address
        ThisThread::sleep_for(500); 
    }
    printf("Connected IP : %s\n",sockAddr.get_ip_address());
}

// This function will converts char into hex format
uint8_t chartohex(char ch) {
    if ((ch <= '9') && (ch >= '0')) {
        ch = ch-'0';
    } if ((ch >= 'A') && (ch <= 'F')) {
        ch = ch-'A' + 10;
    } if ((ch >= 'a') && (ch <= 'f')) {
    ch = ch-'a' + 10;
    }
    return ch;
}

// This function will converts string of required length into hex format.
/*
 This function accepts 3 params
  str = string to convert into hex
output = pointer to store the converted hex data
len = length of data

*/
void string_to_hex(char *str, uint8_t *output, uint8_t len) {
    int j=0;
    uint8_t hex1,hex2;
    for (uint8_t i=0;i<len;) {
        hex1 = chartohex(str[i]);
        hex2 = chartohex(str[i+1]);
        output[j] = (hex2 & 0xff) + ((hex1 & 0xff) << 4);
        i +=2;
        j++;     
    }
}

/* This function implemented only for panid.
Calling this function will return 16bit panid value*/ 
uint16_t panid_value(char *str) {
    uint16_t panid;
    char hex1,hex2,hex3,hex4;
    hex1 = chartohex(str[0]);
    hex2 = chartohex(str[1]);
    hex3 = chartohex(str[2]);
    hex4 = chartohex(str[3]);
    panid =  ((hex1 << 12) | (hex2 << 8) | (hex3 << 4) | hex4);
    return panid;
}   

// Call this function to get detailed information about the network parameters
void scan_network_details(void) {
    link = thread_management_configuration_get(id);
    printf("Panid :0x%04x\nNetwork Name :%s\nChanne1 : %d\n", link->panId, link->name,link->rfChannel);
    printf("Masterkey : ");
    for (int i=0;i< sizeof(link->master_key);i++)                   
        printf("%02x", link->master_key[i]);printf("\n");
    printf("Ext Panid : ");
    for (int i=0;i< 8;i++) 
        printf("%02x", link->extented_pan_id[i]); printf("\n");
    printf("Channel Mask : ");
    for (int i=0;i< 8;i++) {
        if (link->channel_mask[i] >= channel_mask[i])
            printf("%c",link->channel_mask[i]);
        else
            printf("%c",channel_mask[i]);
    }
    printf("\n");
    printf("PSKc : ");
    for (int i=0;i< 16;i++) 
        printf("%x", link->PSKc[i]); printf("\n");
    printf("Security Policy : %d\n",link->securityPolicy);
    printf("Mesh prefix :");
    for (int i=0;i< 8;i++){
        if ((i>0) && (i%2 == 0))
            printf(":");
    printf("%02x", link->mesh_local_ula_prefix[i]);
    } 
    printf("/64\n");
}

// This function called whenever user enters the dataset commit active command.
void datasetcommit_active(void) {
    thread_management_set_request_full_nwk_data(id, true);           
    link->rfChannel = channel;
    for (int i=0;i<sizeof(Network_name);i++)
        link->name[i]= Network_name[i];
    link->panId = panid;
    for (int i=0;i<16;i++)
        link->master_key[i] = masterkey[i];
    for (int i=0;i<8;i++)
       link->extented_pan_id[i] = extpanid[i];
    for (int i=0;i<8;i++)
       link->channel_mask[i] = channel_mask[i];
    for (int i=0;i<8;i++)
       link->mesh_local_ula_prefix[i] = meshprefix[i];
    for (int i=0;i<16;i++)
        link->PSKc[i] = psk[i];
    link->securityPolicy = securitypolicy;
    thread_management_link_configuration_store(id,link);            
}

// This function called when user entered command follows with get * command 
// THis function accepts param as a input string to distinguish between commands
void get_values(char *networkparameter, uint8_t len) {
    if (!strncmp(networkparameter,"masterkey",len)) {
        for (int i=0;i< sizeof(linkcopy->master_key);i++)                   
            printf("%02x", linkcopy->master_key[i]);printf("\n");
    } else if (!strncmp(networkparameter,"panid",len)) {
        printf("0x%04x\n", linkcopy->panId);
    } else if (!strncmp(networkparameter,"networkname",len)) {
        printf("%s\n", linkcopy->name);
    } else if (!strncmp(networkparameter,"extpanid",len)) {
        for (int i=0;i< 8;i++)  
            printf("%02x", linkcopy->extented_pan_id[i]); printf("\n");
    } else if (!strncmp(networkparameter,"channel",len)) {
        printf("%d\n",linkcopy->rfChannel);
    } else if (!strncmp(networkparameter,"channelmask",len)) {
        for (int i=0;i< 8;i++) { 
            if (linkcopy->channel_mask[i] >= channel_mask[i]) {
               printf("%c",linkcopy->channel_mask[i]);
            } else {
               printf("%c",channel_mask[i]);
            }
        }
        printf("\n");
    } else if(!strncmp(networkparameter,"psk",len)) {
        for(int i=0;i< 16;i++) 
            printf("%x", linkcopy->PSKc[i]); printf("\n");
    } else if(!strncmp(networkparameter,"securitypolicy",len)) {
        printf("%d\n", linkcopy->securityPolicy);
    } else if(!strncmp(networkparameter,"prefix",len)) {
        for (int i=0;i< 8;i++) { 
             if ((i>0) && (i%2 == 0)) {
                printf(":");
             }
            printf("%02x", linkcopy->mesh_local_ula_prefix[i]);
        } 
        printf("/64\n");
    }
}

// This function gives the list of commands with prefix of dataset
void dataset_commands_List(void)  {//call this function when dataset help command called
   printf("commit\n");
   printf("channel\n");
   printf("channelmask\n");
   printf("extpanid\n");
   printf("masterkey\n");
   printf("networkname\n");
   printf("panid\n");
   printf("psk\n");
   printf("prefix\n");
   printf("securitypolicy\n");
}

// This function store the network parameters into respective buffers and updates these details whenever the dataset commit active command called.
uint8_t values_to_set(char *networkparameter, char *value) {
    uint8_t len = strlen(value)-1;
    uint8_t out=0;
    if (!strcmp(networkparameter,"masterkey")) {
       if (len == 32) {
        string_to_hex(value,masterkey,len);
        out = 1;
       }
    } else if (!strcmp(networkparameter,"panid")) {
       if(len == 4) {
            panid = panid_value(value);
             out = 1;
       }
    } else if (!strcmp(networkparameter,"networkname")) {
       if(len > 1) {
            strcpy((char *)Network_name,value);
            out = 1;
       }
    } else if (!strcmp(networkparameter,"extpanid")) { //giving hardfault
       if(len == 16) {
            string_to_hex(value,extpanid,len);
            out = 1;
       }
    } else if (!strcmp(networkparameter,"channel")) {
        uint8_t ch = atol(value);
        if ((ch >= 11) && (ch <= 26)) {
            channel = ch;
            out = 1;
        }
    } else if (!strcmp(networkparameter,"channelmask")) {
       strcpy((char *)channel_mask,value);
    } else if (!strcmp(networkparameter,"psk")) {
        if(len == 32) {
            string_to_hex(value,psk,len);
            out = 1;
        }
    } else if (!strcmp(networkparameter,"securitypolicy")) {//not updating value
        uint8_t val = atol(value);
        if ((val>=0) && (val<=255)) {
            securitypolicy = val;
            out = 1;
        }
    } else if (!strcmp(networkparameter,"prefix")) {
        if (len == 16) {
            string_to_hex(value,meshprefix,len);
            out = 1;
         }
    }
    return out;
}

/* This function handles the commands
It accepts str as a input parameter.
calling this function to validates the command structure.*/
void commands_list(void) {
    printf("thread..\n");
    printf("info\n");
    printf("dataset..\n");
    printf("get..\n");
    printf("channel\n");
    printf("channelmask\n");
    printf("extpanid\n");
    printf("masterkey\n");
    printf("networkname\n");
    printf("panid\n");
    printf("psk\n");
    printf("prefix\n");
    printf("securitypolicy\n");
}

// This function called with get help command
void getcmds_list(void) {
   printf("channel\n");
   printf("channelmask\n");
   printf("extpanid\n");
   printf("masterkey\n");
   printf("networkname\n");
   printf("panid\n");
   printf("psk\n");
   printf("prefix\n");
   printf("securitypolicy\n");
}

//This function access the commands start with thread keyword
void threadkeyword_cmds(char *cmd, uint8_t len) {
    if (len) {
        if (strncmp(cmd,"start",len) == 0) {//start to establish a connection
            mesh_connect(); //this function connects the device into network
            printf("Done\n");
        } else if (strncmp(cmd,"stop",len) == 0) {//to disconnect from network
            mesh_disconnect(); //this function disconnects the device from the network
            printf("Done\n");
        } else if (strncmp(cmd,"help",len) == 0) {
            printf("start\n"); // print when command format doesn't match
            printf("stop\n");
        }
    }
//  else
//     printf("Invalid command\n");// print when command format doesn't match  
}

void dataset_keyword_cmds(char *networkparameter, char *value, uint8_t netwrkparam_len, uint8_t value_len) {
    uint8_t donebyte = 0;
    if (netwrkparam_len > 0) {
        if (strncmp(networkparameter,"help",netwrkparam_len-1) == 0) { // to list the available commands to start with dataset
            if (value_len == 0)  //nothing is there in 3rd place
                dataset_commands_List();
            else 
                printf("Invalid command\n");// print when command format doesn't match
        }
        if (value_len > 0) {//if some data there in 2rd place of the command
            if (strcmp(networkparameter,"commit") == 0) { // this to commit the dataset as active
                if (strncmp(value,"active",value_len-1) == 0) {     
                    printf("Done\n");
                    datasetcommit_active(); //commit dataset as active
                    getcmd_count = 1;
                }
            } else {  
                donebyte = values_to_set(networkparameter,value); // this function updates the network parameters
                if (donebyte)
                    printf("Done\n");
            }           
        }
    }    
}

//This function prints the networkwork parameter details, 
//This function called only user request for network details with get keyword
void get_keyword_cmds(char *networkparam,uint8_t param_len) {
    if (param_len > 0) {
        if (strncmp((char *)networkparam, "help",param_len-1)==0)
            getcmds_list();
        else {
            if (getcmd_count == 1) {//read only once
                linkcopy = thread_management_configuration_get(id);
                getcmd_count--;    
            }
            get_values(networkparam,param_len-1);
        }  
    }
}
//Have to make it perfect, just implemented for test purpose
// change later

char *coap_server_ipaddr;
void coap_request_cmds(char *str)
{
    SocketAddress addr;
    char *request_method_type, *host_address, *uri_path, *msg_type, *payload;
    uint8_t request_method_type_len, host_address_len, uri_path_len, msg_type_len;
    uint16_t payload_len;
    uint8_t msg_type_con_noncon;
    request_method_type = strtok_r(str," ", &host_address);
    host_address = strtok_r(host_address," ", &uri_path);
    uri_path = strtok_r(uri_path," ", &msg_type);
    msg_type = strtok_r(msg_type," ", &payload);
    request_method_type_len = strlen(request_method_type);
    host_address_len = strlen(host_address);
    uri_path_len = strlen(uri_path);
    msg_type_len = strlen(msg_type);
    payload_len = strlen(payload);

  // coap_server_ipaddr = request_method_type;
    if (strncmp((char *)request_method_type, "get",request_method_type_len) == 0) {
        if(strncmp((char *)msg_type, "con", msg_type_len-1) == 0) {
            msg_type_con_noncon = COAP_MSG_TYPE_CONFIRMABLE;
        } else {
            msg_type_con_noncon = COAP_MSG_TYPE_NON_CONFIRMABLE;
        }
            client_requestpacket_build(host_address,(uint8_t *)uri_path, COAP_MSG_CODE_REQUEST_GET, msg_type_con_noncon, 0, 0);
    } else if (strncmp((char *)request_method_type, "put",request_method_type_len) == 0) {
        if(strncmp((char *)msg_type, "con", msg_type_len) == 0) {
            msg_type_con_noncon = COAP_MSG_TYPE_CONFIRMABLE;
        } else {
             msg_type_con_noncon = COAP_MSG_TYPE_NON_CONFIRMABLE;
        }
            client_requestpacket_build(host_address,(uint8_t *)uri_path, COAP_MSG_CODE_REQUEST_PUT, msg_type_con_noncon, (uint8_t *)payload, payload_len-1);
    } else if (strncmp((char *)request_method_type, "post",request_method_type_len) == 0) {
        if(strncmp((char *)msg_type, "con", msg_type_len) == 0) {
            msg_type_con_noncon = COAP_MSG_TYPE_CONFIRMABLE;
        } else {
             msg_type_con_noncon = COAP_MSG_TYPE_NON_CONFIRMABLE;
        }
            client_requestpacket_build(host_address, (uint8_t *)uri_path,  COAP_MSG_CODE_REQUEST_POST, msg_type_con_noncon, (uint8_t *)payload, payload_len-1);
    } else if (strncmp((char *)request_method_type, "delete",request_method_type_len) == 0) {
        if(strncmp((char *)msg_type, "con", msg_type_len) == 0) {
            msg_type_con_noncon = COAP_MSG_TYPE_CONFIRMABLE;
        } else {
             msg_type_con_noncon = COAP_MSG_TYPE_NON_CONFIRMABLE;
        }
        client_requestpacket_build(host_address, (uint8_t *)uri_path, COAP_MSG_CODE_REQUEST_DELETE, msg_type_con_noncon, (uint8_t *)payload, payload_len-1);
    }    
}

/* This function handles the commands
It accepts str as a input parameter.
calling this function to validates the command structure.*/

//make some changes in the future
void cli_cmds_Handler(char *str)
{
    uint8_t donebyte =0;
    char *first_string,*second_string,*third_string,*fourth_string;
    uint8_t first_string_len=0, second_string_len=0, third_string_len=0, fourth_string_len=0;
    //segregate strings using ' ' character
    first_string =  strtok_r(str," ", &second_string);
    first_string_len = strlen(first_string);
    
    if (strncmp((char *)first_string, "coap",first_string_len) == 0) {
        coap_request_cmds(second_string);
    } else {
        second_string = strtok_r(second_string," ", &third_string);
        third_string =  strtok_r(third_string," ", &fourth_string); //checking whether there is something in 4th place
        first_string_len = strlen(first_string);
        second_string_len = strlen(second_string);
        third_string_len = strlen(third_string);
        fourth_string_len = strlen(fourth_string); //if yes length of the data
        if (fourth_string_len != 0)  {      //if we are not handling command entry in 4th place.
            printf("Invalid no.of Args\n");
        } else {        //if nothing in 4th place, look for matching command
            // This 'if' condition for to connect and discoonect the device into/from the network.
            if (strncmp((char *)first_string, "thread",first_string_len) == 0) { //To connect Mesh network and disconnect
                if (third_string_len == 0) {    //if some text is entered
                    threadkeyword_cmds(second_string,second_string_len-1);
                }
            } else if (strncmp((char *)first_string, "dataset",first_string_len) == 0) {//check entered command starts with dataset and THis condition to set the network parameters
                dataset_keyword_cmds(second_string,third_string,second_string_len,third_string_len);
            } else if (strncmp((char *)first_string, "get",first_string_len) == 0)  { // This condition to get the information about the network.
                if (third_string_len == 0) {
                    get_keyword_cmds(second_string, second_string_len);
                } else {
                    printf("Invalid command\n"); // print when command format doesn't match
                }
            } else {   //check is it better to check str1 is not zero
                if (second_string_len == 0) {  //only excute this when entered command is info
                    first_string_len = first_string_len - 1;
                    if (strncmp((char *)first_string, "info",first_string_len) == 0) {
                        scan_network_details();  //to get information about the
                    } else if (strncmp((char *)first_string, "ipaddr",first_string_len) == 0) {//to get the default network parameters
                        read_ipaddr(); // This condition to read the IP addr 
                    } else if (strncmp((char *)first_string, "dataset",first_string_len)==0) {
                        scan_network_details();     //to get dataset it may be dataset or info command decide later
                    } else {  //to get values
                        if (getcmd_count == 1) {//read only once
                            linkcopy = thread_management_configuration_get(id);
                            getcmd_count--;    
                        }
                        get_values(first_string,first_string_len);
                    }
                } else { //to set values
                    if (third_string_len == 0) {
                        donebyte = values_to_set(first_string,second_string); // this function updates the network parameters
                        if (donebyte)
                            printf("Done\n");
                    } else {
                        printf("Invalid command\n"); // print when command format doesn't match
                    }
                }   
            }
        } 
    }   
}

// Creates memory for the oneset of networkparameters
link_configuration_s *link_configuration_create(void)
{
  link_configuration_s *link = new link_configuration_s;
   if (!link) {
     return NULL;
  }
   memset(link, 0, sizeof(link_configuration_s));
   link->securityPolicy = SECURITY_POLICY_ALL_SECURITY;        // Set all default values ('1') for security policy flags
   link->securityPolicyExt = SECURITY_POLICY_ALL_SECURITY;     // Set all default values
   return link;
}

// Call this function to copy network details 
link_configuration_s *linkconfiguration_copy(link_configuration_s *addlink, link_configuration_s *configuration_ptr)
{
    if (!addlink || !configuration_ptr) {
        return 0;
    }
    memcpy(addlink->name, configuration_ptr->name, 16);
    memcpy(addlink->PSKc, configuration_ptr->PSKc, 16);
    memcpy(addlink->master_key, configuration_ptr->master_key, 16);
    memcpy(addlink->mesh_local_ula_prefix, configuration_ptr->mesh_local_ula_prefix, 8);
    memcpy(addlink->extented_pan_id, configuration_ptr->extented_pan_id, 8);
    memcpy(addlink->channel_mask, configuration_ptr->channel_mask, 5);
    addlink->key_rotation = configuration_ptr->key_rotation;
    addlink->key_sequence = configuration_ptr->key_sequence;
    addlink->panId = configuration_ptr->panId;
    addlink->rfChannel = configuration_ptr->rfChannel;
    addlink->securityPolicy = configuration_ptr->securityPolicy;
    addlink->securityPolicyExt = configuration_ptr->securityPolicyExt;
    addlink->timestamp = configuration_ptr->timestamp;
    return addlink;
}