#include "Client.h"

int main()
{
	Client* client = new Client;
	try
	{
		client->MainLoop();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "예외 발생: " << e.what() << "\n";
		delete client;
	}
	return 0;
}