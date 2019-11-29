// ConcurrentServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Winsock2.h"
#include "ErrorFunctions.h"
#include <iostream>
#include <string>
#include <list>
#include <time.h>

#define AS_SQUIRT 10
using namespace std;

SOCKET sS;
int serverPort;
char dllName[50];
char namedPipeName[10];

HANDLE hAcceptServer,    // дескриптор потока AcceptServer
    	hConsolePipe,     // дескриптор потока ConsolePipe 
    	hGarbageCleaner,  // дескриптор потока GarbageCleaner
		hDispatchServer;  // дескриптор потока GarbageCleaner

HANDLE hClientConnectedEvent =  CreateEvent(NULL,
	FALSE, //автоматический сброс
	FALSE,
	L"ClientConnected");;

DWORD WINAPI AcceptServer(LPVOID pPrm);  // прототипы функций 
DWORD WINAPI ConsolePipe(LPVOID pPrm);
DWORD WINAPI GarbageCleaner(LPVOID pPrm);
DWORD WINAPI DispatchServer(LPVOID pPrm);    // прототип 

CRITICAL_SECTION scListContact;

enum TalkersCommand {
	START, STOP, EXIT, STATISTICS, WAIT, SHUTDOWN, GETCOMAND
};



TalkersCommand GETCOMMAND = TalkersCommand::START;


struct Contact         // элемент списка подключений       
{
	enum TE {               // состояние  сервера подключения  
		EMPTY,              // пустой элемент списка подключений 
		ACCEPT,             // подключен (accept), но не обслуживается
		CONTACT             // передан обслуживающему серверу  
	}    type;     // тип элемента списка подключений 
	enum ST {               // состояние обслуживающего сервера  
		WORK,               // идет обмен данными с клиентом
		ABORT,              // обслуживающий сервер завершился не нормально 
		TIMEOUT,            // обслуживающий сервер завершился по времени 
		FINISH              // обслуживающий сервер завершился  нормально 
	}      sthread; // состояние  обслуживающего сервера (потока)

	SOCKET      s;         // сокет для обмена данными с клиентом
	SOCKADDR_IN prms;      // параметры  сокета 
	int         lprms;     // длина prms 
	HANDLE      hthread;   // handle потока (или процесса) 
	HANDLE      htimer;    // handle таймера
	HANDLE		serverHThtead;// handle обслуживающего сервера который в последствие может зависнуть

	char msg[50];           // сообщение 
	char srvname[15];       //  наименование обслуживающего сервера 

	Contact(TE t = EMPTY, const char* namesrv = "") // конструктор 
	{
		memset(&prms, 0, sizeof(SOCKADDR_IN));
		lprms = sizeof(SOCKADDR_IN);
		type = t;
		strcpy(srvname, namesrv);
		msg[0] = 0;
	};

	void SetST(ST sth, const char* m = "")
	{
		sthread = sth;
		strcpy(msg, m);
	}
};
typedef list<Contact> ListContact;

ListContact contacts;




bool AcceptCycle(int squirt)
{
	bool rc = false;
	Contact c(Contact::ACCEPT, "EchoServer");

	while (squirt-- > 0 && rc == false)
	{
	
		if ((c.s = accept(sS,
			(sockaddr*)&c.prms, &c.lprms)) == INVALID_SOCKET)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				throw  SetErrorMsgText("accept:", WSAGetLastError()); 
		}
		else
		{			
			rc = true;               // подключился
			EnterCriticalSection(&scListContact);
			contacts.push_front(c);
		    SetEvent(hClientConnectedEvent); // сигнальное состояние события
			LeaveCriticalSection(&scListContact);	
		
		}
	}
	return rc;
};

