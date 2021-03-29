#include "microshell.h"

char **envp;

int ft_strlen(char *str)
{
    int i;

    i = 0;
    if (str)
        while (str[i])
            i++;
    return(i);
}

char *ft_strdup(char *str)
{
    int i;
    char *newstr;

    i = -1;
    if (!str)
        return (NULL);
    if ((newstr = malloc(sizeof(char) * (ft_strlen(str) + 1))))
    {
        while (str[++i])
            newstr[i] = str[i];
        newstr[i] = 0;
    }
    return (newstr);
}

int not_cd(t_cmd **com, int pip)
{
    int i;
    pid_t pid;
    int ret;

    i = -1;
    while (com[++i])
    {
        if (pipe(com[i]->pipe) || (pid = fork()) < 0)
            return (0);
        else if (pid == 0)
        {
            if (i == 0)
            {
                if (dup2(com[i]->pipe[1], 1) < 0)
                    return (0);
            }
            else
            {
                if (i < pip - 1)
                    if (dup2(com[i]->pipe[1], 1) < 0)
                        return (0);
                if (dup2(com[i - 1]->pipe[0], 0) < 0)
                    return (0);
            }
            if ((ret = execve(com[i]->line[0], com[i]->line, envp)) < 0)
                write(2, "error: cannot execute ", 22) &&
                    write(2, com[i]->line[0], ft_strlen(com[i]->line[0])) &&
                        write(2, "\n", 1);
            exit(ret);
        }
        waitpid(pid, 0, 0);
        close(com[i]->pipe[1]);
        if (!com[i + 1])
            close(com[i]->pipe[0]);
        if (i > 0)
            close(com[i - 1]->pipe[0]);
    }
    return (1);
}

int to_exec(t_cmd **com, int pip)
{
    int ret;
    pid_t pid;

    if (!com)
        return (0);
    if (!strcmp("cd", com[0]->line[0]))
    {
        if (!com[0]->line[1] || com[0]->line[2] != NULL)
            return (write(2, "error: cd: bad arguments\n", 25));
        else if (chdir(com[0]->line[1]))
            return (write(2, "error: cd: cannot change directory to ", 38) &&
                write(2, com[0]->line[1], ft_strlen(com[0]->line[1])) &&
                    write(2, "\n", 1));
    }
    else if (pip == 1)
    {
        if ((pid = fork()) < 0)
            return (0);
        else if (pid == 0)
        {
            if ((ret = execve(com[0]->line[0], com[0]->line, envp)) < 0)
                write(2, "error: cannot execute ", 22) &&
                    write(2, com[0]->line[0], ft_strlen(com[0]->line[0])) &&
                        write(2, "\n", 1);
            exit (ret);
        }
        waitpid(pid, 0, 0);
    }
    else
        if (!not_cd(com, pip))
            return (0);
    return (1);
}

t_cmd *filllst(char **cmd, int word)
{
    int i;
    t_cmd *elem;

    i = -1;
    if (!(elem = malloc(sizeof(t_cmd))) || !(elem->line = malloc(sizeof(char*) * (word + 1))))
        return (NULL);
    while (++i < word)
        if (!(elem->line[i] = ft_strdup(cmd[i])))
            return (NULL);
    elem->line[i] = NULL;
    return (elem);
}

void comm_free(t_cmd **cmd)
{
    int i;
    int j;

    i = -1;
    while (cmd[++i])
    {
        j = -1;
        while(cmd[i]->line[++j])
            free(cmd[i]->line[j]);
        free(cmd[i]->line);
        free(cmd[i]);
    }
    free(cmd);
}

int newlst(char **cmd, int count, int n, int pip)
{
    int i;
    int j;
    int word;
    int start;
    t_cmd **com;

    i = -1;
    j = -1;
    start = 0;
    if (!strcmp(cmd[0], ";") || !strcmp(cmd[0], "|"))
        return (1);
    if (!(com = malloc(sizeof(t_cmd*) * (pip + 1))))
        return (0);
    while (n-- > 0)
    {
        word = 0;
        while (cmd[++i] && i < count && strcmp("|", cmd[i]))
            word++;
        if (!(com[++j] = filllst(&cmd[start], word)))
            return (0);
        start = i + 1;
    }
    com[++j] = NULL;
    if (!to_exec(com, pip))
        return (0);
    comm_free(com);
    return (1);
}

int commands(char **cmd, int i, int p)
{
    while (cmd[++i] && strcmp(";", cmd[i]))
        p += !strcmp("|", cmd[i]) ? 1 : 0;
    if (!newlst(cmd, i, p, p))
        return (0);
    return (!cmd[i] || !cmd[i + 1] ? 1 : commands(&cmd[i + 1], -1, 1));
}

int main(int argc, char **argv, char **env)
{
    envp = env;
    if (argc > 1)
        if (!commands(&argv[1], -1, 1))
            write(2, "error: fatal\n", 13);
    return (0);
}
