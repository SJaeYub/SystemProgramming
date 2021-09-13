#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_LEN 1024

typedef struct list {
    char *name;
    char *value;
    struct list *prev;
    struct list *next;
}list_t;

list_t *head;
list_t *tail;

void history_printout();
void history_init();
void history_insert(char* cmd);
int join(char* com1[], char *com2[]);
void fatal(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int status;
    int runningStatus = 1;
    int fd;
    int work_num = 0;
    char* input_msg;
    char* input_msg_dum[MAX_LEN];
    char* cmd_tmp[MAX_LEN] = {0};
    char* cmd_all[MAX_LEN] = {0};
    char* cmd_msg[MAX_LEN] = {0};

    history_init();

    while(runningStatus) {
    
        printf("small_shell> ");
        fflush(stdout);
        int redirection_out = 0;
        int redirection_in = 0;
        int background = 0;
        int pipe_flag = 0;
        int fds[2];
        int pipe_tmp;
        int subpid;
        int cmd_len;
        int direction_check[3] = {0};
        char* cmd_msg2[MAX_LEN];
        
        work_num = 1;
  
        input_msg = (char*)malloc(MAX_LEN * sizeof(char));
        fgets(input_msg, MAX_LEN, stdin);
        strcpy(cmd_all, input_msg);

        if(strchr(input_msg, '|') != NULL) {
            int z = 0, pipe_pos = 0, x = 0, c = 0, ret = 0;
            char *cmd_tmp_dum[MAX_LEN];
            char *one[MAX_LEN] = {0,};
            char *two[MAX_LEN] = {0,};
            strcpy(input_msg_dum, input_msg);
            history_insert(input_msg_dum);
            cmd_tmp_dum[0] = strtok(input_msg_dum, " \n");
            while(cmd_tmp_dum[z] != NULL) {
                z++;
                cmd_tmp_dum[z] = strtok(NULL, " \n");
            }

            for(int i = 0; cmd_tmp_dum[i] != NULL; i++) {
                if(strcmp(cmd_tmp_dum[i], "|") == 0) {
                    pipe_pos = i;
                    break;
                }
            }

            for(x = 0; x < pipe_pos; x++) {
                one[x] = cmd_tmp_dum[x];
            }
            one[x + 1] = NULL;

            for(c = 0; cmd_tmp_dum[c] != NULL; c++) {
                two[c] = cmd_tmp_dum[c + pipe_pos + 1];
            }
            two[c + 1] = NULL;

            ret = join(one, two);
        }

        else if(strchr(input_msg, '|') == NULL) {

        cmd_tmp[0] = strtok(input_msg, "();\n");
        int i = 0;
        while(cmd_tmp[i] != NULL) {
            i++;
            cmd_tmp[i] = strtok(NULL, "();\n");
        }    
      
        if(strchr(cmd_tmp[i - 1], '&') != 0){

            background = 1;
        }

        for(int k = 0; k < i; k++) {
            char* cmd_all2[MAX_LEN];
            strcpy(cmd_all2, cmd_tmp[k]);
            int b = 0;
            while(cmd_all2[b] != NULL) {
                b++;
            }
            cmd_all2[b] = NULL;
            cmd_msg[0] = strtok(cmd_tmp[k], " \n");
            int j = 0;
        
            while(cmd_msg[j] != NULL) {
                j++;
                cmd_msg[j] = strtok(NULL, " \n");
            }

            cmd_len = j;
       
            if(strcmp(cmd_msg[0], "history") == 0) {
                history_printout();
            }
        
            if(k == 0) {
                if(strcmp(cmd_msg[0], "history") != 0) {
                    history_insert(cmd_all);
                }
            }

            if(strcmp(cmd_msg[0], "cd") == 0) {
                if(strcmp(cmd_msg[1], "~") == 0) {
                    chdir(getenv("HOME"));
                }
                else {
                    chdir(cmd_msg[1]);
                }
            }  

            for(int k = 0; cmd_msg[k] != NULL; k++) {
                if(strcmp(cmd_msg[k], "<") == 0) {
                    direction_check[0] = 1;
                }
                else if(strcmp(cmd_msg[k], ">") == 0) {
                    direction_check[1] = 1;
                }
                else if(strcmp(cmd_msg[k], ">>") == 0) {
                    direction_check[2] = 1;
                }
            }

            pid_t pid = fork();
            if(pid < 0) {
                perror("fork error");
                exit(0);
            }
            else if(pid == 0) {
                if(background == 1) {
                    signal(SIGINT, SIG_IGN);
                    if(k == 0) {
                        printf("[%d] %d\n", work_num++, getpid());
                    }
                }
                else {
                    signal(SIGINT, SIG_IGN);
                }

                if(direction_check[1] == 1) {
                    fd = open(cmd_msg[cmd_len - 1], O_RDWR | O_CREAT | S_IROTH, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    int v = 0;
                    for(v = cmd_len - 2; cmd_msg[v] != NULL; v++) {
                        cmd_msg[v] = cmd_msg[v + 2];
                    }
                    cmd_msg[v] = NULL;
                }
                else if(direction_check[2] == 1) {
                    fd = open(cmd_msg[cmd_len - 1], O_RDWR | O_APPEND| S_IROTH, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    int v = 0;
                    for(v = cmd_len - 2; cmd_msg[v] != NULL; v++) {
                        cmd_msg[v] = cmd_msg[v + 2];
                    }
                    cmd_msg[v] = NULL;
                }
                else if(direction_check[0] == 1) {
                    fd = open(cmd_msg[cmd_len - 1], O_RDONLY, 0644);
                    dup2(fd, 0);
                    close(fd);
                    int v = 0;
                    for(v = cmd_len - 2; cmd_msg[v] != NULL; v++) {
                        cmd_msg[v] = cmd_msg[v + 2];
                    }
                    cmd_msg[v] = NULL;
                }

                
                cmd_msg[cmd_len] = NULL;
                cmd_msg[cmd_len + 1] = NULL;

                execvp(cmd_msg[0], cmd_msg);

                exit(0);
            }
            else {
                waitpid(pid, &status, 0);
            }
        }
        }
    }

    return 0;
}

void history_init(void) {
    head = (list_t*)malloc(sizeof(list_t));
    tail = (list_t*)malloc(sizeof(list_t));

    head->name = NULL;
    head->value = NULL;
    head->prev = head;
    head->next = tail;

    tail->name = NULL;
    tail->value = NULL;
    tail->prev = head;
    tail->next = tail;
}

void history_insert(char* cmd) {
    char* tmp;
    list_t* t;
    
    t = (list_t*)malloc(sizeof(list_t));
    t->value = (char*)malloc(strlen(cmd) + 1);
    strcpy(t->value, cmd);

    t->prev = tail->prev;
    t->next = tail;
    tail->prev->next = t;
    tail->prev = t;
}

void history_printout() {
    int n;
    list_t* t;

    t = head->next;
    int i = 1;
    while(t != tail) {
        
        printf("%d %s\n", i, t->value);
        t = t->next;
        i++;
    }
}

int join(char *com1[], char *com2[]) {
    int p[2], status;

    switch(fork()) {
        case -1:
        fatal("1st fork call in join");
        case 0:
        break;
        default:
        wait(&status);
        return(status);
    }

    if(pipe(p) == -1) {
        fatal("pipe call in join");
    }

    switch(fork()) {
        case -1:
        fatal("2nd fork call in join");
        case 0:
        dup2(p[1], 1);
        close(p[0]);
        close(p[1]);
        execvp(com1[0], com1);
        fatal("1st execvp call in join");
        default:
        dup2(p[0], 0);
        close(p[0]);
        close(p[1]);
        execvp(com2[0], com2);
        fatal("2nd execvp call in join");
    }
    return 0;
}
