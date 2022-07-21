#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include "config/constants.h"
#include "config/protocol_header.h"
#include "utils/stdio_custom.h"

// Symmetric 1 key for encrypt and decrypt
static _Atomic unsigned int client_count = 0;
static int uid = 10;
static int roomUid = 1;
int firstElo, secondElo;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t auth_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "models/rooms.h"

Client *clients[MAX_CLIENT];
Room *rooms[MAX_ROOM];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "utils/queue_handler.h"

int i = 0;

float Probability(int rating1, int rating2)
{
    return 1.0 * 1.0 / (1 + 1.0 * pow(10, 1.0 * (rating1 - rating2) / 400));
}

void EloRating(int Ra, int Rb, int K, int d)
{
    float Pb = Probability(Ra, Rb);

    float Pa = Probability(Rb, Ra);

    int a, b;
    if (d == 1)
    {
        firstElo = Ra + K * (1 - Pa);
        secondElo = Rb + K * (0 - Pb);
    }

    else if (d == 0)
    {
        firstElo = Ra + K * (0 - Pa);
        secondElo = Rb + K * (1 - Pb);
    }
    else
    {

        firstElo = Ra + K * (0.5 - Pa);
        secondElo = Rb + K * (0.5 - Pb);
    }

    /*fflush(stdout);
    printf( "Ra = %d Rb = %d", Ra,Rb );*/
}

