
#ifndef SERVER_H
#define SERVER_H
#include "../../SocketSendRecvTools.h"

void initializeServer();
void MainServer();
int addNoise(int tmpStartIndex, int* noFlipBits);
int addNoiseRand(int* noflipBits);
int randIndicator();
int addNoiseDet(int tmpStartIndex, int* noFlipped);
void createSocket(SOCKET * mainSocket, char * type, int); //TODO: DELETE INT
void clientConnect(SOCKET * acceptSocket, SOCKET * mainSocket);
void gracefullyDiscSender();
void gracefullyDiscReciever();

#endif 