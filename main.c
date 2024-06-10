#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

char *read_line() 
{
  size_t size = 1;
  char *line = malloc(sizeof(char) * size);
  ssize_t status = getline(&line, &size, stdin);
  if (status == -1)
  {
    fprintf(stderr, "Error reading line\n");
  }
  for (size_t i = 0; i < size; i++)
  {
    if (line[i] == '\n' || line[i] == '\r')
    {
      line[i] = '\0';
      break;
    }
  }
  return line;
}

enum istat { invalid, valid };

enum istat validate(char *input)
{
  return valid;
}

void  handle_env(char *input)
{
  char *env_path = getenv("PATH");
  char *path = malloc(strlen(env_path) + 1);
  if (!path)
  {
    fprintf(stderr, "Memory allocation error\n");
    return;
  }

  strcpy(path, env_path);
  char *dirpath = strtok(path, ":");

  while (dirpath)
  {
    DIR *directory = opendir(dirpath);

    if (directory == NULL)
    {
      dirpath = strtok(NULL, ":");
      continue;
    }
    struct dirent *file;
    while ((file = readdir(directory)))
    {
      if (strcmp(file->d_name, input) == 0)
      {
        printf("%s is %s/%s\n", input, dirpath, file->d_name);
        closedir(directory);
        free(path);
        return;
      }
    }
    dirpath = strtok(NULL, ":");
    closedir(directory);
  }
  free(path);
  printf("%s is not found\n", input);
}

int builtin_check(char *input)
{
  if (!strncmp(input, "exit", 4) && strlen(input) == 4)
  {
    exit(0);
  }
  else if (!strncmp(input, "echo ", 5))
  {
    printf("%s\n", input + 5);
    return 1;
  }
  else if (!strncmp(input, "pwd", 3))
  {
    char pwd_cmd[1024];
    if (getcwd(pwd_cmd, sizeof(pwd_cmd)) != NULL)
      printf("%s\n", pwd_cmd);
    else
      perror("getcwd");
  }
  else if (!strncmp(input, "cd", 2) && (input[2] == ' ' || input[2] == '\0'))
  {
    char *dir = input + 2;
    while (*dir == ' ')
      dir++;
    if (strlen(dir) == 0)
      dir = getenv("HOME");
    if (dir[0] == '~')
    {
      char *home_dir = getenv("HOME");
      if (home_dir)
      {
        size_t home_len = strlen(home_dir);
        size_t dir_len = strlen(dir);
        char *new_dir = malloc(home_len + dir_len);
        if (new_dir)
        {
          strcpy(new_dir, home_dir);
          strcat(new_dir, dir + 1);
          dir = new_dir;
        }
      }
    }
    if (chdir(dir) != 0)
      perror("cd");
    else
    {
      char new_path[1024];
      if (getcwd(new_path, sizeof(new_path)) != NULL)
      {
        printf("Changed directory to %s\n", new_path);
      }
    }
    return 1;
  }
  else if (!strncmp(input, "type ", 5))
  {
    char *command = input + 5;
    if ((!strncmp(command, "exit", 4) && strlen(command) == 4) ||
          (!strncmp(command, "echo", 4) && strlen(command) == 4) ||
          (!strncmp(command, "type", 4) && strlen(command) == 4) ||
          (!strncmp(command, "pwd", 3) && strlen(command) == 3) ||
          (!strncmp(command, "cd", 2) && strlen(command) == 2))
    {
      printf("%s is a builtin\n", command);
    }
    else
        handle_env(command);
    return 1;
  }
  return 0;
}

void handle_input(char *input)
{
  if (builtin_check(input))
    return;
  else
  {
    enum istat status = validate(input);
    switch (status)
    {
      case valid:
        printf("command < %s > is valid\n", input);
        break;
      case invalid:
        fprintf(stderr, "%s: command not found\n", input);
        break;
      default:
        fprintf(stderr, "unknown status\n");
        break;
    }
  }
}

int main()
{
  while (1)
  {
    printf("$ ");
    fflush(stdout);
    char *input = read_line();
    handle_input(input);
    free(input);
  }
  return 0;
}
