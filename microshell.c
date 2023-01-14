#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define PRP "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define COLOR_RESET "\e[0m"

#define BUFFER_SIZE 256

#define HOW_DEEP 3

void prompt();
void giga();
void rm(int argc, char **arguments);
void cd(int argc, char **arguments, char *prev);
void cp(char *from, char *to);
int parce_string(char *input, char **arguments);
void help(int argc, char **argv);
void ls(int argc, char **arguments);
void echo(int argc, char **arguments);
void PrintTable(int argc, char **arguments, char *color);
void cat(int argc, char **arguments);
void clear();
void cat_write(int argc, char **arguments);
void find(int argc, char **arguments, int deep);
void find_task(struct dirent *folder, char **arguments);
void grep(int argc, char **arguments, int deep);
void grep_task(struct dirent *folder, char **arguments);

int main()
{
    char inp[BUFFER_SIZE];
    char prev[BUFFER_SIZE];
    char next[BUFFER_SIZE];
    getcwd(prev, BUFFER_SIZE);
    getcwd(next, BUFFER_SIZE);
    int history_count = -1;
    char history[BUFFER_SIZE][BUFFER_SIZE];
    while ((strcmp(inp, "exit") != 0))
    {
        prompt();
        history_count++;
        scanf(" %[^\n]s", inp);

        strcpy(history[history_count], inp);

        /*string parcing*/
        char **arguments = malloc(sizeof(char *) * BUFFER_SIZE);
        int count = parce_string(inp, arguments);

        /*forking*/
        pid_t id = fork();

        if (id == 0)
        {

            if (strcmp(arguments[0], "help") == 0)
            {
                help(count, arguments);
            }
            else if (strcmp(arguments[0], "ls") == 0)
            {
                if (strcmp(arguments[count - 1], "-l") == 0)
                {
                    if (count == 2)
                    {
                        arguments[2] = arguments[1];
                        arguments[1] = ".";
                    }
                }
                else
                {
                    if (count == 2)
                    {
                        arguments[2] = "bust";
                    }
                    else if (count == 1)
                    {
                        arguments[1] = ".";
                        arguments[2] = "bust";
                    }
                }
                ls(count, arguments);
            }
            else if (strcmp(arguments[0], "history") == 0)
            {
                int i = 0;
                for (; i <= history_count; i++)
                {
                    printf("%d: %s\n", i + 1, history[i]);
                }
            }
            else if (strcmp(arguments[0], "echo") == 0)
            {
                echo(count, arguments);
            }
            else if (strcmp(arguments[0], "clear") == 0)
            {
                clear();
            }
            else if (strcmp(arguments[0], "cat") == 0)
            {
                if (count == 2)
                {
                    cat(count, arguments);
                }
                else if ((count == 5) && (strcmp(arguments[1], ">") == 0) && (strcmp(arguments[3], "<<") == 0))
                {
                    cat_write(count, arguments);
                }
                else
                {
                    printf("\e[1;31mCommand failed: \e[0;37m Syntax error\n");
                }
            }
            else if (strcmp(arguments[0], "rm") == 0)
            {
                rm(count, arguments);
            }
            else if ((strcmp(arguments[0], "giga") == 0) || (strcmp(arguments[0], "sl") == 0))
            {
                giga();
            }
            else if (strcmp(arguments[0], "grep") == 0)
            {
                if (count == 2)
                {
                    arguments[2] = arguments[1];
                    arguments[1] = ".";
                }
                grep(count, arguments, HOW_DEEP);
            }
            else if (strcmp(arguments[0], "find") == 0)
            {
                if (count == 2)
                {
                    arguments[2] = arguments[1];
                    arguments[1] = ".";
                }
                find(count, arguments, HOW_DEEP);
            }
            else if ((strcmp(arguments[0], "exit") != 0) && (strcmp(arguments[0], "cd") != 0))
            {
                execvp(arguments[0], arguments);
                perror("\e[1;31mCommand failed: \e[0;37m");
            }

            return 0;
        }
        else
        {

            wait(NULL);
            if (strcmp(arguments[0], "cd") == 0)
            {
                if (count < 3)
                {
                    cd(count, arguments, prev);
                    strcpy(prev, next);
                    getcwd(next, BUFFER_SIZE);
                }
                else
                {
                    printf("%sError:%s too many arguments\n", RED, COLOR_RESET);
                }
            }
        }
    }
    printf("\n\e[1;32mEXIT SUCCESSFUL%s\n\n", COLOR_RESET);
    return 0;
}

