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

    tmp += 80 * sizeof(char);

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

void GameMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data

    char * tmp = _data;

    memcpy(tmp, data.c_str(), 8 * sizeof(char));
}

int GameMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data

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

    //Serializar los campos type, nick y message en el buffer _data

    char * tmp = _data;

    memcpy(tmp, &sa, sizeof(struct sockaddr));

    tmp += sizeof(struct sockaddr);

    memcpy(tmp, &sa_len, sizeof(socklen_t));

    tmp += sizeof(socklen_t);

    memcpy(tmp, &turn, sizeof(bool));
}

int StartMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data

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
        std::cout << "Esperando mensaje\n";
        socket.recv(obj, sock);
        std::cout << "Mensaje recibido\n";
        bool found = false;

        auto it3 = clients.begin();
        for (auto it = names.begin(), it2 = opponents.begin(); it != names.end() && it2 != opponents.end() && it3 != clients.end(); ++it, ++it2, ++it3)
        {

            if (*it == obj.opponent && *it2 == obj.name)
            {
                //Manda a cada uno un mensaje con el puerto del otro y su turno
                //Begin message p1, p2 = ...
                std::cout << "Oponente encontrado\n";
                StartMessage s1;
                StartMessage s2;
                s1.sa = sock->getSockaddr();
                s1.sa_len = sock->getSockLen();
                int r = rand() % 2;
                s1.turn = false;
                Socket s = **it3;
                s2.sa = s.getSockaddr();
                s2.sa_len = s.getSockLen();
                s2.turn = true;
                //s2.turn = 1-r;
                socket.send(s2, *sock);
                socket.send(s1, **it3);

                // struct sockaddr_in* addr1 = (struct sockaddr_in*) &(sock->sa);
                // struct sockaddr_in* addr2 = (struct sockaddr_in*) &(s.sa);
                // std::cout <<"Direccion de " << obj.name << " que se envia a " << obj.opponent << addr1->sin_addr.s_addr << " " << addr1->sin_port << std::endl;
                // std::cout <<"Direccion de " << obj.opponent << " que se envia a " << obj.name << addr2->sin_addr.s_addr << " " << addr2->sin_port << std::endl;


                found = true;
                names.erase(it);
                opponents.erase(it2);
                clients.erase(it3);
                break;
            }
        }
        
        if (!found){
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

    socket.send(em, socket);   


    StartMessage bgn;
    socket.recv(bgn);
    myTurn = bgn.turn;
    // struct sockaddr_in* addr1 = (struct sockaddr_in*) &(bgn.sa);
    // std::cout << addr1->sin_addr.s_addr << " " << addr1->sin_port << std::endl;
    peer = Socket(&bgn.sa, bgn.sa_len);
    //socket.bind();
    //system("clear");
    render();

    std::thread in_thread(&NimClient::input_thread, this);

    net_thread();
    in_thread.join();
}

bool NimClient::isGameOver()
{
    for (int i = 0; i < game.size(); i++)
    {
        if (game[i] != GONE) return false;
    }
    return true;
}

void NimClient::render()
{
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

bool NimClient::procesaInput(std::string s)
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
            //system("clear");

            if (procesaInput(msg))
            {
                GameMessage gmsg(msg);
                socket.send(gmsg, peer);
            }
            render();
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
            GameMessage obj;
            Socket * s;
            std::cout << "Esperando mensaje\n";
            socket.recv(obj, s);

            struct sockaddr_in* addr1 = (struct sockaddr_in*) &(s->sa);
            std::cout << addr1->sin_addr.s_addr << " " << addr1->sin_port << std::endl;

            //system("clear");
            std::cout << obj.data << std::endl;
            procesaInput(obj.data);
            render();
        }	
        if (isGameOver()) break;
    }
}

