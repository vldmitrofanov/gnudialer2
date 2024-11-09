// This file is a derivative work of Rob Tougher's
// c++ sockets implementation.  Please see socket.LICENSE
// and AUTHORS for details.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

#ifndef SOCKET
#define SOCKET

// Don't forget to make sure MAXCONNECTIONS is ALWAYS high enough...
const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 1000;
const int MAXRECV = 4096;

class Socket
{
 public:
  Socket();
  virtual ~Socket();

  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen() const;
  bool accept ( Socket& ) const;

  // Client initialization
  bool connect ( const std::string host, const int port );

  // Data Transimission
  bool send ( const std::string ) const;
  int recv ( std::string& ) const;


  void set_non_blocking ( const bool );

  bool is_valid() const { return m_sock != -1; }
  int timeout;

 private:

  int m_sock;
  sockaddr_in m_addr;

};

#include "SocketFunctions.h"

class ClientSocket : private Socket
{
  public:

    ClientSocket ( std::string host, int port ) {
      Socket::create();
      Socket::connect(host,port);
      timeout = 0;
      }
    ~ClientSocket(){};

  void setRecvTimeout(int milliseconds) { timeout = milliseconds; }

  const ClientSocket& operator << ( const std::string& s ) const
  {
    Socket::send ( s ) ;

    return *this;

  }

  const ClientSocket& operator >> ( std::string& s ) const
  {
    Socket::recv ( s );
    return *this;
  }

};



class ServerSocket : private Socket
{
 public:

  ServerSocket ( int port );
  ServerSocket (){};
  ~ServerSocket();

  void setRecvTimeout(int milliseconds) { timeout = milliseconds; }

  const ServerSocket& operator << ( const std::string& ) const;
  const ServerSocket& operator >> ( std::string& ) const;

  void accept ( ServerSocket& );

};

ServerSocket::ServerSocket ( int port )
{
  if ( ! Socket::create() )
    {
//      throw SocketException ( "Could not create server socket." );
	std::cerr << "Could not create server socket.";
    }

  if ( ! Socket::bind ( port ) )
    {
//      throw SocketException ( "Could not bind to port." );
	std::cerr << "Could not bind to port.";
    }

  if ( ! Socket::listen() )
    {
//      throw SocketException ( "Could not listen to socket." );
	std::cerr << "Could not listen to socket.";
    }

}

ServerSocket::~ServerSocket()
{
}

const ServerSocket& ServerSocket::operator << ( const std::string& s ) const
{
  if ( ! Socket::send ( s ) )
    {
//      throw SocketException ( "Could not write to socket." );
	std::cerr << "Could not write to socket.";
    }

  return *this;

}

const ServerSocket& ServerSocket::operator >> ( std::string& s ) const
{
  if ( ! Socket::recv ( s ) )
    {
//      throw SocketException ( "Could not read from socket." );
//	std::cerr << "Could not read from socket.";
    }

  return *this;
}

void ServerSocket::accept ( ServerSocket& sock )
{
  if ( ! Socket::accept ( sock ) )
    {
//      throw SocketException ( "Could not accept socket." );
	std::cerr << "Could not accept socket.";
    }
}

// void ServerSocket::setRecvTimeout(int milliseconds) { timeout = milliseconds; }

#endif
