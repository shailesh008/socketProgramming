/*

 * client.c

 *
 *  Created on: 20-Feb-2018
 *      Author: Shailesh
 *			Stud ID: 00001402507
 */

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

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<time.h>
#include<stdint.h>
struct Datapacket{
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

// function to load the packet with the data
struct Datapacket initialise() {
	struct Datapacket data;
	data.packetID = PACKETID;
	data.clientID = CLIENTID;
	data.type = DATATYPE;
	data.endpacketID = ENDPACKETID;
	return data;
}
// function to print the packet contents
void print(struct Datapacket data) {
	printf(" packetID: %x\n",data.packetID);
	printf("Client id : %hhx\n",data.clientID);
	printf("data: %x\n",data.type);
	printf("Segment no : %d \n",data.segment_No);
	printf("length %d\n",data.length);
	printf("payload: %s\n",data.payload);
	printf("end of datapacket id : %x\n",data.endpacketID);
}

int main(){

	struct Datapacket data;
	struct rejectpacket recievedpacket;
	struct sockaddr_in cliaddr;
	socklen_t addr_size;
	FILE *fp;
	char line[255];
	int sockfd;
	int n = 0;
	int counter = 0;
	int segmentNo = 1;


	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0) {
		printf("socket failed\n");
	}

	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port=htons(PORT);
	addr_size = sizeof cliaddr ;

	// Configuring the socket to timeout in 3 secs.
	struct timeval tv;
	tv.tv_sec = TIMEOUT;  /* 3 Secs Timeout */
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	data = initialise();
	fp = fopen("input.txt", "rt");
	if(fp == NULL)
	{
		printf("cannot open file\n");
		exit(0);
	}
	while(fgets(line, sizeof(line), fp) != NULL) {
		n = 0;
		counter = 0;
		printf("%s",line);
		data.segment_No = segmentNo;
		strcpy(data.payload,line);
		data.length = strlen(data.payload);
		if(segmentNo == 7) {
			data.length++;
		}
		if(segmentNo == 9) {
			data.segment_No = data.segment_No + 4;
		}
		if(segmentNo == 6) {
			data.segment_No = 3;
		}
		if(segmentNo == 8) {
			data.endpacketID= 0;
		}
		if(segmentNo != 8) {
			data.endpacketID = ENDPACKETID;
		}

		print(data);
		while(n <= 0 && counter < 3) {
			sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
			n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);

			if(n <= 0 ) {
				printf("No response from server for three seconds sending the packet again\n");
				counter ++;
			}
			else if(recievedpacket.type == ACKPACKET  ) {
				printf("Ack packet recieved \n \n \n");
				//}
			}
			else if(recievedpacket.type == REJECTPACKETCODE ) {
				printf("Reject Packet recieved \n");
				printf("type : %x \n" , recievedpacket.subcode);
				if(recievedpacket.subcode == LENGTHMISMATCHCODE ) {
					printf("Length mismatch error\n");
				}
				else if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) {
					printf("END OF PACKET IDENTIFIER MISSING \n");
				}
				else if(recievedpacket.subcode == OUTOFSEQUENCECODE ) {
					printf("OUT OF SEQUENCE ERROR \n");
				}
				else if(recievedpacket.subcode == DUPLICATECODE) {
					printf("DUPLICATE PACKET RECIEVED BY THE SERVER \n");
				}
			}
		}


		if(counter >= 3 ) {
			printf("Server does not respond");
			exit(0);
		}
		segmentNo++;
		printf("--------------------------------------------------------------------------------------------------------------\n");
	}
}
