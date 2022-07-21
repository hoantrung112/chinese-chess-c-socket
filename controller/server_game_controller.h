void handleCreateRoom(int *isLogin, int *flag, Client *cli, char buffer[]);
void handleJoin(int *isLogin, int *number, Client *cli);
void handlePlay(int *number, Client *cli);
void handleListRooms(Client *cli);
void handleLeave(Client *cli);
void handleStart(Client *cli);

// sua lai cac ham theo pointer
void handleCreateRoom(int *isLogin, int *flag, Client *cli, char buffer[])
{

    char tmp[BUFF_SIZE];
    RoomNode *node = (RoomNode *)malloc(sizeof(RoomNode));

    if (strcmp(buffer, "CREATE RANK") == 0 && *isLogin == 0)
    {
        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, "ROOM_FAIL|");
        strcat(buffer, "You need to login\n");
        // sprintf(buffer, "You need to login\n");
        send_message(buffer, cli->uid);
    }
    else
    {
        *flag = 0;

        pthread_mutex_lock(&rooms_mutex);

        for (node = root_r; node != NULL; node = node->next)
        {
            if (node != NULL)
            {
                if (node->room.player1->uid == cli->uid)
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, "ROOM_ALR|");
                    strcat(buffer, "you are already in the room\n");
                    // sprintf(buffer, "you are already in the room\n");
                    send_message(buffer, cli->uid);
                    *flag = 1;
                    break;
                }

                if (node->room.player2 != 0)
                {
                    if (node->room.player2->uid == cli->uid)
                    {
                        bzero(buffer, BUFF_SIZE);
                        strcpy(buffer, "ROOM_ALR|");
                        strcat(buffer, "you are already in the room\n");
                        // sprintf(buffer, "you are already in the room\n");
                        send_message(buffer, cli->uid);
                        *flag = 1;
                        break;
                    }
                }
            }
        }

        pthread_mutex_unlock(&rooms_mutex);

        if (*flag != 1)
        {
            // clients settings
            Room *room = (Room *)malloc(sizeof(Room));
            room->player1 = cli;
            room->player2 = 0;
            room->uid = roomUid;
            printf("Roomid create:%d\n", room->uid);
            if (strcmp(buffer, "CREATE") == 0)
            {
                strcpy(room->room_type, "[NORMAL] ");
            }
            else
            {
                strcpy(room->room_type, "[RANK] ");
            }
            strcpy(room->state, room->room_type);
            strcat(room->state, "waiting for second player");

            // add room to queue
            // queue_add_room(room);

            insert_at_head_r(*room);
            // insertAfterCurrent(*room);
            traverse_list_r(root_r);

            bzero(buffer, BUFF_SIZE);
            bzero(tmp, BUFF_SIZE);
            strcpy(buffer, "ROOM_SUCC|");
            sprintf(tmp, "you created a new room number %i\n", roomUid);
            strcat(buffer, tmp);

            send_message(buffer, cli->uid);
            roomUid++;
        }
    }
}
void handleJoin(int *isLogin, int *number, Client *cli)
{
    char buffer[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    int researched = 0;
    int already = 0;
    pthread_mutex_lock(&rooms_mutex);
    RoomNode *p = (RoomNode *)malloc(sizeof(RoomNode));

    for (p = root_r; p != NULL; p = p->next)
    {
        if (p != NULL)
        {
            if (p->room.player1->uid == cli->uid)
            {

                // checkLogin
                if (strstr(p->room.state, "[RANK] ") && *isLogin == 0)
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, "JOIN_FAIL|");
                    strcat(buffer, "You need to login\n");
                    send_message(buffer, cli->uid);
                    break;
                }

                already = 1;

                bzero(buffer, BUFF_SIZE);
                bzero(tmp, BUFF_SIZE);
                strcpy(buffer, "JOIN_ALR|");
                sprintf(tmp, "you are already in the room number: %i\n", p->room.uid);
                strcat(buffer, tmp);

                send_message(buffer, cli->uid);
                break;
            }
        }
    }

    pthread_mutex_unlock(&rooms_mutex);

    if (already == 1)
    {
        // continue;
        return;
    }

    pthread_mutex_lock(&rooms_mutex);

    for (p = root_r; p != NULL; p = p->next)
    {
        if (p != NULL)
        {
            if (p->room.uid == *number)
            {

                researched = 1;

                if (p->room.player2 != 0)
                {

                    if (p->room.player2->uid == cli->uid)
                    {
                        bzero(buffer, BUFF_SIZE);

                        strcpy(buffer, "ROOM_ALR|");

                        strcat(buffer, "you are already in the room\n");
                        send_message(buffer, cli->uid);
                        break;
                    }

                    bzero(buffer, BUFF_SIZE);
                    bzero(tmp, BUFF_SIZE);
                    strcpy(buffer, "ROOM_FULL|");
                    sprintf(tmp, "room number: %i, is already full\n", p->room.uid);
                    strcat(buffer, tmp);
                    // sprintf(buffer, "room number: %i, is already full\n", rooms[i]->uid);
                    send_message(buffer, cli->uid);
                    break;
                }

                // rooms[i]->player2 = cli;
                p->room.player2 = cli;
                if (strstr(p->room.state, "[RANK] ") && *isLogin == 0)
                {
                    p->room.player2 = 0;
                    // rooms[i]->player2 = 0;
                    //  printf("Xin chao %d", *isLogin);
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, "JOIN_FAIL|");
                    strcat(buffer, "You need to login\n");
                    send_message(buffer, cli->uid);
                    break;
                }
                else
                {
                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting start");
                    bzero(buffer, BUFF_SIZE);
                    printf("%s enter the room number: %i\n", cli->player_info.username, *number);

                    bzero(tmp, BUFF_SIZE);
                    strcpy(buffer, "JOIN_SUCC|");
                    sprintf(tmp, "'%s' entered your room\n", cli->player_info.username);
                    strcat(buffer, tmp);
                    // sprintf(buffer, "'%s' entered your room\n", cli->player_info.username);
                    send_message(buffer, p->room.player1->uid);

                    bzero(buffer, BUFF_SIZE);
                    bzero(tmp, BUFF_SIZE);
                    strcpy(buffer, "JOIN_SUCC|");
                    sprintf(tmp, "you has entered the room number: %i\n", *number);
                    strcat(buffer, tmp);
                    // sprintf(buffer, "you has entered the room number: %i\n", number);
                    send_message(buffer, cli->uid);
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&rooms_mutex);

    if (researched == 0)
    {
        bzero(buffer, BUFF_SIZE);
        bzero(tmp, BUFF_SIZE);
        strcpy(buffer, "ROOM_NOT_FOUND|");
        sprintf(tmp, "could not find the room number %i\n", *number);
        strcat(buffer, tmp);
        // sprintf(buffer, "could not find the room number %i\n", number);
        send_message(buffer, cli->uid);
    }
}

void handleListRooms(Client *cli)
{
    char buffer[BUFF_SIZE];

    pthread_mutex_lock(&rooms_mutex);
    bzero(buffer, BUFF_SIZE);
    strcpy(buffer, "ROOM_LISTS|");
    RoomNode *p;

    if (root_r == NULL)
    {

        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, "NO_LISTS|No room available\n|");
        send_message(buffer, cli->uid);
    }
    else
    {
        char *list = (char *)malloc(BUFF_SIZE * sizeof(char));
        for (p = root_r; p != NULL; p = p->next)
        {
            if (p != NULL)
            {

                if (p->room.player2 != 0)
                {
                    sprintf(list, "%i)\n    room state: %s  \n    player1: %s - elo_rating: %d\n    player2: %s - elo_rating: %d\n", p->room.uid, p->room.state, p->room.player1->player_info.username, p->room.player1->player_info.elo_rating, p->room.player2->player_info.username, p->room.player2->player_info.elo_rating);
                }
                else
                { // remove by roomUid
                    sprintf(list, "%i)\n    room state: %s \n    player1: %s - elo_rating: %d\n", p->room.uid, p->room.state, p->room.player1->player_info.username, p->room.player1->player_info.elo_rating);
                }
                strcat(buffer, list);
            }
        }
        if (list != NULL)
        {

            send_message(buffer, cli->uid);
            free(list);
        }
    }

    pthread_mutex_unlock(&rooms_mutex);
}

