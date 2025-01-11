#include "Server.h"

std::vector<ClientInfo> clientInfos;

// 생성자
Server::Server()
{
	m_Block = 0;
	m_NonBlock = 1;

	m_AddrLength = sizeof(m_ClientAddress);
	m_ClientCount = 0;

	m_IsRun = false;

	m_EnterMessage = " has joined the chat.\n";
	m_LeaveMessage = " has left the chat.\n";

	InitializationSocket();
	CreatSocket();
	SetBind();
	SetListen();
}

// 소멸자
Server::~Server()
{
	Stop();
}

// 소켓 초기화
void Server::InitializationSocket()
{
	// 소켓 초기화 2.2버전
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_WsaData))
	{
		std::cerr << "WSAStartup failed\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "소켓 초기화 성공\n";

	InitializeCriticalSection(&m_CriticalSection);

	m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_IOCP)
	{
		std::cerr << "IOCP Creation Error\n";
		DeleteCriticalSection(&m_CriticalSection);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}

// 소켓 생성
void Server::CreatSocket()
{
	// 소켓 생성 IPv4, TCP, 프로토콜 자동 연결
	m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_ServerSocket)
	{
		std::cerr << "Socket creation failed\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	//SetNonBlocking(m_ServerSocket);
	std::cout << "소켓 생성 성공\n";
}
// 소켓 바인드
void Server::SetBind()
{
	// 소켓 주소 초기화
	ZeroMemory(&m_ServerAddress, sizeof(m_ServerAddress));
	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_port = htons(static_cast<int>(Constant::PORT));
	m_ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);;

	// 주소와 포트 할당
	if (SOCKET_ERROR == bind(m_ServerSocket, (struct sockaddr*)&m_ServerAddress, sizeof(m_ServerAddress)))
	{
		std::cerr << "Bind failed\n";
		closesocket(m_ServerSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::cout << "소켓 바인드 성공\n";
}

// 소켓 연결 대기
void Server::SetListen()
{
	// 수신 대기 상태(클라이언트 연결 요청을 받을 수 있는 상태)
	if (SOCKET_ERROR == listen(m_ServerSocket, SOMAXCONN))
	{
		std::cerr << "Listen failed\n";
		closesocket(m_ServerSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::cout << "소켓 연결 대기 성공\n";

	CreateIoCompletionPort((HANDLE)m_ServerSocket, m_IOCP, (DWORD)m_ServerSocket, 0);
}

// 소켓 연결
bool Server::SetAccept()
{
	m_ClientSocket = accept(m_ServerSocket, (SOCKADDR*)&m_ClientAddress, &m_AddrLength);
	if (INVALID_SOCKET == m_ClientSocket)
	{
		//std::cerr << "Accept failed\n";
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(m_ClientSocketsMutex);
		m_Clients.push_back(m_ClientSocket);
		clientInfos.push_back(ClientInfo(Client(), ""));
		clientInfos[m_ClientCount].first.m_Client = m_ClientSocket;
		clientInfos[m_ClientCount].first.m_Number = m_ClientCount;
		std::thread(&Server::ClientHandler, this, m_ClientSocket, m_ClientCount).detach();
		m_ClientCount++;
	}
	return true;
}

// 서버 IP주소 출력하기
void Server::PrintIP()
{
	memset(m_HostName, 0, sizeof(m_HostName));
	memset(m_IpAddr, 0, sizeof(m_IpAddr));

	int error = gethostname(m_HostName, sizeof(m_HostName));
	if (0 == error)
	{
		m_HostInfo = gethostbyname(m_HostName);
		strcpy(m_IpAddr, inet_ntoa(*(struct in_addr*)m_HostInfo->h_addr_list[0]));
	}
	std::cout << "이 컴퓨터의 IP 주소: " << m_IpAddr << "\n";
	std::cout << "\n";
}

// 메인 루프
void Server::MainLoop()
{
	// 서버 Flag ON
	m_IsRun = true;
	std::cout << "서버 시작!\n";

	// 메인 루프 시작
	while (m_IsRun)
	{
		if (!SetAccept())
		{

		}
	}
}

// 메인 루프 중지
void Server::Stop()
{
	// 서버 Flag OFF
	m_IsRun = false;
	closesocket(m_ServerSocket);
	std::cout << "서버 종료\n";

	{
		// 자원 잠금 후 모드 클라이언트 연결 해제
		std::lock_guard<std::mutex> lock(m_ClientSocketsMutex);
		for (SOCKET socket : m_Clients)
		{
			closesocket(socket);
		}
		// 벡터 비우기
		m_Clients.clear();
		clientInfos.clear();
	}
}

// 모든 클라이언트 메시지 수신
void Server::ClientHandler(SOCKET _clientSock, const int _clientNumber)
{
	char buffer[static_cast<int>(Constant::IP)] = { 0 };

	recv(_clientSock, buffer, sizeof buffer, 0);
	clientInfos[_clientNumber].second = buffer;

	std::string message = "\n" + clientInfos[_clientNumber].second + m_EnterMessage + "\n";
	BroadcastMessage(message, _clientSock);

	ZeroMemory(&buffer, sizeof(buffer));

	int result;
	// 메시지 받기
	while ((result = recv(_clientSock, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		buffer[result] = '\0';
		std::string message = clientInfos[_clientNumber].second + ": " + std::string(buffer) + "\n";
		BroadcastMessage(message, _clientSock);
	}

	{
		std::lock_guard<std::mutex> lock(m_ClientSocketsMutex);
		closesocket(m_ClientSocket);
		m_Clients.erase(remove(m_Clients.begin(), m_Clients.end(), m_ClientSocket), m_Clients.end());
		message = clientInfos[_clientNumber].second + m_LeaveMessage + "\n";
		BroadcastMessage(message, m_ClientSocket);
	}
}

// 모든 클라이언트 메시지 송신
void Server::BroadcastMessage(const std::string& _message, const SOCKET _sender)
{
	Buffer::SaveChat(_message);

	for (SOCKET client : m_Clients)
	{
		if (client != _sender)
		{
			if (send(client, _message.c_str(), _message.size(), 0) == SOCKET_ERROR)
			{
				std::cerr << "Send failed\n";
			}
		}
	}
}

void Buffer::SaveChat(const std::string _chat)
{

}