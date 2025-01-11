#include "Server.h"

int main()
{
	Server* server = new Server;
	try
	{
		server->PrintIP();
		server->MainLoop();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "예외 발생: " << e.what() << "\n";
		delete server;
	}
}