void handleLeave(Client *cli)
{
    char buffer[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    int remove_room = 0;
    int room_number = 0;
    RoomNode *p = (RoomNode *)malloc(sizeof(RoomNode));

    pthread_mutex_lock(&rooms_mutex);

    for (p = root_r; p != NULL; p = p->next)
    {
        if (p != NULL)
        {
            if (p->room.player1->uid == cli->uid)
            {
                if (p->room.player2 != 0)
                {
                    // owner leave
                    bzero(buffer, BUFF_SIZE);
                    bzero(tmp, BUFF_SIZE);
                    strcpy(buffer, "LEAVE_ROOM|");
                    sprintf(tmp, "%s left the room, now you are the owner\n", p->room.player1->player_info.username);
                    strcat(buffer, tmp);
                    // sprintf(buffer, "%s left the room, now you are the owner\n", rooms[i]->player1->player_info.username);
                    send_message(buffer, p->room.player2->uid);

                    p->room.player1 = p->room.player2;
                    p->room.player2 = 0;
                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting for secound player");
                }
                else
                { // ko co ai

                    // TODO : xoa room 1 trc roi ms xoa dc room 2
                    remove_room = 1;
                    room_number = p->room.uid;
                }

                bzero(buffer, BUFF_SIZE);
                bzero(tmp, BUFF_SIZE);
                strcpy(buffer, "LEAVE_ROOM|");
                sprintf(tmp, "you left the room %i\n", p->room.uid);
                strcat(buffer, tmp);
                // sprintf(buffer, "you left the room %i\n", rooms[i]->uid);
                send_message(buffer, cli->uid);
                break;
            }
            else if (p->room.player2 == 0)
            {
                continue;
            }
            else if (p->room.player2->uid == cli->uid)
            {
                // other leave
                bzero(buffer, BUFF_SIZE);
                bzero(tmp, BUFF_SIZE);
                strcpy(buffer, "LEAVE_ROOM|");
                sprintf(tmp, "%s left the room\n", p->room.player2->player_info.username);
                strcat(buffer, tmp);

                // sprintf(buffer, "%s left the room\n", rooms[i]->player2->player_info.username);
                send_message(buffer, p->room.player1->uid);

                p->room.player2 = 0;
                strcpy(p->room.state, p->room.room_type);
                strcat(p->room.state, "waiting for secound player");

                bzero(buffer, BUFF_SIZE);
                bzero(tmp, BUFF_SIZE);
                strcpy(buffer, "LEAVE_ROOM|");
                sprintf(tmp, "you left the room %i\n", p->room.uid);
                strcat(buffer, tmp);
                // sprintf(buffer, "you left the room %i\n", rooms[i]->uid);
                send_message(buffer, cli->uid);
                break;

            } // else continue;
        }
    }

    pthread_mutex_unlock(&rooms_mutex);
    // printf("HEllo1235\n");
    if (remove_room == 1)
    {
        printf("Room delete:%d\n", room_number);
        // queue_remove_room(room_number);

        delete_at_pos_r(room_number);

        roomUid *= 2; // OR  roomUid --
    }
}

void handlePlay(int *number, Client *cli)
{
    char buffer[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    RoomNode *p = (RoomNode *)malloc(sizeof(RoomNode));

    pthread_mutex_lock(&rooms_mutex);

    for (p = root_r; p != NULL; p = p->next)
    {

        if (p != NULL)
        {
            if (p->room.player2 == 0)
            {
                continue;
            }
            if (p->room.player1->uid == cli->uid || p->room.player2->uid == cli->uid)
            {
                if (p->room.game->is_over == 0)
                {
                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting start");
                    bzero(buffer, BUFF_SIZE);
                    // check this
                    strcpy(buffer, H_GAME_OVER);
                    strcat(buffer, "|");
                    strcat(buffer, "game over\n");
                    send_message(buffer, cli->uid);
                    // break;
                    continue;
                }

                setbuf(stdin, 0);
                bzero(buffer, BUFF_SIZE);
                sprintf(buffer, "%i", *number);

                // int linhaJogada = posicoes[*number - 1][0];
                // int colunaJogada = posicoes[*number - 1][1];
                if (p->room.game->client_turn == p->room.player1->uid)
                {
                    send_message(buffer, p->room.player2->uid);
                    // p->room.game->board[linhaJogada][colunaJogada] = 'X';
                    p->room.game->client_turn = p->room.player2->uid;
                }
                else if (p->room.game->client_turn == p->room.player2->uid)
                {
                    send_message(buffer, p->room.player1->uid);
                    // p->room.game->board[linhaJogada][colunaJogada] = 'O';
                    p->room.game->client_turn = p->room.player1->uid;
                }

                for (int iterator = 0; iterator < 3; iterator++)
                {
                    // if (
                    //     (
                    //         (p->room.game->board[iterator][0] == p->room.game->board[iterator][1]) && (p->room.game->board[iterator][1] == p->room.game->board[iterator][2]) && p->room.game->board[iterator][0] != '-') ||
                    //     ((p->room.game->board[0][iterator] == p->room.game->board[1][iterator]) && (p->room.game->board[1][iterator] == p->room.game->board[2][iterator]) && p->room.game->board[0][iterator] != '-')
                    //     )
                    // {
                    //     p->room.game->is_over = 0;
                    // }
                }

                // if (
                //     (
                //         (p->room.game->board[0][0] == p->room.game->board[1][1]) && (p->room.game->board[1][1] == p->room.game->board[2][2]) && p->room.game->board[0][0] != '-') ||
                //     ((p->room.game->board[0][2] == p->room.game->board[1][1]) && (p->room.game->board[1][1] == p->room.game->board[2][0]) && p->room.game->board[0][2] != '-'))
                // {
                //     p->room.game->is_over = 0;
                // }

                sleep(1);
                char append[20] = "";  // New variable
                char append1[20] = ""; // New variable

                if (p->room.game->is_over == 0)
                {

                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting start");

                    if (p->room.game->client_turn == p->room.player1->uid)
                    {

                        bzero(buffer, BUFF_SIZE);
                        bzero(tmp, BUFF_SIZE);
                        // TODO : NG 2 WIN CAP NHAT elo_rating

                        strcpy(buffer, strtok(p->room.state, " "));
                        strcat(buffer, "|");
                        if (strstr(p->room.state, "[RANK]"))
                        {

                            EloRating(p->room.player1->player_info.elo_rating, p->room.player2->player_info.elo_rating, 30, 2);

                            sprintf(append, "%d", secondElo); // put the int into a string

                            strcat(append, "|");
                            sprintf(append1, "%d", firstElo);
                            strcat(append, append1);
                            strcat(append, "|");

                            p->room.player1->player_info.elo_rating = firstElo;
                            p->room.player2->player_info.elo_rating = secondElo;
                            save_data(update_player_data(p->room.player1->player_info.username, firstElo));
                            save_data(update_player_data(p->room.player2->player_info.username, secondElo));
                            traverse_list_p(root_p);
                        }

                        // send to player1
                        strcpy(tmp, buffer);
                        strcat(buffer, append);
                        strcat(buffer, "lose|");
                        printf("Player 1 lose:%s\n", buffer);

                        send_message(buffer, p->room.player1->uid);
                        // sau khi cap nhat elo_rating
                        //  pass name and elo_rating user

                        sleep(0.5);
                        strcat(tmp, append);
                        strcat(tmp, "win|");
                        send_message(tmp, p->room.player2->uid);
                        printf("Player 2 win:%s\n", tmp);
                    }
                    else if (p->room.game->client_turn == p->room.player2->uid)
                    {

                        // TODO : NG 1 WIN CAP NHAT elo_rating
                        bzero(buffer, BUFF_SIZE);
                        bzero(tmp, BUFF_SIZE);
                        strcpy(buffer, strtok(p->room.state, " "));
                        strcat(buffer, "|");
                        // save data to file
                        if (strstr(p->room.state, "[RANK]"))
                        {
                            EloRating(p->room.player1->player_info.elo_rating, p->room.player2->player_info.elo_rating, 30, 1);

                            sprintf(append, "%d", firstElo); // put the int into a string
                            strcat(append, "|");
                            sprintf(append1, "%d", secondElo);
                            strcat(append, append1);
                            strcat(append, "|");

                            p->room.player1->player_info.elo_rating = firstElo;
                            p->room.player2->player_info.elo_rating = secondElo;
                            save_data(update_player_data(p->room.player1->player_info.username, firstElo));
                            save_data(update_player_data(p->room.player2->player_info.username, secondElo));
                            traverse_list_p(root_p);
                        }
                        strcpy(tmp, buffer);
                        strcat(buffer, append);
                        strcat(buffer, "win|");
                        // strcat(buffer, strcat(append,"win2|"));
                        printf("Player 1 win:%s\n", buffer);

                        send_message(buffer, p->room.player1->uid);

                        sleep(0.5);
                        // send to 2
                        strcat(tmp, append);
                        strcat(tmp, "lose|");
                        send_message(tmp, p->room.player2->uid);
                        printf("Player 2 lose:%s\n", tmp);
                    }

                    bzero(buffer, BUFF_SIZE);
                    bzero(tmp, BUFF_SIZE);
                    // GAME ROUND =0
                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting start");
                    p->room.game->round = 0;
                    strcpy(buffer, H_GAME_OVER);
                    strcat(buffer, "|");
                    strcat(buffer, "ok");
                    send_message(buffer, p->room.player1->uid);

                    sleep(0.5);

                    send_message(buffer, p->room.player2->uid);
                    break;
                }
                p->room.game->round++;
                // TODO : DRAW PART
                if (p->room.game->round == 10)
                {
                    bzero(p->room.state, 100);
                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting start");
                    p->room.game->round = 1;

                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, strtok(p->room.state, " "));
                    strcat(buffer, "|");
                    // save data to file
                    if (strstr(p->room.state, "[RANK]"))
                    {

                        EloRating(p->room.player1->player_info.elo_rating, p->room.player2->player_info.elo_rating, 30, 1);

                        sprintf(append, "%d", secondElo); // put the int into a string
                        strcat(append, "|");
                        sprintf(append1, "%d", firstElo);
                        strcat(append, append1);
                        strcat(append, "|");

                        p->room.player1->player_info.elo_rating = firstElo;
                        p->room.player2->player_info.elo_rating = secondElo;
                        save_data(update_player_data(p->room.player1->player_info.username, firstElo));
                        save_data(update_player_data(p->room.player2->player_info.username, secondElo));
                        traverse_list_p(root_p);
                    }

                    strcat(buffer, strcat(append, "draw|"));
                    printf("%s\n", buffer);

                    send_message(buffer, p->room.player1->uid);

                    sleep(0.5);

                    send_message(buffer, p->room.player2->uid);

                    bzero(buffer, BUFF_SIZE);

                    strcpy(p->room.state, p->room.room_type);
                    strcat(p->room.state, "waiting start");
                    p->room.game->round = 0;
                    strcpy(buffer, H_GAME_OVER);
                    strcat(buffer, "|");
                    strcat(buffer, "ok");
                    send_message(buffer, p->room.player1->uid);

                    sleep(0.5);

                    send_message(buffer, p->room.player2->uid);
                    break;
                }
                if (p->room.game->client_turn == p->room.player1->uid)
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, H_PLAYER_TURN);
                    strcat(buffer, "|");

                    strcat(buffer, "1");
                    // sprintf(buffer, "vez1\n");
                    send_message(buffer, p->room.player1->uid);

                    sleep(0.5);

                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, H_PLAYER_TURN);
                    strcat(buffer, "|");
                    strcat(buffer, "2");
                    // sprintf(buffer, "vez2\n");
                    send_message(buffer, p->room.player2->uid);
                }
                else if (p->room.game->client_turn == p->room.player2->uid)
                {
                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, H_PLAYER_TURN);
                    strcat(buffer, "|");
                    strcat(buffer, "2");
                    // sprintf(buffer, "vez2\n");
                    send_message(buffer, p->room.player1->uid);

                    sleep(0.5);

                    bzero(buffer, BUFF_SIZE);
                    strcpy(buffer, H_PLAYER_TURN);
                    strcat(buffer, "|");
                    strcat(buffer, "1");
                    // sprintf(buffer, "vez1\n");
                    send_message(buffer, p->room.player2->uid);
                }
            }
            else
                continue;
            // break;
        }
    }

    pthread_mutex_unlock(&rooms_mutex);
}

