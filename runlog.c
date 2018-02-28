#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

extern char **environ;

pid_t child_pid;
int child_status;

void handler(int sig) {
    if(sig != SIGCHLD) {
        return;
    }

    if(waitpid(child_pid,&child_status,WNOHANG) == child_pid) {
        if(WIFEXITED(child_status)) {
            exit(WEXITSTATUS(child_status));
        }
        else if(WIFSIGNALED(child_status)) {
            kill(0,WTERMSIG(child_status));
            sleep(5);
            exit(1);
        }
    }
}


pid_t bg(char *argv[], int fd) {
    pid_t child = fork();
    char *p = getenv("PATH");
    char *f = argv[0];
    int flen = strlen(f);
    char *q = NULL;
    char t[4096];

    if(child == -1) {
        fprintf(stderr,"Failed to fork: %s\n",strerror(errno));
        return child;
    }

    if(child != 0) {
        return child;
    }


    if(!p) {
        p = "/bin:/usr/bin:/usr/local/bin";
    }

    dup2(fd,fileno(stdout));
    dup2(fd,fileno(stderr));

    if(strchr(f,'/') ) {
        return execve(f,argv,environ);
    }
    else {
        while(p) {
            strcpy(t,p);
             q = strchr(t,':');
             if(q) {
               *q = 0;
             }
             if(strlen(t)) {
                 strcat(t,"/");
             }
             if( (strlen(t) + flen) > sizeof(t)) {
                 fprintf(stderr,"Path too long, bailing");
                 exit(1);
             }
             strcat(t,f);
             execve(t,argv,environ);
             if(errno != ENOENT) {
                 exit(1);
             }
             q = strchr(p,':');
             if(q) {
                 p = q + 1;
             }
             else {
                 p = 0;
             }
        }
        fprintf(stderr,"Unable to find binary\n");
        exit(1);
    }

    exit(0);

    return 0;
}

__attribute__ ((noreturn))
void usage(char *prog, int code) {
    fprintf(stderr,"Usage: %s [-s sleeptime] [-c sleepchar] [-t timeout] [-l /path/to/log] [-h] -- program\n",prog);
    exit(code);
}


int main(int argc, char *argv[]) {
    char *prog = argv[0];
    int c = 0, timeout = 60, sleeptime = 5, elapsed = 0;
    extern char *optarg;
    extern int optind, optopt;
    char file[4096] = "/tmp/runbg-XXXXXX";
    int fd = -1;
    int fflag = 0;
    struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
    int cols = 0;
    int sigsent = 0;
    char *tchar = ".";

    signal(SIGCHLD,handler);

    while ((c = getopt(argc,argv,":t:l:s:c:h")) != -1) {
        switch(c) {
              case ':':
                  fprintf(stderr,"Option -t requires a timeout\n");
                  usage(prog,1);
                  break;
              case 'c':
                  tchar = optarg;
                  break;
              case 's':
                  sleeptime = atoi(optarg);
                  if (sleeptime == 0) {
                      fprintf(stderr,"Bad sleeptime value, defaulting to 30 seconds\n");
                      sleeptime = 30;
                  }
                  break;
              case 'h':
                  usage(prog,0);
                  break;
              case 'l':
                  memset(file,0,sizeof(file));
                  strcpy(file,optarg);
                  fflag = 1;
                  break;
              case 't':
                  timeout = atoi(optarg);
                  if (timeout == 0) {
                      fprintf(stderr,"Bad timeout value, defaulting to 60 minutes\n");
                      timeout = 60;
                  }
                  break;
        }
    }
    
    argc -= optind;
    argv += optind;

    if(argc <= 0) {
        usage(prog,1);
    }

    if(fflag == 0) {
        fd = mkstemp(file); 
    }
    else {
        fd = open(file,O_RDWR | O_CREAT | O_EXCL, 0600);
    }

    if(fd == -1) {
        fprintf(stderr,"Failed to open temporary file\n");
        return 1;
    }

    fprintf(stdout,"%s\n",file);
    fflush(stdout);

    child_pid = bg(argv,fd);
    if(child_pid == -1) {
        return 1;
    }

    timeout = timeout * 60;

    while(1) {

        tv.tv_sec = sleeptime;
        select(0, NULL, NULL, NULL, &tv);
        elapsed += sleeptime;

        fprintf(stderr,"%s",tchar);
        cols++;
        if(cols == 70) {
            fprintf(stderr,"\n");
            cols = 0;
        }
        fflush(stderr);
        if(elapsed >= timeout) {
            if(sigsent == 0) {
                fprintf(stderr,"\nReached timeout, sending SIGTERM\n");
                fflush(stderr);
                kill(child_pid,SIGTERM);
                sigsent = 1;
            } else {
                fprintf(stderr,"\nReached timeout, sending SIGKILL\n");
                fflush(stderr);
                kill(child_pid,SIGKILL);
            }
        }
    }

    return 0;

}
