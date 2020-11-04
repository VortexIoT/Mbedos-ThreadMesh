#include "Clicmd.h"
#include "stdio.h"
#include "socket_api.h"
#include "mesh_nvm.h"
#include "mbed.h"
#include "ThreadInterface.h"
mbed::RawSerial pc(USBTX, USBRX,115200);
//UnbufferedSerial pc(USBTX, USBRX,115200);
MeshInterface *mesh;

/*********************  Variable Decalrations START **************************/

/*  Network Parameters */
    uint8_t extpanid[8] = {0xf1, 0xb5, 0xa1, 0xb2,0xc4, 0xd5, 0xa1, 0xbd };
    uint8_t masterkey[16] ={0xa4, 0xb1, 0x14, 0x95, 0xa8, 0x89, 0x5e, 0x6e, 0xc2, 0x93, 0xf5, 0x3d, 0x54, 0x86, 0x18, 0xd6};//{ 0x00, 0xeb, 0x64, 0x37, 0x02, 0x4c, 0x86, 0x8d, 0xdd, 0x2b, 0x18, 0xde, 0x62, 0xc7, 0x98, 0x68};
    uint16_t panid= 0x0700;;
    uint8_t Network_name[16] = "Thread Network";
    uint16_t channel = 22;
    uint8_t meshprefix[8] = {0xfd, 0x0, 0x0d, 0xb8, 0x0, 0x0, 0x0, 0x0};
    uint8_t channel_mask[9] = "07fff800";
    uint8_t psk[16] = MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PSKC;
    uint8_t securitypolicy = 255;
   



link_configuration_s *linkcopy;
link_configuration_s *link;
nwk_interface_id id1 = IF_IPV6;
volatile uint8_t flag=0;
volatile char Rx_buff[256];
uint8_t Receive_buff_length=0;
uint8_t getcdatacnt=0;
uint8_t getcmd_count=1;
SocketAddress sockAddr;



 /*********************  Variable Decalrations END **************************/




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


void init(int x)
{
    printf("test_function add: %d\n",x);

}



int j=0;
/* Receive interrupt function.This function will be automatically called when data entered through the serial terminal*/
void ISR_Rx()
 {
   char databyte=pc.getc();
   
    if (databyte != '\n')
    {
        if(databyte == 0x08)
         j = j-1;
        else
            Rx_buff[j++] = databyte;
    }
    else 
    {
        Receive_buff_length = j;
        Rx_buff[j] = '\0';
        flag = 1;
        j=0;    
    }
}

//Transmit interrupt function
void  Transmit_interrupt(char *Data_buff)
{
  pc.puts(Data_buff);

}
/***************  END ******************/


// This function connects the device into the network and also it return the device Connected IP address
 uint8_t Thread_start(void)
 {
     uint8_t error;
    mesh = MeshInterface::get_default_instance();  //returns pointer to the mesh interface
    if (!mesh) {
        printf("Error! MeshInterface not found!\n");
        return -1;
    }
    thread_eui64_trace();  
    mesh_nvm_initialize();
    printf("Connecting...\n");
    error = mesh->connect();  
       if (error) {
        printf("Connection failed! %d\n", error);
       return error;
    }
    ThisThread::sleep_for(100); 
  
   SocketAddress sockAddr;
   if (NSAPI_ERROR_OK != mesh->get_ip_address(&sockAddr)) //local ip address
   {
        ThisThread::sleep_for(500); 
   }
   printf("Connected IP : %s\n",sockAddr.get_ip_address());
   link = thread_management_configuration_get(id1);
   getcmd_count = 1;
  return 1;

 }
 void Thread_stop(void)
 {
     mesh->disconnect();
 }
// Call this function whenever want to read the IP address of the network device
 void Read_IPaddr(void)
 {
     SocketAddress sockAddr;
   if (NSAPI_ERROR_OK != mesh->get_ip_address(&sockAddr)) //local ip address
   {
        ThisThread::sleep_for(500); 
   }
  printf("Connected IP : %s\n",sockAddr.get_ip_address());
  
 }