void send_message(char *message, int uid)
{

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (clients[i])
        {
            if (clients[i]->uid == uid)
            {

                if (write(clients[i]->sockfd, message, strlen(message)) < 0)
                {
                    printf("ERROR: write to descriptor failed\n");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

#include "auth/auth_server.h"
#include "auth/security.h"
#include "controller/server_game_controller.h"

void *handle_client(void *arg)
{
    char buffer[BUFF_SIZE];
    char command[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    int number;
    char name[USERNAME_LEN];
    int leave_flag = 0;
    int flag = 0;
    int isRank = 0;
    int isLogin = 0; /* logout then isLogin=0 */
    Client *cli = (Client *)arg;
    char user[100];
    char pass[100];

    // name nhan tin hieu
    recv(cli->sockfd, name, USERNAME_LEN, 0);

    cli->player_info.elo_rating = 1200;
    cli->player_info.is_online = 0;
    strcpy(cli->player_info.username, "unknown");

    sprintf(buffer, "> %s has joined\n", cli->player_info.username);
    printf("%s", buffer);

    bzero(buffer, BUFF_SIZE);
    strcpy(buffer, H_SELECT_MODE);
    strcat(buffer, "|");
    strcat(buffer, "ok1");
    send_message(buffer, cli->uid);

    bzero(buffer, BUFF_SIZE);
    char *p;
    while (leave_flag == 0)
    {
        command[0] = '\x00';
        number = 0;

        int receive = recv(cli->sockfd, buffer, BUFF_SIZE, 0);

        if (receive > 0)
        {
            if (strlen(buffer) > 0)
            {

                // send_message(buffer, cli->uid);
                handle_new_line_char(buffer, strlen(buffer));
                printf("> client: '%s' has been send '%s' command\n", cli->player_info.username, buffer);
                sscanf(buffer, "%[^|]|%i", &command[0], &number);

                if (strstr(buffer, H_GUEST_MODE))
                {
                    handleGuest(name, cli, buffer);
                }
                else if (strstr(buffer, H_REGISTER))
                { // TODO:luu vao file
                    handleReg(cli, buffer);
                }
                else if (strstr(buffer, H_LOG_OUT))
                {
                    handleLogOut(&isLogin, cli, buffer);
                }
                else if (strstr(buffer, H_SIGN_IN))
                {
                    handleLogin(&isLogin, cli, buffer);
                }

                else if (strcmp(buffer, H_CREATE_ROOM_CASUAL) == 0 || strcmp(buffer, H_CREATE_ROOM_RANKED) == 0)
                {
                    handleCreateRoom(&isLogin, &flag, cli, buffer);
                }

                // random
                else if (strcmp(command, H_JOIN_ROOM) == 0)
                {
                    handleJoin(&isLogin, &number, cli);
                }
                else if (strcmp(command, H_LIS_ROOM) == 0)
                {
                    handleListRooms(cli);
                }
                else if (strcmp(command, H_LEAVE_ROOM) == 0)
                {
                    handleLeave(cli);
                }
                else if (strcmp(command, H_START_GAME) == 0)
                {
                    handleStart(cli);
                }
                else if (strcmp(command, H_PLAY) == 0)
                {
                    handlePlay(&number, cli);
                }
                else
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, H_INVALID_CMD);
                    strcat(buffer, "|");
                    strcat(buffer, "Invalid command\n");
                    send_message(buffer, cli->uid);
                }
            }
        }
        else if (receive == 0 || strcmp(buffer, "exit") == 0)
        {

            // TODO: THEM PHAN LOGOUT VAO DAY
            if (isLogin == 1)
            {

                isLogin = 0;
                PlayerNode *p;
                for (p = root_p; p != NULL; p = p->next)
                {
                    if (strcmp(cli->player_info.username, p->player.username) == 0)
                    {
                        p->player.is_online = 0;

                        save_data(p->player);
                        // TODO : luu data vaofile khi logout
                    }
                }
                traverse_list_p(root_p);
            }
            sprintf(buffer, "%s has left\n", cli->player_info.username);
            printf("%s", buffer);
            leave_flag = 1;
        }
        else
        {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buffer, BUFF_SIZE);
    }

    bzero(buffer, BUFF_SIZE);
    strcpy(buffer, H_EXIT);
    strcat(buffer, "|");
    strcat(buffer, "bye");
    send_message(buffer, cli->uid);

    close(cli->sockfd);
    queue_remove_client(cli->uid);
    free(cli);
    client_count--;
    pthread_detach(pthread_self());

    return NULL;
}

void print_header()
{
    printf("--------------- Chinese chess playground server ---------------\n");
}
int main(int argc, char **argv)
{
    printf("Run here\n");
    if (argc != 2)
    {
        printf("Syntax error!\n-->Usage : ./server [PORT_NUMBER] (i.e. ./server 8888) !\n");
        exit(EXIT_FAILURE);
    }
    // int sockfd = setup_server(atoi(argv[1]));
    int sockfd;
    struct sockaddr_in serv_addr;

    // validate input port number
    int const PORT = atoi(argv[1]);
    if (PORT == 0)
    {
        print_err("Invalid port number specified");
        exit(EXIT_FAILURE);
    }

    // socket construction
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        print_err("Failed to construct a listener socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Signals
    signal(SIGPIPE, SIG_IGN);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_err("Failed to bind the listener socket");
    }

    int option = 1;
    int connection_fd = 0;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    // start listening
    if (listen(sockfd, MAX_CLIENT) < 0)
    {
        print_err("Failed to listen");
        return EXIT_FAILURE;
    }

    clear_screen();

    print_header();
    import_text_file("./database/players.txt");
    traverse_list_p(root_p);
    while (1)
    {
        socklen_t cli_len = sizeof(cli_addr);
        connection_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);

        // check dor max clients
        if ((client_count + 1) == MAX_CLIENT)
        {
            printf("Maximun of clients are connected, Connection rejected");
            close(connection_fd);
            continue;
        }
        // printf("New connection:%d\n", connection_fd);
        //  clients settings
        Client *cli = (Client *)malloc(sizeof(Client));
        cli->address = cli_addr;
        cli->sockfd = connection_fd;
        cli->uid = uid++;
        printf("Uid:%d\n", cli->uid);
        // add client to queue
        queue_add_client(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);

        // reduce CPU usage
        sleep(1);
    }

    return EXIT_SUCCESS;
}
