// ClientNPt.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <ctime>
using namespace std;
#define _CRT_SECURE_NO_WARNINGS
#define _itoa_s insted
#pragma warning(disable : 4996)

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
	HANDLE hPipe;
	bool check;
	char rbuf[50];
	char wbuf[50] = "HelloFromTClient";
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	DWORD dwRead;
	setlocale(LC_ALL, "rus");

	try
	{
		if ((hPipe = CreateFile(
			L"\\\\BNT\\pipe\\ConsolePipe",
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, NULL,
			NULL)) == INVALID_HANDLE_VALUE)
			throw  SetPipeError("createfile:", GetLastError());
	}
	catch (string ErrorPipeText)
	{
		cout << endl << ErrorPipeText;
	}

	try {
		/*	if (check = SetNamedPipeHandleState
		(
		hPipe,    // [in] дескриптор именованного канала
		&dwMode,   // [in]  указатель на новое состояние канала
		NULL,   // [in]  указатель на макс. к-во байтов
		NULL    // [in]  указатель на  интервал задержки
		) == FALSE)throw  SetPipeError("SetHendleState:", GetLastError());*/
	}
	catch (string ErrorPipeText)
	{
		cout << endl << ErrorPipeText;
	}
	unsigned int n;
	cout << "Введите количество сообщений:  "; cin >> n;
	for (unsigned int i = 1; i <= n; i++) {
		char wbuf[40] = "HelloFromTClient";
		itoa(i, wbuf + sizeof("HelloFromTClient") - 1, 10);
		check = TransactNamedPipe
		(
			hPipe,   // [in] дескриптор  канала
			wbuf,   // [in] указатель на буфер для записи  
			sizeof(wbuf),   // [in] размер буфера для записи
			rbuf,   // [out] указатель на буфер для чтения 
			sizeof(rbuf),   // [in] размер буфера для чтения
			&dwRead,   // [out] количество прочитанных байт
			NULL    // [in,out] для асинхронного доступа         
		);
		cout << "Отправлено:   " << wbuf << endl;
		cout << "Получено от сервера:   " << rbuf << endl;
	}
	CloseHandle(hPipe);
	system("pause");
	return 0;
}