// This function will converts char into hex format
uint8_t chartohex(char ch)
{
    if((ch <= '9') && (ch>='0'))
    {
        ch = ch-'0';
    }
    if((ch >= 'A') && (ch <= 'F'))
    {
        ch = ch-'A' + 10;
    }
    if((ch >='a') && (ch <= 'f'))
    ch = ch-'a' + 10;
    return ch;
}
// This function will converts string of required length into hex format.
/*
 This function accepts 3 params
  str = string to convert into hex
output = pointer to store the converted hex data
len = length of data

*/
void string_to_hex(char *str, uint8_t *output,uint8_t len)
{
    int j=0;
    uint8_t ch,ch1;
   
    for(uint8_t i=0;i<len;)
    {
        ch = chartohex(str[i]);
        ch1 =chartohex(str[i+1]);
        output[j] = (ch1&0xff)+((ch&0xff)<<4);
        i +=2;
        j++;     
    }
}
/* This function implemented only for panid.
Calling this function will return 16bit panid value*/ 
uint16_t panid_value(char *str)
{
    uint16_t panid;
    char ch1,ch2,ch3,ch4;
    ch1 = chartohex(str[0]);
    ch2 = chartohex(str[1]);
    ch3 = chartohex(str[2]);
    ch4 = chartohex(str[3]);
    panid =  ((ch1<<12)| (ch2 << 8)| (ch3 <<4)|ch4);
    return panid;
}   

// Call this function to get detailed information about the network parameters
void Scan_Network_details(void)
{

    link = thread_management_configuration_get(id1);
    printf("Panid :0x%04x\nNetwork Name :%s\nChanne1 : %d\n", link->panId, link->name,link->rfChannel);
    printf("Masterkey : ");
    for(int i=0;i< sizeof(link->master_key);i++)                   
        printf("%02x",  link->master_key[i]);printf("\n");
    
    printf("Ext Panid :");
    for(int i=0;i< 8;i++) 
        printf("%02x",    link->extented_pan_id[i]); printf("\n");
        printf("Channel Mask :");
      //  if(link->channel_mask >=channel_mask)
        {
            for(int i=0;i< 8;i++)  
            {
                if(link->channel_mask[i] >= channel_mask[i])
                    printf("%c",link->channel_mask[i]);
                else
                    printf("%c",channel_mask[i]);
            }
             printf("\n");
        }
    printf("PSKc :");
    for(int i=0;i< 16;i++) 
        printf("%x",    link->PSKc[i]); printf("\n");
    printf("Security Policy : %d\n",link->securityPolicy);
    printf("Mesh prefix :");
    for(int i=0;i< 8;i++)  
    {
     if((i>0) && (i%2 == 0))
            printf(":");
       
        printf("%02x",    link->mesh_local_ula_prefix[i]);

     } 
     printf("/64\n");
}

// This function called whenever user enters the dataset commit active command.
void datasetcommit_active(void)
{
    thread_management_set_request_full_nwk_data(id1,true);           
    link->rfChannel = channel;
    for(int i=0;i<sizeof(Network_name);i++)
        link->name[i]= Network_name[i];
    link->panId = panid;
    for(int i=0;i<16;i++)
        link->master_key[i] = masterkey[i];
    for(int i=0;i<8;i++)
       link->extented_pan_id[i] = extpanid[i];
    for(int i=0;i<8;i++)
       link->channel_mask[i] = channel_mask[i];
    for(int i=0;i<8;i++)
       link->mesh_local_ula_prefix[i] = meshprefix[i];
    for(int i=0;i<16;i++)
        link->PSKc[i] = psk[i];
        link->securityPolicy = securitypolicy;
    thread_management_link_configuration_store(id1,link);
                        
}

