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

SOCKET recieverSocket;
int indexErr;
int ipChannel;
int portChannel = SERVER_PORT_SENDER;
char lettersPacket[16];
char finalDecodedBuffer[13];
FILE* filePtr;
char fileName[100];//[6] = {'c','.','t','x','t','\0'}; //TODO CHANGE
//char bufferSend [SENDER_PACKET_SIZE] = {'H', 'e', 'l', 'l', 'o', '\0'};
//char lettersPacket[NO_LETTERS_PACKET];
SOCKET acceptSocketReciever;
char recieveBuffer[SENDER_PACKET_SIZE];

void mainReciever()
{
    int i;
    TransferResult_t statusRecieve;
    int connectStatus, shutRes;
    char* afterHamming;


    // Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (StartupRes != NO_ERROR)
    {
        printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
        // Tell the user that we could not find a usable WinSock DLL.                                  
        return;
    }

    connectStatus = createConnectSocketReciever();

    if (connectStatus != NO_ERROR)
    {
        printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
        // Tell the user that we could not find a usable WinSock DLL.                                  
        sender_cleanup_1();
        assert(0);
    }
    //SendBuffer(bufferSend, SENDER_PACKET_SIZE, senderSocket);

    printf("CLIENT CONNECT!");

    do
    {
        for (int i = 0; i < 4; i++) {
        
            statusRecieve = ReceiveBuffer(recieveBuffer, SENDER_PACKET_SIZE, acceptSocketReciever);
            if (statusRecieve == TRNS_FAILED)
            {
                //TODO: handle
                printf("FAIL!");
                assert(0);
            }

            if (statusRecieve == TRNS_DISCONNECTED)
            {
                gracefullyDisC(&acceptSocketReciever);
                //gracefullyDisC(&acceptSocketReciever);
                break;
            }
                printf("THE MESSAGE: %s\n", recieveBuffer);
                //for (j=0;j<4;j++)
            reHamming();
            changeErrorBit();
            lettersPacket[4 * i] = recieveBuffer;
            
        }
        mergingString();
        
        
    } while (statusRecieve == TRNS_SUCCEEDED);

    //filePtr = fopen(fileName, "r");
    //assert(filePtr);
    //lettersPacket[0] = fgetc(filePtr);

    
    shutRes = shutdown(recieverSocket, SD_SEND);
    if (shutRes == SOCKET_ERROR)
    {
        printf("shutdown failed with error %ld. Ending program\n", WSAGetLastError());
        assert(0);
    }
    //recieve final trans - maybe how many transmitted to server
    printf("I AM CLUENT AND SHTTING");
    closesocket(recieverSocket);

    //print how many bytes in file
    //print how many sent ( /26 * 31)


}

int createConnectSocketReciever()
{
    SOCKADDR_IN service;
    int connectRes;

    // Create a socket.    
    recieverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (recieverSocket == INVALID_SOCKET)
    {
        printf("Error at socketSender( ): %ld\n", WSAGetLastError());
        sender_cleanup_1();
        //assert(0);
        //goto server_cleanup_1;
    }

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(S123);
    service.sin_port = htons(portChannel); //The htons function converts a u_short from host to TCP/IP network byte order 


    connectRes = connect(recieverSocket, (SOCKADDR*)&service, sizeof(service));

    printf("waiting to connect...");

    return connectRes;
}

void sender_cleanup_1()
{
    if (WSACleanup() == SOCKET_ERROR)
        printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
    assert(0);
}

//TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd )
//{
//	const char* CurPlacePtr = Buffer;
//	int BytesTransferred;
//	int RemainingBytesToSend = BytesToSend;
//	
//	while ( RemainingBytesToSend > 0 )  
//	{
//		/* send does not guarantee that the entire message is sent */
//		BytesTransferred = send (sd, CurPlacePtr, RemainingBytesToSend, 0);
//		if ( BytesTransferred == SOCKET_ERROR ) 
//		{
//			printf("send() failed, error %d\n", WSAGetLastError() );
//			return TRNS_FAILED;
//		}
//		
//		RemainingBytesToSend -= BytesTransferred;
//		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
//	}
//
//	return TRNS_SUCCEEDED;
//}


char* addHamming(int noBlock) //input: pointer to 26 bits,  output: a new int pointer to the addition to 31 bits 
{
    char* beforeHamming = (char*)calloc(4, sizeof(char));
    char* newHamming;

    int i, lowLim;
    switch (noBlock)
    {
    case 1:
        lowLim = 0;
        break;

    case 2:
        lowLim = 3;
        break;

    case 3:
        lowLim = 6;
        break;

    case 4:
        lowLim = 9;
        break;
    }

    for (i = 0; i < 4; i++)
    {
        beforeHamming[i] = lettersPacket[lowLim + i];
    }

    //newHamming = actualAddHam(beforeHamming);
    //return newHamming;
    return beforeHamming;
}

