/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//#define NUM_OF_WORKER_THREADS 2
//TransferResult_t ReceiveBuffer( char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd );

//#define MAX_LOOPS 2
#define SEND_STR_SIZE 35
#define RAND_MAX 0x7FFF;
//SOCKET mainSocket = INVALID_SOCKET;
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
        // Tell the user that we could not find a usable WinSock DLL.   
        assert(0);
        //return;
    }

    createSocket(&socketSender, "sender", 63106);
    createSocket(&socketReciever, "reciever", 63107);
}

void MainServer()
{
    double noRetransBytes=0;
    int noFlipBitsTotal=0, tempNoFlip=0, noRetransBytesInt=0;

	TransferResult_t statusRecieve, statusSend;
    int startIndex = det_n - 1;
	//// Initialize Winsock.
 //   WSADATA wsaData;
 //   int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           

 //   if ( StartupRes != NO_ERROR )
	//{
 //       printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
	//	// Tell the user that we could not find a usable WinSock DLL.                                  
	//	return;
	//}

    /*printf("sender socket : IP address = %d port = %d", ip, port);
    printf("receiver socket : IP address = %d port = %d", ip, port);*/

    /*createSocket(&socketSender, SERVER_PORT_SENDER);
    createSocket(&socketReciever, SERVER_PORT_RECIEVER);*/
 
    /* The WinSock DLL is acceptable. Proceed. */

    //printf( "Waiting for a client to connect...\n" );
    
	clientConnect(&acceptSocketSender, &socketSender);
	clientConnect(&acceptSocketReciever, &socketReciever);

    //statusRecieve = ReceiveBuffer(senderBuffer, SENDER_PACKET_SIZE, acceptSocketSender);
	//printf("THE MESSAGE: %s", senderBuffer);

    do
    {
        statusRecieve = ReceiveBuffer(beforeNoiseBuffer, SENDER_PACKET_SIZE, acceptSocketSender);
        if (statusRecieve == TRNS_FAILED)
        {
            //TODO: handle
            printf("failed with recieving from sender\n");
            assert(0);
        }

        if (statusRecieve == TRNS_DISCONNECTED)
        {
            gracefullyDiscSender();
            gracefullyDiscReciever();
            break;
        }
		//printf("THE MESSAGE: %s\n", beforeNoiseBuffer);
        tempNoFlip = 0;
        startIndex = addNoise(startIndex, &tempNoFlip); //start index is relevant only in deterministic

        noFlipBitsTotal += tempNoFlip;

        // in addNoiseDet function, for the first loop we will reset firstIndex=n-1
        //sendToRecieverClient
        
        statusSend = SendBuffer(afterNoiseBuffer, SENDER_PACKET_SIZE, acceptSocketReciever);
        if (statusSend == TRNS_FAILED)
        {
            printf("PROBLEN WITH CHANNEL SENDING\n");
            assert(0);
        }
        
        noRetransBytes += 3.875;
    }while(statusRecieve == TRNS_SUCCEEDED);

    //printf("DONE!");
    noRetransBytesInt = (int)noRetransBytes;
    printf("retransmitted %d bytes, flipped %d bits\n", noRetransBytesInt, noFlipBitsTotal);

    // recieve SIZE_BUFFER packet. happens until the packet has 0.
    // while recieveBuffer return value >0 
    // gracefully shtdown


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
    //*bits31Num = *bits31Num ^ a;
    if (counter >= 31)
        tmpStartIndex = counter - 31;// det_n + (counter - det_n) - 31;

    else
    {
        do
        {
            *bits31Num = *bits31Num ^ a;
            a = a << det_n;
            counter += det_n;
            //*bits31Num = *bits31Num ^ a;
            tempFlipped++;
        } while (counter < 31);
        tmpStartIndex = counter - 31;// det_n + (counter - det_n) - 31;
    }

    *(int*)afterNoiseBuffer = *bits31Num;
    *noFlipped = tempFlipped;
    return (tmpStartIndex);

}

void createSocket(SOCKET * mainSocket, char * type, int num) //TODO: DELETE NUM FIELD
{
    
	//unsigned long Address;
	SOCKADDR_IN service;
	int bindRes, size;
	int ListenRes;


    // Create a socket.    
    *mainSocket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( *mainSocket == INVALID_SOCKET ) 
	{
        printf( "Error at createSocket: %ld\n", WSAGetLastError() );
        assert(0);
	}

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1"); //TODO: conctract the port in next line!

    //service.sin_addr.s_addr = htonl(INADDR_ANY); //TODO: conctract the port in next line!
    //service.sin_port = htons( 0 ); //The htons function converts a u_short from host to TCP/IP network byte order 
	                                   //( which is big-endian ).

    service.sin_port = htons(num); //TODO: DELETE AND LEAVE THE ABOVE LINE
	/*
		The three lines following the declaration of sockaddr_in service are used to set up 
		the sockaddr structure: 
		AF_INET is the Internet address family. 
		"127.0.0.1" is the local IP address to which the socket will be bound. 
	    2345 is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
    bindRes = bind( *mainSocket, ( SOCKADDR* ) &service, sizeof( service ) );
	if ( bindRes == SOCKET_ERROR ) 
	{
        printf( "bind() failed with error %ld. Ending program\n", WSAGetLastError() );
		//server_cleanup_2(mainSocket);
        assert(0);
	}
    
    // Listen on the Socket.
	ListenRes = listen( *mainSocket, SOMAXCONN );
    if ( ListenRes == SOCKET_ERROR ) 
	{
        printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
		//server_cleanup_2(mainSocket);
        assert(0);
	}

    size = sizeof(service);
    getsockname(*mainSocket, (struct sockaddr*)&service, &size);
    printf("%s socket: IP address = %s port = %d\n", type, inet_ntoa(service.sin_addr), ntohs(service.sin_port));

}

void clientConnect(SOCKET * acceptSocket, SOCKET * mainSocket)
{
    *acceptSocket = accept( *mainSocket, NULL, NULL );
		if ( *acceptSocket == INVALID_SOCKET )
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
			//server_cleanup_3(); //TODO: do all gracefully closing like cleanup_3 do in thread function
            assert(0);
		}

    //printf( "Client Connected.\n" );
}

//void server_cleanup_1()
//{
//    if ( WSACleanup() == SOCKET_ERROR )		
//		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError() );
//        assert(0);
//}

//void server_cleanup_2(SOCKET * mainSocket)
//{
//    if ( closesocket( *mainSocket ) == SOCKET_ERROR )
//		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError() ); 
//    server_cleanup_1();
//}

void gracefullyDiscSender()
{
	int shutRes;
    //SOCKET socketSender = INVALID_SOCKET;

	// sendfinaltransition
	shutRes = shutdown(acceptSocketSender, SD_SEND);
	if ( shutRes == SOCKET_ERROR ) 
	{
        printf( "shutdown failed with error %ld. Ending program\n", WSAGetLastError( ) );
        assert(0);
	}
	//printf("SHTTING DOWN SENDER\n");
	closesocket(acceptSocketSender);
    //closesocket(socketSender);
}

void gracefullyDiscReciever() //TODO: unite
{
    int shutRes = shutdown(acceptSocketReciever, SD_SEND);
    if (shutRes == SOCKET_ERROR)
    {
        printf("shutdown failed with error %ld. Ending program\n", WSAGetLastError());
        assert(0);
    }
    closesocket(acceptSocketReciever);
    //closesocket(socketReciever);

}

int main(int argc, char *argv[])
{
    char doContinue[5];// = {'/0'};
    
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

