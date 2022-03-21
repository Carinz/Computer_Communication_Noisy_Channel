#define _CRT_SECURE_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <assert.h>
#include <stdlib.h>

#include "Ws2tcpip.h"

#include "recieverClient.h"
#include "../../socketShared.h"
#include "../../SocketSendRecvTools.h"

SOCKET recieverSocket;
int indexErr;
char * ipChannel;
int portChannel;
unsigned char lettersPacket[16];
char finalDecodedBuffer[14];
FILE* filePtr;
char fileName[100];//[6] = {'c','.','t','x','t','\0'}; //TODO CHANGE
//char bufferSend [SENDER_PACKET_SIZE] = {'H', 'e', 'l', 'l', 'o', '\0'};
//char lettersPacket[NO_LETTERS_PACKET];
//SOCKET acceptSocketReciever;
unsigned char recieveBuffer[SENDER_PACKET_SIZE];

void mainReciever()
{
    //int i;
    double bytesRecieved=0.0;
    int j, tmpFliped=0, actualNoBytes=0, bytesRecievedInt=0;

    TransferResult_t statusRecieve;
    int connectStatus, shutRes, noCorrected=0;
    unsigned char* afterHamming;
    //FILE* fileptr;

    // Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (StartupRes != NO_ERROR)
    {
        printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
        // Tell the user that we could not find a usable WinSock DLL.                                  
        assert(0);
        //return;
    }

    connectStatus = createConnectSocketReciever();

    if (connectStatus != NO_ERROR)
    {
        printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
        // Tell the user that we could not find a usable WinSock DLL.                                  
        //sender_cleanup_1();
        assert(0);
    }
    //SendBuffer(bufferSend, SENDER_PACKET_SIZE, senderSocket);

    //printf("CLIENT CONNECT!\n");

    filePtr = fopen(fileName, "w");
    assert(filePtr);

    do
    {
        for (int i = 0; i < 4; i++) {
        
            statusRecieve = ReceiveBuffer(recieveBuffer, SENDER_PACKET_SIZE, recieverSocket);
            if (statusRecieve == TRNS_FAILED)
            {
                //TODO: handle
                printf("failed with recieving from channel\n");
                assert(0);
            }

            if (statusRecieve == TRNS_DISCONNECTED)
            {
                gracefullyDiscFromChannel(&recieverSocket);
                //gracefullyDisC(&acceptSocketReciever);
                break;
            }

            //printf("THE MESSAGE: %s\n", recieveBuffer);
            //for (j=0;j<4;j++)
            tmpFliped = calculateIndexErr();
            noCorrected += tmpFliped;
            //changeErrorBit();
            for (j = 0; j < 4; j++)
            {
                lettersPacket[i * 4 + j] = recieveBuffer[j]; //only flipped bit
            }
            //lettersPacket[4 * i] = recieveBuffer; 
            
        }

        if (statusRecieve == TRNS_SUCCEEDED)
        {
            mergingString();
            //fwrite(finalDecodedBuffer, 1, 13, filePtr);
            fputs(finalDecodedBuffer, filePtr);

            bytesRecieved += 15.5; //TODO: check
            actualNoBytes += 13;
        }
        
    } while (statusRecieve == TRNS_SUCCEEDED);

    //assert(filePtr);
    //lettersPacket[0] = fgetc(filePtr);

    
    /*shutRes = shutdown(recieverSocket, SD_SEND);
    if (shutRes == SOCKET_ERROR)
    {
        printf("shutdown failed with error %ld. Ending program\n", WSAGetLastError());
        assert(0);
    }*/
    //recieve final trans - maybe how many transmitted to server
    /*printf("I AM CLUENT AND SHTTING\n");
    closesocket(recieverSocket);*/

    //print how many bytes in file
    //print how many sent ( /26 * 31)
    fclose(filePtr);

    bytesRecievedInt = (int)bytesRecieved;

    printf("recieved: %d bytes\n", bytesRecievedInt);
    printf("wrote: %d bytes\n", actualNoBytes);
    printf("corrected %d errors\n", noCorrected);
   

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
        //sender_cleanup_1();
        assert(0);
        //goto server_cleanup_1;
    }

    portChannel = 63107; //TODO: DELETE
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ipChannel);
    service.sin_port = htons(portChannel); //The htons function converts a u_short from host to TCP/IP network byte order 


    connectRes = connect(recieverSocket, (SOCKADDR*)&service, sizeof(service));

    //printf("waiting to connect...");

    return connectRes;
}

//void sender_cleanup_1()
//{
//    if (WSACleanup() == SOCKET_ERROR)
//        printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
//    assert(0);
//}

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


//char* addHamming(int noBlock) //input: pointer to 26 bits,  output: a new int pointer to the addition to 31 bits 
//{
//    char* beforeHamming = (char*)calloc(4, sizeof(char));
//    char* newHamming;
//
//    int i, lowLim;
//    switch (noBlock)
//    {
//    case 1:
//        lowLim = 0;
//        break;
//
//    case 2:
//        lowLim = 3;
//        break;
//
//    case 3:
//        lowLim = 6;
//        break;
//
//    case 4:
//        lowLim = 9;
//        break;
//    }
//
//    for (i = 0; i < 4; i++)
//    {
//        beforeHamming[i] = lettersPacket[lowLim + i];
//    }
//
//    //newHamming = actualAddHam(beforeHamming);
//    //return newHamming;
//    return beforeHamming;
//}

