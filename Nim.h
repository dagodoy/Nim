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
 *  Mensaje de registro en el servidor
 *
 *  +-------------------+
 *  | name: char[12]    | Nombre del jugador
 *  |                   |
 *  +-------------------+
 *  | opponent: char[12]| Nombre del jugador con el que se quiere enfrentar
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

/**
 *  Mensaje de comunicacion entre clientes
 *
 *  +-------------------+
 *  | data: char[8]     | Contenido del mensaje
 *  |                   |
 *  +-------------------+
 *
 */

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


/**
 *  Mensaje de servidor a cliente con la informacion de otro cliente
 *
 *  +-------------------+
 *  | sa: sockaddr      | Informacion del socket del otro cliente
 *  |                   |
 *  +-------------------+
 *  | sa_len: socklen_t | 
 *  |                   |
 *  +-------------------+
 *  | turn: bool        | Si el jugador juega primero
 *  +-------------------+
 *
 */

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
 *  Clase para el servidor de enlazado de jugadores
 */
class P2PServer
{
public:
    P2PServer(const char * s, const char * p): socket(s, p)
    {
        socket.bind();
    };

    /**
     *  Thread principal del servidor recibe mensajes en el socket y
     *  guarda los clientes hasta que encuentra con quien emparejarlos
     */
    void do_messages();

private:
    /**
     *  Lista de clientes conectados al servidor, representados por
     *  su socket
     */
    std::vector<std::unique_ptr<Socket>> clients;

    /**
     *  Nombres de los jugadores
     */
    std::vector<std::string> names;
    /**
     *  Nombres de quien busca cada jugador
     */
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
     * @param n nombre del usuario
     * @param o oponente del usuario
     */
    NimClient(const char * s, const char * p, const char * n, const char * o):socket(s, p),
        name(n), opponent(o){for (int i = 0; i < GAME_SIZE; i++) game.push_back(NORMAL); run();};


    /**
    *  Enum del estado del juego
    */
    enum Stick
    {
        GONE   = 0,
        NORMAL = 1,
        SELECTED  = 2
    };

    /**
    *  Constantes del juego
    */
    const int GAME_SIZE = 6;
    const int MAX_MOVES = 3;

    /**
     *  Busca a su rival y lanza los threads cuando el servidor se lo da
     */
    void run();

    /**
     *  Envía el mensaje de logout al servidor
     */
    void logout();

    /**
     *  Procesa el input segun la logica de juego
     */
    bool processInput(std::string s);

    bool isGameOver();

    void render();

    /**
     *  Rutina principal para el Thread de E/S. Lee datos de STDIN (std::getline)
     *  y los envía por red vía el Socket.
     */
    void input_thread();

    /**
     *  Rutina del thread de Red. Recibe datos de la red 
     */
    void net_thread();

private:

    /**
     * Socket para comunicar con el otro jugador
     */
    Socket peer;
    /**
     * Socket para comunicar con el servidor
     */
    Socket socket;

    /**
     * Nombre del usuario
     */
    std::string name;
    /**
     * Nombre del oponente
     */
    std::string opponent;
    /**
     * Turno del usuario
     */
    bool myTurn;
    /**
     * Estado del juego
     */
    std::vector<Stick> game;
};

