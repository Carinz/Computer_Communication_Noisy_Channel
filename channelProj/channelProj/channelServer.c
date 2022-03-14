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
//SOCKET mainSocket = INVALID_SOCKET;
SOCKET socketSender = INVALID_SOCKET;
SOCKET socketReciever = INVALID_SOCKET;

SOCKET acceptSocketSender;
SOCKET acceptSocketReciever;

char senderBuffer[SENDER_PACKET_SIZE];

void MainServer()
{
	TransferResult_t statusRecieve;    

	// Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           

    if ( StartupRes != NO_ERROR )
	{
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

    createSocket(&socketSender, SERVER_PORT_SENDER);
    createSocket(&socketReciever, SERVER_PORT_RECIEVER);
 
    /* The WinSock DLL is acceptable. Proceed. */

    printf( "Waiting for a client to connect...\n" );
    
	clientConnect(&acceptSocketSender, &socketSender);
	clientConnect(&acceptSocketReciever, &socketReciever);

    //statusRecieve = ReceiveBuffer(senderBuffer, SENDER_PACKET_SIZE, acceptSocketSender);
	//printf("THE MESSAGE: %s", senderBuffer);

    do
    {
        statusRecieve = ReceiveBuffer(senderBuffer, SENDER_PACKET_SIZE, acceptSocketSender);
        if (statusRecieve == TRNS_FAILED)
        {
            //TODO: handle
            printf("FAIL!");
            assert(0);
        }

        if (statusRecieve == TRNS_DISCONNECTED)
        {
            gracefullyDisSender(&acceptSocketSender);
            gracefullyDisReciever(&acceptSocketReciever);
            break;
        }
		printf("THE MESSAGE: %s\n", senderBuffer);
        //TODO: addNoise()
        sendToRecieverClient()
    }while(statusRecieve == TRNS_SUCCEEDED);

    

    // recieve SIZE_BUFFER packet. happens until the packet has 0.
    // while recieveBuffer return value >0 
    // gracefully shtdown


}

void createSocket(SOCKET * mainSocket, u_short serverPort)
{
    
	//unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;


    // Create a socket.    
    *mainSocket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( *mainSocket == INVALID_SOCKET ) 
	{
        printf( "Error at socket( ): %ld\n", WSAGetLastError( ) );
        server_cleanup_1();
        //assert(0);
		//goto server_cleanup_1;
    }

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr( S123 );
    service.sin_port = htons( serverPort ); //The htons function converts a u_short from host to TCP/IP network byte order 
	                                   //( which is big-endian ).
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
        printf( "bind( ) failed with error %ld. Ending program\n", WSAGetLastError( ) );
		server_cleanup_2(mainSocket);
        //assert(0);
	}
    
    // Listen on the Socket.
	ListenRes = listen( *mainSocket, SOMAXCONN );
    if ( ListenRes == SOCKET_ERROR ) 
	{
        printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
		server_cleanup_2(mainSocket);
	}

}

void clientConnect(SOCKET * acceptSocket, SOCKET * mainSocket)
{
    *acceptSocket = accept( *mainSocket, NULL, NULL );
		if ( *acceptSocket == INVALID_SOCKET )
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
			//server_cleanup_3(); //TODO: do all gracefully closing like cleanup_3 do in thread function
		}

    printf( "Client Connected.\n" );
}

void server_cleanup_1()
{
    if ( WSACleanup() == SOCKET_ERROR )		
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError() );
        assert(0);
}

void server_cleanup_2(SOCKET * mainSocket)
{
    if ( closesocket( *mainSocket ) == SOCKET_ERROR )
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError() ); 
    server_cleanup_1();
}

void gracefullyDiscSender(SOCKET * acceptSocket)
{
	int shutRes;

	// sendfinaltransition
	shutRes = shutdown(*acceptSocket, SD_SEND);
	if ( shutRes == SOCKET_ERROR ) 
	{
        printf( "shutdown failed with error %ld. Ending program\n", WSAGetLastError( ) );
        assert(0);
	}
	printf("SHTTING DOWN");
	closesocket(*acceptSocket);
}

int main(int argc, char *argv[])

{
	MainServer();
}

//TransferResult_t ReceiveBuffer( char* OutputBuffer, int BytesToReceive, SOCKET sd )
//{
//	char* CurPlacePtr = OutputBuffer;
//	int BytesJustTransferred;
//	int RemainingBytesToReceive = BytesToReceive;
//	
//	while ( RemainingBytesToReceive > 0 )  
//	{
//		/* send does not guarantee that the entire message is sent */
//		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
//		if ( BytesJustTransferred == SOCKET_ERROR ) 
//		{
//			printf("recv() failed, error %d\n", WSAGetLastError() );
//			return TRNS_FAILED;
//		}		
//		else if ( BytesJustTransferred == 0 )
//			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.
//
//		RemainingBytesToReceive -= BytesJustTransferred;
//		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
//
//		if (*(CurPlacePtr-1) == EOF)
//		{
//			break;
//		}
//	}
//
//	return TRNS_SUCCEEDED;
//}