// ClientNPct.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "rus");
	BOOL check;
	char wbuf[30] = "HelloFromCallNamedPipe";
	char rbuf[30];
	DWORD dwRead;


	check = CallNamedPipe
	(
		L"\\\\BNT\\pipe\\ConsolePipe",   // [in] указатель на имя канала
		wbuf,   // [in] указатель на буфер для записи  
		sizeof(wbuf),   // [in] размер буфера для записи
		rbuf,   // [out] указатель на буфер для чтения 
		sizeof(rbuf),   // [in] размер буфера для чтения
		&dwRead,  // [out] количество прочитанных байт
		NMPWAIT_USE_DEFAULT_WAIT  //  [in] интервал ожидания         
	);


	if (check || GetLastError() == ERROR_MORE_DATA)
	{
		cout << "Отправлено:   " << wbuf << endl;
		cout << "Получено от сервера:   " << rbuf << endl;
	}


	return 0;
}