//char* actualAddHam(char* beforeHamming)
//{
//    char* withHamming = (char*)calloc(4, sizeof(char));
//
//
//}

int calculateIndexErr()
{
    int b1, b2, b4, b8, b16, c1, c2, c4, c8, c16, totalXors, bit31Num;
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
    c1 = xorTree(c1);
    c1 = c1 ^ b1;

    c2 = bits31Num & 53687917; // 11 0011 0011 0011 0110 0110 1101 0000 00
    c2 = xorTree(c2);
    c2 = c2 ^ b2;

    c4 = bits31Num & 63162254; //11 1100 0011 1100 0111 1000 1110 0000 00
    c4 = xorTree(c4);
    c4 = c4 ^ b4;

    c8 = bits31Num & 66848752; //11 1111 1100 0000 0111 1111 0000 0000 00
    c8 = xorTree(c8);
    c8 = c8 ^ b8;

    c16 = bits31Num & 67106816;
    c16 = xorTree(c16);
    c16 = c16 ^ b16;

    //linking all the bits to one variable- the error bit index
    indexErr = c1 | (c2 << 1) | (c4 << 2) | (c8 << 3) | (c16 << 4);

    //convertion of the error bit index
    if (indexErr == 0)
        return 0;

    
    if (indexErr == 3)
        indexErr = indexErr - 3;

    else if (indexErr > 4 && indexErr < 8)
        indexErr = indexErr - 4;

    else if (indexErr > 8 && indexErr < 16)
        indexErr = indexErr - 5;

    else if (indexErr > 16)
        indexErr = indexErr - 6;
    else
        return 1;
    //TODO: verify that its ok might be a problem
    changeErrorBit();
    return 1;
}

//fixing the error bit 
void changeErrorBit() {
    //int bits31Num = (int*)calloc(1, sizeof(int));
    int bits31Num = *((int*)recieveBuffer);
    //int copyBuffer = bits31Num;
    int bil = 1;

    bil = bil << (indexErr);
    bits31Num = bits31Num ^ bil;

    *((int*)recieveBuffer) = bits31Num;
    
}

void mergingString() {
    //nullify all the hamming index bits
    char temp=0;
    unsigned int *bits31Num = (int*)(lettersPacket);
    for (int i = 0; i < 4; i++)
    {
        *(bits31Num+i) = *(bits31Num+i) & 67108863; //0000 0011 1111 1111 ... 1111 32bit
    }
    //first shirshur

    temp = lettersPacket[4] & 63; //0011 1111 
    lettersPacket[3] = temp<<2 | lettersPacket[3];
    //lettersPacket[3] = temp;
    *(bits31Num + 1) = (*(bits31Num+1)) >> 6;


    //second shirshur
    temp = lettersPacket[8] & 15; //0000 1111
    lettersPacket[6] = temp<<4 | lettersPacket[6];
    *(bits31Num + 2) = (*(bits31Num+2)) >> 4;


    //third shishur
    temp = lettersPacket[12] & 3; //0000 0011
    lettersPacket[10] = temp << 6 | lettersPacket[10];
    *(bits31Num + 3) = *(bits31Num + 3) >> 2;

    finalDecodedBuffer[0] = lettersPacket[0];
    finalDecodedBuffer[1] = lettersPacket[1];
    finalDecodedBuffer[2] = lettersPacket[2];
    finalDecodedBuffer[3] = lettersPacket[3];
    finalDecodedBuffer[4] = lettersPacket[4];
    finalDecodedBuffer[5] = lettersPacket[5];
    finalDecodedBuffer[6] = lettersPacket[6];
    finalDecodedBuffer[7] = lettersPacket[8];
    finalDecodedBuffer[8] = lettersPacket[9];
    finalDecodedBuffer[9] = lettersPacket[10];
    finalDecodedBuffer[10] = lettersPacket[12];
    finalDecodedBuffer[11] = lettersPacket[13];
    finalDecodedBuffer[12] = lettersPacket[14];
    finalDecodedBuffer[13] = '\0';

}

void gracefullyDiscFromChannel()
{
    int shutRes;
    
    //printf("I AM CLUENT AND SHTTING\n");
    shutRes = shutdown(recieverSocket, SD_SEND);
    if (shutRes == SOCKET_ERROR)
    {
        printf("shutdown failed with error %ld. Ending program\n", WSAGetLastError());
        assert(0);
    }
    closesocket(recieverSocket); 
}

int xorTree(unsigned int num)
{
    int retVal = 0;
    while (num > 0)
    {
        retVal = retVal ^ (num & 1);
        num = num >> 1;
    }
    return retVal;

}

int main(int argc, char* argv[])
{
   
    ipChannel = argv[1];
    portChannel = atoi(argv[2]);
    //mainSender();
    //printf("Welcome to the Reciever:\n");
    printf("enter file name:\n");
    gets(fileName);
    //fileName=args[1];
    while (strcmp(fileName, "quit"))
    {
        mainReciever();
        printf("enter file name:\n");
        gets(fileName);
    }
}

