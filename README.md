# Thread Mesh application for Mbed OS version 5

Below functionalities are implemeted in this application:

-- In this application Mesh network intially configured as a Router and it will change network state from router to child/leader/SED/MED depends on the network

-- Set of cli commands are added to update the network parameters to form a thread network.

-- Get commands are added to get the network information

-- AQM sensors are interfaced with the controller.

-- UDP socket opened on mesh network to talk with coap end-points.

-- COAP server and client interfaces are implemented and tested by inerfacing with ot-cli and rcp boards.

-- Commands are added to coap request methods to get/post/put/delete the information from/to coap server resource file.

-- Interfaced external flash for DFU update.

NOTE : Before playing with any of the cli commands and also with UDP connection, device must be connected to the network using "thread start" command. 

Use This link to find the commandset document (https://vortexiot.sharepoint.com/:w:/r/sites/VortexIoTSharepoint/_layouts/15/Doc.aspx?sourcedoc=%7B6C67CBE8-1529-4E44-B0ED-457687112DE0%7D&file=Features_Implemented_on_Mbed.docx&action=default&mobileredirect=true).

