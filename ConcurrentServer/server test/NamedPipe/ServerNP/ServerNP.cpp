// ServerNP.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <ctime>
using namespace std;
// SERWER

string  GetErrorMsgText(int code)    // cформировать текст ошибки 
{
	string msgText;

	switch (code)                      // проверка кода возврата  
	{
	case WSAEINTR:          msgText = "WSAEINTR";         break;
	case WSAEACCES:         msgText = "WSAEACCES";        break;
	case WSAEFAULT:         msgText = "WSAEFAULT";        break;
	case WSAEINVAL:         msgText = "WSAEINVAL";        break;
	case WSAEMFILE:         msgText = "WSAEMFILE";        break;
	case WSAEWOULDBLOCK:    msgText = "WSAEWOULDBLOCK";   break;
	case WSAEINPROGRESS:    msgText = "WSAEINPROGRESS";   break;
	case WSAEALREADY:       msgText = "WSAEALREADY";      break;
	case WSAENOTSOCK:       msgText = "WSAENOTSOCK";      break;
	case WSAEDESTADDRREQ:   msgText = "WSAEDESTADDRREQ";  break;
	case WSAEMSGSIZE:       msgText = "WSAEMSGSIZE";      break;
	case WSAEPROTOTYPE:     msgText = "WSAEPROTOTYPE";    break;
	case WSAENOPROTOOPT:    msgText = "WSAENOPROTOOPT";   break;
	case WSAEPROTONOSUPPORT:msgText = "WSAEPROTONOSUPPORT"; break;
	default:                msgText = "***ERROR***";      break;
	};
	return msgText;
};
string  SetPipeError(string msgText, int code)
{
	return  msgText + GetErrorMsgText(code);
};

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	char rbuf[40]; //буфер для чтения
	char wbuf[40]; //буфер для записи
	DWORD dwRead; // Количество принятых байт
	DWORD dwWrite; // Количество записанных байт
	HANDLE hPipe; // дескриптор канала
	DWORD myTime = 5;
	try
	{
		if ((hPipe = CreateNamedPipe(L"\\\\.\\pipe\\BOX",
			PIPE_ACCESS_DUPLEX,           //дуплексный канал 
			PIPE_TYPE_MESSAGE | PIPE_WAIT,  // сообщения|синхронный
			1, NULL, NULL,               // максимум 1 экземпляр
			INFINITE, NULL)) == INVALID_HANDLE_VALUE)throw SetPipeError("create:", GetLastError());
		if (!ConnectNamedPipe(hPipe, NULL))   throw SetPipeError("connect:", GetLastError());

	}
	catch (string ErrorPipeText)
	{
		printf("\n%s", ErrorPipeText.c_str());
	}
	while (true) {
		ConnectNamedPipe(hPipe, NULL);
		for (;;) {
			bool check = ReadFile
			(
				hPipe,   // [in] дескриптор  канала
				rbuf,   // [out] указатель на буфер  ввода
				sizeof(rbuf),   // [in] количество читаемых байт
				&dwRead,   // [out] количество прочитанных байт  
				NULL    // [in,out] для асинхронной обработки 
			);
			if (check == FALSE) break;
			printf("Сообщение от клиента:  %s\n", rbuf);
			WriteFile
			(
				hPipe,   // [in] дескриптор  канала
				rbuf,   // [in] указатель на буфер  вывода
				sizeof(rbuf),   // [in] количество записываемых байт
				&dwWrite,   // [out] количество записанных байт  
				NULL    // [in,out] для асинхронной обработки 
			);
		}
		puts("--------------Канал закрыт-----------------");
		DisconnectNamedPipe(hPipe);
	}
	CloseHandle(hPipe);
	system("pause");
	return 0;
}

