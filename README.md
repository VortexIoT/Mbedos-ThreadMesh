# Thread Mesh application for Mbed OS version 5

Below functionalities are implemeted in this application:

-- In this application Mesh network configured as a Router.

-- Set of cli commands are added to update the network parameters to form a thread network.

-- Temperature sensor interfaced with the controller using I2C bus.

-- UDP socket opened on mesh network.

-- CoAP communication implemented on mesh network.

NOTE : Before playing with any of the cli commands and also with UDP connection device must be connected to the network using "thread start" command. 

Use This link to find the commandset document (https://vortexiot.sharepoint.com/:w:/r/sites/VortexIoTSharepoint/_layouts/15/Doc.aspx?sourcedoc=%7B6C67CBE8-1529-4E44-B0ED-457687112DE0%7D&file=Features_Implemented_on_Mbed.docx&action=default&mobileredirect=true).

This board acts both as a server as well as client, to configure as a server please press 2.

To bind mbed board with a specific client, use below command:

    coap ipaddress a and press enter. 

here ipaddress -> client ip address.
'a' is just a character used for test purpose. # needs to be change in future


coap client commands:

coap get ipadress uri-path msg_type
coap post ipaddress uri-path msg_type value

To get temperature sensor data: coap get ipaddress sensor/temp/value
To post temperature scaling factor: coap post ipaddress sensor/temp/scaling

To get humidity sensor data: coap get ipaddress sensor/hum/value
To post humidity scaling factor: coap post ipaddress sensor/hum/scaling

To interface with gateway please use libcoap commands to read sensor data.





