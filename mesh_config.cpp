
#include "mesh_config.h"
#include "socket_api.h"
#include "InternetSocket.h"
#include "ThreadInterface.h"
#include "cli_cmd.h"

extern nwk_interface_id id;// = IF_IPV6;
//This function generates the EUI64
void thread_eui64_trace()
{
    #define LOWPAN 1
    #define THREAD 2
    #if MBED_CONF_NSAPI_DEFAULT_MESH_TYPE == THREAD && (MBED_VERSION >= MBED_ENCODE_VERSION(5,10,0))
    uint8_t eui64[8] = {0};
    static_cast<ThreadInterface*>(mesh)->device_eui64_get(eui64);
    printf("Device EUI64 address = %02x%02x%02x%02x%02x%02x%02x%02x\n", eui64[0], eui64[1], eui64[2], eui64[3], eui64[4], eui64[5], eui64[6], eui64[7]);
    #endif
}

// This function connects the device into the network and also it return the device Connected IP address
uint8_t mesh_connect(void)
{
     uint8_t error;
    mesh = MeshInterface::get_default_instance();  //returns pointer to the mesh interface
    if (!mesh) {
        printf("Error! MeshInterface not found!\n");
        return -1;
    }
    thread_eui64_trace();  //This function generates the EUI64
  //  mesh_nvm_initialize();  //initializes the non-volatile memory
    printf("Connecting...\n");
    error = mesh->connect();  
       if (error) {
        printf("Connection failed! %d\n", error);
       return error;
    }
    //ThisThread::sleep_for(100); //
  
   SocketAddress sockAddr;
   while (NSAPI_ERROR_OK != mesh->get_ip_address(&sockAddr)) //local ip address
   {
        ThisThread::sleep_for(500); //keep this loop on until get the IP address with offering 500ms for each turn  
   }
   printf("Connected IP : %s\n",sockAddr.get_ip_address());
   link = thread_management_configuration_get(id);
   thread_management_set_link_timeout(id,80);
   getcmd_count = 1;
  return 1;

}

 //Disconnects the device from network
void mesh_disconnect(void)
{
    mesh->disconnect();
}
// Call this function whenever want to read the IP address of the network device
void read_ipaddr(void)
{
    SocketAddress sockAddr;
    while (NSAPI_ERROR_OK != mesh->get_ip_address(&sockAddr)) {//local ip address
        ThisThread::sleep_for(500); 
    }
    printf("Connected IP : %s\n",sockAddr.get_ip_address());
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
            nwparams_change_identification = 1;
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
            nwparams_change_identification = 1;
            channel = ch;
            out = 1;
        }
    } else if (!strcmp(networkparameter,"channelmask")) {
       strcpy((char *)channel_mask,value);
       nwparams_change_identification = 1;
       out = 1;
    } else if (!strcmp(networkparameter,"psk")) {
        if(len == 32) {
            string_to_hex(value,psk,len);
            nwparams_change_identification = 1;
            out = 1;
        }
    } else if (!strcmp(networkparameter,"securitypolicy")) {//not updating value
        uint8_t val = atol(value);
        if ((val>=0) && (val<=255)) {
            securitypolicy = val;
            nwparams_change_identification = 1;
            out = 1;
        }
    } else if (!strcmp(networkparameter,"prefix")) {
        if (len == 16) {
            string_to_hex(value,meshprefix,len);
            nwparams_change_identification = 1;
            out = 1;
         }
    }
    return out;
}
//This function updates the network parameters
void datasetcommit_active(void) {
    int ret=2;
    link_configuration *locallink = new link_configuration;
    memset(locallink, 0, sizeof(link_configuration_s));
    if( ( keeping_nw_default_details == 1 ) || ( nwparams_change_identification == 1 ) )
    {
     //   printf("dataset.......................\n");
        nwparams_change_identification = 0;
        locallink->rfChannel = channel;
        memcpy(locallink->name, Network_name, 16);
        memcpy(locallink->PSKc, psk, 16);
        memcpy(locallink->master_key, masterkey, 16);
        memcpy(locallink->mesh_local_ula_prefix, meshprefix, 8);
        memcpy(locallink->extented_pan_id,extpanid, 8);
        memcpy(locallink->channel_mask, "07fff800", 8);
        locallink->panId = panid;
        locallink->securityPolicy = securitypolicy;        // Set all default values ('1') for security policy flags
        locallink->securityPolicyExt = SECURITY_POLICY_ALL_SECURITY;     // Set all default values
        locallink->key_sequence = local_key_sequence + 1;  //check this later
        locallink->key_rotation = local_key_rotation;
        locallink->channel_page = local_channel_page;
        locallink->timestamp = 0x10000; //test purpose
        locallink->version = 0;
        keeping_nw_default_details = 1;
    } else {
    //   if(keeping_nw_default_details != 1)
        {
            memcpy(locallink->master_key, masterkey, 16);
        //  memcpy(locallink->mesh_local_ula_prefix, meshprefix, 8);
            locallink->panId = panid;
        }
    }
    ret = thread_management_link_configuration_store( id, locallink );   
  //  if(ret == 0)
  //  ack_frm_server_to_client();    
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