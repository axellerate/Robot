/*
Kris and Sandeep's Robot Controller

*/



#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include "PktDef.h"
#include <iostream>

using namespace std;

PktDef::PktDef(){
	int iResult = 1;
	cout << "Starting up TCP server" << endl;

	//Initialize WinSock library using...
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Create a IPPROTO_TCP SOCKET using...
	WelcomeSocket = socket(AF_INET, SOCK_STREAM, 0);

	
	//allocate server information for the client
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = INADDR_ANY;
	SvrAddr.sin_port = htons(5000);

	//bind the WelcomeSocket to the server
	bind(WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr));

	listen(WelcomeSocket, 1);

	cout << "Waiting for the robot to connect" << endl;
	ConnectionSocket = SOCKET_ERROR;

	RxBuffer = new char[128];
	SxBuffer = new char[128];
	while (1) {
		if ((ConnectionSocket = accept(WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
			return;
		}
		else{

			cout << "Robot has connected to the server!" << endl;

			//Gets the initial command (Drive, Sleep or Get the Status)
			GetCommand();

			if (command == 15 || command == 5 || command == 0){

				if (command == 0){
					
					//Put the robot to sleep
					Sleep();

				}

				if (command == 5){
				
					//Get status of the robot
					Status();

				}
				
				if(command == 15){

					//Move the robot
					Move();
				}
				
			}
			else{
				cout << "You have entered an invalid command... please restart." << endl;
			}

		
		}
	}
}


void PktDef::GetCommand(){
	//what do you want the robot to do?
	cout << "Enter a packet ID(15 = DRIVE | 5 = STATUS | 0 = SLEEP): ";
	cin >> command;
	packet_str.PacketId = (unsigned char)command;

}

void PktDef::NumberOfCommands(){

	//how many commands do you want to send to the robot?
	cout << "Enter the number of commands (1 to 10): ";
	cin >> size;
	packet_str.CmdListSize = (unsigned char)size;
	packet_str.Direction = new unsigned char[size];
	packet_str.Duration = new unsigned char[size];

}

void PktDef::Commands(){

	int direction, duration;
	char * commands = new char[size * 2];
	int command_i = 0;
	for (int i = 0; i < size; i++){

		cout << "Enter a direction(1 = F | 2 = B | 3 = L | 4 = R):";
		cin >> direction;
		commands[command_i] = (unsigned int)direction;
		command_i++;
		packet_str.Direction[i] = (unsigned char)direction;
		cout << "Enter a duration: ";
		cin >> duration;
		commands[command_i] = (unsigned int)duration;
		command_i++;
		packet_str.Duration[i] = (unsigned char)duration;

	}

	SxBuffer[0] = packet_str.PacketId;
	SxBuffer[1] = packet_str.CmdListSize;

	int index = 0;
	for (int h = 0; h < size * 2; h++){
		SxBuffer[h + 2] = commands[index];
		index++;

	}

}

int PktDef::CountOnesFromInteger(unsigned int value) {
	int count;
	for (count = 0; value != 0; count++, value &= value - 1);
	return count;
}

void PktDef::CalculateParity(){

	size = (size * 2) + 2;
	short unsigned int parity = 0;

	for (int i = 0; i < size; i++){
		int value = (unsigned int)SxBuffer[i];
		parity = parity + CountOnesFromInteger(value);
	}

	SxBuffer[size] = parity;

	//testing purposes
	/*for (int i = 0; i < size + 1; i++){
		cout << (int)SxBuffer[i] << endl;

	}*/

}


void PktDef::SendPacket(){

	send(ConnectionSocket, SxBuffer, size + 1, 0);

}

void PktDef::GetResponse(){
	recv(ConnectionSocket, RxBuffer, 128, 0);
	//testing purposes
	/*for (int i = 0; i < 13 + 1; i++){
		cout << "Element " << i << ": " << (int)RxBuffer[i] << endl;

	}*/
	response_str.PacketId = RxBuffer[0];
	response_str.CmdListSize = RxBuffer[1];
	response_str.StatusData = new unsigned char[(int)response_str.CmdListSize];
	int index = 0;
	for (int i = 2; i < (int)response_str.CmdListSize + 2; i++){
		response_str.StatusData[index] = RxBuffer[i];
		index++;
	}
	response_str.Parity = RxBuffer[12];
	cout << "*Response Data*" << endl;
	cout << "PacketId: " << (int)response_str.PacketId << endl;
	cout << "Size: " << (int)response_str.CmdListSize << endl;

	for (int i = 0; i < (int)response_str.CmdListSize; i++){
		cout << "Body element " << i + 1 << ": " << (int)response_str.StatusData[i] << endl;
	}
	cout << "Parity: " << (int)response_str.Parity << endl;

}

void PktDef::Sleep(){

	cout << "Putting the robot to sleep..." << endl;
	size = 0;
	packet_str.CmdListSize = (unsigned char)size;
	packet_str.Direction = new unsigned char[size];
	packet_str.Duration = new unsigned char[size];
	Commands();
	//calculates the parity
	CalculateParity();

	//Sends the constructed packet to the robot
	SendPacket();
	closesocket(ConnectionSocket);
	cout << "Socket closed."
		<< endl;
	char close;
	cin >> close;

}

void PktDef::Move(){

	//Gets the number of commands the user wants to enter
	NumberOfCommands();

	//Gets each of the commands from the user
	Commands();

	//calculates the parity
	CalculateParity();

	//Sends the constructed packet to the robot
	SendPacket();

}


void PktDef::Status(){

	size = 0;
	packet_str.CmdListSize = (unsigned char)size;
	packet_str.Direction = new unsigned char[size];
	packet_str.Duration = new unsigned char[size];
	Commands();
	//calculates the parity
	CalculateParity();

	//Sends the constructed packet to the robot
	SendPacket();
	//Gets the status response from the robot
	GetResponse();

}