#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
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
#include "auth/security.h"
#include "utils/message/message.h"

#include "auth/auth_client.h"
#include "controller/client_game_controller.h"

volatile sig_atomic_t flag = 0;
int sockfd = 0;
char username[USERNAME_LEN];
int player = 1;

pthread_t lobby_thread;     // send message thread
pthread_t recv_msg_thread;  // recv msg thread
pthread_t multiplayer_game; // game thread

char ip[IP_LEN];
int port;
void launch();
void catch_ctrl_c_and_exit()
{
    flag = 1;
}

void showPositions()
{
    printf("\nPosition map:");
    printf("\n 7 | 8 | 9");
    printf("\n 4 | 5 | 6");
    printf("\n 1 | 2 | 3\n");
}

char winState[100] = "";    /*state game:win or lose*/
char updatedElo[100] = "";  /*elo_player_1*/
char updatedElo1[100] = ""; /*elo_player_2*/

char gameType[100]; /*game mode: normal or rank*/

// dua tren turn declare
void split(char a[100], int playerTurn)
{
    // printf("Str:%s\n",a);
    bzero(gameType, 100);
    bzero(winState, 100);

    bzero(updatedElo, 100);
    bzero(updatedElo1, 100);

    char *token = strtok(a, "|");

    strcpy(gameType, token);

    // if rank
    if (strcmp(gameType, "[RANK]") == 0)
    {
        token = strtok(NULL, "|");

        strcat(updatedElo, token);
        token = strtok(NULL, "|");
        strcat(updatedElo1, token);
    }

    token = strtok(NULL, "|");
    strcat(winState, token);
}

void showBoard(char board[3][3], char *errorMessage)
{
    int line;

    clear_screen();

    if (*errorMessage != '\x00')
    {
        printf("Warning: %s!\n\n", errorMessage);
        *errorMessage = '\x00';
    }

    printf("#############\n");

    for (line = 0; line < 3; line++)
    {
        printf("# %c | %c | %c #", board[line][0], board[line][1], board[line][2]);

        printf("\n");
    }

    printf("#############\n");

    showPositions();
}

void createBoard(char board[3][3])
{
    int line, col;

    for (line = 0; line < 3; line++)
    {
        for (col = 0; col < 3; col++)
        {
            board[line][col] = '-';
        }
    }
}

void menu();

void *lobby(void *arg);

void recv_msg_handler();

