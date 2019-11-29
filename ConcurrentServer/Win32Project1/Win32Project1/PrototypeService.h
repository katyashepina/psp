#pragma once
#include "ErrorFunctions.h"
#include <time.h>
#include <string>
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

string runServer;

const std::string currentDateTime() {
	time_t     now = time(0);
	tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

void CALLBACK ASStartMessage(DWORD p)
{
	char *hostName = new char(4);
	gethostname(hostName, sizeof(hostName));
	printf("RunServer: %s  start time:  %s\n", runServer.c_str(), currentDateTime().c_str());

}

void CALLBACK ASFinishMessage(DWORD p)
{
	printf(" finish time: %s\n", currentDateTime().c_str());

}

void QueueUserAPCWrapper(PAPCFUNC functionName, Contact *contact) {
	QueueUserAPC(functionName, contact->hthread, 0);


}

void send_msg_to_client(Contact* contact)
{
	if (send(contact->s, contact->msg, sizeof(contact->msg), NULL) == SOCKET_ERROR)
		throw  SetErrorMsgText("send:", WSAGetLastError());
}
DWORD WINAPI EchoServer(LPVOID pPrm)
{
	DWORD rc = 0;
	Contact *contact = (Contact*)(pPrm);
	try
	{
		runServer = "EchoServer";
		QueueUserAPCWrapper(ASStartMessage, contact);
		puts("entry to echo server\ntransmission{");
		int lobuf, libuf;
		contact->sthread = contact->WORK;
		contact->type = contact->CONTACT;
		strcpy(contact->msg, "start transmission");
		send_msg_to_client(contact);
		//int	whenINeedToStop = 1;
		while (true) {
			if ((libuf = recv(contact->s, contact->msg, sizeof(contact->msg), NULL)) == SOCKET_ERROR)
				throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*
			send_msg_to_client(contact);
			if (atoi(contact->msg) == 0) break;
		}
		puts("end of transmission}");
	}
	catch (...)
	{

		puts("error in dll");
		contact->sthread = contact->ABORT;
		contact->type = contact->EMPTY;
		rc = contact->sthread;

		QueueUserAPCWrapper(ASFinishMessage, contact);
		CancelWaitableTimer(contact->htimer);
		ExitThread(rc);
	}
	contact->sthread = contact->FINISH;
	contact->type = contact->EMPTY;
	rc = contact->sthread;

	QueueUserAPCWrapper(ASFinishMessage, contact);
	CancelWaitableTimer(contact->htimer);
	ExitThread(rc);

}

DWORD WINAPI TimeServer(LPVOID pPrm)
{
	DWORD rc = 0;
	Contact *contact = (Contact*)(pPrm);
	runServer = "TimeServer";
	int lobuf, libuf;
	QueueUserAPCWrapper(ASStartMessage, contact);
	strcpy(contact->msg, currentDateTime().c_str());
	send_msg_to_client(contact);
	contact->sthread = contact->FINISH;
	contact->type = contact->EMPTY;
	rc = contact->sthread;
	QueueUserAPCWrapper(ASFinishMessage, contact);
	CancelWaitableTimer(contact->htimer);
	ExitThread(rc);
}

DWORD WINAPI ServiceServer01(LPVOID pPrm)
{
	DWORD rc = 0;
	Contact *contact = (Contact*)(pPrm);
	runServer = "ServiceServer01";
	QueueUserAPCWrapper(ASStartMessage, contact);
	srand(time(NULL));
	int lobuf, libuf, randNumber;
	randNumber = rand() % 100 + 1;
	sprintf(contact->msg, "%d", randNumber);
	send_msg_to_client(contact);
	contact->sthread = contact->FINISH;
	contact->type = contact->EMPTY;
	rc = contact->sthread;
	QueueUserAPCWrapper(ASFinishMessage, contact);
	CancelWaitableTimer(contact->htimer);
	ExitThread(rc);
}