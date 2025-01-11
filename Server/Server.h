#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <WinSock2.h>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

enum class Constant
{
	PORT = 8080,
	BUFFER_SIZE = 255,
	IP = 50,
};

class Server
{
public: // Server Socket ���� ����
	WSADATA m_WsaData;
	SOCKET m_ServerSocket;
	SOCKET m_ClientSocket;
	SOCKADDR_IN m_ServerAddress;
	SOCKADDR_IN m_ClientAddress;

public: // �����ڿ��� �ʱ�ȭ�ؾ��ϴ� ����
	u_long m_Block;
	u_long m_NonBlock;

	int m_AddrLength;
	int m_ClientCount;
	
	bool m_IsRun;

	std::string m_EnterMessage;
	std::string m_LeaveMessage;

public: // �� ��
	HANDLE m_IOCP;
	std::vector<SOCKET> m_Clients;
	std::mutex m_ClientSocketsMutex;
	PHOSTENT m_HostInfo;
	CRITICAL_SECTION m_CriticalSection;
	char m_HostName[static_cast<int>(Constant::IP)];
	char m_IpAddr[static_cast<int>(Constant::IP)];
	

public: // ������ �Ҹ���
	Server();
	~Server();

public: // ��� �Լ�
	void InitializationSocket();		       // ���� �ʱ�ȭ
	void CreatSocket();					       // ���� ����
	void SetBind();						       // ���� ���ε�
	void SetListen();					       // ���� ���� ���
	bool SetAccept();						   // ���� ����
	void SetNonBlocking(SOCKET _serverSocket); // SOCKET NonBlocking
	void PrintIP();						       // ���� IP�ּ� ����ϱ�
	void MainLoop();					       // ���� ����
	void Stop();						       // ���� ���� ����
	void ClientHandler(SOCKET _clientSock, const int _clientNumber); // ��� Ŭ���̾�Ʈ �޽��� ����
	void BroadcastMessage(const std::string& _message, const SOCKET _sender); // ��� Ŭ���̾�Ʈ �޽��� �۽�
};

class Client
{
public:
	SOCKET m_Client;
	int m_Number = -1;
};

typedef std::pair<Client, std::string> ClientInfo;
extern std::vector<ClientInfo> clientInfos;

class Buffer // ���� ����
{
public:
	std::queue<std::string> m_Chats;
	std::queue<std::string*> m_ChatAddresses;

public:
	static void SaveChat(const std::string _chat); // ä�� ����
};