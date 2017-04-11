/*
NAME:        udpServer0 
DESCRIPTION:    The program creates a datagram socket in the inet 
                domain, binds it to port 12345 and receives any message
                arrived to the socket and prints it out
*/


#include <stdio.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<inttypes.h>
#include <time.h>


#define BUFLEN 512
#define PORT 8888
#define DATASIZE 3
struct subScriber{		
	uint32_t subNo;
	uint8_t technology;
	uint8_t paid;
}dataBase[DATASIZE];

void loadDataBase();
int checkPaidExist(uint32_t subNo);

char* buildPacket(uint16_t paketType, char* current);
int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	struct timeval tv;

	slen = sizeof(si_other);
	loadDataBase();
	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//keep listening for data
	int seqCount = 0;
	while (1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received

		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		
		char* current = buf;
		uint16_t startId = *(current);
		uint8_t clientId = *(current+2);
		uint16_t accPer = *(current+3);
		uint8_t seqNo = *(current+5);
		uint8_t len = *(current+6);
		uint8_t tech = *(current + 7);
		uint32_t subNo = *((uint32_t*)(current + 8));
		uint16_t endId = *(current + 12);
		
		printf("startId: %x\n", startId);
		printf("clinetId: %x\n", clientId);
		printf("type: %x\n", accPer);
		printf("seg: %d\n", seqNo);
		printf("len: %d\n", len);
		printf("tech: %d\n", tech);
		printf("subNo: %" PRIu32 "\n", subNo);
		printf("endId: %x\n", endId);
		

		int pocketSize = 14;
		int result = checkPaidExist(subNo);
		uint16_t tempType = 0;
		if (seqNo == 5)
		{
			printf("==SEND NOTHING BACK==\n");
		}
		else
		{
			if (result == 0)
			{
				printf("==NOT PAID==\n");
				tempType = 0xFFF9;
			}
			else if (result == 2)
			{
				printf("==NOT EXIST==\n");
				tempType = 0xFFFA;
			}
			else
			{
				printf("==PERMIT==\n");
				tempType = 0xFFFB;
			}
			char* reply = buildPacket(tempType, buf);
			//now reply the client with the same data	
			if (sendto(s, reply/*buf*/, 14/*recv_len*/, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
			{

				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			free(reply);
		}
		
	}
	closesocket(s);
	WSACleanup();
	return 0;
}

void loadDataBase()
{
	//char const* const fileName = argv[1]; /* should check that argc > 1 */
	FILE* file = fopen("D:\\data.txt", "r"); /* should check the result */
	char line[256];
	int count=0;
	while (fgets(line, sizeof(line), file)) {
		/* note that fgets don't strip the terminating \n, checking its
		presence would allow to handle lines longer that sizeof(line) */
		char* val1 = strtok(line, "\t");
		dataBase[count].technology = atoi(strtok(NULL, "\t"));
		dataBase[count].paid = atoi(strtok(NULL, "\t"));
		int temp1 = atoi(strtok(val1, "-"));
		int temp2 = atoi(strtok(NULL, "-"));
		int temp3 = atoi(strtok(NULL, "-"));
		dataBase[count].subNo = temp1 * 10000000 + temp2 * 10000 + temp3;
		printf("%" PRIu32 " |	%d	|	%d\n", dataBase[count].subNo, dataBase[count].technology, dataBase[count].paid);
		count++;
	}
	/* may check feof here to make a difference between eof and io failure -- network
	timeout for instance */
	fclose(file);
	return;
}

int checkPaidExist(uint32_t subNo)
{
	for (int i = 0; i < DATASIZE; i++)
	{
		if (dataBase[i].subNo == subNo)
		{
			if (dataBase[i].paid == 1)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	return 2;
}


char* buildPacket(uint16_t paketType, char* current)
{
	uint16_t startId = *(current);
	uint8_t clientId = *(current + 2);
	uint16_t type = paketType;
	uint8_t seqNo = *(current + 5);
	uint8_t len = *(current + 6);
	uint8_t tech = *(current + 7);
	uint32_t subNo = ((uint32_t*)(current + 8));
	uint16_t endID = *(current + 12);
	char* response = malloc(14);
	memcpy(response, &startId, sizeof(startId));
	memcpy(response + 2, &clientId, sizeof(clientId));
	memcpy(response + 3, &type, sizeof(type));
	memcpy(response + 5, &seqNo, sizeof(seqNo));
	memcpy(response + 6, &len, sizeof(len));
	memcpy(response + 7, &tech, sizeof(tech));
	memcpy(response + 8, &subNo, sizeof(subNo));
	memcpy(response + 12, &endID, sizeof(endID));
	return response;
}