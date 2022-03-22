
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <assert.h>

#include "channelServer.h"
#include "../../socketShared.h"
#include "../../SocketSendRecvTools.h"



#define SEND_STR_SIZE 35
#define RAND_MAX 0x7FFF;
SOCKET socketSender = INVALID_SOCKET;
SOCKET socketReciever = INVALID_SOCKET;

SOCKET acceptSocketSender;
SOCKET acceptSocketReciever;

int senderPort;
int recieverPort;
int whichNoise;
int det_n;
int prob;
int randSeed;
char* whichMode;

unsigned char beforeNoiseBuffer[SENDER_PACKET_SIZE];
unsigned char afterNoiseBuffer[SENDER_PACKET_SIZE];


void initializeServer()
{
    // Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (StartupRes != NO_ERROR)
    {
        printf("error %ld at WSAStartup(), ending program.\n", WSAGetLastError());
        assert(0);
    }

    createSocket(&socketSender, "sender");
    createSocket(&socketReciever, "reciever"); 
}

void MainServer()
{
    double noRetransBytes=0;
    int noFlipBitsTotal=0, tempNoFlip=0, noRetransBytesInt=0;

	TransferResult_t statusRecieve, statusSend;
    int startIndex = det_n - 1;
	    
	clientConnect(&acceptSocketSender, &socketSender);
	clientConnect(&acceptSocketReciever, &socketReciever); 


    do
    {
        statusRecieve = ReceiveBuffer(beforeNoiseBuffer, SENDER_PACKET_SIZE, acceptSocketSender);
        if (statusRecieve == TRNS_FAILED)
        {
            printf("failed with recieving from sender\n");
            assert(0);
        }

        if (statusRecieve == TRNS_DISCONNECTED)
        {
            gracefullyDiscSender();
            gracefullyDiscReciever();
            break;
        }
        tempNoFlip = 0;
        startIndex = addNoise(startIndex, &tempNoFlip); //start index is relevant only in deterministic

        noFlipBitsTotal += tempNoFlip;

        
        statusSend = SendBuffer(afterNoiseBuffer, SENDER_PACKET_SIZE, acceptSocketReciever);
        if (statusSend == TRNS_FAILED)
        {
            printf("PROBLEN WITH CHANNEL SENDING\n");
            assert(0);
        }
        
        noRetransBytes += 3.875;
    }while(statusRecieve == TRNS_SUCCEEDED);

    noRetransBytesInt = (int)noRetransBytes;
    printf("retransmitted %d bytes, flipped %d bits\n", noRetransBytesInt, noFlipBitsTotal);



}

int addNoise(int tmpStartIndex, int * noFlipBits) //returns start index to next packet if deterministic
{
    if (!strcmp(whichMode,"-d")) // deterministric
    {
        return addNoiseDet(tmpStartIndex,noFlipBits);
    }

    else //random
    {
        return addNoiseRand(noFlipBits);
    }
}

int addNoiseRand(int * noflipBits)
{
    int i;
    int doesItFlip; //1 in probability prob/2^16 ; 0 in 1-prob

    int numToXor=0;

    for (i = 0; i < 32; i++)
    {
        doesItFlip = randIndicator();
        *noflipBits += doesItFlip;
        numToXor = numToXor | (doesItFlip << i);
    }
    numToXor = numToXor ^ *((int*)beforeNoiseBuffer);
    *(int*)afterNoiseBuffer = numToXor;
    return 0;

}

int randIndicator()
{
    int rand15 = rand();
    int rand2 = rand() % 2;

    int finalRand = rand15 | (rand2 << 15);
    if (finalRand <= prob)
    {
        return 1; //flip!
    }
    else
    {
        return 0;
    }
}


int addNoiseDet(int tmpStartIndex, int * noFlipped) //startIndex 0 to 30
{
    unsigned int* bits31Num = (int*)(beforeNoiseBuffer);
    int a = 1; 
    int counter = tmpStartIndex;

    int tempFlipped=0;

    a = a << tmpStartIndex;
    if (counter >= 31)
        tmpStartIndex = counter - 31;// det_n + (counter - det_n) - 31;

    else
    {
        do
        {
            *bits31Num = *bits31Num ^ a;
            a = a << det_n;
            counter += det_n;
            tempFlipped++;
        } while (counter < 31);
        tmpStartIndex = counter - 31;// det_n + (counter - det_n) - 31;
    }

    *(int*)afterNoiseBuffer = *bits31Num;
    *noFlipped = tempFlipped;
    return (tmpStartIndex);

}

void createSocket(SOCKET * mainSocket, char * type) 
{
	SOCKADDR_IN service;
	int bindRes, size;
	int ListenRes;
    char* ipAddress;
    char hostname[1024];
    struct hostent* entry;

    // Create a socket.    
    *mainSocket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( *mainSocket == INVALID_SOCKET ) 
	{
        printf( "Error at createSocket: %ld\n", WSAGetLastError() );
        assert(0);
	}

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_ANY); 
    service.sin_port = htons( 0 ); 
 

    bindRes = bind( *mainSocket, ( SOCKADDR* ) &service, sizeof( service ) );
	if ( bindRes == SOCKET_ERROR ) 
	{
        printf( "bind() failed with error %ld. Ending program\n", WSAGetLastError() );
        assert(0);
	}
    
    // Listen on the Socket.
	ListenRes = listen( *mainSocket, SOMAXCONN );
    if ( ListenRes == SOCKET_ERROR ) 
	{
        printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
        assert(0);
	}

    size = sizeof(service);
    ASSERT(getsockname(*mainSocket, (struct sockaddr*)&service, &size) == NO_ERROR, "getsockname failed");
    ASSERT(gethostname(hostname, 1024) == 0, "gethostname failed");
    entry = gethostbyname(hostname);
    ipAddress = inet_ntoa(*((struct in_addr*)entry->h_addr_list[0]));
    printf("%s socket: IP address = %s port = %d\n", type, ipAddress, ntohs(service.sin_port));

}

void clientConnect(SOCKET * acceptSocket, SOCKET * mainSocket)
{
    *acceptSocket = accept( *mainSocket, NULL, NULL );
		if ( *acceptSocket == INVALID_SOCKET )
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
            assert(0);
		}

}


void gracefullyDiscSender()
{
	int shutRes;

	shutRes = shutdown(acceptSocketSender, SD_SEND);
	if ( shutRes == SOCKET_ERROR ) 
	{
        printf( "shutdown failed with error %ld. Ending program\n", WSAGetLastError( ) );
        assert(0);
	}
	closesocket(acceptSocketSender);
}

void gracefullyDiscReciever() 
{
    int shutRes = shutdown(acceptSocketReciever, SD_SEND);
    if (shutRes == SOCKET_ERROR)
    {
        printf("shutdown failed with error %ld. Ending program\n", WSAGetLastError());
        assert(0);
    }
    closesocket(acceptSocketReciever);

}

int main(int argc, char *argv[])
{
    char doContinue[10];
    
    if (argc == 3)
    {
        whichMode = argv[1];
        det_n = atoi(argv[2]);
    }

    else if (argc == 4)
    {
        whichMode = argv[1];
        prob = atoi(argv[2]);
        randSeed = atoi(argv[3]);
        srand(randSeed);
    }

    else
    {
        printf("number of args dont match!\n");
        assert(0);
    }

    initializeServer();

    do{
        MainServer();
        printf("continue? (yes/no)\n");
        gets(doContinue);
    }while(!strcmp(doContinue, "yes"));

    closesocket(socketSender);
    closesocket(socketReciever);

}