void prompt()
{

    /*USER*/
    printf("\e[0;35m%s@", getenv("USER"));
    printf("\e[1;34m@");

    /*HOST*/
    char host[20];
    gethostname(host, 19 * sizeof(char));
    printf("\e[1;35m%s%s~", host, COLOR_RESET);

    /*current working dir*/
    char s[100];
    printf("%s$ ", getcwd(s, 100));
}

int parce_string(char *input, char **arguments)
{
    int i = 0;
    int count = 0;
    char *word = strtok(input, " ");
    arguments[0] = word;
    i = 1;
    while (word != NULL)
    {
        word = strtok(NULL, " ");
        arguments[i] = word;
        i++;
    }

    arguments[i] = NULL;
    free(word);
    i = 0;
    while (arguments[i] != NULL)
    {
        count++;
        i++;
    }

    return count;
}

void cp(char *from, char *to)
{
    char buffer[BUFFER_SIZE];
    int oryginal, bites, copy;

    oryginal = open(from, O_RDONLY | O_CREAT, 0644);

    copy = open(to, O_WRONLY | O_CREAT, 0644);

    while ((bites = read(oryginal, &buffer, BUFFER_SIZE)) > 0)
    {
        write(copy, &buffer, bites);
    }

    close(oryginal);
    close(copy);
}

