/*

 * server.c
 *
 *  Created on: 20-Feb-2018
 *      Author: Shailesh
 *			Stud ID: 00001402507
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<strings.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>
#define PORT 32000
#define PACKETID 0XFFFF
#define CLIENTID 0XFF
#define DATATYPE 0XFFF1
#define ENDPACKETID 0XFFFF
#define TIMEOUT 3
#define ACKPACKET 0XFFF2
#define REJECTPACKETCODE 0XFFF3
#define LENGTHMISMATCHCODE 0XFFF5
#define ENDPACKETIDMISSINGCODE 0XFFF6
#define OUTOFSEQUENCECODE 0XFFF4
#define DUPLICATECODE 0XFFF7
struct datapacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_No;
	uint16_t endpacketID;
};
// function to print the packet
void show(struct datapacket data) {
	printf("Received the following:\n");
	printf(" packetID: %hx\n",data.packetID);
	printf("Client id : %hhx\n",data.clientID);
	printf("data: %x\n",data.type);
	printf("Segment no : %d\n",data.segment_No);
	printf("length %d\n",data.length);
	printf("payload: %s\n",data.payload);
	printf("end of packet id : %x\n",data.endpacketID);
}
// function to generate the reject packet
struct rejectpacket generaterejectpacket(struct datapacket data) {
	struct rejectpacket reject;
	reject.packetID = data.packetID;
	reject.clientID = data.clientID;
	reject.segment_No = data.segment_No;
	reject.type = REJECTPACKETCODE;
	reject.endpacketID = data.endpacketID;
	return reject;
}
// function to generate the ack packet
struct ackpacket generateackpacket(struct datapacket data) {
	struct ackpacket ack;
	ack.packetID = data.packetID;
	ack.clientID = data.clientID;
	ack.segment_No = data.segment_No;
	ack.type = ACKPACKET ;
	ack.endpacketID = data.endpacketID;
	return ack;
}

int main(int argc, char**argv)
{
	int sockfd,n;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	struct datapacket data;
	struct ackpacket  ack;
	struct rejectpacket reject;

	// to store what all packets recieved.
	int buffer[20];
	for(int j = 0; j < 20;j++) {
		buffer[j] = 0;
	}
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	int expectedPacket = 1;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(PORT);
	bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
	addr_size = sizeof serverAddr;
	printf("Server started \n");
	for(; ;) {
		// recieving the packet from client
		n = recvfrom(sockfd,&data,sizeof(struct datapacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		printf("New : \n");
		show(data);
		buffer[data.segment_No]++;
		if(data.segment_No == 10 || data.segment_No == 11) {
			buffer[data.segment_No] = 1;
		}
		int length = strlen(data.payload);

		if(buffer[data.segment_No] != 1) {
			reject = generaterejectpacket(data);
			reject.subcode = DUPLICATECODE;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("DUPLICATE PACKET RECIEVED! \n\n");
		}

		else if(length != data.length) {
			reject = generaterejectpacket(data);

			reject.subcode = LENGTHMISMATCHCODE ;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("LENGTH MISMATCH ERROR! \n\n");
		}
		else if(data.endpacketID != ENDPACKETID ) {
			reject = generaterejectpacket(data);

			reject.subcode = ENDPACKETIDMISSINGCODE ;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("END OF PACKET IDENTIFIER MISSING\n\n");

		}
		else if(data.segment_No != expectedPacket && data.segment_No != 10 && data.segment_No != 11) {
			reject = generaterejectpacket(data);

			reject.subcode = OUTOFSEQUENCECODE;
			sendto(sockfd,&reject,sizeof(struct rejectpacket),0,(struct sockaddr *)&serverStorage,addr_size);

			printf("OUT OF SEQUENCE ERROR \n\n");
		}
		else {
			if(data.segment_No == 10) {
				sleep(7);
			}
			ack = generateackpacket(data);
			sendto(sockfd,&ack,sizeof(struct ackpacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		expectedPacket++;
		printf("----------------------------------------------------------------------\n");
	}
}
