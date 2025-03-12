#include <iostream>
#include <string>
#include <cstdlib> 
#include "Server.hpp"

bool verif_num_port(const std::string& str) 
{
    for (size_t i = 0; i < str.length(); ++i) 
    {
        if (str[i] < '0' || str[i] > '9') 
            return false; 
    }
    return true; 
}

int ft_atoi(const std::string& str) 
{
    int result = 0;
    for (size_t i = 0; i < str.length(); ++i) 
    {
        if (str[i] >= '0' && str[i] <= '9') 
            result = result * 10 + (str[i] - '0');
        else 
            break;
    }
    return result;
}

int main(int argc, char const *argv[]) 
{
    if (argc != 3) 
    {
      std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
      return 1;
    }

  std::string s_port = argv[1];
  std::string pass = argv[2];

  if (!verif_num_port(s_port)) 
  {
      std::cerr << "Erreur : Le port doit contenir uniquement des chiffres." << std::endl;
      return 1;
  }

  int n_port = ft_atoi(s_port);

  if (n_port < 1000 || n_port > 65535)
  {
      std::cerr << "Erreur : Le port doit ertre un nombre entre 1000 et 65535." << std::endl;
      return 1;
  }

  if (pass.empty()) 
  {
      std::cerr << "Erreur : Le mot de passe ne peut pas etre vide" << std::endl;
      return 1;
  }

  std::cout << "Port : " << n_port << std::endl;
  std::cout << "Mot de passe : " << pass << std::endl;
  try {
    Server server(n_port, pass);
    server.start();
} catch (const std::exception& e) {
    std::cerr << "Erreur : " << e.what() << std::endl;
    return 1;
}

  std::cout << "Serveur IRC demare sur le port " << n_port << std::endl;

  return 0;
}