void help(int argc, char **argv)
{

    if (argc == 1)
    {
        printf("\nMade by Krzysztof Gawron\n");
        printf("SO MICROSHELL\n\n");
        printf("Dostępne komendy wlasnej implementacji: \n");
        printf("- cp [plik oryginalny] [nazwa kopii]\n");
        printf("- exit\n");
        printf("- cd [folder]\n");
        printf("- echo [opcjonalny kolor] [tekst]\n");
        printf("- ls [opcjonalny folder, podstawowo roboczy] [dostepne flagi: -l]\n");
        printf("- find [opcjonalny folder] [plik]\n");
        printf("- rm [plik]\n");
        printf("- cat [plik] | cat > [nowy_plik] << [komenda zakonczenia wpisywnia]\n");
        printf("- help [nazwa komendy wlasnej implementacji]\n");
        printf("- grep [opcjonalna lokalizacja, domyslnie katalog roboczy i w dol] [slowo szukane]\n");
        printf("- clear\n");
        printf("- history\n");
        printf("- giga (uzycie na wlasna odpowiedzialnosc)(uzywac tylko gdy terminal jest wielkosci calego ekranu)\n");
        printf("\n");
    }
    else
    {
        if (strcmp(argv[1], "cp") == 0)
        {
            printf("\nCp kopiuje plik -> cp [plik kopiowany] [nazwa skopiowanego pliku]\n\n");
        }
        else if (strcmp(argv[1], "exit") == 0)
        {
            printf("\nWychodzi z powłoki\n\n");
        }
        else if (strcmp(argv[1], "history") == 0)
        {
            printf("\nWypisuje uzyte wczesniej komendy\n\n");
        }
        else if (strcmp(argv[1], "giga") == 0)
        {
            printf("\nBBB)\n\n");
        }
        else if (strcmp(argv[1], "ls") == 0)
        {
            printf("\nWypisuje zawartość katalogu, przy braku dopisanej ścieżki katalogu wypisuje zawartość katalogu roboczego");
            printf("\nFlaga -l wypisuje szczegoly w liscie\n\n");
        }
        else if (strcmp(argv[1], "help") == 0)
        {
            printf("\nWypisuje liste dostępnych komend\n\n");
        }
        else if (strcmp(argv[1], "cd") == 0)
        {
            printf("\nZmienia bierzący katalog, myslnik (-) cofa do poprzedniego katalogu: cd -\n\n");
        }
        else if (strcmp(argv[1], "echo") == 0)
        {
            printf("\nWypisuje na ekranie wpisany tekst, dostepne kolory:\n");
            printf("%s - czerwony (-red)\n%s - niebieski (-blue)\n%s - zielony (-green)\n%s - zolty (-yellow)\n", RED, BLU, GRN, YEL);
            printf("%s - fioletowy (-purple)\n%s - cyjan (-cyan)\n%s - czarny (-black)\n%s - bialy (-nie wpisuj koloru)%s\n\n",PRP, CYN, BLK, WHT, COLOR_RESET);
        }
        else if (strcmp(argv[1], "cat") == 0)
        {
            printf("\nWypisuje zawartosc pliku przy syntaxie (cat [nazwa pliku])\nlub pozwala zapisywac wpisywany w konsole tekskt przy uzyciu (cat > [nowy_plik] << [komenda zakonczenia wpisywania])\n\n");
        }
        else if (strcmp(argv[1], "rm") == 0)
        {
            printf("\nUsuwa podany plik (rm [plik])\n\n");
        }
        else if (strcmp(argv[1], "clear") == 0)
        {
            printf("\nCzysci terminal\n\n");
        }
        else if (strcmp(argv[1], "grep") == 0)
        {
            printf("\nWyszukuje folderze i podfolderach z glebokoscia %d plikow zawierajacych podane slowo i wypisuje ile razy sie ono powtarza\n\n", HOW_DEEP);
        }
        else if (strcmp(argv[1], "find") == 0)
        {
            printf("\nSzuka pliku w podanym folderze oraz w kazdym folderze znalezionym, max glebokosc to %d\n\n", HOW_DEEP);
        }
        else
        {
            printf("\nNie ma takiego polecenia we wlasnej implementacji\n\n");
        }
    }
}

void cd(int argc, char **arguments, char *prev)
{
    if (argc == 1)
    {
        if (chdir("/home") != 0)
            perror("\e[1;31mFailed: \e[0;37m");
    }
    else if (strcmp(arguments[1], "-") == 0)
    {
        if (chdir(prev) != 0)
            perror("\e[1;31mFailed: \e[0;37m");
    }
    else if (chdir(arguments[1]) != 0)
    {
        perror("\e[1;31mFailed: \e[0;37m");
    }
}

void echo(int argc, char **arguments)
{
    if (strcmp(arguments[1], "-red") == 0)
    {
        PrintTable(argc, arguments, RED);
    }
    else if (strcmp(arguments[1], "-black") == 0)
    {
        PrintTable(argc, arguments, BLK);
    }
    else if (strcmp(arguments[1], "-green") == 0)
    {
        PrintTable(argc, arguments, GRN);
    }
    else if (strcmp(arguments[1], "-yellow") == 0)
    {
        PrintTable(argc, arguments, YEL);
    }
    else if (strcmp(arguments[1], "-blue") == 0)
    {
        PrintTable(argc, arguments, BLU);
    }
    else if (strcmp(arguments[1], "-purple") == 0)
    {
        PrintTable(argc, arguments, PRP);
    }
    else if (strcmp(arguments[1], "-cyan") == 0)
    {
        PrintTable(argc, arguments, CYN);
    }
    else
    {
        PrintTable(argc, arguments, WHT);
    }

    return;
}

