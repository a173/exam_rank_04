#ifndef MICROSHELL_H
# define MICROSHELL_H
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct s_cmd
{
    char **line;
    int pipe[2];
}   t_cmd;


#endif