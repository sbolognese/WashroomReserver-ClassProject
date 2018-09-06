#ifndef _NETWORK_H
#define _NETWORK_H

//Determines if string a is equal to string b
int Network_areEqual(char* a, char* b);

//For threads to create a UDP connection and grab incoming messages
void* Network_listenToUDP(void * val);

void Network_start();

void Network_stop();

void Network_portInit(int id);

#endif