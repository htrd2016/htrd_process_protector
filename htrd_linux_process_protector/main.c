#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

char* replace(char src[], char srcChar, char desChar, int length)
{
    char *p = src;
    while(p<src+length)
    {
        if(*p == srcChar)
        {
             *p = desChar;
        }
        p++;
    }
    return src;
}

int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
    int fd;
    char buf[16];
  
    fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0)
    {
       printf("can't open %s: %m\n", filename);
       return -1;
    }
    if (lockfile(fd) == -1) 
    {
       if (errno == EACCES || errno == EAGAIN) 
       {
          printf("file: %s already locked", filename);
          close(fd);
          return 1;
       }
       printf("can't lock %s: %m\n", filename);
       return -1;
     }
     ftruncate(fd, 0);
     sprintf(buf, "%ld", (long)getpid());
     write(fd, buf, strlen(buf) + 1);
     return 0;
}

long get_file_last_modifyed_time(const char *file_name)
{
    struct stat buf;
    int result;
    result = stat(file_name, &buf);
    if (result != 0)
    {
       perror("get file stat error!!!\n");
       return -1;
    }
    else
    {
      return buf.st_mtime;
    }
    return -1;
}

long get_current_time()
{
    return time(0);
}

void stop_process(int signo) 
{
    printf("got a signal %d, about to stop!!!\n", signo);
    _exit(0);
}

int main(int argc, char **argv)
{
    char file_name[512] = {0};
    int timeout_to_restart = -1;
    
    long file_last_modifyed_time = -1;
    long current_time = -1;
    char stop_service_cmd[1024] = {0};
    char start_service_cmd[1024] = {0};

    char lock_file_name[512] = {"/tmp/"};
    if (argc<5)
    {
       printf("useage<path><file name to check><command to stop the service><command to start the service><timeout if file not modifyed(second) to restart service>\n");
       return -1;
    }

    signal(SIGINT, stop_process);
    strcpy(file_name, argv[1]);
    strcpy(stop_service_cmd, argv[2]);
    strcpy(start_service_cmd, argv[3]);
    timeout_to_restart = atoi(argv[4]);
    strcat(lock_file_name, stop_service_cmd);

    replace(lock_file_name, ' ', '-', strlen(lock_file_name));
    printf("%s\n", lock_file_name);

    printf("file name=%s,service name=%s, command to stop the service=%s,time out to restart=%d,lock file name=%s\n",
            file_name, stop_service_cmd, start_service_cmd, timeout_to_restart, lock_file_name);
   
    if (already_running(lock_file_name))
    {
        printf("process %s already exists or errors ,about to exit!!\n", lock_file_name);
        return -1;
    }

    while(1)
    {
       file_last_modifyed_time = get_file_last_modifyed_time(file_name);
       if(file_last_modifyed_time == -1)
       {
           printf("error get file %s stat failed!!!\n", file_name);
           perror("error get file stat failed!!!\n");
           sleep(30);
           continue;
       }
       current_time = get_current_time();
       if(current_time-file_last_modifyed_time>timeout_to_restart)
       {
           printf("to excute %s\n", stop_service_cmd);
           system(stop_service_cmd);
           sleep(1);
           printf("to excute %s\n", start_service_cmd);
           system(start_service_cmd);
           sleep(60);
       }
       else
       {
           printf("servre is ok(%s modifyed at %ld)!\n", file_name, file_last_modifyed_time);
       }
       
       sleep(1);
    }

    return 0;
}
