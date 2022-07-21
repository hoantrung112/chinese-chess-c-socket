#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> // struct sockaddr_in
#include "players.h"

// Client model
typedef struct
{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    Player player_info;
} Client;

// Game model
typedef struct
{
    char *FEN_string;
    int is_over;
    int round;
    int client_turn;
} Game;

// Room model
typedef struct
{
    Client *player1;
    Client *player2;
    unsigned int uid;
    char state[100];
    Game *game;
    char room_type[10];
} Room;

typedef struct node_t
{
    Room room;
    struct node_t *next;
} RoomNode;

#include "../services/rooms_service.h"