// This function called when user entered command follows with get * command 
// THis function accepts param as a input string to distinguish between commands
void Get_values(char *param)
{
      uint8_t len=0;
      len = strlen(param) - 1;
    if(!strncmp(param,"masterkey",len))
    {
      //  printf("Masterkey : ");
        for(int i=0;i< sizeof(linkcopy->master_key);i++)                   
            printf("%02x",  linkcopy->master_key[i]);printf("\n");
    }
    else if(!strncmp(param,"panid",len))
    {
        printf("0x%04x\n", linkcopy->panId);
    }
    else if(!strncmp(param,"networkname",len))
    {
        printf("%s\n",linkcopy->name);
    }
    else if(!strncmp(param,"extpanid",len))
    {
           // printf("Ext Panid :");
        for(int i=0;i< 8;i++)  //addlinkconfig->master_key
            printf("%02x",    linkcopy->extented_pan_id[i]); printf("\n");
    }
    else if(!strncmp(param,"channel",len))
    {
        printf("%d\n",linkcopy->rfChannel);
    }
    else if(!strncmp(param,"channelmask",len))
    {
        for(int i=0;i< 8;i++)  //addlinkconfig->master_key
        {
            if(linkcopy->channel_mask[i] >= channel_mask[i])
               printf("%c",linkcopy->channel_mask[i]);
            else
               printf("%c",channel_mask[i]);
        }
                printf("\n");
    }
    else if(!strncmp(param,"psk",len))
    {
     //   printf("PSKc :");
        for(int i=0;i< 16;i++)  //addlinkconfig->master_key
            printf("%x",    linkcopy->PSKc[i]); printf("\n");
    }
    else if(!strncmp(param,"securitypolicy",len))
    {
        printf("%d\n",linkcopy->securityPolicy);
    }
    else if(!strncmp(param,"prefix",len))
    {
     //   printf("Mesh prefix :");
        for(int i=0;i< 8;i++)  //addlinkconfig->master_key
        {
        if((i>0) && (i%2 == 0))
                printf(":");
        
            printf("%02x",    linkcopy->mesh_local_ula_prefix[i]);

        } 
        printf("/64\n");
    }
}
// This function gives the list of commands with prefix of dataset
void Commands_List(void)  //call this function when dataset help command called
{
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
uint8_t Values_to_set(char *str, char *value)
{
    uint8_t len = strlen(value)-1;
    uint8_t out=0;
   if(!strcmp(str,"masterkey"))
   {
       if(len == 32)
       {
        string_to_hex(value,masterkey,len);
        out = 1;
       }
   }
   else if(!strcmp(str,"panid"))
   {
       if(len == 4)
       {
        panid = panid_value(value);
        out = 1;
       }
   }
   else if(!strcmp(str,"networkname"))
   {
       if(len > 1)
       {
        strcpy((char *)Network_name,value);
        out = 1;
       }
   }
   else if(!strcmp(str,"extpanid"))  //giving hardfault
   {
       if(len == 16)
       {
       string_to_hex(value,extpanid,len);
       out = 1;
       }
   }
   else if(!strcmp(str,"channel"))
   {
       uint8_t ch = atol(value);
       if((ch >= 11) && (ch <= 26))
       {
        channel = ch;
        out = 1;
       }
   }
   else if(!strcmp(str,"channelmask"))
   {
       strcpy((char *)channel_mask,value);
   }
   else if(!strcmp(str,"psk"))
   {
       if(len == 32)
       {
        string_to_hex(value,psk,len);
        out = 1;
       }
   }
   else if(!strcmp(str,"securitypolicy")) //not updating value
   {
       uint8_t val = atol(value);
       if((val>=0) && (val<=255)) 
       {
        securitypolicy = val;
        out = 1;
       }
   }
   else if(!strcmp(str,"prefix"))
   {
       if(len == 16)
       {
        string_to_hex(value,meshprefix,len);
        out = 1;
       }
   }
   return out;
}
/* This function handles the commands

It accepts str as a input parameter.
calling this function to validates the command structure.
void Clicmd_format_making(char *str)
{
    char *str1,*str2,*str3,*str4; 
    uint8_t strlen1 = 0,strlen2 = 0,strlen3 = 0,strlen4=0;
    uint8_t donebyte =0;
    str1=  strtok_r(str," ", &str2);
    strlen1 = strlen(str1);
    strlen2 = strlen(str2);
// This 'if' condition for to connect and discoonect the device into/from the network.
    if(!strncmp((char *)str1, "thread",strlen1-1))//To connect Mesh network and disconnect
    {   
        str2 = strtok_r(str2," ",&str3);
        strlen2 = strlen(str2);
        strlen3 = strlen(str3);
        if(!strncmp(str2,"start",strlen2-1)) //start to establish a connection
        {
            if(!strlen3)  //nothing is there in 3rd place
            {
                mesh_connection(); //this function connects the device into network
                printf("Done\n");
            }
                    
            else {
                printf("Invalid command\n"); // print when command format doesn't match
            }  
        }
        else if(!strncmp(str2,"stop",strlen2-1)) //to disconnect from network
        {
            if(!strlen3)
            {
                mesh->disconnect(); //this function disconnects the device from the network
                printf("Done\n");
            }
            else {
                printf("Invalid command\n");// print when command format doesn't match
            }
        }
        else {
        printf("Invalid command\n");// print when command format doesn't match
        }
       
    }
    // This condition to get the information about the network.
    else if(!strncmp((char *)str1, "info",strlen1-1))//to get the default network parameters
    {   
        if(!strlen2)    //only excute this when entered command is info
        {
            Scan_Network_details();  //to get information about the
           // link = thread_management_configuration_get(id1);
          //  linkcopy=link_configuration_create();   //created mem space for addlink here
          //  linkconfiguration_copy(linkcopy,link); //copying actual parameters to created mem space
        }
        else 
            printf("Invalid command\n"); // print when command format doesn't match
       
    }
    // This condition to read the IP addr
    else if(!strncmp((char *)str1, "ipaddr",strlen1-1))//to get the default network parameters
    {   
        if(!strlen2)    //only excute this when entered command is info
        {
            Read_IPaddr();
        }     
    }
    // THis condition to set the network parameters
    else if(!strncmp((char *)str1, "dataset",strlen1)) //check entered command starts with dataset
    {
        if(strlen2)
        {
        //    printf("hi\n");
            str2 = strtok_r(str2," ",&str3);
            strlen2 = strlen(str2);
            strlen3 = strlen(str3);
            if(!strncmp(str2,"help",strlen2-1))  // to list the available commands to start with dataset
            {
            //    printf("help\n");
                if(!strlen3)  //nothing is there in 3rd place
                {
                    // printf("print all dataset commands list");  //print all  dataset commands list
                    Commands_List();
                }
                    
                else {
                   printf("Invalid command\n");// print when command format doesn't match
                }  
            }
            if(strlen3) //if some data there in 2rd place of the command
            {
                str3 = strtok_r(str3," ",&str4);
                strlen4 = strlen(str4);
                if(!strlen4)
                {
                     if(!strcmp(str2,"commit"))  // this to commit the dataset as active
                    {
                        if(!strncmp(str3,"active",strlen3-1))
                        {
                           
                            printf("Done\n");
                             datasetcommit_active(); //commit dataset as active
                             getcmd_count = 1;
                        }
                    }
                    else
                     {  
                    
                        donebyte = Values_to_set(str2,str3); // this function updates the network parameters
                        if(donebyte)
                        printf("Done\n");
                    }
                }
                    
                else {
                    printf("Invalid no.of Args\n"); // print when command format doesn't match
                }
            }
        }
        else {
          //  Scan_Network_details();     //to get dataset it may be dataset or info command decide later
           printf("Invalid command\n");// print when command format doesn't match
        }
    }
   
    else if(!strncmp((char *)str1, "get",strlen1)) //check entered command starts with get
    {
        str2 = strtok_r(str2," ",&str3);
        strlen2 = strlen(str2);
        strlen3 = strlen(str3);
        if(!strlen3)
        {
            if(strlen2 > 0)
            {
                if(getcmd_count == 1) //read only once
                {
                    linkcopy = thread_management_configuration_get(id1);
                    getcmd_count--;    
                }
                Get_values(str2);
            }
        }
        else
            printf("Invalid command\n"); // print when command format doesn't match
    } 
    else {
        printf("Invalid command\n"); // print when command format doesn't match
    }
}*/

void Commands_list(void)
{
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
void getcmds_list(void)
{
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



/* This function handles the commands

It accepts str as a input parameter.
calling this function to validates the command structure.*/
void Clicmd_format_making(char *str)
{
    char *str1,*str2,*str3,*str4; 
    uint8_t strlen1 = 0,strlen2 = 0,strlen3 = 0,strlen4=0;
    uint8_t donebyte =0;
 //   strlen1 = strlen(str);
    //    if(strlen1 <= 1) {
    //printf("> \n");
    //}
  //  if(strlen1 > 1)
  //else
    {
        str1=  strtok_r(str," ", &str2);
        strlen1 = strlen(str1);
        strlen2 = strlen(str2);
      //  printf("%s: %s: %d  %d:-\n", str1,str2,strlen1,strlen2);

    // This 'if' condition for to connect and discoonect the device into/from the network.
        if(strncmp((char *)str1, "thread",strlen1) == 0)//To connect Mesh network and disconnect
        {   
            str2 = strtok_r(str2," ",&str3);
            strlen2 = strlen(str2);
            strlen3 = strlen(str3);
            if(strlen3 == 0) //if some text is entered
            {
                if(strlen2)
                {
                    if(strncmp(str2,"start",strlen2-1) == 0) //start to establish a connection
                    {
                        Thread_start(); //this function connects the device into network
                        printf("Done\n");
                    }
                    else if(strncmp(str2,"stop",strlen2-1) == 0) //to disconnect from network
                    {
                        Thread_stop(); //this function disconnects the device from the network
                        printf("Done\n");
                    }
                    else if(strncmp(str2,"help",strlen2-1) == 0)
                    {
                        printf("start\n"); // print when command format doesn't match
                        printf("stop\n");
                    }
                }
            else
                printf("Invalid command\n");// print when command format doesn't match
            }           
        }    
        // THis condition to set the network parameters
        else if(strncmp((char *)str1, "dataset",strlen1) == 0) //check entered command starts with dataset
        {
            if(strlen2 > 0)
            {
            //    printf("hi\n");
                str2 = strtok_r(str2," ",&str3);
                strlen2 = strlen(str2);
                strlen3 = strlen(str3);
                if(strncmp(str2,"help",strlen2-1) == 0)  // to list the available commands to start with dataset
                {
                //    printf("help\n");
                    if(strlen3 == 0)  //nothing is there in 3rd place
                    {
                        Commands_List();
                    }
                        
                    else {
                    printf("Invalid command\n");// print when command format doesn't match
                    }  
                }
                if(strlen3 > 0) //if some data there in 2rd place of the command
                {
                    str3 = strtok_r(str3," ",&str4);
                    strlen4 = strlen(str4);
                    if(strlen4 == 0)
                    {
                        if(strcmp(str2,"commit") == 0)  // this to commit the dataset as active
                        {
                            if(strncmp(str3,"active",strlen3-1) == 0)
                            {
                            
                                printf("Done\n");
                                datasetcommit_active(); //commit dataset as active
                                getcmd_count = 1;
                            }
                        }
                        else
                        {  
                        
                            donebyte = Values_to_set(str2,str3); // this function updates the network parameters
                            if(donebyte)
                            printf("Done\n");
                        }
                    }    
                    else {
                        printf("Invalid no.of Args\n"); // print when command format doesn't match
                    }
                }
            }

        }
        else if(strncmp((char *)str1, "get",strlen1) == 0) //check entered command starts with get
        {
            str2 = strtok_r(str2," ",&str3);
            strlen2 = strlen(str2);
            strlen3 = strlen(str3);
            if(strlen3 == 0)
            {
                if(strlen2 > 0)
                {
                   if(strncmp((char *)str2, "help",strlen2-1)==0)
                        getcmds_list();
                    else 
                    {
                        if(getcmd_count == 1) //read only once
                        {
                            linkcopy = thread_management_configuration_get(id1);
                            getcmd_count--;    
                        }
                        Get_values(str2);
                    }
                }
            }
            else {
                printf("Invalid command\n"); // print when command format doesn't match
            }
        } 
        // This condition to get the information about the network.
        else
        {   //check is it better to check str1 is not zero
            if(strlen2 == 0)    //only excute this when entered command is info
            {
                strlen1 = strlen1 - 1;
           //     printf("len :%d",strlen1);
                if(strncmp((char *)str1, "info",strlen1) == 0)
                {
                    Scan_Network_details();  //to get information about the
                }
                else if(strncmp((char *)str1, "ipaddr",strlen1) == 0)//to get the default network parameters
                    Read_IPaddr(); // This condition to read the IP addr 
                else if(strncmp((char *)str1, "dataset",strlen1)==0)
			        Scan_Network_details();     //to get dataset it may be dataset or info command decide later
             /*   else if(strncmp((char *)str1, "get",strlen1)==0)
                    getcmds_list();
                else if(strncmp((char *)str1, "thread",strlen1)==0)
                {
                    printf("start\n"); // print when command format doesn't match
                    printf("stop\n");
                }*/
                else 
                {  //to get values
                    if(getcmd_count == 1) //read only once
                    {
                        linkcopy = thread_management_configuration_get(id1);
                        getcmd_count--;    
                    }
                    Get_values(str1);
                }
            }
            else 
            {
                str2 = strtok_r(str2," ",&str3);
                strlen3 = strlen(str3);
                if(strlen3 == 0)
                {
                    donebyte = Values_to_set(str1,str2); // this function updates the network parameters
                    if(donebyte)
                        printf("Done\n");
                }
                else
                    printf("Invalid command\n"); // print when command format doesn't match
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