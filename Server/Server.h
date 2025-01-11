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
public: // Server Socket 연결 변수
	WSADATA m_WsaData;
	SOCKET m_ServerSocket;
	SOCKET m_ClientSocket;
	SOCKADDR_IN m_ServerAddress;
	SOCKADDR_IN m_ClientAddress;

public: // 생성자에서 초기화해야하는 변수
	u_long m_Block;
	u_long m_NonBlock;

	int m_AddrLength;
	int m_ClientCount;
	
	bool m_IsRun;

	std::string m_EnterMessage;
	std::string m_LeaveMessage;

public: // 그 외
	HANDLE m_IOCP;
	std::vector<SOCKET> m_Clients;
	std::mutex m_ClientSocketsMutex;
	PHOSTENT m_HostInfo;
	CRITICAL_SECTION m_CriticalSection;
	char m_HostName[static_cast<int>(Constant::IP)];
	char m_IpAddr[static_cast<int>(Constant::IP)];
	

public: // 생성자 소멸자
	Server();
	~Server();

public: // 기능 함수
	void InitializationSocket();		       // 소켓 초기화
	void CreatSocket();					       // 소켓 생성
	void SetBind();						       // 소켓 바인드
	void SetListen();					       // 소켓 연결 대기
	bool SetAccept();						   // 소켓 연결
	void SetNonBlocking(SOCKET _serverSocket); // SOCKET NonBlocking
	void PrintIP();						       // 서버 IP주소 출력하기
	void MainLoop();					       // 메인 루프
	void Stop();						       // 메인 루프 중지
	void ClientHandler(SOCKET _clientSock, const int _clientNumber); // 모든 클라이언트 메시지 수신
	void BroadcastMessage(const std::string& _message, const SOCKET _sender); // 모든 클라이언트 메시지 송신
};

class Client
{
public:
	SOCKET m_Client;
	int m_Number = -1;
};

typedef std::pair<Client, std::string> ClientInfo;
extern std::vector<ClientInfo> clientInfos;

class Buffer // 원형 버퍼
{
public:
	std::queue<std::string> m_Chats;
	std::queue<std::string*> m_ChatAddresses;

public:
	static void SaveChat(const std::string _chat); // 채팅 저장
};