char* actualAddHam(char* beforeHamming)
{
    char* withHamming = (char*)calloc(4, sizeof(char));


}

void reHamming()
{
    int b1, b2, b4, b8, b16, c1, c2, c4, c8, c16, totalXors, bit31Num, indexErr;
    int bits31Num = *((int*)recieveBuffer);

    //chossing 27-31 bits

    b1 = bits31Num & 67108864; // 0000 0000 0000 0000 0000 0000 0010 0000 
    //b1 = xorTree(b1);
    b1 = b1 >> 26;

    b2 = bits31Num & 134217728; // 0000 0000 0000 0000 0000 0000 0001 0000
    b2 = b2 >> 27;

    b4 = bits31Num & 268435456; //0000 0000 0000 0000 0000 0000 0000 1000
    b4 = b4 >> 28;

    b8 = bits31Num & 536870912; //0000 0000 0000 0000 0000 0000 000 0100
    b8 = b8 >> 29;

    b16 = bits31Num & 1073741824;//0000 0000 0000 0000 0000 0000 000 0010
    b16 = b16 >> 30;



    //calculation of Hamming bits- the index of the error bit
    c1 = bits31Num & 44739931; // 10 1010 1010 1010 1101 0101 1011 0000 00
    c1 = xorTree(b1);
    c1 = c1 ^ b1;

    b2 = bits31Num & 53687917; // 11 0011 0011 0011 0110 0110 1101 0000 00
    b2 = xorTree(b2);
    c2 = c2 ^ b2;

    b4 = bits31Num & 63162254; //11 1100 0011 1100 0111 1000 1110 0000 00
    b4 = xorTree(b4);
    c4 = c4 ^ b4;

    b8 = bits31Num & 66848752; //11 1111 1100 0000 0111 1111 0000 0000 00
    b8 = xorTree(b8);
    c8 = c8 ^ b8;

    b16 = bits31Num & 67106816;
    b16 = xorTree(b16);
    c16 = c16 ^ b16;

    //linking all the bits to one variable- the error bit index
    indexErr = c16 | (c8 << 1) | (c4 << 2) | (c2 << 3) | (c1 << 4);

    //convertion of the error bit index
    if (indexErr == 3)
        indexErr = indexErr - 2;

    else if (indexErr > 4 && indexErr < 8)
        indexErr = indexErr - 3;

    else if (indexErr > 8 && indexErr < 16)
        indexErr = indexErr - 4;

    else if (indexErr > 16)
        indexErr = indexErr - 5;
    else
        return;

    changeErrorBit();
}

//fixing the error bit 
void changeErrorBit() {
    //int bits31Num = (int*)calloc(1, sizeof(int));
    int bits31Num = *((int*)recieveBuffer);
    //int copyBuffer = bits31Num;
    int bil = 1;

    bil = bil << (indexErr - 1);
    bits31Num = bits31Num ^ bil;

    *(int*)recieveBuffer = bits31Num;
    
}

void mergingString() {
    //nullify all the hamming index bits
    char temp=0;
    int *bits31Num = (int*)(lettersPacket);
    for (int i = 0; i < 4; i++)
    {
        lettersPacket[4 * i] = lettersPacket[4 * i] & 33554431; //0000 0011 1111 1111 ... 1111 32bit
    }
    //first shirshur
    temp = lettersPacket[4] & 63; //0011 1111 
    lettersPacket[3] = temp<<2 | lettersPacket[3];
    //lettersPacket[3] = temp;
    bits31Num = *(bits31Num+1) >> 6;


    //second shirshur
    temp = lettersPacket[7] & 3; //0000 0011
    lettersPacket[6] = temp<<6 | lettersPacket[6];
    bits31Num = *(bits31Num+2) >> 2;


    //third shishur
    temp = lettersPacket[10] & 15; //0000 1111
    lettersPacket[9] = temp << 4 | lettersPacket[9];
    bits31Num = *(bits31Num + 3) >> 4;

}

int main(int argc, char* argv[])
{
    //ipChannel = atoi(args[1]);
    //portChannel = atoi(args[2]);

    //mainSender();
    printf("Welcome to the Reciever:");
    //gets(fileName);
    //fileName=args[1];
    while (strcmp(fileName, "quit"))
    {
        mainReciever();
        printf("enter file name:");
        //gets(fileName);
    }
}