void PrintTable(int argc, char **arguments, char *color)
{
    int i = 2;
    if (strcmp(color, "\e[0;37m") == 0)
    {
        i--;
    }
    for (; i < argc; i++)
    {
        printf("%s%s ", color, arguments[i]);
    }
    printf("\n%s", COLOR_RESET);
}

void cat_write(int argc, char **arguments)
{
    char buffer[BUFFER_SIZE];
    int new, bites;
    char nl[] = "\n";

    new = open(arguments[2], O_WRONLY | O_CREAT, 0644);

    scanf(" %[^\n]s", buffer);

    while ((strcmp(buffer, arguments[4])) != 0)
    {
        bites = strlen(buffer) * sizeof(char);
        write(new, &buffer, bites);
        write(new, &nl, 1);
        scanf(" %[^\n]s", buffer);
    }

    close(new);
}

void cat(int argc, char **arguments)
{
    char buffer[BUFFER_SIZE];
    int oryginal, bites;

    oryginal = open(arguments[1], O_RDONLY);
    if (oryginal == -1)
    {
        perror("\e[1;31mCommand failed: \e[0;37m");
        return;
    }
    while ((bites = read(oryginal, &buffer, BUFFER_SIZE)) > 0)
    {
        write(STDOUT_FILENO, &buffer, bites);
    }

    close(oryginal);
    printf("\n");

    return;
}

void rm(int argc, char **arguments)
{
    if (remove(arguments[1]) != 0)
    {
        perror("\e[1;31mCommand failed: \e[0;37m");
    }
    return;
}

void find_task(struct dirent *folder, char **arguments)
{
    if (strcmp(folder->d_name, arguments[2]) == 0)
    {
        char s[100];
        getcwd(s, 100);
        printf("%s/%s\n", s, folder->d_name);
    }
}

void grep_task(struct dirent *folder, char **arguments)
{
    char buffer[BUFFER_SIZE];
    int oryginal, bites;

    oryginal = open(folder->d_name, O_RDONLY);
    if (oryginal == -1)
    {
        return;
    }
    int counter = 0;
    char *cur_itr = buffer;
    while ((bites = read(oryginal, &buffer, BUFFER_SIZE)) > 0)
    {
        char *occurance = strstr(cur_itr, arguments[2]);
        while (occurance)
        {
            counter++;
            cur_itr = occurance + 1;
            occurance = strstr(cur_itr, arguments[2]);
        }
    }
    if (counter != 0)
    {
        char s[100];
        getcwd(s, 100);
        printf("%s/%s : %d\n", s, folder->d_name, counter);
    }
    close(oryginal);
    return;
}

void rek_check(int argc, char **arguments, int deep,
               void (*task)(struct dirent *, char **))
{
    if (deep == 0)
    {
        return;
    }
    DIR *dir;
    struct dirent *folder;
    chdir(arguments[1]);
    char **new_args = malloc(sizeof(char *) * 4);
    new_args[0] = ":D";
    new_args[1] = "";
    new_args[2] = arguments[2];
    if (((dir = opendir(".")) == NULL) && (strcmp(arguments[0], ":D")))
    {
        perror("\e[1;31mError\e[0;37m");
        chdir("..");
        return;
    }
    else if ((dir == NULL) && (strcmp(arguments[0], ":D") == 0))
    {
        chdir("..");
        return;
    }
    else
    {
        while ((folder = readdir(dir)) != NULL)
        {
            if ((folder->d_type == 4) && (strcmp(".", folder->d_name)) && (strcmp("..", folder->d_name)))
            {
                new_args[1] = folder->d_name;
                rek_check(argc, new_args, deep - 1, task);
            }
            else
            {
                if (!(!strcmp(folder->d_name, ".") || !strcmp(folder->d_name, "..")))
                    task(folder, arguments);
            }
        }
    }
    closedir(dir);
    chdir("..");
    return;
}
void grep(int argc, char **arguments, int deep)
{
    rek_check(argc, arguments, deep, grep_task);
}
void find(int argc, char **arguments, int deep)
{
    rek_check(argc, arguments, deep, find_task);
}

