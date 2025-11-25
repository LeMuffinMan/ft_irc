#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include "NetworkState.hpp"

class Server {
private:
	// int port;
	std::string _port;
	std::vector<int> clients_fd;
	const std::string _password;

	Server(); // On ne veut pas de serveur sans mdp ni sans port
public:
	int getPort() const;
	int checkPassword();
	void RunServer();


	void reply(Client* client, std::string message);
	void replyChannel(Channel& channel, std::string message);
	void replyChannelOnlyOP(Channel& channel, std::string message);
	void broadcast(NetworkState&, std::string message);
	void noticeServers(NetworkState&, std::string message);
	//A voir pour ce qu'on garde de la canonique 
	// Server(std::string password, std::string port, NetworkState &networkState); // TODO: Implement Server(Networkstate&)
	Server(std::string password, std::string port);
	~Server();
	// Server(const Server& other);
	// Server& operator=(const Server& other);
};

#endif 
