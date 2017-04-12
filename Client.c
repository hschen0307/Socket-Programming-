
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include<inttypes.h>
#include<string.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512
#define PORT 8888


struct dataPocket buildPocket();*/
char* BuildCase(int caseNum, int* wholeSize);
int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	int receiveLen;
	//char * message = NULL;
	WSADATA wsa;


	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
	int currentSeq = 0;
	int count = 0;
	int serVerNot = 0;
	//start communication
	while (1)
	{
		int input = 7;
		if (currentSeq != 7)
		{
			if (serVerNot == 1)
			{
				printf("Server does not respond.\n");
				serVerNot = 0;
			}
			printf("Enter caseNum : ");
			scanf("%d", &input);
			currentSeq = input;
		}
		else
		{
			if (count == 2)
			{
				currentSeq = 0;
				count++;
				printf("RESENDING...\n");
				printf("%d \n", count);
				count = 0;
				serVerNot = 1;
			}
			else
			{
				count++;
				printf("RESENDING...\n");
				printf("%d \n", count);
			}
		}
		//onePacket = BuildCase(input, &wholeSize);
		uint16_t startId = 0;
		uint8_t clientId = 0;
		uint16_t type = 0;
		uint8_t seqNo = 0;
		char* payLoad = NULL;
		uint8_t len = 0;
		uint16_t endID = 0;
		int payloadlen =0;
		int forEndID = 0;
		char* onePacket = NULL;
		int wholeSize = 0;
		int caseNum = input;
		if (caseNum == 1 || caseNum == 2 || caseNum == 3 || caseNum ==4)
		{
			startId = 0xFFFF;
			clientId = 0xF0;
			type = 0xFFF1;
			seqNo = caseNum;
			if (caseNum == 1)
			{
				payLoad = "Hello World";
			}
			else if (caseNum == 2)
			{
				payLoad = "ra is the average of the active user¡¦s ratings, in other words, D¡¦s ratings, which is (5+4+2+3)/4 = 3.5";
			}
			else if (caseNum == 3)
			{
				payLoad = "Deviation from the average reflect whether she is positive or negative about the item";
			}
			else
			{
				payLoad = "To account for users different ratings levels (e.g., some users tend to give higher ratings), base predictions on differences from a user¡¦s average rating";
			}
			len = strlen(payLoad) + 1;
			endID = 0xFFFF;
			payloadlen = strlen(payLoad) + 1;
			forEndID = 7 + strlen(payLoad) + 1;
			wholeSize = 9 + strlen(payLoad) + 1;

		}
		else if (caseNum == 5)
		{
			startId = 0xFFFF;
			clientId = 0xF0;
			type = 0xFFF1;
			seqNo = 5;
			payLoad = "ENDMISSING Web enables near-zero-cost dissemination of information";
			len = strlen(payLoad) + 1;
			payloadlen = strlen(payLoad) + 1;
			forEndID = 7 + strlen(payLoad) + 1;
			wholeSize = 9 + strlen(payLoad) + 1;
			onePacket = malloc(wholeSize);
		}
		else if (caseNum == 6)
		{
			startId = 0xFFFF;
			clientId = 0xF0;
			type = 0xFFF1;
			seqNo = 6;
			endID = 0xFFFF;
			payLoad = "LENGTH MISSMATCH Web enables near-zero-cost dissemination of information";
			len = 5;
			payloadlen = strlen(payLoad) + 1;
			forEndID = 7 + strlen(payLoad) + 1;
			wholeSize = 9 + strlen(payLoad) + 1;
			onePacket = malloc(wholeSize);
		}
		else
		{
			startId = 0xFFFF;
			clientId = 0xF0;
			type = 0xFFF1;
			seqNo = 7;
			payLoad = "Deviation from the average reflect whether she is positive or negative about the item";
			len = strlen(payLoad) + 1;
			endID = 0xFFFF;
			payloadlen = strlen(payLoad) + 1;
			forEndID = 7 + strlen(payLoad) + 1;
			wholeSize = 9 + strlen(payLoad) + 1;
			onePacket = malloc(wholeSize);
		}

		onePacket = malloc(wholeSize);
		memcpy(onePacket, &startId, sizeof(startId));
		memcpy(onePacket + 2, &clientId, sizeof(clientId));
		memcpy(onePacket + 3, &type, sizeof(type));
		memcpy(onePacket + 5, &seqNo, sizeof(seqNo));
		memcpy(onePacket + 6, &len, sizeof(len));
		memcpy(onePacket + 7, payLoad, payloadlen);
		memcpy(onePacket + forEndID, &endID, sizeof(endID));
	
		if (sendto(s, onePacket, 512, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		free(onePacket);
		fd_set fds;
		int n;
		struct timeval tv;

		// Set up the file descriptor set.
		FD_ZERO(&fds);
		FD_SET(s, &fds);

		// Set up the struct timeval for the timeout.
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		// Wait until timeout or data received.
		/**/
		int rv = select(s, &fds, NULL, NULL, &tv);
		if (rv == SOCKET_ERROR)
		{
			// select error...
			printf("ERROR WHEN WAITING RESPONSE\n");
		}
		else if (rv == 0)
		{
			// timeout, socket does not have anything to read
			printf("TIMEOUT\n");
			
		}
		else
		{
			//receive a reply and print it
			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', BUFLEN);
			//try to receive some data, this is a blocking call
			if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
			{
				//printf("recvfrom() failed with error code : %d", WSAGetLastError());
				printf("Waiting for Response");
				//exit(EXIT_FAILURE);
			}

			char* current = buf;
			//uint16_t startId = *(current);
			//uint8_t clinetId = *(current + 2);
			uint16_t rtype = *(current + 3);
			if (rtype != 0xFFF2)
			{
				printf("RECEIVE REJECT\n");
				uint16_t rSub = *(current + 5);
				printf("SUBCODE %x\n", rSub);
			}
			else
			{
				printf("RECEIVE ACK\n");
			}
			
		}
		
	}
	closesocket(s);
	WSACleanup();
	return 0;
}



