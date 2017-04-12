#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<inttypes.h>
#include <time.h>


#define BUFLEN 512
#define PORT 8888
#define DATASIZE 10
struct subScriber {
	uint32_t subNo;
	uint8_t technology;
	uint8_t paid;
}dataBase[DATASIZE];

void loadDataBase();
char* buildReject(uint16_t paketType, char* current);
char* buildAck(char* current);

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	struct timeval tv;

	slen = sizeof(si_other);
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
	int prePacket = 0;
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

		
		char* current = buf;
		uint16_t startId = *(current);
		uint8_t clientId = *(current + 2);
		uint16_t type = *(current + 3);
		uint8_t seqNo = *(current + 5);
		uint8_t len = *(current + 6);
		char* payLoad = current + 7;
		int payloadlen = strlen(payLoad) + 1;
		uint16_t endId = *(current + 7 + payloadlen);

		printf("startId: %x\n", startId);
		printf("clinetId: %x\n", clientId);
		printf("type: %x\n", type);
		printf("seg: %d\n", seqNo);
		printf("len: %d\n", len);
		printf("payLoad: %s\n", payLoad);
		printf("endId: %x\n", endId);
		
		
		//uint32_t subNo = 4086668821;
		char* reply = NULL;
		int replySize = 0;
		uint16_t tempType = 0;
		if (seqNo == 7)
		{
			printf("NOT SEND ANYTHING BACK\n");
		}
		else if (endId != startId)
		{
			printf("==END OF PACKET MISSING==\n");
			tempType = 0xFFF6;
			reply = buildReject(tempType, buf);
			replySize = 10;
		}
		else if (len != payloadlen)
		{
			printf("==LENGTH MISMATCH==\n");
			tempType = 0xFFF5;
			reply = buildReject(tempType, buf);
			replySize = 10;
		}
		else if(seqNo == prePacket)
		{
			printf("==DUPLICATE==\n");
			tempType = 0xFFF7;
			reply = buildReject(tempType, buf);
			replySize = 10;
		}
		else if (seqNo != prePacket+1)
		{
			printf("==OUR OF SEQUENCE==\n");
			tempType = 0xFFF4;
			reply = buildReject(tempType, buf);
			replySize = 10;
		}
		else 
		{
			printf("==ACCEPT==\n");
			printf("==SEND ACK==\n");
			reply = buildAck(buf);
			replySize = 8;
			prePacket = seqNo;
		}
		if (seqNo != 7)
		{
			//now reply the client with the same data
			if (sendto(s, reply, replySize, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
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
	int count = 0;
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
		printf("%"  PRIu32 "\n", dataBase[i].subNo);
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


char* buildReject(uint16_t paketType, char* current)
{
	uint16_t startId = *(current);
	uint8_t clientId = *(current + 2);
	uint16_t type = 0XFFF3;
	uint16_t rSub = paketType;
	uint8_t rSeg = *(current + 7);
	uint16_t endId = *(current + 8);
	char* response = malloc(10);
	memcpy(response, &startId, sizeof(startId));
	memcpy(response + 2, &clientId, sizeof(clientId));
	memcpy(response + 3, &type, sizeof(type));
	memcpy(response + 5, &rSub, sizeof(rSub));
	memcpy(response + 7, &rSeg, sizeof(rSeg));
	memcpy(response + 8, &endId, sizeof(endId));
	return response;
}

char* buildAck(char* current)
{
	uint16_t startId = *(current);
	uint8_t clientId = *(current + 2);
	uint16_t type = 0XFFF2;
	uint8_t rSeg = *(current + 7);
	uint16_t endId = *(current + 8);
	char* response = malloc(8);
	memcpy(response, &startId, sizeof(startId));
	memcpy(response + 2, &clientId, sizeof(clientId));
	memcpy(response + 3, &type, sizeof(type));
	memcpy(response + 5, &rSeg, sizeof(rSeg));
	memcpy(response + 6, &endId, sizeof(endId));
	return response;
}
