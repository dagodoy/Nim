#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ServerMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data

    char * tmp = _data;

    memcpy(tmp, name.c_str(), 80 * sizeof(char));

    tmp += 8 * sizeof(char);

    memcpy(tmp, opponent.c_str(), 80 * sizeof(char));
}

int ServerMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data

    char * tmp = bobj;

    name = tmp;
    tmp += 80 * sizeof(char);
    opponent = tmp;

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
        std::unique_ptr<Socket> sock_ptr(sock); 
        bool found = false;

        auto it3 = clients.begin();
        for (auto it = names.begin(), it2 = opponents.begin(); it != names.end() && it2 != opponents.end() && it3 != clients.end(); ++it, ++it2, ++it3)
        {
            if (*it == obj.opponent && *it2 == obj.name)
            {
                //Manda a cada uno un mensaje con el puerto del otro y su turno
                //Begin message p1, p2 = ...
                socket.send(obj, *sock);
                socket.send(obj, **it3);


                found = true;
                names.erase(it);
                opponents.erase(it2);
                clients.erase(it3);
                break;
            }
        }
        
        if (!found){
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

    serverSocket.send(em, serverSocket);   

    //BeginMessage bgn;
    //serverSocket.recv(bgn);
    //myTurn = bgn.turn;
    //socket = bgn.socket;
}

bool NimClient::isGameOver()
{
    for (int i = 0; i < game.size(); i++)
    {
        if (game[i] != GONE) return true;
    }
    return false;
}

bool NimClient::procesaInput(std::string s)
{
    bool valid = false;
    std::stringstream str;
    int aux = 0;
    str << s;
    str >> aux;
    if (aux > 0)
        if (aux > game.size())
            std::cout << "No hay tantos palos\n";
        else
        {
            aux -=1;
            if (game[aux] == GONE)
                std::cout << "Ese palo ya esta eliminado\n";
            else 
            {
                valid = true;
                if (game[aux] == NORMAL) game[aux] = SELECTED;
                else if (game[aux] == SELECTED) game[aux] = NORMAL;
            }
        }
    else if (s == "Pass")
    {
        for (int i = 0; i < game.size(); i++)
        {
            if (game[i] == SELECTED) game[i] = GONE;
        }
        if (isGameOver())
            if (myTurn)
                std::cout << "¡Has perdido!";
            else
                std::cout << "¡Has ganado!";
        else
            myTurn = !myTurn;
        
        valid = true;
    }
    else 
        std::cout << "Comando no valido\n";
    return valid;
}

void NimClient::input_thread()
{
    while (true)
    {
        if(myTurn )
        {
	        std::string msg;
	        std::getline(std::cin, msg);

            if (procesaInput(msg))
            {
                //PeerMsg pmsg;
                //pmsg.msg = msg;
                //socket.send(msg, socket);
            }
        }
        if (isGameOver()) break;
    }
}

void NimClient::net_thread()
{
    while(true)
    {
        if (!myTurn && !isGameOver())
        {
            //Recibir Mensajes de red
            ServerMessage obj;
            Socket * sock = 0;
            socket.recv(obj, sock);

            //procesaInput(obj.msg);
        }	
        if (isGameOver()) break;
    }
}

