#include "Nim.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ServerMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    char * tmp = _data;

    memcpy(tmp, name.c_str(), 12 * sizeof(char));

    tmp += 12 * sizeof(char);

    memcpy(tmp, opponent.c_str(), 12 * sizeof(char));
}

int ServerMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    char * tmp = bobj;

    name = tmp;

    tmp += 12 * sizeof(char);

    opponent = tmp;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void GameMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    char * tmp = _data;

    memcpy(tmp, data.c_str(), 8 * sizeof(char));
}

int GameMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    char * tmp = bobj;

    data = tmp;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void StartMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    char * tmp = _data;

    memcpy(tmp, &sa, sizeof(struct sockaddr));

    tmp += sizeof(struct sockaddr);

    memcpy(tmp, &sa_len, sizeof(socklen_t));

    tmp += sizeof(socklen_t);

    memcpy(tmp, &turn, sizeof(bool));
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int StartMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    char * tmp = bobj;

    memcpy(&sa, tmp, sizeof(struct sockaddr));

    tmp += sizeof(struct sockaddr);

    memcpy(&sa_len, tmp, sizeof(socklen_t));

    tmp += sizeof(socklen_t);

    memcpy(&turn, tmp, sizeof(bool));

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void P2PServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        //Recibir Mensajes y si ya esperaban al remitente los manda a jugar
        //Si no lo añade al vector de clientes y este espera hasta que llegue quien le toca

        ServerMessage obj;
        Socket * sock;
        socket.recv(obj, sock);
        std::cout << "Ha entrado " << obj.name << " buscando a " << obj.opponent << std::endl;
        bool found = false;

        auto it3 = clients.begin();
        for (auto it = names.begin(), it2 = opponents.begin(); it != names.end() && it2 != opponents.end() && it3 != clients.end(); ++it, ++it2, ++it3)
        {
            if (*it == obj.opponent && *it2 == obj.name)
            {
                //Manda a cada uno un mensaje con el puerto del otro y su turno
                std::cout << "Oponente encontrado\n";
                StartMessage s1, s2;
                Socket s = **it3;
                s1.sa = sock->getSockaddr();
                s1.sa_len = sock->getSockLen();

                s2.sa = s.getSockaddr();
                s2.sa_len = s.getSockLen();

                int r = rand() % 2;
                s1.turn = r;
                s2.turn = 1-r;

                socket.send(s2, *sock);
                socket.send(s1, **it3);

                //Elimina al jugador que estaba esperando del servidor
                found = true;
                names.erase(it);
                opponents.erase(it2);
                clients.erase(it3);
                break;
            }
        }
        
        if (!found){
            //Añade al jugador al servidor si no encuentra a su oponente
            std::unique_ptr<Socket> sock_ptr(sock); 
            clients.push_back(std::move(sock_ptr));
            names.push_back(obj.name);
            opponents.push_back(obj.opponent);
        }
    }

}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void NimClient::run()
{
    ServerMessage em(name, opponent);

    //Se conecta al servidor
    socket.send(em, socket);   


    StartMessage bgn;
    //Espera a que el servidor le envie su rival
    socket.recv(bgn);
    myTurn = bgn.turn;
    peer = Socket(&bgn.sa, bgn.sa_len);

    //Llama al primer render
    system("clear");
    render();

    //Lanza los threads
    std::thread in_thread(&NimClient::input_thread, this);

    net_thread();
    in_thread.join();
}

bool NimClient::isGameOver()
{
    //Comprueba si queda algun palo sin eliminar
    for (int i = 0; i < game.size(); i++)
    {
        if (game[i] != GONE) return false;
    }
    return true;
}

void NimClient::render()
{
    //Dibuja el estado del juego
    std::string output;
    if (myTurn) std::cout << "Tu turno\n";
    else std::cout << "Turno del oponente\n";
    for (int i = 0; i < game.size(); i++)
    {
        if (game[i] == GONE) output.push_back('X');
        else if (game[i] == NORMAL) output.push_back('|');
        else if (game[i] == SELECTED) output.push_back('I');
    }
    std::cout << output << std::endl;
}

bool NimClient::processInput(std::string s)
{
    bool valid = false;
    std::stringstream str;
    int aux = 0;
    str << s;
    str >> aux;
    //aux es 0 cuando str no es entero
    if (aux > 0)
        if (aux > game.size())
            std::cout << "No hay tantos palos\n";
        else
        {
            aux -=1;
            if (game[aux] == GONE)
                std::cout << "Ese palo ya esta eliminado\n";
            else if (game[aux] == SELECTED) 
            {
                game[aux] = NORMAL;
                valid = true;
            }
            else
            {
                int n = 0;
                for (int i = 0; i < game.size(); i++)
                {
                    if (game[i] == SELECTED) n++;
                }
                if (n >= MAX_MOVES) std::cout << "Ya has seleccionado el numero maximo de palos \n";
                else
                {
                    valid = true;
                    if (game[aux] == NORMAL) game[aux] = SELECTED;
                }
            }
        }
    else if (s == "Pass")
    {
        for (int i = 0; i < game.size(); i++)
        {
            if (game[i] == SELECTED)
            {
                game[i] = GONE;
                valid = true;
            } 
        }
        //valid es true cuando hay al menos un palo seleccionado
        if (valid)
        {
            if (isGameOver())
                if (myTurn)
                    std::cout << "¡Has perdido!\n";
                else
                    std::cout << "¡Has ganado!\n";
            else
                myTurn = !myTurn;
        }
        else std::cout << "Tienes que elegir al menos un palo\n";
    }
    else 
        std::cout << "Comando no valido\n";
    return valid;
}

void NimClient::input_thread()
{
    while (true)
    {
        if(myTurn)
        {
            std::string msg;
	        std::getline(std::cin, msg);
            system("clear");

            //processInput devuelve false si el mensaje no es valido
            if (processInput(msg))
            {
                GameMessage gmsg(msg);
                socket.send(gmsg, peer);
            }
            if (isGameOver()) return;
            else render();
        }
    }
}

void NimClient::net_thread()
{
    while(true)
    {
        if (!myTurn && !isGameOver())
        {
            //Recibir Mensajes de red
            GameMessage obj;
            socket.recv(obj);

            system("clear");
            processInput(obj.data);
            if (isGameOver()) return;
            else render();
        }	
        
    }
}