void *multiplayerGame(void *arg)
{
    int playerTurn;
    playerTurn = player; // ngchoi 1 hay 2
    char namePlayer1[32];
    char namePlayer2[32];

    strcpy(namePlayer1, username);

    char board[3][3];
    int iterator;
    int linePlay, colPlay;
    int positionPlay;
    int round = 1;
    int gameStatus = 1;
    int valid_play = 0;
    int played;
    int numberPlayed;
    // position array    (row,col)
    int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

    char errorMessage[255] = {'\x00'};
    char *nameCurrentPlayer;  /*username player 1*/
    char *nameCurrentPlayer1; /*username opponent*/

    // message
    char message[BUFF_SIZE] = {};
    char rep[BUFF_SIZE] = {};
    char status[100];

    // TODO : FORMAT   PLAYER2| NAME_PLAYER_2
    int receive = recv(sockfd, rep, BUFF_SIZE, 0);

    if (receive > 0)
    {
        // handle message
        strcpy(status, strtok(rep, "|"));
        strcpy(message, strtok(NULL, "|"));
        setbuf(stdin, 0);
        handle_new_line_char(message, strlen(message));
        sscanf(message, "%s", &namePlayer2[0]);

        setbuf(stdout, 0);
        setbuf(stdin, 0);

        bzero(message, BUFF_SIZE);
        bzero(rep, 100);
        createBoard(board);
        round = 1;

        while (round < 10 && gameStatus == 1)
        {
            if (playerTurn == 1)
            {
                nameCurrentPlayer = (char *)&namePlayer1;
                nameCurrentPlayer1 = (char *)&namePlayer2;
            }
            else
            {
                nameCurrentPlayer1 = (char *)&namePlayer1;
                nameCurrentPlayer = (char *)&namePlayer2;
            }

            showBoard(board, (char *)&errorMessage);

            printf("\nRound: %d", round);
            printf("\nPlayer: %s\n", nameCurrentPlayer);

            while (valid_play == 0)
            {
                bzero(message, BUFF_SIZE);
                bzero(rep, 100);

                int receive = recv(sockfd, rep, BUFF_SIZE, 0);
                // strcpy(message, message);
                printf("Receive:%s\n", rep);
                if (receive > 0)
                {
                    valid_play = 1;
                    strcpy(status, strtok(rep, "|"));
                    strcpy(message, strtok(NULL, "|"));
                    setbuf(stdin, 0);
                    setbuf(stdout, 0);
                    // your turn
                    if (strcmp(message, "1") == 0)
                    {
                        do
                        { // Turn
                            printf("Enter a position: ");
                            scanf("%d", &positionPlay);

                            linePlay = posicoes[positionPlay - 1][0];
                            colPlay = posicoes[positionPlay - 1][1];

                            if (board[linePlay][colPlay] == 'X' || board[linePlay][colPlay] == 'O')
                            {
                                printf("Invalid move\n");
                            }

                        } while (board[linePlay][colPlay] == 'X' || board[linePlay][colPlay] == 'O');
                        if (valid_play == 1)
                        {
                            handle_make_move(positionPlay, message);
                            // sprintf(message, "PLAY|%i\n", positionPlay);
                            send(sockfd, message, strlen(message), 0);
                            bzero(message, BUFF_SIZE);
                        }
                    }
                    else if (strcmp(message, "2") == 0)
                    {
                        printf("The other player is playing...\n");

                        played = 0;

                        while (played == 0)
                        { // TODO: NHAN MOVE

                            int receive = recv(sockfd, message, BUFF_SIZE, 0);
                            // strcpy(message, message);

                            if (receive > 0)
                            {
                                sscanf(message, "%i", &numberPlayed);

                                linePlay = posicoes[numberPlayed - 1][0];
                                colPlay = posicoes[numberPlayed - 1][1];

                                played = 1;
                            }
                        }

                        valid_play = 1;
                    }
                }
                else
                {
                    valid_play = 0;
                }
            }

            if (playerTurn == 1)
            {
                board[linePlay][colPlay] = 'X';
                playerTurn = 2;
            }
            else
            {
                board[linePlay][colPlay] = 'O';
                playerTurn = 1;
            }

            for (iterator = 0; iterator < 3; iterator++)
            {
                if (
                    (
                        (board[iterator][0] == board[iterator][1]) && (board[iterator][1] == board[iterator][2]) && board[iterator][0] != '-') ||
                    ((board[0][iterator] == board[1][iterator]) && (board[1][iterator] == board[2][iterator]) && board[0][iterator] != '-'))
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (board[0][0] == board[1][1]) && (board[1][1] == board[2][2]) && board[0][0] != '-') ||
                ((board[0][2] == board[1][1]) && (board[1][1] == board[2][0]) && board[0][2] != '-'))
            {
                gameStatus = 0;
            }

            round++;
            valid_play = 0;
            bzero(message, BUFF_SIZE);
        }

        bzero(message, BUFF_SIZE);
        // TODO : FORMAT | message
        int receive = recv(sockfd, message, BUFF_SIZE, 0);
        // strcpy(message, message);
        if (receive > 0)
        {
            setbuf(stdin, 0);
            setbuf(stdout, 0);

            showBoard(board, (char *)&errorMessage);
            split(message, playerTurn);
            // printf("\nPlayer '%s' win!\nUpdated elo: '%s'", winState,updatedElo);
            if (strcmp(gameType, "[NORMAL]") == 0)
            {
                if (strcmp(winState, "win") == 0)
                {
                    printf("\n You win\n");
                    if (playerTurn == 1)
                    {
                        // nameCurrentPlayer = (char *)&namePlayer1;
                        // nameCurrentPlayer1 = (char *)&namePlayer2;
                        printf("\nPlayer '%s' lose!\n", nameCurrentPlayer);
                    }
                    else
                    {
                        printf("\nPlayer '%s' lose!\n", nameCurrentPlayer1);
                    }
                }
                else if (strcmp(winState, "lose") == 0)
                {
                    printf("\nYou lose");
                    if (playerTurn == 1)
                    {
                        // nameCurrentPlayer = (char *)&namePlayer1;
                        // nameCurrentPlayer1 = (char *)&namePlayer2;
                        printf("\nPlayer '%s' win!\n", nameCurrentPlayer);
                    }
                    else
                    {
                        printf("\n Player %s  win!", nameCurrentPlayer1);
                    }
                }
                else
                {
                    printf("\nDraw match");
                }
            }

            else
            {
                if (strcmp(winState, "win") == 0)
                {
                    printf("\nYou win!\nUpdated elo: '%s'", updatedElo); // khi tk1 win
                    printf("\nPlayer '%s' lose!\nUpdated elo: '%s'", nameCurrentPlayer1, updatedElo1);
                }
                else if (strcmp(winState, "lose") == 0)
                {

                    printf("\nYou lose!\nUpdated elo: '%s'", updatedElo1);
                    printf("\nPlayer '%s' win!\nUpdated elo: '%s'", nameCurrentPlayer, updatedElo);
                }
                else
                {
                    printf("\nDraw match");

                    if (playerTurn == 1)
                    {
                        printf("\nYour  elo: '%s'", updatedElo);
                        printf("\nPlayer '%s'  elo: '%s'", nameCurrentPlayer, updatedElo1);
                    }
                    else
                    {
                        printf("\nYour  elo: '%s'", updatedElo1);
                        printf("\nPlayer '%s'  elo: '%s'", nameCurrentPlayer1, updatedElo);
                    }
                }
            }
            printf("\nEnd of the game!\n");

            sleep(6);

            if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0)
            {
                printf("ERROR: pthread\n");
                return NULL;
            }

            if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
            {
                printf("ERROR: pthread\n");
                return NULL;
            }

            pthread_detach(pthread_self());   // thu hoi tai nguyen
            pthread_cancel(multiplayer_game); // huy thread ngay
        }
    }

    return NULL;
}
char username[USERNAME_LEN]; // hold username temp when not sign_in
void *lobby(void *arg)
{
    char buffer[BUFF_SIZE] = {};
    char uname[USERNAME_LEN], pass[PASSWORD_LEN];
    int i;
    while (1)
    {
        str_overwrite_stdout();
        fgets(buffer, BUFF_SIZE, stdin);
        for (i = 0; buffer[i] != '\0'; i++)
        {
            /*
             * If current character is lowercase alphabet then
             * convert it to uppercase.
             */
            if (buffer[i] >= 'a' && buffer[i] <= 'z')
            {
                buffer[i] = buffer[i] - 32;
            }
        }
        handle_new_line_char(buffer, BUFF_SIZE);

        if (strcmp(buffer, "GUEST") == 0)
        {
            memset(uname, '\0', (strlen(uname) + 1));
            printf("Run here");
            guest(uname, buffer);
            printf("Run here1");
            strcpy(username, uname);
        }

        if (strcmp(buffer, "JOIN") == 0)
        {
            handle_join_room(buffer);
        }
        if (strcmp(buffer, "SIGNUP") == 0)
        {
            memset(uname, '\0', (strlen(uname) + 1));
            memset(pass, '\0', (strlen(pass) + 1));
            sign_up(uname, pass, buffer);
        }
        if (strcmp(buffer, "LOGIN") == 0)
        {

            sign_in(uname, pass, buffer);
            strcpy(username, uname);
        }
        if (strcmp(buffer, "LOGOUT") == 0)
        {

            memset(buffer, '\0', (strlen(buffer) + 1));
            logOut(username, buffer);

            // str_overwrite_stdout();
        }
        if (strcmp(buffer, "EXIT") == 0)
        {
            break;
        }
        else
        {
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(buffer, BUFF_SIZE);
    }

    catch_ctrl_c_and_exit();

    return NULL;
}

void menu()
{
    printf("Comandos:\n");
    printf("\t -list\t\t\t  List all tic-tac-toe rooms\n");
    printf("\t -create\t\t   Normal Room\n");
    printf("\t -create rank\t\t  Ranked room\n");
    printf("\t -join\t\t  Join in one tic-tac-toe room\n");
    printf("\t -leave\t\t\t  Back of the one tic-tac-toe room\n");
    printf("\t -start\t\t\t  Starts one tic-tac-toe game\n");
    printf("\t -sign_in\t\t\t  Logged in to save your account\n");
    printf("\t -signup\t\t  Dont' have an account? Register\n");
    printf("\t -exit\t\t\t  Close terminal\n\n");
}
void logged_menu()
{
    printf("Commands:\n");
    printf("\t -list\t\t\t  List all tic-tac-toe rooms\n");
    printf("\t -create\t\t   Normal Room\n");
    printf("\t -create rank\t\t  Ranked room\n");
    printf("\t -join\t\t  Join in one tic-tac-toe room\n");
    printf("\t -leave\t\t\t  Back of the one tic-tac-toe room\n");
    printf("\t -start\t\t\t  Starts one tic-tac-toe game\n");
    printf("\t -logout\t\t  Starts one tic-tac-toe game\n\n");
}
void select_mode()
{

    printf("guest  -signin as guest\n");
    printf("signin  -signin\n");
    printf("signup -register new account\n\n");
}
void recv_msg_handler()
{
    char message[BUFF_SIZE] = {};
    char buffer[BUFF_SIZE] = {};
    char header[100];
    // response res;
    clear_screen();

    while (1)
    {

        int receive = recv(sockfd, buffer, BUFF_SIZE, 0);
        strcpy(header, strtok(buffer, "|"));
        strcpy(message, strtok(NULL, "|"));
        // int receive = recv(sockfd, &res, sizeof(res), 0);
        // strcpy(message, message);
        if (receive > 0)
        {
            if (strcmp(header, H_SELECT_MODE) == 0)
            {
                select_mode();
                str_overwrite_stdout();
            }
            else if (strcmp(header, H_MENU) == 0 || strcmp(header, H_GAME_OVER) == 0)
            {

                menu();
                str_overwrite_stdout();
            }
            else if (strcmp(header, H_START_1) == 0)
            {
                pthread_cancel(lobby_thread);
                // pthread_kill(recv_msg_thread, SIGUSR1);

                player = 1;
                if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
                {
                    print_err("Failed to create a multiplayer game thread");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);

                // pthread_kill(lobby_thread, SIGUSR1);
            }
            else if (strcmp(header, H_START_2) == 0)
            {
                pthread_cancel(lobby_thread);
                // pthread_kill(recv_msg_thread, SIGUSR1);

                player = 2;
                if (pthread_create(&multiplayer_game, NULL, (void *)multiplayerGame, NULL) != 0)
                {
                    print_err("Failed to create a multiplayer game thread");
                    exit(EXIT_FAILURE);
                }
                pthread_detach(pthread_self());
                pthread_cancel(recv_msg_thread);

                // pthread_kill(lobby_thread, SIGUSR1);
            }
            else if (strcmp(header, H_SUC_SIGN_IN) == 0) // signed in successfully
            {

                print_msg("Signed in successfully");
                strcpy(username, username);
                strcpy(username, message);
                clear_screen();
                printf("Hello:%s\n", username);
                logged_menu();
                str_overwrite_stdout();
            }
            else if (strcmp(header, H_SUC_LOG_OUT) == 0)
            {
                printf(">Good bye %s\n", username);
                strcpy(username, "");
                strcpy(username, username);
                sleep(1);
                clear_screen();
                menu();
                str_overwrite_stdout();
            }
            else if (strcmp(header, H_LIST_ROOM_AVAIL) == 0)
            {
                printf("%s", message);
                str_overwrite_stdout();
            }
            else if (receive == 0)
            {
                break;
            }
            else
            {
                // printf("%s---%s", status,message);
                printf("[%s]-[SERVER] %s", header, message);
                str_overwrite_stdout();
            }

            bzero(message, BUFF_SIZE);
            bzero(buffer, BUFF_SIZE);
            bzero(header, 100);
        }
    }
}

int conectGame(char *ip, int port)
{
    setbuf(stdin, 0);
    // printf("1.Play as guest");
    // printf("2.Login");
    /*
    do
    {
        printf("Enter your username(guest): ");
        fgets(username, BUFF_SIZE, stdin);
        handle_new_line_char(username, BUFF_SIZE);

        // strcpy(username, "murilo");

        if (strlen(username) > USERNAME_LEN - 1 || strlen(username) < 2)
        {
            printf("Enter username corretly (length>1)\n");
        }
    } while (strlen(username) > USERNAME_LEN - 1 || strlen(username) < 2);*/
    strcpy(username, "GO");
    struct sockaddr_in server_addr;

    // socket settings
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // connect to the server
    // int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        print_err("Failed to connect to server");
        return EXIT_FAILURE;
    }

    // send the username
    send(sockfd, username, USERNAME_LEN, 0);

    if (pthread_create(&lobby_thread, NULL, &lobby, NULL) != 0)
    {
        print_err("Failed to create a lobby thread");
        return EXIT_FAILURE;
    }

    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
    {
        print_err("Failed to create a receiving thread");
        return EXIT_FAILURE;
    }

    while (1)
    {
        if (flag)
        {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}

void game()
{
    char board[3][3];
    int iterator;
    int linePlay, colPlay;
    int positionPlay;
    int playerTurn = 1;
    int round = 0;
    int gameStatus = 1;
    int optionRestart;
    int gaming = 1;

    char errorMessage[255] = {'\x00'};

    char namePlayer1[32];
    char namePlayer2[32];
    char *nameCurrentPlayer;

    setbuf(stdin, 0);

    printf("1st player's username: ");
    fgets(namePlayer1, 32, stdin);

    namePlayer1[strlen(namePlayer1) - 1] = '\x00';

    printf("2nd player's username: ");
    fgets(namePlayer2, 32, stdin);

    namePlayer2[strlen(namePlayer2) - 1] = '\x00';

    while (gaming == 1)
    {
        createBoard(board);
        round = 0;
        playerTurn = 1;

        while (round < 9 && gameStatus == 1)
        {
            showBoard(board, (char *)&errorMessage);

            if (playerTurn == 1)
            {
                nameCurrentPlayer = (char *)&namePlayer1;
            }
            else
            {
                nameCurrentPlayer = (char *)&namePlayer2;
            }

            int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};

            printf("\nRound: %d", round);
            printf("\nPlayer: %s\n", nameCurrentPlayer);
            printf("Enter a position: ");
            scanf("%d", &positionPlay);

            if (positionPlay < 1 || positionPlay > 9)
            {
                errorMessage[0] = '\x70';
                errorMessage[1] = '\x6F';
                errorMessage[2] = '\x73';
                errorMessage[3] = '\x69';
                errorMessage[4] = '\x63';
                errorMessage[5] = '\x61';
                errorMessage[6] = '\x6F';
                continue;
            }

            linePlay = posicoes[positionPlay - 1][0];
            colPlay = posicoes[positionPlay - 1][1];

            if (board[linePlay][colPlay] != '-')
            {
                errorMessage[0] = '\x70';
                errorMessage[1] = '\x6F';
                errorMessage[2] = '\x73';
                errorMessage[3] = '\x69';
                errorMessage[4] = '\x63';
                errorMessage[5] = '\x61';
                errorMessage[6] = '\x6F';
                errorMessage[7] = '\x20';
                errorMessage[8] = '\x6A';
                errorMessage[9] = '\x61';
                errorMessage[10] = '\x20';
                errorMessage[11] = '\x65';
                errorMessage[12] = '\x6D';
                errorMessage[13] = '\x20';
                errorMessage[14] = '\x75';
                errorMessage[15] = '\x73';
                errorMessage[16] = '\x6F';
                continue;
            }

            if (playerTurn == 1)
            {
                board[linePlay][colPlay] = 'X';
                playerTurn = 2;
            }
            else
            {
                board[linePlay][colPlay] = 'O';
                playerTurn = 1;
            }

            for (iterator = 0; iterator < 3; iterator++)
            {
                if (
                    (
                        (board[iterator][0] == board[iterator][1]) && (board[iterator][1] == board[iterator][2]) && board[iterator][0] != '-') ||
                    ((board[0][iterator] == board[1][iterator]) && (board[1][iterator] == board[2][iterator]) && board[0][iterator] != '-'))
                {
                    gameStatus = 0;
                }
            }

            if (
                (
                    (board[0][0] == board[1][1]) && (board[1][1] == board[2][2]) && board[0][0] != '-') ||
                ((board[0][2] == board[1][1]) && (board[1][1] == board[2][0]) && board[0][2] != '-'))
            {
                gameStatus = 0;
            }

            round++;
        }

        showBoard(board, (char *)&errorMessage);

        printf("\nOther player '%s' win!", nameCurrentPlayer);

        printf("\nEnd of the game!\n");
        printf("\nDo you want to start a game?");
        printf("\n1 - Yes");
        printf("\n2 - No");
        printf("\nChoose an option and press ENTER: ");

        scanf("%d", &optionRestart);

        switch (optionRestart)
        {
        case 1:
            gameStatus = 1;
            break;
        case 2:
            launch();
            break;
        }
    }
}

void launch()
{
    int choice = -1;

    clear_screen();

    signal(SIGINT, catch_ctrl_c_and_exit);

    while (choice < 1 || choice > 3)
    {
        printf("Welcome to Chinese chess playground!\n");
        printf("0 - Exit\n");
        printf("1 - Play locally\n");
        printf("2 - Play online\n");
        printf("3 - About us\n");
        printf("Choose an option and press ENTER: ");

        scanf("%d", &choice);

        switch (choice)
        {
        case 0:
            clear_screen();
            choice = 0;
            printf("Farewell ! Until next time!\n");
            exit(EXIT_SUCCESS);
            break;
        case 1:
            clear_screen();
            choice = 0;
            game();
            break;
        case 2:
            clear_screen();
            choice = 0;
            exit(conectGame(ip, port));
            break;
        case 3:
            clear_screen();
            choice = 0;
            printf("About the game!\n");
            break;
        default:
            clear_screen();
            choice = 0;
            printf("Invalid option!\n");
            continue;
            break;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Syntax error!\n-->Usage : ./client [SERVER_IP_ADDRESS] [PORT_NUMBER] (i.e. ./client 127.0.0.1 8888) !\n");
        exit(EXIT_FAILURE);
    }
    //
    port = atoi(argv[2]);
    strcpy(ip, argv[1]);
    // validate port number input
    if (port == 0)
    {
        perror("Invalid port number specified!");
        exit(EXIT_FAILURE);
    }
    launch();

    return 0;
}