void handleStart(Client *cli)
{
    char buffer[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    int startgame = 0;
    Room *room_game;

    RoomNode *p = (RoomNode *)malloc(sizeof(RoomNode));
    pthread_mutex_lock(&rooms_mutex);

    for (p = root_r; p != NULL; p = p->next)
    {

        if (p != NULL)
        {
            if (p->room.player1->uid == cli->uid)
            {
                if (p->room.player2 != 0)
                {
                    startgame = 1;
                    room_game = &(p->room);
                    break;
                }

                bzero(buffer, BUFF_SIZE);

                strcpy(buffer, H_ERR_START_GAME);
                strcat(buffer, "|");

                strcat(buffer, "2 players are required to start the game\n");
                // sprintf(buffer, "2 players are required to start the game\n");
                send_message(buffer, cli->uid);
                break;
            }
            else if (p->room.player2 == 0)
            {
                continue;
            }
            else if (p->room.player2->uid == cli->uid)
            {
                bzero(buffer, BUFF_SIZE);
                strcpy(buffer, H_ERR_START_GAME);
                strcat(buffer, "|");

                strcat(buffer, "only the owner of the room can start\n");
                // sprintf(buffer, "only the owner of the room can start\n");
                send_message(buffer, cli->uid);
                break;
            }
        }
    }

    pthread_mutex_unlock(&rooms_mutex);

    if (startgame == 1)
    {
        room_game->game = (Game *)malloc(sizeof(Game));
        // is_over 1: play 0: win/lost  -1:draw
        room_game->game->is_over = 1;
        room_game->game->round = 1;
        room_game->game->client_turn = room_game->player1->uid;
        strcpy(room_game->state, room_game->room_type);

        strcat(room_game->state, "playing now");

        for (int linha = 0; linha < 3; linha++)
        {
            for (int coluna = 0; coluna < 3; coluna++)
            {
                // room_game->game->board[linha][coluna] = '-';
            }
        }

        sleep(1);

        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, H_START_1);
        strcat(buffer, "|");
        strcat(buffer, "start game");

        send_message(buffer, room_game->player1->uid);

        sleep(0.1);

        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, H_START_2);
        strcat(buffer, "|");
        strcat(buffer, "start game1");
        send_message(buffer, room_game->player2->uid);

        sleep(1);

        // send name
        bzero(buffer, BUFF_SIZE);
        bzero(tmp, BUFF_SIZE);
        strcpy(buffer, H_LEAVE_ROOM);
        strcat(buffer, "|");
        sprintf(tmp, "%s\n", room_game->player2->player_info.username);
        strcat(buffer, tmp);

        send_message(buffer, room_game->player1->uid);

        sleep(0.1);

        bzero(buffer, BUFF_SIZE);
        bzero(tmp, BUFF_SIZE);
        strcpy(buffer, H_LEAVE_ROOM);
        strcat(buffer, "|");
        sprintf(tmp, "%s\n", room_game->player1->player_info.username);
        strcat(buffer, tmp);
        send_message(buffer, room_game->player2->uid);

        sleep(1);

        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, H_PLAYER_TURN);
        strcat(buffer, "|");

        strcat(buffer, "1");
        // sprintf(buffer, "vez1\n");
        send_message(buffer, room_game->player1->uid);

        sleep(0.2);

        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, H_PLAYER_TURN);
        strcat(buffer, "|");

        strcat(buffer, "2");
        // sprintf(buffer, "vez2\n");
        send_message(buffer, room_game->player2->uid);
    }
}
