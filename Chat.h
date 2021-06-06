#include <string>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <memory>
#include <sstream>
#include <thread>

#include "Serializable.h"
#include "Socket.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/**
 *  Mensaje del protocolo de la aplicación de Chat
 *
 *  +-------------------+
 *  | Tipo: uint8_t     | 0 (login), 1 (mensaje), 2 (logout)
 *  +-------------------+
 *  | Nick: char[8]     | Nick incluido el char terminación de cadena '\0'
 *  +-------------------+
 *  |                   |
 *  | Mensaje: char[80] | Mensaje incluido el char terminación de cadena '\0'
 *  |                   |
 *  +-------------------+
 *
 */
class ServerMessage: public Serializable
{
public:
    static const size_t MESSAGE_SIZE = sizeof(char) * 12 * 2;

    ServerMessage(){};

    ServerMessage(const std::string& n, const std::string& m):name(n),opponent(m){};

    void to_bin();

    int from_bin(char * bobj);

    std::string name;
    std::string opponent;
};


class GameMessage: public Serializable
{
public:
    static const size_t MESSAGE_SIZE = sizeof(char) * 8;

    GameMessage(){};

    GameMessage(const std::string& n):data(n){};

    void to_bin();

    int from_bin(char * bobj);

    std::string data;
};


class StartMessage: public Serializable
{
public:
    static const size_t MESSAGE_SIZE = sizeof(struct sockaddr) + sizeof(socklen_t) + sizeof(bool);

    StartMessage(){};

    StartMessage(const struct sockaddr& s, const socklen_t& l, bool b):sa(s),sa_len(l), turn(b){};

    void to_bin();

    int from_bin(char * bobj);

    struct sockaddr sa;
    socklen_t sa_len;
    bool turn;
};
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el servidor de chat
 */
class P2PServer
{
public:
    P2PServer(const char * s, const char * p): socket(s, p)
    {
        socket.bind();
    };

    /**
     *  Thread principal del servidor recive mensajes en el socket y
     *  lo distribuye a los clientes. Mantiene actualizada la lista de clientes
     */
    void do_messages();

private:
    /**
     *  Lista de clientes conectados al servidor de Chat, representados por
     *  su socket
     */
    std::vector<std::unique_ptr<Socket>> clients;

    std::vector<std::string> names;
    std::vector<std::string> opponents;
    /**
     * Socket del servidor
     */
    Socket socket;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 *  Clase para el cliente de chat
 */
class NimClient
{
public:
    /**
     * @param s dirección del servidor
     * @param p puerto del servidor
     * @param n nick del usuario
     */
    NimClient(const char * s, const char * p, const char * n, const char * o):socket(s, p),
        name(n), opponent(o){for (int i = 0; i < GAME_SIZE; i++) game.push_back(NORMAL); run();};

    enum Stick
    {
        GONE   = 0,
        NORMAL = 1,
        SELECTED  = 2
    };

    const int GAME_SIZE = 6;
    const int MAX_MOVES = 3;

    /**
     *  Envía el mensaje de login al servidor
     */
    void run();

    /**
     *  Envía el mensaje de logout al servidor
     */
    void logout();

    bool processInput(std::string s);

    bool isGameOver();

    void render();

    /**
     *  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)
     *  y los envía por red vía el Socket.
     */
    void input_thread();

    /**
     *  Rutina del thread de Red. Recibe datos de la red y los "renderiza"
     *  en STDOUT
     */
    void net_thread();

private:

    /**
     * Socket para comunicar con el servidor
     */
    Socket peer;
    Socket socket;

    /**
     * Nick del usuario
     */
    std::string name;
    std::string opponent;
    bool myTurn;
    std::vector<Stick> game;
};

