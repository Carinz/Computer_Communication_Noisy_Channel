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

#include "socketShared.h"
#include "SocketSendRecvTools.h"
#include "channelServer.h"
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//#define NUM_OF_WORKER_THREADS 2

#define MAX_LOOPS 2
#define SEND_STR_SIZE 35
//SOCKET mainSocket = INVALID_SOCKET;
SOCKET socketSender = INVALID_SOCKET;
SOCKET socketReciever = INVALID_SOCKET;

SOCKET acceptSocketSender;
SOCKET acceptSocketReciever;

char senderBuffer[SENDER_PACKET_SIZE];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
//SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//static int FindFirstUnusedThreadSlot();
//static void CleanupWorkerThreads();
//static DWORD ServiceThread( SOCKET *t_socket );

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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

    do
    {
        statusRecieve = ReceiveBuffer(senderBuffer, SENDER_PACKET_SIZE, socketSender);
        if (statusRecieve == TRNS_FAILED)
        {
            //TODO: handle
            printf("FAIL!");
            assert(0);
        }

        if (statusRecieve == TRNS_DISCONNECTED)
        {
            gracefullyDisC(sock);
            gracefullyDisC();
            break;
        }
        //TODO: encodeHamming()
        //sendToRecieverClient
    }while(statusRecieve == TRNS_SUCCEEDED);

    

    // recieve SIZE_BUFFER packet. happens until the packet has 0.
    // while recieveBuffer return value >0 
    // gracefully shtdown

//server_cleanup_3:

//	CleanupWorkerThreads();

}

void createSocket(SOCKET * mainSocket, u_short serverPort)
{
    
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;


    // Create a socket.    
    *mainSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( *mainSocket == INVALID_SOCKET ) 
	{
        printf( "Error at socket( ): %ld\n", WSAGetLastError( ) );
        server_cleanup_1();
        //assert(0);
		//goto server_cleanup_1;
    }

    // Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system. 
		The following code demonstrates how to bind a socket that has already been created to an IP address 
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number. 
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
	// Create a sockaddr_in object and set its values.
	// Declare variables

	Address = inet_addr( SERVER_ADDRESS_STR );
	if ( Address == INADDR_NONE )
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
				SERVER_ADDRESS_STR );
		server_cleanup_2(mainSocket);
        //assert(0);
	}

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = Address;
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

gracefullyDisC()
{

}


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

// static int FindFirstUnusedThreadSlot()
// { 
// 	int Ind;

// 	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
// 	{
// 		if ( ThreadHandles[Ind] == NULL )
// 			break;
// 		else
// 		{
// 			// poll to check if thread finished running:
// 			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], 0 ); 
				
// 			if ( Res == WAIT_OBJECT_0 ) // this thread finished running
// 			{				
// 				CloseHandle( ThreadHandles[Ind] );
// 				ThreadHandles[Ind] = NULL;
// 				break;
// 			}
// 		}
// 	}

// 	return Ind;
// }

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

// static void CleanupWorkerThreads()
// {
// 	int Ind; 

// 	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
// 	{
// 		if ( ThreadHandles[Ind] != NULL )
// 		{
// 			// poll to check if thread finished running:
// 			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], INFINITE ); 
				
// 			if ( Res == WAIT_OBJECT_0 ) 
// 			{
// 				closesocket( ThreadInputs[Ind] );
// 				CloseHandle( ThreadHandles[Ind] );
// 				ThreadHandles[Ind] = NULL;
// 				break;
// 			}
// 			else
// 			{
// 				printf( "Waiting for thread failed. Ending program\n" );
// 				return;
// 			}
// 		}
// 	}
// }

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
// static DWORD ServiceThread( SOCKET *t_socket ) 
// {
// 	char SendStr[SEND_STR_SIZE];

// 	BOOL Done = FALSE;
// 	TransferResult_t SendRes;
// 	TransferResult_t RecvRes;

// 	strcpy( SendStr, "Welcome to this server!" );

// 	SendRes = SendString( SendStr, *t_socket );
	
// 	if ( SendRes == TRNS_FAILED ) 
// 	{
// 		printf( "Service socket error while writing, closing thread.\n" );
// 		closesocket( *t_socket );
// 		return 1;
// 	}
	
// 	while ( !Done ) 
// 	{		
// 		char *AcceptedStr = NULL;
		
// 		RecvRes = ReceiveString( &AcceptedStr , *t_socket );

// 		if ( RecvRes == TRNS_FAILED )
// 		{
// 			printf( "Service socket error while reading, closing thread.\n" );
// 			closesocket( *t_socket );
// 			return 1;
// 		}
// 		else if ( RecvRes == TRNS_DISCONNECTED )
// 		{
// 			printf( "Connection closed while reading, closing thread.\n" );
// 			closesocket( *t_socket ); //TODO: ALONACARINA add gracefully disconnect here
// 			return 1;
// 		}
// 		else
// 		{
// 			printf( "Got string : %s\n", AcceptedStr );
// 		}

// 		//After reading a single line, checking to see what to do with it
// 		//If got "hello" send back "what's up?"
// 		//If got "how are you?" send back "great"
// 		//If got "bye" send back "see ya!" and then end the thread
// 		//Otherwise, send "I don't understand"
		
// 		if ( STRINGS_ARE_EQUAL( AcceptedStr , "hello" ) ) 
// 			{ strcpy( SendStr, "what's up?" );} 
// 		else if ( STRINGS_ARE_EQUAL( AcceptedStr , "how are you?" ) ) 
// 			{ strcpy( SendStr, "great" ); }
// 		else if ( STRINGS_ARE_EQUAL( AcceptedStr, "bye" )) 
// 		{
// 			strcpy( SendStr, "see ya!" );
// 			Done = TRUE;
// 		}
// 		else 
// 			{ strcpy( SendStr, "I don't understand" ); }

// 		SendRes = SendString( SendStr, *t_socket );
	
// 		if ( SendRes == TRNS_FAILED ) 
// 		{
// 			printf( "Service socket error while writing, closing thread.\n" );
// 			closesocket( *t_socket );
// 			return 1;
// 		}

// 		free( AcceptedStr );		
// 	}

// 	printf("Conversation ended.\n");
// 	closesocket( *t_socket );
// 	return 0;
// }


int main(int argc, char * args[])
{
	MainServer();
}
