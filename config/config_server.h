// #include <arpa/inet.h>  // htons(), inet_addr()
// #include <dirent.h>     // opendir(), readdir(), closedir()
// #include <errno.h>      // errno
// #include <netinet/in.h> // inet_addr(), bind()
// #include <signal.h>     // signal()
// #include <stdbool.h>    // bool
// #include <stdio.h>
// #include <stdlib.h>     // strtol()
// #include <string.h>     // bzero(), strlen(), strcmp(), strcpy(), strtok(), strrchr(), memcmp()
// #include <sys/socket.h> // socket(), inet_addr(), bind(), listen(), accept(), recv(), send()
// #include <sys/types.h>  // socket()
// #include <unistd.h>     // close()
// #include <ctype.h>
// #include <sys/stat.h>   //mkdir()
// #include <sys/wait.h>   //waitpid();

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include "../utils/message/message.h"

// int setup_listener(int portno)
// {
//     int sockfd;
//     struct sockaddr_in serv_addr;

//     sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0)
//     {
//         error("ERROR opening listener socket.");
//     }

//     memset(&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr.s_addr = INADDR_ANY;
//     serv_addr.sin_port = htons(portno);

//     if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
//     {
//         error("ERROR binding listener socket.");
//     }

//     printf("[DEBUG] Listener set.\n");

//     return sockfd;
// }
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
    strcpy(buffer, "SELECT_MODE|");
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

                if (strstr(buffer, "GUEST"))
                {
                    handleGuest(name, cli, buffer);
                }
                else if (strstr(buffer, "SIGNUP"))
                { // TODO:luu vao file
                    handleReg(cli, buffer);
                }
                else if (strstr(buffer, "LOGOUT"))
                {
                    handleLogOut(&isLogin, cli, buffer);
                }
                else if (strstr(buffer, "LOGIN"))
                {
                    handleLogin(&isLogin, cli, buffer);
                }

                else if (strcmp(buffer, "CREATE") == 0 || strcmp(buffer, "CREATE RANK") == 0)
                {
                    handleCreateRoom(&isLogin, &flag, cli, buffer);
                }

                // random
                else if (strcmp(command, "JOIN") == 0)
                {
                    handleJoin(&isLogin, &number, cli);
                }
                else if (strcmp(command, "LIST") == 0)
                {
                    handleListRooms(cli);
                }
                else if (strcmp(command, "LEAVE") == 0)
                {
                    handleLeave(cli);
                }
                else if (strcmp(command, "START") == 0)
                {
                    handleStart(cli);
                }
                else if (strcmp(command, "PLAY") == 0)
                {
                    handlePlay(&number, cli);
                }
                else
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, "INVALID_CMD|");
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
                for (p = root; p != NULL; p = p->next)
                {
                    if (strcmp(cli->player_info.username, p->player.username) == 0)
                    {
                        p->player.is_online = 0;

                        saveData1(p->player);
                        // TODO : luu data vaofile khi logout
                    }
                }
                traversingList2(root);
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
    strcpy(buffer, "EXIT|");
    strcat(buffer, "bye");
    send_message(buffer, cli->uid);

    close(cli->sockfd);
    queue_remove_client(cli->uid);
    free(cli);
    client_count--;
    pthread_detach(pthread_self());

    return NULL;
}
int setup_server(int input_port)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // validate input port number
    int const PORT = atoi(input_port);
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
    return sockfd;
}