

/*void Clicmd_format_making(char *str)
{
    char *keyword,*networkparameter,*value,*str4; 
    uint8_t keyword_len = 0,networkparameter_len = 0,strlen3 = 0,strlen4=0;
    uint8_t donebyte =0;

    char *first_string,*second_string,*third_string,*fourth_string;
    uint8_t first_string_len=0, second_string_len=0, third_string_len=0, fourth_string_len=0;

    {
        first_string =  strtok_r(str," ", &second_string);
        second_string = strtok_r(second_string," ", &third_string);
        third_string =  strtok_r(third_string," ", &fourth_string);
        first_string_len = strlen(first_string);
        second_string_len = strlen(second_string);
        third_string_len = strlen(third_string);
        fourth_string_len = strlen(fourth_string);
    
    if(fourth_string_len != 0)
    {
        printf("Invalid no.of Args\n");
    }
    else {

        keyword=  strtok_r(str," ", &networkparameter);
        keyword_len = strlen(keyword);
        networkparameter_len = strlen(networkparameter);
      //  printf("%s: %s: %d  %d:-\n", str1,str2,strlen1,strlen2);

    // This 'if' condition for to connect and discoonect the device into/from the network.
        if(strncmp((char *)keyword, "thread",keyword_len) == 0)//To connect Mesh network and disconnect
        {   
            networkparameter = strtok_r(networkparameter," ",&value);
            networkparameter_len = strlen(networkparameter);
            strlen3 = strlen(value);
            if(third_string_len == 0) //if some text is entered
            {
                threadkeyword_cmds(second_string,second_string_len-1);
                if(networkparameter_len)
                {
                    if(strncmp(networkparameter,"start",networkparameter_len-1) == 0) //start to establish a connection
                    {
                        Thread_start(); //this function connects the device into network
                        printf("Done\n");
                    }
                    else if(strncmp(networkparameter,"stop",networkparameter_len-1) == 0) //to disconnect from network
                    {
                        Thread_stop(); //this function disconnects the device from the network
                        printf("Done\n");
                    }
                    else if(strncmp(networkparameter,"help",networkparameter_len-1) == 0) //help to get list of commands
                    {
                        printf("start\n"); 
                        printf("stop\n");
                    }
                }
            else
                printf("Invalid command\n");// print when command format doesn't match
            }           
        }    
        // THis condition to set the network parameters
        else if(strncmp((char *)keyword, "dataset",keyword_len) == 0) //check entered command starts with dataset
        {
            if(networkparameter_len > 0)
            {
                networkparameter = strtok_r(networkparameter," ",&value);
                networkparameter_len = strlen(networkparameter);
                strlen3 = strlen(value);
                if(strncmp(networkparameter,"help",networkparameter_len-1) == 0)  // to list the available commands to start with dataset
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
                    value = strtok_r(value," ",&str4); //check if there is any data entered in 4th place
                    strlen4 = strlen(str4);
                    if(strlen4 == 0) //if no check for below set of commands
                    {
                        if(strcmp(networkparameter,"commit") == 0)  // this to commit the dataset as active
                        {
                            if(strncmp(value,"active",strlen3-1) == 0)
                            {
                            
                                printf("Done\n");
                                datasetcommit_active(); //commit dataset as active
                                getcmd_count = 1;
                            }
                        }
                        else
                        {  
                            donebyte = Values_to_set(networkparameter,value); // this function updates the network parameters
                            if(donebyte)
                            printf("Done\n");
                        }
                    }    
                    else {  //if something in 4th place reply back with this
                        printf("Invalid no.of Args\n"); // print when command format doesn't match
                    }
                }
            }

        }
        else if(strncmp((char *)keyword, "get",keyword_len) == 0) //check entered command starts with get
        {
            networkparameter = strtok_r(networkparameter," ",&value);
            networkparameter_len = strlen(networkparameter);
            strlen3 = strlen(value);
            if(strlen3 == 0)
            {
                if(networkparameter_len > 0)
                {
                   if(strncmp((char *)networkparameter, "help",networkparameter_len-1)==0)
                        getcmds_list();
                    else 
                    {
                        if(getcmd_count == 1) //read only once
                        {
                            linkcopy = thread_management_configuration_get(id);
                            getcmd_count--;    
                        }
                        Get_values(networkparameter);
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
            if(networkparameter_len == 0)    //only excute this when entered command is info
            {
                keyword_len = keyword_len - 1;
           //     printf("len :%d",strlen1);
                if(strncmp((char *)keyword, "info",keyword_len) == 0)
                {
                    Scan_Network_details();  //to get information about the
                }
                else if(strncmp((char *)keyword, "ipaddr",keyword_len) == 0)//to get the default network parameters
                    Read_IPaddr(); // This condition to read the IP addr 
                else if(strncmp((char *)keyword, "dataset",keyword_len)==0)
			        Scan_Network_details();     //to get dataset it may be dataset or info command decide later
                else 
                {  //to get values
                    if(getcmd_count == 1) //read only once
                    {
                        linkcopy = thread_management_configuration_get(id);
                        getcmd_count--;    
                    }
                    Get_values(keyword);
                }
            }
            else 
            {
                networkparameter = strtok_r(networkparameter," ",&value);
                strlen3 = strlen(value);
                if(strlen3 == 0)
                {
                    donebyte = Values_to_set(keyword,networkparameter); // this function updates the network parameters
                    if(donebyte)
                        printf("Done\n");
                }
                else
                    printf("Invalid command\n"); // print when command format doesn't match
            }   
        }
    }
    }
}*/