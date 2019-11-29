// ServerT.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib") 
#include <string>
#include <iostream>
#include <ctime>
#include <chrono>

using namespace std;
using namespace chrono;
//...................................................................
string  GetErrorMsgText(int code)    // cформировать текст ошибки 
{
	string msgText;
	switch (code)                      // проверка кода возврата  
	{
	case WSAEINTR:          msgText = "WSAEINTR";         break;
	case WSAEACCES:         msgText = "WSAEACCES";        break;

	case WSAEFAULT:          msgText = "WSAEFAULT";         break;
	case WSAEINVAL:         msgText = "WSAEINVAL";        break;
	case WSAEMFILE:          msgText = "WSAEMFILE";         break;
	case WSAEWOULDBLOCK:         msgText = "WSAEWOULDBLOCK";        break;
	case WSAEINPROGRESS:          msgText = "WSAEINPROGRESS";         break;
	case WSAEALREADY:         msgText = "WSAEALREADY";        break;
	case WSAENOTSOCK:          msgText = "WSAENOTSOCK";         break;
	case WSAEDESTADDRREQ:         msgText = "WSAEDESTADDRREQ";        break;
	case WSAEMSGSIZE:          msgText = "WSAEMSGSIZE";         break;
	case WSAEPROTOTYPE:         msgText = "WSAEPROTOTYPE";        break;
	case WSAENOPROTOOPT:          msgText = "WSAENOPROTOOPT";         break;
	case WSAEPROTONOSUPPORT:         msgText = "WSAEPROTONOSUPPORT";        break;
	case WSAESOCKTNOSUPPORT:          msgText = "WSAESOCKTNOSUPPORT";         break;
	case WSAEOPNOTSUPP:         msgText = "WSAEOPNOTSUPP";        break;
	case WSAEPFNOSUPPORT:          msgText = "WSAEPFNOSUPPORT";         break;
	case WSAEAFNOSUPPORT:         msgText = "WSAEAFNOSUPPORT";        break;
	case WSAEADDRINUSE:          msgText = "WSAEADDRINUSE";         break;
	case WSAEADDRNOTAVAIL:         msgText = "WSAEADDRNOTAVAIL";        break;
	case WSAENETDOWN:          msgText = "WSAENETDOWN";         break;
	case WSAENETUNREACH:         msgText = "WSAENETUNREACH";        break;
	case WSAENETRESET:          msgText = "WSAENETRESET";         break;
	case WSAECONNABORTED:         msgText = "WSAECONNABORTED";        break;
	case WSAECONNRESET:          msgText = "WSAECONNRESET";         break;
	case WSAENOBUFS:         msgText = "WSAENOBUFS";        break;
	case WSAEISCONN:          msgText = "WSAEISCONN";         break;
	case WSAENOTCONN:         msgText = "WSAENOTCONN";        break;
	case WSAESHUTDOWN:          msgText = "WSAESHUTDOWN";         break;
	case WSAETIMEDOUT:         msgText = "WSAETIMEDOUT";        break;
	case WSAECONNREFUSED:          msgText = "WSAECONNREFUSED";         break;
	case WSAEHOSTDOWN:         msgText = "WSAEHOSTDOWN";        break;
	case WSAEHOSTUNREACH:          msgText = "WSAEHOSTUNREACH";         break;
	case WSAEPROCLIM:         msgText = "WSAEPROCLIM";        break;
	case WSASYSNOTREADY:          msgText = "WSASYSNOTREADY";         break;
	case WSAVERNOTSUPPORTED:         msgText = "WSAVERNOTSUPPORTED";        break;
	case WSANOTINITIALISED:          msgText = "WSANOTINITIALISED";         break;
	case WSAEDISCON:         msgText = "WSAEDISCON";        break;
	case WSATYPE_NOT_FOUND:          msgText = "WSATYPE_NOT_FOUND";         break;
	case WSAHOST_NOT_FOUND:         msgText = "WSAHOST_NOT_FOUND";        break;
	case WSATRY_AGAIN:          msgText = "WSATRY_AGAIN";         break;
	case WSANO_RECOVERY:         msgText = "WSANO_RECOVERY";        break;
	case WSANO_DATA:          msgText = "WSANO_DATA";         break;
	case WSA_INVALID_HANDLE:         msgText = "WSA_INVALID_HANDLE";        break;
	case WSA_INVALID_PARAMETER:          msgText = "WSA_INVALID_PARAMETER";         break;
	case WSA_IO_INCOMPLETE:         msgText = "WSA_IO_INCOMPLETE";        break;
	case WSA_IO_PENDING:          msgText = "WSA_IO_PENDING";         break;
	case WSA_NOT_ENOUGH_MEMORY:         msgText = "WSA_NOT_ENOUGH_MEMORY";        break;
	case WSA_OPERATION_ABORTED:          msgText = "WSA_OPERATION_ABORTED";         break;
	case WSAEINVALIDPROCTABLE:         msgText = "WSAEINVALIDPROCTABLE";        break;
	case WSAEINVALIDPROVIDER:          msgText = "WSAEINVALIDPROVIDER";         break;
	case WSAEPROVIDERFAILEDINIT:         msgText = "WSAEPROVIDERFAILEDINIT";        break;
		//..........коды WSAGetLastError ..........................
	case WSASYSCALLFAILURE: msgText = "WSASYSCALLFAILURE"; break;
	default:                msgText = "***ERROR***";      break;
	};
	return msgText;
};

