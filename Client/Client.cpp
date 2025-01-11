#include "Client.h"

// 생성자
Client::Client()
{
	InitializationClientName();
	InitializationSocket();
	CreateSocket();
	ConnectToServer();
}

// 소멸자
Client::~Client()
{
	Disconnect();
}

// 소켓 초기화
void Client::InitializationSocket()
{
	// 소켓 초기화 2.2버전
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_WsaData))
	{
		std::cerr << "WSAStartup failed\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "소켓 초기화 성공\n";
}

// 소켓 생성
void Client::CreateSocket()
{
	// 소켓 생성 IPv4, TCP, 프로토콜 자동 연결
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_ClientSocket)
	{
		std::cerr << "Socket creation failed\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::cout << "소켓 생성 성공\n";
}

// 서버 연결
void Client::ConnectToServer()
{
	std::cout << "서버 IP를 입력하세요 : ";
	std::cin >> m_ServerIP;

	// 서버 주소 구조체 초기화
	ZeroMemory(&m_ServerAddress, sizeof(m_ServerAddress));
	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_port = htons(static_cast<int>(Constant::PORT));
	m_ServerAddress.sin_addr.s_addr = inet_addr(m_ServerIP);

	// 클라이언트를 서버에 연결
	if (SOCKET_ERROR == connect(m_ClientSocket, (SOCKADDR*)&m_ServerAddress, sizeof(m_ServerAddress)))
	{
		std::cerr << "Connection to server failed." << WSAGetLastError() << "\n";
		Sleep(3000);
		closesocket(m_ClientSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::thread(&Client::ReceiveMessages, this, m_ClientSocket).detach();
	send(m_ClientSocket, m_ClientName.c_str(), sizeof(m_ClientName), 0);
}

// 이름 설정
void Client::InitializationClientName()
{
	std::cout << "이름 입력하세요 : ";
	std::cin >> m_ClientName;

	if (m_ClientName.find("error") != std::string::npos) {
		std::cerr << "Client name error: " << m_ClientName << "\n";
		closesocket(m_ClientSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}

// 메인 루프
void Client::MainLoop()
{
	std::cout << "서버 접속!\n";

	while (true)
	{
		std::cout << "ME: ";
		std::getline(std::cin, m_ClientName);

		if (m_ClientName.empty())
		{
			std::cout << "\r";
			continue;
		}

		send(m_ClientSocket, m_ClientName.c_str(), m_ClientName.length() + 1, 0); // 메세지 보내기

		if (!strcmp(m_ClientName.c_str(), "exit"))
		{
			std::cout << "\nserver와 연결 해제\n";
			Sleep(3000);
			closesocket(m_ClientSocket);
			WSACleanup();
			exit(EXIT_FAILURE);
			break;
		}
	}
}

// 연결 해제
void Client::Disconnect()
{
	if (INVALID_SOCKET != m_ClientSocket)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

// 메시지 받기
void Client::ReceiveMessages(SOCKET sock)
{
	char buffer[static_cast<int>(Constant::BUFFER_SIZE)];
	while (true)
	{
		int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived <= 0)
		{
			std::cout << "Receive failed\n";
			break;
		}

		buffer[bytesReceived] = '\0';
		std::cout << "\r" << buffer;
		std::cout << "ME: ";
	}
}