void CommandsCycle(TalkersCommand& cmd)      // цикл обработки команд
{
	int  squirt = 0;
	while (cmd != EXIT)     // цикл обработки команд консоли и подключений
	{
		switch (cmd)
		{			
			case START: cmd = GETCOMMAND; // возобновить подключение клиентов
				squirt = AS_SQUIRT; 		
				break;
			case STOP:  cmd = GETCOMMAND; // остановить подключение клиентов   
				squirt = 0;
				break;
			case EXIT: cmd = GETCOMMAND; // Команда завершает работу сервера
				squirt = 0;
				break;
			case WAIT:  cmd = GETCOMMAND; // Команда приостанавливает подключение клиентов до тех пор, пока не обслужится последний клиент, подключенный к серверу.   
				squirt = 0;
				break;
			case SHUTDOWN: cmd = GETCOMMAND; // Команда равносильна последовательности команд: wait, exit.    
				squirt = 0;
				break;
			case GETCOMAND:  cmd = GETCOMMAND; // Служебная команда, которая не предназначена для ввода с консоли управления, а устанавливается сервером для указания, что сервер готов принять и обработать, очередную команду управления.
	
				break;
		};
		if (AcceptCycle(squirt))   //цикл  запрос/подключение (accept)
		{
		cmd = GETCOMMAND;
		//.... запуск потока EchoServer.......................
	
		}
		else SleepEx(0, TRUE);    // выполнить асинхронные процедуры 
		
	};
};

DWORD WINAPI AcceptServer(LPVOID pPrm)    // прототип 
{
	DWORD rc = 0;    // код возврата 
	WSADATA wsaData;
	try
	{
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			throw  SetErrorMsgText("Startup:", WSAGetLastError());
		if ((sS = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
			throw  SetErrorMsgText("socket:", WSAGetLastError());

		SOCKADDR_IN serv;                     // параметры  сокета sS
		sockaddr_in clnt;
		SOCKET cS;
		int lclnt = sizeof(clnt);
		serv.sin_family = AF_INET;           // используется IP-адресация  
		serv.sin_port = htons(serverPort);          // порт 2000
		serv.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.0.111"); ;   // любой собственный IP-адрес 

		if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
			throw  SetErrorMsgText("bind:", WSAGetLastError());

		if (listen(sS, SOMAXCONN) == SOCKET_ERROR)
			throw  SetErrorMsgText("listen:", WSAGetLastError());
		
		u_long nonblk;

		if (ioctlsocket(sS, FIONBIO, &(nonblk = 1)) == SOCKET_ERROR)
			throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());

		CommandsCycle(*((TalkersCommand*)pPrm));
		
		/*if (closesocket(sS) == SOCKET_ERROR)
			throw  SetErrorMsgText("closesocket:", WSAGetLastError());

		if (WSACleanup() == SOCKET_ERROR)
			throw  SetErrorMsgText("Cleanup:", WSAGetLastError());*/
	}
	catch (string errorMsgText)
	{
		cout << endl << errorMsgText;
	}
	cout << "shutdown acceptServer" << endl;
	ExitThread(*(DWORD*)pPrm);  // завершение работы потока
}

DWORD WINAPI ConsolePipe(LPVOID pPrm)    // прототип 
{
	DWORD rc = 0;    // код возврата

	char rbuf[40]; //буфер для чтения

	DWORD dwRead; // Количество принятых байт
	DWORD dwWrite; // Количество записанных байт
	HANDLE hPipe; // дескриптор канала
	DWORD myTime = 5;

	try
	{
		char namedPipeConnectionString[50] = "\\\\.\\pipe\\";
		strcat(namedPipeConnectionString, namedPipeName);
		
		if ((hPipe = CreateNamedPipeA(namedPipeConnectionString,
			PIPE_ACCESS_DUPLEX,           //дуплексный канал 
			PIPE_TYPE_MESSAGE | PIPE_WAIT,  // сообщения|синхронный
			1, NULL, NULL,               // максимум 1 экземпляр
			INFINITE, NULL)) == INVALID_HANDLE_VALUE)throw SetPipeError("create:", GetLastError());
		if (!ConnectNamedPipe(hPipe, NULL))   throw SetPipeError("connect:", GetLastError());

	}
	catch (string ErrorPipeText)
	{
		cout << endl << ErrorPipeText;
	}
	while (1) {
		cout << "enter to named pipe" << endl;
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
			cout << "Сообщение от клиента:  " << rbuf << endl;

			if (!strcmp(rbuf, "start")) {
				strcpy(rbuf, "start");
				GETCOMMAND = START;
			}
			else if (!strcmp(rbuf, "stop")) {
				strcpy(rbuf, "stop");
				GETCOMMAND = STOP;
			}			
			else if (!strcmp(rbuf, "exit")) {
				strcpy(rbuf, "exit");
				GETCOMMAND = EXIT;
			}
			else if (!strcmp(rbuf, "statistics")) {
				strcpy(rbuf, "statistics");
				GETCOMMAND = STATISTICS;
			}
			else if (!strcmp(rbuf, "wait")) {
				strcpy(rbuf, "wait");
				GETCOMMAND = WAIT;
			}
			else if (!strcmp(rbuf, "shutdown")) {
				strcpy(rbuf, "shutdown");
				GETCOMMAND = SHUTDOWN;
			}
			else if (!strcmp(rbuf, "getcomand")) {
				strcpy(rbuf, "getcomand");
				GETCOMMAND = GETCOMAND;
			}
			else {
				strcpy(rbuf, "nocmd");
			}

			WriteFile
			(
				hPipe,   // [in] дескриптор  канала
				rbuf,   // [in] указатель на буфер  вывода
				sizeof(rbuf),   // [in] количество записываемых байт
				&dwWrite,   // [out] количество записанных байт  
				NULL    // [in,out] для асинхронной обработки 
			);
		}

		cout << "--------------Канал закрыт-----------------" << endl;
		DisconnectNamedPipe(hPipe);
	}
	CloseHandle(hPipe);

	ExitThread(rc);  // завершение работы потока
}

