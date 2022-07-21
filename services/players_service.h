// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>

PlayerNode *prev_p, *cur_p, *root_p = NULL;

PlayerNode *make_new_node_p(Player player)
{
    PlayerNode *newNode = (PlayerNode *)malloc(sizeof(PlayerNode));
    newNode->player = player;
    newNode->next = NULL;
    return newNode;
}

Player read_node_p()
{
    Player newPlayer;
    printf("Input your username:");
    scanf("%s", newPlayer.username);
    printf("Input your password (no more than 30 characters):");
    scanf("%s", newPlayer.password);
    // printf("Input your elo rating:");
    // scanf("%d", &tmp.elo_rating);
    return newPlayer;
}

void display_node_p(PlayerNode *p)
{
    if (p == NULL)
    {
        print_err("Trouble fetching pointer address");
        return;
    }
    Player tmp = p->player;
    printf("%-20s\t%-17s\t%-25d%-25d%-p\n", tmp.username, tmp.password, tmp.elo_rating, tmp.is_online, p->next);
}

void insert_at_head_p(Player p)
{
    PlayerNode *newNode = make_new_node_p(p);
    newNode->next = root_p;
    root_p = newNode;
    cur_p = root_p;
}

void insert_after_cur_p(Player p)
{
    PlayerNode *newNode = make_new_node_p(p);
    if (root_p == NULL)
    {
        root_p = newNode;
        cur_p = root_p;
    }
    else if (cur_p == NULL)
    {
        print_err("Failed to insert new player");
        return;
    }
    else
    {
        newNode->next = cur_p->next;
        cur_p->next = newNode;
        prev_p = cur_p;
        cur_p = cur_p->next;
    }
}

void insert_before_cur_p(Player p)
{
    PlayerNode *newNode = make_new_node_p(p);
    PlayerNode *tmp = root_p;

    while (tmp != NULL && tmp->next != cur_p && cur_p != NULL)
    {
        tmp = tmp->next;
        prev_p = tmp;
    }

    if (root_p == NULL)
    {
        root_p = newNode;
        cur_p = root_p;
        prev_p = NULL;
    }
    else
    {
        newNode->next = cur_p;
        if (cur_p == root_p)
        {
            root_p = newNode;
        }
        else
        {
            prev_p->next = newNode;
        }
        cur_p = newNode;
    }
}

void free_list_p(PlayerNode *root_p)
{
    PlayerNode *p = root_p;
    while (p != NULL)
    {
        root_p = root_p->next;
        free(p);
        p = root_p;
    }
}

void delete_first_node_p()
{
    PlayerNode *del = root_p;
    if (del == NULL)
    {
        return;
    }
    root_p = del->next;
    free(del);
    cur_p = root_p;
    prev_p = NULL;
}

void delete_cur_node_p()
{
    if (cur_p == NULL)
    {
        return;
    }
    if (cur_p == root_p)
    {
        delete_first_node_p();
    }
    else
    {
        prev_p->next = cur_p->next;
        free(cur_p);
        cur_p = prev_p->next;
    }
}

PlayerNode *reverse_list_p(PlayerNode *root_p)
{
    PlayerNode *cur_p, *prev_p;
    cur_p = prev_p = NULL;
    while (root_p != NULL)
    {
        cur_p = root_p;
        root_p = root_p->next;
        cur_p->next = prev_p;
        prev_p = cur_p;
    }
    return prev_p;
}

void delete_at_pos_p(int pos)
{
    cur_p = root_p;
    int i;
    for (i = 0; i < pos; i++)
    {
        if (cur_p->next != NULL)
        {
            prev_p = cur_p;
            cur_p = cur_p->next;
        }
        else
        {
            break;
        }
    }
    if (i < pos || pos < 0)
    {
        print_err("Failed to delete a player");
        return;
    }
    delete_cur_node_p();
}

void insert_at_pos_p(Player ele, int pos)
{
    int i;
    cur_p = root_p;
    if (pos <= 0)
    {
        insert_before_cur_p(ele);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur_p->next) != NULL)
        {
            cur_p = cur_p->next;
        }
        insert_after_cur_p(ele);
    }
}

void traverse_list_p(PlayerNode *root_p)
{
    PlayerNode *p;
    for (p = root_p; p != NULL; p = p->next)
    {
        display_node_p(p);
    }
}

Player update_player_data(char *name, int elo)
{
    PlayerNode *p;

    for (p = root_p; p != NULL; p = p->next)
    {
        if (strcmp(p->player.username, name) == 0)
        {
            p->player.elo_rating = elo;
            traverse_list_p(root_p);
            printf("Player %s has been updated successfully!\n", p->player.username);
            return p->player;
        }
    }
}

