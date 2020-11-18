/*
#include "mesh_config.h"
#include "socket_api.h"
#include "InternetSocket.h"
#include "ThreadInterface.h"


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

void datasetcommit_active(void)
{
    thread_management_set_request_full_nwk_data(id,true);           
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
                        
}*/