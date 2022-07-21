RoomNode *root_r = NULL, *prev_r, *cur_r;

RoomNode *make_new_node_r(Room room)
{
    RoomNode *new_room = (RoomNode *)malloc(sizeof(RoomNode));
    new_room->room = room;
    new_room->next = NULL;
    return new_room;
}

// RoomNode read_node_r()
// {
//     RoomNode tmp;
//     printf("Input name:");
//     scanf("%s", tmp.username);
//     printf("Input pass:");
//     scanf("%s", tmp.pass);
//     printf("Input elo:");
//     scanf("%d", &tmp.elo_rating);
//     return tmp;
// }

void display_node_r(RoomNode *p)
{
    if (p == NULL)
    {
        printf("Pointer problem\n");
        return;
    }
    Room tmp = p->room;
    printf("Game room:%d\n", tmp.uid);
    /*
    printf("Round:%d\nPlayer Turn:%d", tmp.game->round, tmp.game->client_turn);
    printf("Room\n");
    printf("Roomid:%d\nRoom type:%-20s\nState:%-100s\n", tmp.uid, tmp.roomType, tmp.state);
    printf("Player\n");*/
    printf("User 1:%s\nElo:%d\n", tmp.player1->player_info.username, tmp.player1->player_info.elo_rating);
    // printf("User 2:%s\nElo:%d", tmp.player2->player_info.username, tmp.player2->player_info.elo_rating);
}

void insert_at_head_r(Room room)
{
    pthread_mutex_lock(&rooms_mutex);
    RoomNode *new_room = make_new_node_r(room);
    new_room->next = root_r;
    root_r = new_room;
    cur_r = root_r;
    pthread_mutex_unlock(&rooms_mutex);
}

void insert_after_cur_r(Room room)
{
    RoomNode *new_room = make_new_node_r(room);
    if (root_r == NULL)
    {
        root_r = new_room;
        cur_r = root_r;
    }
    else if (cur_r == NULL)
    {
        printf("Current pointer is nULL.\n");
        return;
    }
    else
    {
        new_room->next = cur_r->next;
        cur_r->next = new_room;
        prev_r = cur_r;
        cur_r = cur_r->next;
    }
}

void insert_before_cur_r(Room e)
{
    RoomNode *new_room = make_new_node_r(e);
    RoomNode *tmp = root_r;

    while (tmp != NULL && tmp->next != cur_r && cur_r != NULL)
    {
        tmp = tmp->next;
        prev_r = tmp;
    }

    if (root_r == NULL)
    {
        root_r = new_room;
        cur_r = root_r;
        prev_r = NULL;
    }
    else
    {
        new_room->next = cur_r;
        if (cur_r == root_r)
        {
            root_r = new_room;
        }
        else
        {
            prev_r->next = new_room;
        }
        cur_r = new_room;
    }
}

void free_list_r(RoomNode *root_r)
{
    RoomNode *to_free = root_r;
    while (to_free != NULL)
    {
        root_r = root_r->next;
        free(to_free);
        to_free = root_r;
    }
}

void delete_first_node_r()
{
    RoomNode *del = root_r;
    if (del == NULL)
    {
        return;
    }
    root_r = del->next;
    free(del);
    cur_r = root_r;
    prev_r = NULL;
}

void delete_cur_node_r()
{
    if (cur_r == NULL)
        return;
    if (cur_r == root_r)
        delete_first_node_r();
    else
    {
        prev_r->next = cur_r->next;
        free(cur_r);
        cur_r = prev_r->next;
    }
}

RoomNode *reverse_list_r(RoomNode *root_r)
{
    RoomNode *cur_r, *prev_r;
    cur_r = prev_r = NULL;
    while (root_r != NULL)
    {
        cur_r = root_r;
        root_r = root_r->next;
        cur_r->next = prev_r;
        prev_r = cur_r;
    }
    return prev_r;
}

void delete_at_pos_r(int roomNumber)
{
    printf("ROOM:%d\n", roomNumber);
    int pos = -1;
    RoomNode *p = root_r;
    int index = 0;
    int found = 0;
    pthread_mutex_lock(&rooms_mutex);

    // traverse till then end of the linked list
    while (p != NULL)
    {
        pos++;
        if (p->room.uid == roomNumber)
        {
            found = 1;
            break;
        }
        p = p->next;
    }
    /*
    for(p=root_r;p!=NULL;p++){
        pos = pos + 1;
        if(p->room.uid == roomNumber){
            found = 1;
            break;
        }
    }*/
    if (found == 0)
    {
        pos = -1;
    }
    cur_r = root_r;
    int i;
    for (i = 0; i < pos; i++)
    {
        if (cur_r->next != NULL)
        {
            prev_r = cur_r;
            cur_r = cur_r->next;
        }
        else
            break;
    }
    if (i < pos || pos < 0)
    {
        printf("No node at postion.\n");
        return;
    }
    delete_cur_node_r();

    pthread_mutex_unlock(&rooms_mutex);
}

void insert_at_pos_r(Room ele, int pos)
{
    int i;
    cur_r = root_r;
    if (pos <= 0)
    {
        insert_before_cur_r(ele);
        return;
    }

    for (i = 0; i < pos; i++)
    {
        if ((cur_r->next) != NULL)
            cur_r = cur_r->next;
        insert_after_cur_r(ele);
    }
}

void traverse_list_r(RoomNode *root_r)
{
    RoomNode *p;
    for (p = root_r; p != NULL; p = p->next)
        display_node_r(p);
}
/*
void importTextFile(char *filename)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    Room tmp;
    int i = 0;
    free_list_r(root_r);
    while (fscanf(fp, "%s %s %d %s %d", tmp.username, tmp.pass, &tmp.elo_rating, tmp.ip, &tmp.status) != EOF)
    {
        insert_at_head_r(tmp);
        i++;
    }
    printf("Import %d record to linked list.\n", i);
    fclose(fp);
}

// Found the name at line
void getLine(char *filename, char *name)
{
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open.\n");
        return;
    }
    Room tmp;
    int i = 0;
    int line = 0;
    int found = 0;
    while (fscanf(fp, "%s %s %d %s %d", tmp.username, tmp.pass, &tmp.elo_rating, tmp.ip, &tmp.status) != EOF)
    {
        line = line + 1;
        if (strcmp(name, tmp.username) == 0)
        {
            found = 1;
            break;
        }
    }
    if (found == 1)
    {
        printf("Found %s at line %d \n", name, line);
    }
    else
        printf("Found at line 0");
    fclose(fp);
}   */
