
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <assert.h>
#include <stdlib.h>

#include "senderClient.h"
#include "socketShared.h"
#include "SocketSendRecvTools.h"

SOCKET senderSocket;
int ipChannel ;
int portChannel = SERVER_PORT_SENDER; 
FILE * filePtr;
char fileName [100];//[6] = {'c','.','t','x','t','\0'}; //TODO CHANGE
//char bufferSend [SENDER_PACKET_SIZE] = {'H', 'e', 'l', 'l', 'o', '\0'};
char lettersPacket[NO_LETTERS_PACKET];

void mainSender()
{
    int i;
	TransferResult_t statusRecieve;    
    int connectStatus, shutRes;
    char * afterHamming;


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
    //SendBuffer(bufferSend, SENDER_PACKET_SIZE, senderSocket);

    printf("CLIENT CONNECT!");

    filePtr = fopen(fileName, "r");
    assert(filePtr);
    lettersPacket[0] = fgetc(filePtr);

    while(lettersPacket[0] != EOF) //not EOF
    {
        for(i=1 ; i<=NO_LETTERS_PACKET-1 ; i++) //reading next 12 left
        {
            lettersPacket[i] = fgetc(filePtr);
        }

        for(i=1 ; i<=4 ; i++) //actual sending
        {
            afterHamming = addHamming(i);
            SendBuffer(afterHamming, 4, senderSocket);
        }
        lettersPacket[0] = fgetc(filePtr);
    }

    fclose(filePtr);

    shutRes = shutdown(senderSocket, SD_SEND);
	if ( shutRes == SOCKET_ERROR ) 
	{
        printf( "shutdown failed with error %ld. Ending program\n", WSAGetLastError( ) );
        assert(0);
	}
    //recieve final trans - maybe how many transmitted to server
    printf("I AM CLUENT AND SHTTING");
	closesocket(senderSocket);

    //print how many bytes in file
    //print how many sent ( /26 * 31)


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
    service.sin_addr.s_addr = inet_addr( S123 );
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


char * addHamming(int noBlock) //input: pointer to 26 bits,  output: a new int pointer to the addition to 31 bits 
{
    char * beforeHamming = (char*)calloc(4,sizeof(char));
    char * newHamming;

    int i, lowLim;
    switch(noBlock)
    {
        case 1:
        lowLim=0;
        break;

        case 2:
        lowLim=3;
        break;

        case 3:
        lowLim=6;
        break;

        case 4:
        lowLim=9;
        break;
    }

    for(i=0 ; i<4 ; i++)
    {
        beforeHamming[i]=lettersPacket[lowLim+i];
    }

    //newHamming = actualAddHam(beforeHamming);
    //return newHamming;
    return beforeHamming;
}

char * actualAddHam(char * beforeHamming)
{
    return NULL;
}


int main(int argc, char *argv[])
{
    //ipChannel = atoi(args[1]);
    //portChannel = atoi(args[2]);
    
    //mainSender();
    printf("enter file name:");
    gets(fileName);
    //fileName=args[1];
    while(strcmp(fileName, "quit"))
	{
        mainSender();
        printf("enter file name:");
        gets(fileName);
    }
}

