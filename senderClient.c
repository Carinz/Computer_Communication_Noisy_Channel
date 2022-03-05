
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <assert.h>

#include "senderClient.h"
#include "socketShared.h"
#include "SocketSendRecvTools.h"

SOCKET senderSocket;
int ipChannel ;
int portChannel = 6342; 
FILE * filePtr;
char bufferSend [SENDER_PACKET_SIZE] = {'H', 'e', 'l', 'l', 'o', '\0'};

void mainSender()
{
	TransferResult_t statusRecieve;    
    int connectStatus, shutRes;
	// Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           

    if ( StartupRes != NO_ERROR )
	{
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

    connectStatus = createConnectSocketSender();

    if (connectStatus != NO_ERROR)
    {
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
        sender_cleanup_1();
        assert(0);
    }

    SendBuffer(bufferSend, SENDER_PACKET_SIZE, senderSocket);

    shutRes = shutdown(senderSocket, SD_SEND);
	if ( shutRes == SOCKET_ERROR ) 
	{
        printf( "shutdown failed with error %ld. Ending program\n", WSAGetLastError( ) );
        assert(0);
	}
	closesocket(senderSocket);
    // do
    // {
    //     //statusRecieve = ReceiveBuffer(senderBuffer, SENDER_PACKET_SIZE, acceptSocketSender);

    //     if (statusRecieve == TRNS_FAILED)
    //     {
    //         //TODO: handle
    //         printf("FAIL!");
    //         assert(0);
    //     }

    //     if (statusRecieve == TRNS_DISCONNECTED)
    //     {
    //         gracefullyDisC(&acceptSocketSender);
    //         gracefullyDisC(&acceptSocketReciever);
    //         break;
    //     }
    //     //TODO: addNoise()
    //     //sendToRecieverClient
    // }while(statusRecieve == TRNS_SUCCEEDED);

}

int createConnectSocketSender()
{
    SOCKADDR_IN service;
	int connectRes;

    // Create a socket.    
    senderSocket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( senderSocket == INVALID_SOCKET ) 
	{
        printf( "Error at socketSender( ): %ld\n", WSAGetLastError( ) );
        sender_cleanup_1();
        //assert(0);
		//goto server_cleanup_1;
    }

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons( portChannel ); //The htons function converts a u_short from host to TCP/IP network byte order 
	
    connectRes = connect(senderSocket, (SOCKADDR*)&service, sizeof (service));

    printf("waiting to connect...");

    return connectRes;
}

void sender_cleanup_1()
{
    if ( WSACleanup() == SOCKET_ERROR )		
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError() );
        assert(0);
}

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd )
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;
	
	while ( RemainingBytesToSend > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send (sd, CurPlacePtr, RemainingBytesToSend, 0);
		if ( BytesTransferred == SOCKET_ERROR ) 
		{
			printf("send() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}
		
		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

int main(int argc, char * args[])
{
    //ipChannel = atoi(args[1]);
    //portChannel = atoi(args[2]);
    printf("enter file name:");
    //kelet
    
    mainSender();

	// do{
    //     mainSender();
    // }while();
}

