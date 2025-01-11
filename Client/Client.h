#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <thread>

#pragma comment(lib, "ws2_32") // ���̺귯�� ��ũ

enum class Constant
{
	PORT = 8080,
	BUFFER_SIZE = 255,
	IP = 32,
};

class Client
{
public: // Client Socket ���� ����
	SOCKET m_ClientSocket;
	WSADATA m_WsaData;
	SOCKADDR_IN m_ServerAddress;

	char m_ServerIP[static_cast<int>(Constant::IP)];

public:
	std::string m_ClientName;

public: // ������ �Ҹ���
	Client();
	~Client();

public: // ��� �Լ�
	void InitializationSocket();	 // ���� �ʱ�ȭ
	void CreateSocket();			 // ���� ����
	void ConnectToServer();			 // ���� ����
	void InitializationClientName(); // �̸� ����
	void MainLoop();				 // ���� ����
	void Disconnect();				 // ���� ����
	void ReceiveMessages(SOCKET sock); // �޽��� �ޱ�
};