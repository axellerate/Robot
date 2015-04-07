/*
Kris and Sandeep's Robot Controller

*/


#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

struct Robot_Pkt{
	//15 = DRIVE/5 = STATUS/0 = SLEEP
	unsigned char PacketId;
	//1 - 20 BYTES (10 for directions, 10 for durations)
	unsigned char CmdListSize;
	//1 = FORWARDS/2 = BACKWARDS/3 = LEFT/ 4 = RIGHT
	unsigned char * Direction;
	//duration in seconds
	unsigned char * Duration;
	//number of bits set to 1
	unsigned char Parity;
};


struct Response_Pkt{
	unsigned char PacketId;
	unsigned char CmdListSize;
	unsigned char* StatusData;
	unsigned char Parity;

};

class PktDef{

	Robot_Pkt packet_str;
	Response_Pkt response_str;
	sockaddr_in SvrAddr;
	SOCKET WelcomeSocket, ConnectionSocket;
	WSADATA wsaData;
	int command;
	int size;
	char * SxBuffer;
	char * RxBuffer;


public:
	PktDef();
	void GetCommand();
	void NumberOfCommands();
	void Commands();
	void CalculateParity();
	void SendPacket();
	int CountOnesFromInteger(unsigned int);
	void GetResponse();

	void Sleep();
	void Move();
	void Status();


};