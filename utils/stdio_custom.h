void str_overwrite_stdout()
{
    printf("\r%s", "> ");
    fflush(stdout);
}

void handle_new_line_char(char *arr, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (arr[i] == '\n')
        {
            arr[i] = '\0';
            break;
        }
    }
}

void clear_screen()
{
    system("clear");
}
