
//Code re-used from assignment 2 solution
#define MSG_MAX_LEN 1000
// #define PORT 12345
#define PORT_COUNT 2
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include "network.h"
#include "washroom.h"
#include "stall.h"

static int PORT;
static int washroomNumber;
static pthread_t networkThread;


//a looping function run on a thread that creates a udp connection and grabs incoming messages
//sends out associated messages based on the sent commands
void* Network_listenToUDP(void * val){
	char message[MSG_MAX_LEN];

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(PORT);

	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	while(1){
		unsigned int sin_len = sizeof(sin);
		int bytesRx = recvfrom(socketDescriptor, message, MSG_MAX_LEN, 0, (struct sockaddr *) &sin, &sin_len);
		fflush(stdout);
		char *token = strtok(message, " ");
		if(washroomNumber == atoi(token)){
			token = strtok(NULL, " ");
			if (Network_areEqual(token, "stats")){
				
				int array[Washroom_getTotalStallCount()];
				Washroom_getStallStates(array);
				char* stats = malloc(sizeof(array)*sizeof('a')*2 + 1);
				int k = 0;
				for(int i=0; i< Washroom_getTotalStallCount()*2; i+=2){
					stats[i] = array[k] + 48;
					stats[i+1] = ' ';//separate data by blank space
					k++;
					if(i==(Washroom_getTotalStallCount()*2)-2){
						stats[i+1] = '\0';
					}
				}
				sprintf(message, "%d_#statResult_%s", washroomNumber, stats);
				sin_len = sizeof(sin);
				sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				free(stats);
			}  else if (Network_areEqual(token, "reserve")){
				int ok = Washroom_reserveStall();
				if(!ok){
					sprintf(message, "%d_#warning-box_", washroomNumber);
				} else {
					sprintf(message, "%d_#success-box_", washroomNumber);
				}
				sin_len = sizeof(sin);
				sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
			
			} else if (Network_areEqual(token, "getOpenStalls")){
				int numOpen = Washroom_getOpenStallCount();
				sprintf(message, "%d_#openStalls_%d", washroomNumber, numOpen);
				sin_len = sizeof(sin);
				sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
			
			} else if (Network_areEqual(token, "getTotalStalls")){
				int total = Washroom_getTotalStallCount();
				sprintf(message, "%d_#totalStalls_%d", washroomNumber ,total);
				sin_len = sizeof(sin);
				sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
			 
			 } else if (Network_areEqual(token, "getMaintenance")){
				int array[Washroom_getTotalStallCount()];
				Washroom_getStallStates(array);
				char* retVal= malloc(sizeof(array)*sizeof('a')*2 + 1);
				int i = 0;
				for(int k=0; k< Washroom_getTotalStallCount(); k++){
					if(array[k]==2){
						retVal[i] = k + 48;
						retVal[i+1] = ' ';//separate data by blank space
						i+=2;
					}
				}
				retVal[i/2+1]='\0';
				sprintf(message, "%d_MAINTENANCE_%s", washroomNumber,retVal);
				sin_len = sizeof(sin);
				sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
			 	
			 } else {
				 	if(Network_areEqual(token, "adminPass")){
				 		token = strtok(NULL, " ");
				 		if (Washroom_verifyPassword(atoi(token))){
				 			sprintf(message, "%d_PASSWORD_OK", washroomNumber);
							sin_len = sizeof(sin);
							sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				 		} else {
				 			sprintf(message, "%d_PASSWORD_BAD", washroomNumber);
							sin_len = sizeof(sin);
							sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
				 		}
				 	} else if (Network_areEqual(token, "serviceRel")) {
				 		token = strtok(NULL, " ");
				 		//very weird bug here. token currenly holds 2 numbers when 1 is expected. ie if we send back "serviceRel 1" token holds 11 here. I dont know why.
					 	
					 	int stallNum = token[0] - '0';
					 	Washroom_releaseStallMaintenance(stallNum);
					 	sprintf(message, "%d_#info-box-service-complete_%s", washroomNumber, "OK");
						sin_len = sizeof(sin);
						sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);

				 	} else if (Network_areEqual(token, "serviceReq")){
				 		token = strtok(NULL, " ");
					 	int stallNum = atoi(token);

					 	Washroom_setStallMaintenance(stallNum);
						sprintf(message, "%d_#info-box_%s", washroomNumber, "OK");
						sin_len = sizeof(sin);
						sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *) &sin, sin_len);
					}

				}
		}	
		message[bytesRx] = 0;
		//printf("Message received(%d bytes): %s\n", bytesRx, message);

	}
	return NULL;
}

void Network_start(){
	pthread_create(&networkThread, NULL, *Network_listenToUDP, NULL);
}

void Network_portInit(int id) { 
	washroomNumber = id;
	if(id == 0) {
		PORT = 12345;
	} else {
		PORT = 12346;
	}
}

void Network_stop(){
	pthread_join(networkThread, NULL);
}

//a helper function to determine if a string a is identical to a string b
int Network_areEqual(char* a, char* b){
	int lenA = strlen(a);
	int lenB = strlen(b);
	if (lenA < lenB) {
		return 0;
	}
	int i=0;
	while(i<lenA && i<lenB){
		if(a[i]==b[i]){
			i++;
		} else {
			return 0;
		}
	}
	return 1;
}