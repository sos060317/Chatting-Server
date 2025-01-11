#include "Server.h"

std::vector<ClientInfo> clientInfos;

// ������
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

// �Ҹ���
Server::~Server()
{
	Stop();
}

// ���� �ʱ�ȭ
void Server::InitializationSocket()
{
	// ���� �ʱ�ȭ 2.2����
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_WsaData))
	{
		std::cerr << "WSAStartup failed\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "���� �ʱ�ȭ ����\n";

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

// ���� ����
void Server::CreatSocket()
{
	// ���� ���� IPv4, TCP, �������� �ڵ� ����
	m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_ServerSocket)
	{
		std::cerr << "Socket creation failed\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	//SetNonBlocking(m_ServerSocket);
	std::cout << "���� ���� ����\n";
}
// ���� ���ε�
void Server::SetBind()
{
	// ���� �ּ� �ʱ�ȭ
	ZeroMemory(&m_ServerAddress, sizeof(m_ServerAddress));
	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_port = htons(static_cast<int>(Constant::PORT));
	m_ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);;

	// �ּҿ� ��Ʈ �Ҵ�
	if (SOCKET_ERROR == bind(m_ServerSocket, (struct sockaddr*)&m_ServerAddress, sizeof(m_ServerAddress)))
	{
		std::cerr << "Bind failed\n";
		closesocket(m_ServerSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::cout << "���� ���ε� ����\n";
}

// ���� ���� ���
void Server::SetListen()
{
	// ���� ��� ����(Ŭ���̾�Ʈ ���� ��û�� ���� �� �ִ� ����)
	if (SOCKET_ERROR == listen(m_ServerSocket, SOMAXCONN))
	{
		std::cerr << "Listen failed\n";
		closesocket(m_ServerSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::cout << "���� ���� ��� ����\n";

	CreateIoCompletionPort((HANDLE)m_ServerSocket, m_IOCP, (DWORD)m_ServerSocket, 0);
}

// ���� ����
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

// ���� IP�ּ� ����ϱ�
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
	std::cout << "�� ��ǻ���� IP �ּ�: " << m_IpAddr << "\n";
	std::cout << "\n";
}

// ���� ����
void Server::MainLoop()
{
	// ���� Flag ON
	m_IsRun = true;
	std::cout << "���� ����!\n";

	// ���� ���� ����
	while (m_IsRun)
	{
		if (!SetAccept())
		{

		}
	}
}

// ���� ���� ����
void Server::Stop()
{
	// ���� Flag OFF
	m_IsRun = false;
	closesocket(m_ServerSocket);
	std::cout << "���� ����\n";

	{
		// �ڿ� ��� �� ��� Ŭ���̾�Ʈ ���� ����
		std::lock_guard<std::mutex> lock(m_ClientSocketsMutex);
		for (SOCKET socket : m_Clients)
		{
			closesocket(socket);
		}
		// ���� ����
		m_Clients.clear();
		clientInfos.clear();
	}
}

// ��� Ŭ���̾�Ʈ �޽��� ����
void Server::ClientHandler(SOCKET _clientSock, const int _clientNumber)
{
	char buffer[static_cast<int>(Constant::IP)] = { 0 };

	recv(_clientSock, buffer, sizeof buffer, 0);
	clientInfos[_clientNumber].second = buffer;

	std::string message = "\n" + clientInfos[_clientNumber].second + m_EnterMessage + "\n";
	BroadcastMessage(message, _clientSock);

	ZeroMemory(&buffer, sizeof(buffer));

	int result;
	// �޽��� �ޱ�
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

// ��� Ŭ���̾�Ʈ �޽��� �۽�
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