DWORD WINAPI GarbageCleaner(LPVOID pPrm)    // прототип 
{
	DWORD rc = 0;    // код возврата 

	while (GETCOMMAND != EXIT) {
		Sleep(2000);
		int listSize = 0;
		int howMuchClean = 0;

		if (contacts.size() != 0) {
			for (auto i = contacts.begin(); i != contacts.end();) {
				if (i->type == i->EMPTY) {

					EnterCriticalSection(&scListContact);
				
					i = contacts.erase(i);
					howMuchClean++;
					listSize = contacts.size();
					LeaveCriticalSection(&scListContact);					
				}
				else
					++i;	
			}		
	
			cout << "GarbageCleaner size of clients @" << howMuchClean << "@" << endl;	
		}
	}
	cout << "shutdown garbageCleaner" << endl;
	ExitThread(rc);  // завершение работы потока
}

HANDLE(*ts)(char*, LPVOID);
HMODULE st;

void CALLBACK ASWTimer(LPVOID Prm, DWORD, DWORD) {
	Contact *contact = (Contact*)(Prm);
	cout << "ASWTimer is calling "<<contact->hthread << endl;

	Beep(500, 100); // звоним!
	TerminateThread(contact->serverHThtead, NULL); 

	EnterCriticalSection(&scListContact);
	CancelWaitableTimer(contact->htimer);
	contact->type = contact->EMPTY;
	contact->sthread = contact->TIMEOUT;
	LeaveCriticalSection(&scListContact);
}

void CALLBACK ASFinishMessage(LPVOID Prm) {
	Contact *contact = (Contact*)(Prm);

	CancelWaitableTimer(contact->htimer);
}

VOID CALLBACK TimerAPCProc(LPVOID, DWORD, DWORD)
{
		Beep(500, 100); // звоним!
		cout << "TimerAPCPRoc is running\n" << endl;
};



