
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
#define DATASIZE 5
struct oneCase{
	uint16_t startId;
	uint8_t clientId;
	uint16_t accPer;
	uint8_t seqNo;
	uint8_t len;
	uint8_t technology;
	uint32_t subNo;
	uint16_t endID;
}allCases[DATASIZE];

void loadCases();
int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	int receiveLen;
	//char * message = NULL;
	WSADATA wsa;

	loadCases();
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

	//start communication
	int currentSeq = 0;
	int count = 0;
	int serVerNot = 0;
	while (1)
	{
		int oneCase = 4;
		if (currentSeq != 4)
		{
			if (serVerNot == 1)
			{
				printf("Server does not respond.\n");
				serVerNot = 0;
			}
			printf("Enter caseNum : ");
			scanf("%d", &oneCase);
			currentSeq = oneCase;
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

		uint16_t startId = allCases[oneCase].startId;
		uint8_t clientId = allCases[oneCase].clientId;
		uint16_t accPer = allCases[oneCase].accPer;
		uint8_t seqNo = allCases[oneCase].seqNo;
		uint8_t len = allCases[oneCase].len;
		uint8_t technology = allCases[oneCase].technology;
		uint32_t subNo = allCases[oneCase].subNo;
		uint16_t endID = allCases[oneCase].endID;

		char* onePacket;
		onePacket = malloc(14);
		memcpy(onePacket, &startId, sizeof(startId));
		memcpy(onePacket + 2, &clientId, sizeof(clientId));
		memcpy(onePacket + 3, &accPer, sizeof(accPer));
		memcpy(onePacket + 5, &seqNo, sizeof(seqNo));
		memcpy(onePacket + 6, &len, sizeof(len));
		memcpy(onePacket + 7, &technology, sizeof(technology));
		memcpy(onePacket + 8, &subNo, sizeof(subNo));
		memcpy(onePacket + 12, &endID, sizeof(endID));

		uint8_t techTest = *(onePacket + 7);
		uint32_t test = *((uint32_t *)(onePacket + 8));
		uint16_t endIDtest = *(onePacket + 12);

		printf("SEND SUBNO: %" PRIu32"\n", test);
		
	
		//send the message
		if (sendto(s, onePacket, 14, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		free(onePacket);
		// socket has something to read
		//int length = sizeof(remoteAddr);
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
			//clear the buffer, it might have previously received data
			memset(buf, '\0', BUFLEN);
			//try to receive some data, this is a blocking call
			if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				printf("Waiting for Response");
				exit(EXIT_FAILURE);
			}

			char* current = buf;
			uint16_t rtype = *(current + 3);
			if (rtype != 0xFFFB)
			{
				printf("RECEIVE: NOT　PERMIT\nTYPE: %x\n", rtype);

			}
			else
			{
				printf("RECEIVE: PERMIT\nTYPE: %x\n", rtype);
			}
		}
		//puts(buf);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}

void loadCases()
{
	//char const* const fileName = argv[1]; /* should check that argc > 1 */
	FILE* file = fopen("D:\\testcase2.txt", "r"); /* should check the result */
	char line[256];
	int count = 0;
	while (fgets(line, sizeof(line), file)) {
		/* note that fgets don't strip the terminating \n, checking its
		presence would allow to handle lines longer that sizeof(line) */
		allCases[count].startId = strtol(strtok(line, " "),NULL,16);
		allCases[count].clientId = strtol(strtok(NULL, " "), NULL, 16);
		allCases[count].accPer = strtol(strtok(NULL, " "), NULL, 16);
		allCases[count].seqNo = atoi(strtok(NULL, " "));
		allCases[count].len = atoi(strtok(NULL, " "));
		allCases[count].technology = atoi(strtok(NULL, " "));
		char* val1 = strtok(NULL," ");
		allCases[count].endID = strtol(strtok(NULL, " "), NULL, 16);
		int temp1 = atoi(strtok(val1, "-"));
		int temp2 = atoi(strtok(NULL, "-"));
		int temp3 = atoi(strtok(NULL, "-"));
		allCases[count].subNo = temp1 * 10000000 + temp2 * 10000 + temp3;
		printf("%" PRIu32 " |	%d	|	%d   | %x\n", allCases[count].subNo, allCases[count].technology, allCases[count].endID, allCases[count].startId);
		count++;
	}
	/* may check feof here to make a difference between eof and io failure -- network
	timeout for instance */
	fclose(file);
	return;
}








