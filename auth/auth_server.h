void handleGuest(char name[], Client *cli, char buffer[]);
void handleLogin(int *isLogin, Client *cli, char buffer[]);
void handleReg(Client *cli, char buffer[]);
void handleLogOut(int *isLogin, Client *cli, char buffer[]);

/*
Handle request join as quest
@param name: handleGuest_name
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleGuest(char name[], Client *cli, char buffer[])
{
    char *p = strtok(buffer, "|");
    strcpy(name, strtok(NULL, "|"));
    strcpy(cli->player_info.username, name);
    bzero(buffer, BUFF_SIZE);
    strcpy(buffer, "MENU|");
    strcat(buffer, "ok");
    send_message(buffer, cli->uid);
}

/*
Handle login request
  @param isLogin : status login(1=yes)
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleLogin(int *isLogin, Client *cli, char buffer[])
{
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    if (*isLogin == 1)
    {
        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, " LGIN_ALR|");
        strcat(buffer, "Already login.Logged out first\n");
        // sprintf(buffer, "Already login.Logged out first\n");
        send_message(buffer, cli->uid);
    }
    else
    {
        int flag = 1;
        PlayerNode *n;
        char *p = strtok(buffer, "|");
        strcpy(username, strtok(NULL, "|"));
        printf("userName received :%s\n", username);
        strcpy(password, strtok(NULL, "|"));
        printf("password received :%s\n", password);

        pthread_mutex_lock(&auth_mutex);
        for (n = root_p; n != NULL; n = n->next)
        {
            if (strcmp(username, n->player.username) == 0 && strcmp(password, n->player.password) == 0)
            {
                if (n->player.is_online == 0)
                {
                    // TODO: them 1 ham *isLogin = status (0 la chua log)
                    *isLogin = 1;
                    n->player.is_online = 1;
                    cli->player_info.elo_rating = n->player.elo_rating;
                    // strcpy(cli->username, cli->player_info.username);
                    strcpy(cli->player_info.username, username);
                    // cli->player_info = (p->player);
                    flag = 0;
                    break;
                }
                else
                {
                    flag = -1;
                    break;
                }
            }
        }

        pthread_mutex_unlock(&auth_mutex);

        if (flag == 0)
        {
            traverse_list_p(root_p);
            bzero(buffer, BUFF_SIZE);
            strcpy(buffer, "LGIN_SUCC|");
            strcat(buffer, username);
            send_message(buffer, cli->uid); // login succes
        }
        else if (flag == -1)
        {

            bzero(buffer, BUFF_SIZE);
            strcpy(buffer, "LGIN_ALR|");
            strcat(buffer, "Already login\n");
            // sprintf(buffer, "Already login\n");
            send_message(buffer, cli->uid);
        }
        else
        {
            bzero(buffer, BUFF_SIZE);
            strcpy(buffer, "LGIN_FAIL|");
            strcat(buffer, "Login failure\n");
            // sprintf(buffer, "Login failure\n");
            send_message(buffer, cli->uid);
        }
    }
}

/*
Handle register request
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleReg(Client *cli, char buffer[])
{
    char user[100];
    char pass[100];

    char *p = strtok(buffer, "|");
    strcpy(user, strtok(NULL, "|"));
    printf("userName register received :%s\n", user);
    strcpy(pass, strtok(NULL, "|"));
    printf("password register received :%s\n", pass);
    encrypt(pass, KEY);
    int flag = 1;
    PlayerNode *n;

    pthread_mutex_lock(&reg_mutex);

    for (n = root_p; n != NULL; n = n->next)
    {
        if (strcmp(user, n->player.username) == 0)
        {
            flag = 0;
            break;
        }
    }

    pthread_mutex_unlock(&reg_mutex);

    if (flag == 0)
    {
        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, "REG_FAIL|");
        strcat(buffer, "Name exist.Register fail.\n");
        // sprintf(buffer, "Name exist.Register fail.\n");
        send_message(buffer, cli->uid);
    }
    else
    {
        Player player;
        strcpy(player.username, user);
        strcpy(player.password, pass);
        player.elo_rating = 1200;
        player.is_online = 0;
        char str[1000] = "";
        char tmp[50];

        strcat(str, user);
        strcat(str, " ");
        strcat(str, pass);
        strcat(str, " ");
        sprintf(tmp, "%d", player.elo_rating);
        strcat(str, tmp);
        strcat(str, " ");
        sprintf(tmp, "%d\n", player.is_online);
        strcat(str, tmp);
        append(str);

        insert_at_head_p(player);
        traverse_list_p(root_p);
        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, "REG_SUCC|");
        strcat(buffer, "Register Successful\n");
        // sprintf(buffer, "Register Successful\n");
        send_message(buffer, cli->uid);
    }
}
/*
Handle logout request
@param isLogin : status login(1=yes)
@param cli: client information
@param buffer: buffer_send
@return
 */
void handleLogOut(int *isLogin, Client *cli, char buffer[])
{
    if (*isLogin == 0)
    {
        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, "LOGOUT_FAIL|");
        strcat(buffer, "You need to login first\n");
        // sprintf(buffer, "Login First\n");
        send_message(buffer, cli->uid);
    }
    else
    {
        char *p = strtok(buffer, "|");
        char us[100];
        strcpy(us, strtok(NULL, "|"));

        *isLogin = 0;
        cli->player_info.elo_rating = 1200; /*cap nhat elo client*/
        // strcpy(cli->player_info.username, cli->username);
        PlayerNode *n;
        for (n = root_p; n != NULL; n = n->next)
        {
            if (strcmp(us, n->player.username) == 0)
            {
                n->player.is_online = 0;
                // displayNode2(n);
                save_data(n->player);
                // TODO : luu data vaofile khi logout
                // saveData("database.txt",n->player);
            }
        }
        traverse_list_p(root_p);
        bzero(buffer, BUFF_SIZE);
        strcpy(buffer, "LOGOUT_SUCC|");
        strcat(buffer, "Logout Successful\n");
        // sprintf(buffer, "Logout Successful\n");
        send_message(buffer, cli->uid);
    }
}