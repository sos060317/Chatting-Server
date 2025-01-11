#include "Client.h"

// ������
Client::Client()
{
	InitializationClientName();
	InitializationSocket();
	CreateSocket();
	ConnectToServer();
}

// �Ҹ���
Client::~Client()
{
	Disconnect();
}

// ���� �ʱ�ȭ
void Client::InitializationSocket()
{
	// ���� �ʱ�ȭ 2.2����
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_WsaData))
	{
		std::cerr << "WSAStartup failed\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "���� �ʱ�ȭ ����\n";
}

// ���� ����
void Client::CreateSocket()
{
	// ���� ���� IPv4, TCP, �������� �ڵ� ����
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_ClientSocket)
	{
		std::cerr << "Socket creation failed\n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	std::cout << "���� ���� ����\n";
}

// ���� ����
void Client::ConnectToServer()
{
	std::cout << "���� IP�� �Է��ϼ��� : ";
	std::cin >> m_ServerIP;

	// ���� �ּ� ����ü �ʱ�ȭ
	ZeroMemory(&m_ServerAddress, sizeof(m_ServerAddress));
	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_port = htons(static_cast<int>(Constant::PORT));
	m_ServerAddress.sin_addr.s_addr = inet_addr(m_ServerIP);

	// Ŭ���̾�Ʈ�� ������ ����
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

// �̸� ����
void Client::InitializationClientName()
{
	std::cout << "�̸� �Է��ϼ��� : ";
	std::cin >> m_ClientName;

	if (m_ClientName.find("error") != std::string::npos) {
		std::cerr << "Client name error: " << m_ClientName << "\n";
		closesocket(m_ClientSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}

// ���� ����
void Client::MainLoop()
{
	std::cout << "���� ����!\n";

	while (true)
	{
		std::cout << "ME: ";
		std::getline(std::cin, m_ClientName);

		if (m_ClientName.empty())
		{
			std::cout << "\r";
			continue;
		}

		send(m_ClientSocket, m_ClientName.c_str(), m_ClientName.length() + 1, 0); // �޼��� ������

		if (!strcmp(m_ClientName.c_str(), "exit"))
		{
			std::cout << "\nserver�� ���� ����\n";
			Sleep(3000);
			closesocket(m_ClientSocket);
			WSACleanup();
			exit(EXIT_FAILURE);
			break;
		}
	}
}

// ���� ����
void Client::Disconnect()
{
	if (INVALID_SOCKET != m_ClientSocket)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

// �޽��� �ޱ�
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