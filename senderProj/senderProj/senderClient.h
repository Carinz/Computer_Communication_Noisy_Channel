
#include "..//..//SocketSendRecvTools.h"

void mainSender();
int createConnectSocketSender();
void sender_cleanup_1();
//TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd );
char * addHamming(int noBlock); //input: pointer to 26 bits,  output: a new int pointer to the addition to 31 bits 
void alignedPacket(char* beforeHammingAligned, char* beforeHamming, int noBlock);
void actualAddHam(char* finalHamm, char* beforeHammingAligned);
int xorTree(unsigned int num);