void PrintPerms(struct dirent *folder)
{
    struct stat buffer;
    if (folder->d_type == 4)
    {
        printf("d");
    }
    else
    {
        printf("-");
    }
    if (!access(folder->d_name, R_OK) && (folder->d_type != 4))
    {
        printf("r");
    }
    else
    {
        printf("-");
    }
    if (!access(folder->d_name, W_OK) && (folder->d_type != 4))
    {
        printf("w");
    }
    else
    {
        printf("-");
    }
    if (!access(folder->d_name, X_OK) && (folder->d_type != 4))
    {
        printf("x   ");
    }
    else
    {
        printf("-   ");
    }
    stat(folder->d_name, &buffer);
    struct passwd *pw = getpwuid(buffer.st_uid);
    struct group *gr = getgrgid(buffer.st_gid);
    time_t time = buffer.st_mtime;
    struct tm ts;
    char buffer_time[80];
    ts = *localtime(&time);
    strftime(buffer_time, sizeof(buffer_time), "%Y  %b:%d  %H:%M:%S", &ts);
    printf("%s  %s  %s  ", pw->pw_name, gr->gr_name, buffer_time);
}

void ls(int argc, char **arguments)
{
    DIR *dir;
    struct dirent *folder;
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    int i = 0;
    int enter = 0;
    int tabu = 25;
    int limit = w.ws_col / tabu;
    char lista[2] = "";
    if (strcmp(arguments[2], "-l") == 0)
    {
        strcpy(lista, "\n");
    }
    if ((dir = opendir(arguments[1])) == NULL)
        perror("\e[1;31mError\e[0;37m");
    else
    {
        while ((folder = readdir(dir)) != NULL)
        {
            if (strcmp(arguments[2], "-l") == 0)
            {
                PrintPerms(folder);
            }
            if (folder->d_type == 4)
            {
                printf("\e[0;92m%s%s%s", folder->d_name, COLOR_RESET, lista);
            }
            else if (!access(folder->d_name, X_OK))
            {
                printf("%s%s%s%s", PRP, folder->d_name, COLOR_RESET, lista);
            }
            else
            {
                printf("%s%s%s", COLOR_RESET, folder->d_name, lista);
            }
            if (!strcmp(arguments[2], "-l"))
            {
                continue;
            }
            i = 0;
            while (strlen(folder->d_name) > tabu)
            {
                tabu += 4;
            }
            for (; i < (tabu - strlen(folder->d_name)); i++)
            {
                printf(" ");
            }
            enter++;
            if (enter == limit)
            {
                printf("\n");
                enter = 0;
            }
        }
        closedir(dir);
    }
    printf("%s\n", COLOR_RESET);
    return;
}

void clear()
{
    printf("\e[1;1H\e[2J");
}

