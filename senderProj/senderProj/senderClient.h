
#include "..//..//SocketSendRecvTools.h"

void mainSender();
int createConnectSocketSender();
char * addHamming(int noBlock); //input: pointer to 26 bits,  output: a new int pointer to the addition to 31 bits 
void alignedPacket(unsigned char* beforeHammingAligned, unsigned char* beforeHamming, int noBlock);
void actualAddHam(unsigned char* finalHamm, unsigned char* beforeHammingAligned);
int xorTree(unsigned int num);
