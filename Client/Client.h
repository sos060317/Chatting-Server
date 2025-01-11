#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <thread>

#pragma comment(lib, "ws2_32") // 라이브러리 링크

enum class Constant
{
	PORT = 8080,
	BUFFER_SIZE = 255,
	IP = 32,
};

class Client
{
public: // Client Socket 연결 변수
	SOCKET m_ClientSocket;
	WSADATA m_WsaData;
	SOCKADDR_IN m_ServerAddress;

	char m_ServerIP[static_cast<int>(Constant::IP)];

public:
	std::string m_ClientName;

public: // 생성자 소멸자
	Client();
	~Client();

public: // 기능 함수
	void InitializationSocket();	 // 소켓 초기화
	void CreateSocket();			 // 소켓 생성
	void ConnectToServer();			 // 서버 연결
	void InitializationClientName(); // 이름 설정
	void MainLoop();				 // 메인 루프
	void Disconnect();				 // 연결 해제
	void ReceiveMessages(SOCKET sock); // 메시지 받기
};