string  SetErrorMsgText(string msgText, int code)
{
	string error = msgText + GetErrorMsgText(code);
	error.append(" ").append(to_string(code));
	return error;
};


int _tmain(int argc, char* argv[])
{
	SOCKET  cS;           // дескриптор сокета 
	WSADATA wsaData;
	for (int connectionCounter = 0; connectionCounter < 1; connectionCounter++) {
		try
		{
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
				throw  SetErrorMsgText("Startup:", WSAGetLastError());
			if ((cS = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
				throw  SetErrorMsgText("socket:", WSAGetLastError());
			//...........................................................
			SOCKET  cC;                          // серверный сокет
			if ((cC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
				throw  SetErrorMsgText("socket:", WSAGetLastError());

			SOCKADDR_IN serv;                    // параметры  сокета сервера
			serv.sin_family = AF_INET;           // используется IP-адресация  
			serv.sin_port = htons(2000);                   // TCP-порт 2000
			serv.sin_addr.s_addr = inet_addr("192.168.43.52");  // адрес сервера
			if ((connect(cC, (sockaddr*)&serv, sizeof(serv))) == SOCKET_ERROR)
				throw  SetErrorMsgText("connect:", WSAGetLastError());
			//..............................................................

			char message[50],                     //буфер ввода 
				obuf[50]; /*= "sever: принято ";  //буфер вывода*/
			int  libuf = 0,                    //количество принятых байт
				lobuf = 0;                    //количество отправленных байь 

			
			char outMessage[5] = "Echo";

			//Sleep(4000);
			if ((lobuf = send(cC, outMessage, strlen(outMessage) + 1, NULL)) == SOCKET_ERROR)
				throw  SetErrorMsgText("send:", WSAGetLastError());


			cout << "seneded: " << outMessage << endl;
			if (!strcmp(outMessage, "Echo")) {

				if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*

				if (strcmp(message, "TimeOUT") == 0) {
					cout << "time out" << endl;
					return -1;
				}
				for (int j = 10; j >= 0; j--) {

					Sleep(1000);
					itoa(j, outMessage, 10);

					if ((lobuf = send(cC, outMessage, strlen(outMessage) + 1, NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("send:", WSAGetLastError());
					cout << "send:" << outMessage << endl;


					if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*


					if (strcmp(message, "TimeOUT") == 0) {
						cout << "time out" << endl;
						return -1;
					}

					cout << "receive:" << message << endl;


				}
			}
			else if (!strcmp(outMessage, "Time")) {

				
				if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*




				cout << "receive:" << message << endl;
			}

			else if (!strcmp(outMessage, "0001")) {

				if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*

				if (strcmp(outMessage, "TimeOUT") == 0) {
					cout << "time out" << endl;
					return -1;
				}
				cout << "recv random message:" << message << endl;
				
			}
			else if (strcmp(outMessage, "Echo") != 0 && strcmp(outMessage, "Time") && strcmp(outMessage, "0001")) {
				if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*

				cout << "receive:" << message << endl;
			}


			if (closesocket(cS) == SOCKET_ERROR)
				throw  SetErrorMsgText("closesocket:", WSAGetLastError());
			if (WSACleanup() == SOCKET_ERROR)
				throw  SetErrorMsgText("Cleanup:", WSAGetLastError());


			//	cout << strlen(obuf);
		}
		catch (string errorMsgText)
		{
			cout << endl << errorMsgText;
		}
	}
	return 0;
}