void giga()
{
    struct winsize w;
    char pic[28][152] = {
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣤⣤⣶⣤⣤⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣾⣿⣿⣿⣿⣿⡿⠋⠉⠛⠛⠛⠿⣿⠿⠿⢿⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀⡀⢀⣽⣷⣆⡀⠙⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⣿⣿⣿⣿⣷⠶⠋⠀⠀⣠⣤⣤⣉⣉⣿⠙⣿⠀⢸⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⠁⠀⠀⠴⡟⣻⣿⣿⣿⣿⣿⣶⣿⣦⡀⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢨⠟⡿⠻⣿⠃⠀⠀⠀⠻⢿⣿⣿⣿⣿⣿⠏⢹⣿⣿⣿⢿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣼⣷⡶⣿⣄⠀⠀⠀⠀⠀⢉⣿⣿⣿⡿⠀⠸⣿⣿⡿⣷⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡿⣦⢀⣿⣿⣄⡀⣀⣰⠾⠛⣻⣿⣿⣟⣲⡀⢸⡿⡟⠹⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠞⣾⣿⡛⣿⣿⣿⣿⣰⣾⣿⣿⣿⣿⣿⣿⣿⣿⡇⢰⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠀⣿⡽⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢿⠿⣍⣿⣧⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣷⣿⣿⣿⣿⣿⣿⣿⣿⣷⣮⣽⣿⣷⣙⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⣹⡿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡧⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡆⠀⠀⠀⠀⠀⠀⠀⠉⠻⣿⣿⣾⣿⣿⣿⣿⣿⣿⡶⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⣀⣠⣤⡴⠞⠛⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠚⣿⣿⣿⠿⣿⣿⠿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⢀⣠⣤⠶⠚⠉⠉⠀⢀⡴⠂⠀⠀⠀⠀⠀⠀⠀⠀⢠⠀⠀⢀⣿⣿⠁⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠞⠋⠁⠀⠀⠀⠀⣠⣴⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⠀⠀⣾⣿⠋⠀⢠⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⡀⠀⠀⢀⣷⣶⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣆⣼⣿⠁⢠⠃⠈⠓⠦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⣿⣿⡛⠛⠿⠿⠿⠿⠿⢷⣦⣤⣤⣤⣦⣄⣀⣀⠀⢀⣿⣿⠻⣿⣰⠻⠀⠸⣧⡀⠀⠉⠳⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠛⢿⣿⣆⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠙⠛⠿⣦⣼⡏⢻⣿⣿⠇⠀⠁⠀⠻⣿⠙⣶⣄⠈⠳⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠈⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⣐⠀⠀⠀⠈⠳⡘⣿⡟⣀⡠⠿⠶⠒⠟⠓⠀⠹⡄⢴⣬⣍⣑⠢⢤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢀⣀⠐⠲⠤⠁⢘⣠⣿⣷⣦⠀⠀⠀⠀⠀⠀⠙⢿⣿⣏⠉⠉⠂⠉⠉⠓⠒⠦⣄⡀⠀⠀⠀\n",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠀⠀⠀⠀⠈⣿⣿⣷⣯⠀⠀⠀⠀⠀⠀⠀⠀⠉⠻⢦⣷⡀⠀⠀⠀⠀⠀⠀⠉⠲⣄⠀\n",
        "⠠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⢦⠀⢹⣿⣏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢻⣷⣄⠀⠀⠀⠀⠀⠀⠈⠳\n",
        "⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⣸⣿⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣽⡟⢶⣄⠀⠀⠀⠀⠀\n",
        "⠯⠀⠀⠀⠒⠀⠀⠀⠀⠀⠐⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⡄⠈⠳⠀⠀⠀⠀\n",
        "⠀⠀⢀⣀⣀⡀⣼⣤⡟⣬⣿⣷⣤⣀⣄⣀⡀⠀⠀⠀⠀⠀⠀⠈⣿⣿⡄⣉⡀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⣿⣿⣄⠀⣀⣀⡀ \n"};
    ioctl(0, TIOCGWINSZ, &w);
    int x = w.ws_col;
    int y = 10;
    int i = 2;
    int col = 0;
    int row = 0;

    for (; i < 152; i = i + 3)
    {
        clear();
        y = 10;
        row = 0;
        for (; row < 28; row++)
        {
            col = 0;
            printf("\033[%d;%dH", y, x);
            for (; col <= i; col++)
            {
                printf("%c", pic[row][col]);
            }
            printf("\n");
            y++;
        }
        x--;
        usleep(70000);
    }
    while (strlen(pic[24]) >= 2)
    {
        clear();
        y = 10;
        i = 0;
        for (; i < 28; i++)
        {
            printf("\033[%d;%dH%s", y, x, pic[i]);
            y++;
        }
        if (x > 0)
        {
            x--;
        }
        else
        {
            i = 0;
            for (; i < 28; i++)
            {
                memmove(pic[i], pic[i] + 3, strlen(pic[i]));
            }
        }
            usleep(70000);
    }
    clear();
}