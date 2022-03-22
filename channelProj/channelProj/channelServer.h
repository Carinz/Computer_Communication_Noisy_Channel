#pragma once

#pragma comment(lib, "ws2_32.lib") 

#ifndef SERVER_H
#define SERVER_H
#include "../../SocketSendRecvTools.h"



#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <winsock2.h>


void initializeServer();
void MainServer();
int addNoise(int tmpStartIndex, int* noFlipBits);
int addNoiseRand(int* noflipBits);
int randIndicator();
int addNoiseDet(int tmpStartIndex, int* noFlipped);
void createSocket(SOCKET * mainSocket, char * type); 
void clientConnect(SOCKET * acceptSocket, SOCKET * mainSocket);
void gracefullyDiscSender();
void gracefullyDiscReciever();

#endif 