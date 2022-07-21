void guest(char username[], char buffer[]);
void sign_in(char username[], char pass[], char buffer[]);
void signUp(char username[], char pass[], char buffer[]);
void logOut(char name[], char buffer[]);

void guest(char username[], char buffer[])
{
    printf("Enter guest's name : ");
    scanf("%s", username);
    memset(buffer, '\0', (strlen(buffer) + 1));
    strcpy(buffer, H_GUEST_MODE);
    strcat(buffer, "|");
    strcat(buffer, username);
}

void sign_in(char username[], char pass[], char buffer[])
{
    printf("Enter userId : ");
    scanf("%s", username);
    printf("Enter password : ");
    scanf("%s", pass);

    encrypt(pass, KEY);
    memset(buffer, '\0', (strlen(buffer) + 1));
    strcpy(buffer, H_SIGN_IN);
    strcat(buffer, "|");
    strcat(buffer, username);
    strcat(buffer, "|");
    strcat(buffer, pass);
}

void sign_up(char username[], char pass[], char buffer[])
{
    printf("Enter userId : ");
    scanf("%s", username);
    printf("Enter password : ");
    scanf("%s", pass);
    memset(buffer, '\0', (strlen(buffer) + 1));
    strcpy(buffer, H_REGISTER);
    strcat(buffer, "|");
    strcat(buffer, username);
    strcat(buffer, "|");
    strcat(buffer, pass);
}

void logOut(char name[], char buffer[])
{
    strcpy(buffer, H_LOG_OUT);
    strcat(buffer, "|");
    strcat(buffer, name);
    strcat(buffer, "|");
}