#define _CRT_SECURE_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <assert.h>
#include <stdlib.h>

#include "Ws2tcpip.h"

#include "senderClient.h"
#include "../../socketShared.h"
#include "../../SocketSendRecvTools.h"

SOCKET senderSocket;
char * ipChannel;
int portChannel; 
FILE * filePtr;
char fileName [100];
unsigned char lettersPacket[NO_LETTERS_PACKET];

void mainSender()
{
    int i, fileLen=0, bytesInt=0;
    double bytesSent = 0.0;
	TransferResult_t statusSend;
    int connectStatus, shutRes;
    unsigned char * afterHamming;


	// Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           

    if ( StartupRes != NO_ERROR )
	{
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
        assert(0);
	}

    connectStatus = createConnectSocketSender();

    if (connectStatus != NO_ERROR)
    {
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
        assert(0);
    }


    filePtr = fopen(fileName, "r");
    assert(filePtr);
    lettersPacket[0] = fgetc(filePtr);

    while(lettersPacket[0] != 255) //not EOF
    {
        for(i=1 ; i<=NO_LETTERS_PACKET-1 ; i++) //reading next 12 left
        {
            lettersPacket[i] = fgetc(filePtr);
        }

        fileLen += 13;
        bytesSent += 15.5; 

        for(i=1 ; i<=4 ; i++) //actual sending
        {
            afterHamming = addHamming(i);
            statusSend = SendBuffer(afterHamming, 4, senderSocket);
            if (statusSend == TRNS_FAILED)
            {
                printf("PROBLEN WITH SENDING\n");
                assert(0);
            }
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

	closesocket(senderSocket);
    bytesInt = (int)bytesSent;

    printf("file length: %d bytes\n", fileLen);
    printf("sent: %d bytes\n", bytesInt); 


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
        assert(0);
    }
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr( ipChannel );
    service.sin_port = htons( portChannel ); 
	
    
    connectRes = connect(senderSocket, (SOCKADDR*)&service, sizeof (service));


    return connectRes;
}


char * addHamming(int noBlock) //input: pointer to 26 bits,  output: a new int pointer to the addition to 31 bits 
{
    unsigned char * beforeHamming = (unsigned char*)calloc(4,sizeof(unsigned char));
    assert(beforeHamming);
    unsigned char* beforeHammingAligned= (unsigned char*)calloc(4, sizeof(unsigned char));
    assert(beforeHammingAligned);
    unsigned char* finalHam = (unsigned char*)calloc(4, sizeof(unsigned char));
    assert(finalHam);

    unsigned char * newHamming;

    int i, lowLim=0;
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

    alignedPacket(beforeHammingAligned, beforeHamming, noBlock); //beforeHammingAligned is updated
    actualAddHam(finalHam, beforeHammingAligned);
    return finalHam;
}

void alignedPacket(unsigned char * beforeHammingAligned, unsigned char * beforeHamming, int noBlock) //actually aligning the packet according to its noBlock
{
    int forShift;
    switch (noBlock)
    {
    case 1:
        beforeHamming[3] = beforeHamming[3] & 3; // 3 = 0000 0011
        forShift = *((int*)beforeHamming);
        *(int*)beforeHammingAligned = forShift;
        break;

    case 2:
        beforeHamming[0] = beforeHamming[0] & 252; // 252 = 1111 1100
        beforeHamming[3] = beforeHamming[3] & 15; // 15 = 0000 1111
        forShift = *((int*)beforeHamming);
        forShift = forShift >> 2;
        *(int*)beforeHammingAligned = forShift;
        break;

    case 3:
        beforeHamming[0] = beforeHamming[0] & 240; // 240 = 1111 0000
        beforeHamming[3] = beforeHamming[3] & 63; // 63 = 0011 1111
        forShift = *((int*)beforeHamming);
        forShift = forShift >> 4;
        *(int*)beforeHammingAligned = forShift;
        break;

    case 4:
        
        forShift = *((int*)beforeHamming);
        forShift = forShift >> 6;
        *(int*)beforeHammingAligned = forShift;
        break;
    }
}

void actualAddHam(unsigned char* finalHamm, unsigned char* beforeHammingAligned)
{
    int accumulativeBits;
    int tempXor=0;
    int bits26Num = *((int*)beforeHammingAligned);
                        
    int b1, b2, b4, b8, b16, totalXors, bit31Num;
    
    b1 = bits26Num & 44739931; // 10 1010 1010 1010 1101 0101 1011
    b1 = xorTree(b1);

    b2 = bits26Num & 53687917; // 11 0011 0011 0011 0110 0110 1101
    b2 = xorTree(b2);

    b4 = bits26Num & 63162254; //11 1100 0011 1100 0111 1000 1110
    b4 = xorTree(b4);

    b8 = bits26Num & 66848752; //11 1111 1100 0000 0111 1111 0000
    b8 = xorTree(b8);

    b16 = bits26Num & 67106816;
    b16 = xorTree(b16);

    b1 = b1 << 26;
    b2 = b2 << 27;
    b4 = b4 << 28;
    b8 = b8 << 29;
    b16 = b16 << 30;

    totalXors = b1 | b2 | b4 | b8 | b16;

    bit31Num = bits26Num | totalXors;
    *(int*)finalHamm = bit31Num;

}

int xorTree(unsigned int num)
{
    int retVal=0;
    while (num > 0)
    {
        retVal = retVal ^ (num & 1);
        num = num >> 1;
    }
    return retVal;
    
}


int main(int argc, char *argv[])
{
    ipChannel = argv[1];
    portChannel = atoi(argv[2]);
    
    printf("enter file name:\n");
    gets(fileName);

    while(strcmp(fileName, "quit"))
	{
        mainSender();
        printf("enter file name:\n");
        gets(fileName);
    }
}