DWORD WINAPI DispatchServer(LPVOID pPrm)  
{	
	DWORD rc = 0;    // код возврата 	
	cout << "marker 1" << endl;
	while (GETCOMMAND != EXIT)     // цикл обработки команд консоли и подключений		
	{
		if (GETCOMMAND != STOP) {

			WaitForSingleObject(hClientConnectedEvent, INFINITE);// ждем пока подключиться клилент
															 // т.е. в acceptCycle событие перейдет в сигнальное состояние
			ResetEvent(hClientConnectedEvent);
			cout << "marker 2" << endl;

			EnterCriticalSection(&scListContact);
			for (auto i = contacts.begin(); i != contacts.end(); i++) {
				if (i->type == i->ACCEPT) {

					u_long nonblk;
					if (ioctlsocket(i->s, FIONBIO, &(nonblk = 0)) == SOCKET_ERROR)
						throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());

					char serviceType[5];// = "Echo", "Time", "0001"
					clock_t start = clock();
					recv(i->s, serviceType, sizeof(serviceType), NULL);
					strcpy(i->msg, serviceType);

					clock_t delta = clock() - start;
					if (delta > 3000) {
						cout << "so long" << endl;
						i->sthread = i->TIMEOUT;
						if ((send(i->s, "TimeOUT", strlen("TimeOUT") + 1, NULL)) == SOCKET_ERROR)
							throw  SetErrorMsgText("send:", WSAGetLastError());

						if (closesocket(i->s) == SOCKET_ERROR)
							throw  SetErrorMsgText("closesocket:", WSAGetLastError());


						i->type = i->EMPTY;
					}
					else if (delta <= 3000) {

						if (strcmp(i->msg, "Echo") != 0 && strcmp(i->msg, "Time") !=0 && strcmp(i->msg, "0001") !=0) {
							if ((send(i->s, "ErrorInquiry", strlen("ErrorInquiry") + 1, NULL)) == SOCKET_ERROR)
								throw  SetErrorMsgText("send:", WSAGetLastError());

							i->sthread = i->ABORT;
							i->type = i->EMPTY;
							if (closesocket(i->s) == SOCKET_ERROR)
								throw  SetErrorMsgText("closesocket:", WSAGetLastError());
						}
						else {
							i->type = i->CONTACT;

							i->hthread = hAcceptServer;

							i->serverHThtead = ts(serviceType, (LPVOID)&(*i));

							const int nTimerUnitsPerSecond = 10000000;
							int time = 2; //время ожидания таймера

							LARGE_INTEGER li;
							li.QuadPart = -(time * nTimerUnitsPerSecond); //2 секунд (куски по 100нс, отрицательное)
							i->htimer = CreateWaitableTimer(NULL, FALSE, NULL);
							SetWaitableTimer(i->htimer, &li, 0, ASWTimer, (LPVOID)&(*i), FALSE);//запускаем таймер
							WaitForSingleObjectEx(i->htimer, 2000, FALSE);//ждем 15 секунд

							SleepEx(NULL, TRUE);

							Sleep(200);
						}
					}

				}

			}
			LeaveCriticalSection(&scListContact);
		
			cout << "marker 3" << endl;
			//Sleep(2000);
		}
	}
	cout << "shutdown dispatchServer" << endl;
	ExitThread(rc);  // завершение работы потока
}




int main(int argc, char* argv[])
{
	
	setlocale(0, "Russian");
	if (argc == 2) {
		serverPort =atoi( argv[1]);
	}
	else if (argc == 3) {
		serverPort = atoi(argv[1]);
		strcpy(dllName, argv[2]);		
		}
	else if (argc == 4) {
		serverPort = atoi(argv[1]);
		strcpy(dllName, argv[2]);
		strcpy(namedPipeName, argv[3]);
	}
	else {
		serverPort = 2000;
		strcpy(dllName , "Win32Project1.dll");	
		strcpy(namedPipeName, "BOX");
	}
	

	cout << "server port " << serverPort << endl;

	volatile TalkersCommand  cmd = START;      // команды сервера 

	InitializeCriticalSection(&scListContact);
	hAcceptServer = CreateThread(NULL, NULL, AcceptServer,
		(LPVOID)&cmd, NULL, NULL),
	hDispatchServer = CreateThread(NULL, NULL, DispatchServer,
		(LPVOID)&cmd, NULL, NULL),
	hGarbageCleaner = CreateThread(NULL, NULL, GarbageCleaner,
		(LPVOID)NULL, NULL, NULL),
	hConsolePipe = CreateThread(NULL, NULL, ConsolePipe,
		(LPVOID)&cmd, NULL, NULL);



	 st = LoadLibraryA(dllName);
	 ts = (HANDLE(*)(char*, LPVOID))GetProcAddress(st, "SSS");


	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);   
	WaitForSingleObject(hDispatchServer, INFINITE);
	CloseHandle(hDispatchServer);
	WaitForSingleObject(hGarbageCleaner, INFINITE);   
	CloseHandle(hGarbageCleaner);
	WaitForSingleObject(hConsolePipe, INFINITE);
	CloseHandle(hConsolePipe);

	FreeLibrary(st);
	return 0;
};