void import_text_file(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        print_err("Failed to connect to database");
        return;
    }
    Player tmp;
    int i = 0;
    free_list_p(root_p);
    while (fscanf(fp, "%s %s %d %d", tmp.username, tmp.password, &tmp.elo_rating, &tmp.is_online) != EOF)
    {
        insert_at_head_p(tmp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp);
}

int get_line_from_file(char *filename, char *name)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        print_err("Failed to retrieve data from database");
    }
    Player tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d %d", tmp.username, tmp.password, &tmp.elo_rating, &tmp.is_online) != EOF)
    {
        line = line + 1;
        if (strcmp(name, tmp.username) == 0)
        {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (found == 1)
    {
        printf("Found %s at line %d \n", name, line);
        return line;
    }
    else
    {
        printf("Found at line 0\n");
    }
    return 0;
}

void save_data(Player p)
{
    // update file path
    int line = get_line_from_file("../database/players.txt", p.username);
    FILE *fPtr;
    FILE *fTemp;
    char path[100];

    char buffer[1000];
    char newline[1000];
    int count;

    /* Remove extra new line character from stdin */
    fflush(stdin);

    printf("Replace '%d' line with: ", line);

    char tmp1[50];
    strcpy(newline, p.username);
    strcat(newline, " ");
    strcat(newline, p.password);
    strcat(newline, " ");
    sprintf(tmp1, "%d", p.elo_rating);
    strcat(newline, tmp1);
    strcat(newline, " ");
    sprintf(tmp1, "%d", p.is_online);
    strcat(newline, tmp1);
    strcat(newline, "\n");

    /*  Open all required files */
    fPtr = fopen("../database/players.txt", "r");
    fTemp = fopen("../database/replace.tmp", "w");

    /* fopen() return NULL if unable to open file in given mode. */
    if (fPtr == NULL || fTemp == NULL)
    {
        /* Unable to open file hence exit */
        printf("\nUnable to open file.\n");
        printf("Please check whether file exists and you have read/write privilege.\n");
        exit(EXIT_SUCCESS);
    }

    /*
     * Read line from source file and write to destination
     * file after replacing given line.
     */
    count = 0;
    while ((fgets(buffer, 1000, fPtr)) != NULL)
    {
        count++;

        /* If current line is line to replace */
        if (count == line)
        {
            fputs(newline, fTemp);
        }
        else
        {
            fputs(buffer, fTemp);
        }
    }

    /* Close all files to release resource */
    fclose(fPtr);
    fclose(fTemp);

    /* Delete original source file */
    remove("../database/players.txt");

    /* Rename temporary file as original file */
    rename("../database/replace.tmp", "players.txt");

    printf("\nSuccessfully replaced '%d' line with '%s'.", line, newline);
}

// skip line to update data
// return ip of player to challegne
/*
char * getNumberActive(char *yourName){
    PlayerNode *p;
    int count=0;
    for(p=root_p;p!=NULL;p=p->next){
      if(strcmp(p->player.username,yourName)!=0 && p->player.is_online==1){
          return p->player.ip;
      }
    }
    return NULL;
} */
void append(char *str)
{
    // append to last line
    FILE *fptr3;
    int i, n;

    char fname[20];

    printf("\n\n Append multiple lines at the end of a text file :\n");
    printf("------------------------------------------------------\n");
    printf(" Input the file name to be opened : ");

    // pthread_mutex_lock(&file_mutex);
    fptr3 = fopen("../database/players.txt", "a");
    printf(" Input the number of lines to be written : ");
    // scanf("%d", &n);
    printf(" The lines are : \n");
    fprintf(fptr3, "%s", str);
    fclose(fptr3);
    // pthread_mutex_unlock(&file_mutex);
}

void save_data_(char *filename, Player user)
{
    FILE *fp;
    // pthread_mutex_lock(&file_mutex);
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
    }
    Player tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d", tmp.username, tmp.password, &tmp.elo_rating) != EOF)
    {
        line = line + 1;
        if (strcmp(user.username, tmp.username) == 0)
        {
            // fprintf(fp, "%s %s %d", tmp.username, tmp.password, tmp.elo_rating);
            fprintf(fp, "tuan %s %d", tmp.password, tmp.elo_rating);
            break;
        }
    }
    fclose(fp);
    // pthread_mutex_lock(&file_mutex);
}