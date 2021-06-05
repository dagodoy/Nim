#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

class Jugador: public Serializable
{
public:
    Jugador(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y)
    {
        strncpy(name, _n, 80);
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        int32_t data_size = 80 * sizeof(char) + 2 * sizeof(int16_t);
        alloc_data(data_size);
        
        char * tmp = _data;

        memcpy(tmp, name, 80 * sizeof(char));

        tmp += 80 * sizeof(char);

        memcpy(tmp, &x, sizeof(int16_t));

        tmp += sizeof(int16_t);

        memcpy(tmp, &y, sizeof(int16_t));
    }

    int from_bin(char * data)
    {
        char * tmp = data;
	
        memcpy(name, tmp, 80 * sizeof(char));

        tmp += 80 * sizeof(char);

        memcpy(&x, tmp, sizeof(int16_t));

        tmp += sizeof(int16_t);

        memcpy(&y, tmp, sizeof(int16_t));
        return 0;
    }


public:
    char name[80];

    int16_t x;
    int16_t y;
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("Player_ONE", 123, 987);

    // 1. Serializar el objeto one_w
    // 2. Escribir la serialización en un fichero
    // 3. Leer el fichero
    // 4. "Deserializar" en one_r
    // 5. Mostrar el contenido de one_r
    int fd = open("./data_jugador", O_CREAT | O_TRUNC | O_RDWR, 0666);

    one_w.to_bin(); 

    write(fd, one_w.data(), one_w.size());

    close(fd);

    fd = open("./data_jugador", O_RDONLY);
	
    int buffsize = 80 * sizeof(char) + 2 * sizeof(int16_t);
    char * buffer =(char*) malloc(buffsize);
    
    read(fd, buffer, buffsize);
    
    one_r.from_bin(buffer);
    
    std::cout << one_r.name << " " << one_r.x << " " << one_r.y << std::endl;
    free(buffer);
    return 0;
}

