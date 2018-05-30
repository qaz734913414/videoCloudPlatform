#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "share.h"

int popenRun(char *cmd, int timeout_100ms) {
    FILE *file;
    char line[1024];
    int cnt = 0;

    file = popen(cmd, "r");
    if(file != NULL) {
        while(fgets(line, 1024, file) != NULL) {
            cnt ++;
            if(cnt > timeout_100ms) {
                APP_WARRING("%s run too long time, break", cmd);
                break;
            }
            usleep(100000);
        }
    }
    else {
        APP_WARRING("popen failed, %s", cmd);
    }
    pclose(file);

    return 0;
}

int blockSend(unsigned int connfd, char *src, int size) {
    int k, l_sendedsize;
    int l_connfd=connfd;
    int l_size=size;
    int ret;
    k = 0;
    l_sendedsize = 0;
    struct timeval timeout;
    fd_set rw;                      

    while(1) {
        FD_ZERO(&rw);
        FD_SET(connfd, &rw);
        timeout.tv_sec = 1;
        timeout.tv_usec =0;
        ret = select(connfd+1, NULL, &rw, NULL, &timeout);  
        if(ret == -1) {
            APP_ERR("err: select -1");
            return -6;
        }
        else if(ret == 0) {
            if(errno == EPIPE || errno == ECONNRESET) {
                APP_ERR("err : select timeout, fd:%d, %d:%s", connfd,  errno, strerror(errno));
                return -5;
            }
            else {
                if(errno != 0) {
                    APP_ERR("select err, fd:%d, %d:%s", connfd, errno, strerror(errno));
                    return -4;
                }
                else {
                    //printf("select err, fd:%d, errno:%d, %s\n", connfd, errno, strerror(errno));
                    return 0;
                }
            }
        }
        else {
            if(FD_ISSET(connfd, &rw)) {
                k = send(l_connfd, src + l_sendedsize, l_size - l_sendedsize, MSG_NOSIGNAL);
                if(k<=0) {
                    if(errno == EPIPE || errno == ECONNRESET) {
                        //APP_ERR("send err:%d:%s, fd:%d", errno, strerror(errno), connfd);
                        return -3;
                    }
                    else {
                        APP_ERR("send err:%d:%s, fd:%d", errno, strerror(errno), connfd);
                        return -2;
                    }
                }
            } else {
                APP_ERR("FD_ISSET err");
                return -1;
            }
        }

        l_sendedsize += k;
        if (l_sendedsize == l_size) {
            return 0;
        } 
    } 
}

int blockRecv(unsigned int connfd, char *dst, int size) {
    int k, l_recvedsize;
    int l_connfd=connfd;
    int l_size=size;
    int ret;
    k = 0;
    l_recvedsize = 0;
    struct timeval timeout;
    fd_set rw;                      

    while(1) {
        FD_ZERO(&rw);
        FD_SET(connfd, &rw);
        timeout.tv_sec = 3;
        timeout.tv_usec =0;
        ret = select(connfd+1, &rw, NULL, NULL, &timeout);  
        if(ret == -1) {
            APP_ERR("err!! select -1");
            return -3;
        }
        else if(ret == 0) {
            if(errno != 0 && errno != ENOENT) {
                printf("select timeout, fd:%d, errno:%d, %s\n", connfd, errno, strerror(errno));
                return -2;
            }
            else {
                if(errno != 0) {
                    APP_ERR("select err, fd:%d, errno:%d, %s", connfd, errno, strerror(errno));
                    return -4;
                }
                else {
                    //printf("select err, fd:%d, errno:%d, %s\n", connfd, errno, strerror(errno));
                    return 1;
                }
            }
        }
        else {
            if(FD_ISSET(connfd, &rw)) {
                //k = recv(l_connfd, dst + l_recvedsize, l_size - l_recvedsize, 0);
                k = recv(l_connfd, dst + l_recvedsize, l_size - l_recvedsize, MSG_NOSIGNAL);
                if(k<=0) {
                    if(k != 0 || errno == ENOENT) {
                        APP_ERR("recv err, %d:%s", errno, strerror(errno));
					    return -5;
                    }
                    else if(errno != 0) {
                        //printf("recv err, %d:%s\n", errno, strerror(errno));
					    return 2;
                    }
                    else {
					    return 0;
                    }
                }
            } else {
                APP_ERR("FD_ISSET err");
                return -1;
            }
        }
		
        l_recvedsize += k;
        if (l_recvedsize == l_size) {
            return 0;
        } 
    } 
}

int putToQueue(queue_common *queue, node_common *new_node) {
    if(queue->queLen < COMMON_QUEUE_MAX_LEN) {
        node_common *new_p;
        new_p = (node_common *) malloc(sizeof(node_common));
        if (new_p == NULL) {
            APP_ERR("malloc failed");
            return -1;
        }
        memcpy(new_p->name, new_node->name, 256);
        new_p->val = new_node->val;
        new_p->arg = new_node->arg;
        new_p->next = NULL;
        if (queue->head == NULL) {
            queue->head = new_p;
            queue->tail = new_p;
        } else {
            queue->tail->next = new_p;
            queue->tail = new_p;
        }
        queue->queLen ++;
        //APP_DEBUG("##putToQueue, queLen:%d", queue->queLen);
    }
    else {
        //APP_WARRING("queLen is too large, %s:%d", new_node->name, queue->queLen);
        return queue->queLen;
    }
    
    return 0;
}

int getFromQueue(queue_common *queue, node_common **new_p) {
    node_common *p = queue->head;
    if(p != NULL) {
        queue->head = p->next;
        if(queue->head == NULL) {
            queue->tail = NULL;
        }
        queue->queLen --;
        if(queue->queLen < 0) {
            APP_WARRING("exception queLen : %d", queue->queLen);
            queue->queLen = 0;
        }
        *new_p = p;
    }
    
    return 0;
}

int delFromQueue(queue_common *queue, void *arg, node_common **new_p, int (*condition)(node_common *p, void *arg)) {
    int ret;
    node_common *prev = queue->head;
    node_common *p = queue->head;
    while(p != NULL) {
        ret = condition(p, arg);
        if(ret == 1) {
            if(p == queue->head && p == queue->tail) {
                queue->head = queue->tail = NULL;
            }
            else if(p == queue->head) {
                queue->head = p->next;
            }
            else if(p == queue->tail) {
                prev->next = NULL;
                queue->tail = prev;
            }
            else {
                prev->next = p->next;
            }
            *new_p = p;
            queue->queLen --;
            if(queue->queLen < 0) {
                APP_WARRING("exception queLen : %d", queue->queLen);
                queue->queLen = 0;
            }
            break;
        }
        prev = p;
        p = p->next;
    }
    
    return 0;
}

int searchFromQueue(queue_common *queue, void *arg, node_common **new_p, int (*condition)(node_common *p, void *arg)) {
    int ret;
    node_common *p = queue->head;
    while(p != NULL) {
        ret = condition(p, arg);
        if(ret == 1) {
            *new_p = p;
            break;
        }
        p = p->next;
    }
    
    return 0;
}

int traverseQueue(queue_common *queue, void *arg, int (*callBack)(node_common *p, void *arg)) {
    node_common *p = queue->head;
    while(p != NULL) {
        //APP_DEBUG("##traverseQueue, queLen:%d", queue->queLen);
        callBack(p, arg);
        p = p->next;
    }
    
    return 0;
}

int freeQueue(queue_common *queue) {
    node_common *p1;
    node_common *p = queue->head;
    while(p != NULL) {
        p1 = p;
        p = p->next;
        if(p1->arg != NULL) {
            //free arg's sub mem first
            free(p1->arg);
        }
        free(p1);
    }
    queue->head = queue->tail = NULL;
    queue->queLen = 0;

    return 0;
}

static int CreateDir(const char *sPathName) {
    char dirName[4096];
    strcpy(dirName, sPathName);

    int len = strlen(dirName);
    if(dirName[len-1]!='/')
        strcat(dirName, "/");

    len = strlen(dirName);
    int i=0;
    for(i=1; i<len; i++)
    {
        if(dirName[i]=='/')
        {
            dirName[i] = 0;
            if(access(dirName, R_OK)!=0)
            {
                if(mkdir(dirName, 0755)==-1)
                {
                    //TODO:errno:ENOSPC(No space left on device)
                    APP_ERR("path %s error %s", dirName, strerror(errno));
                    return -1;
                }
            }
            dirName[i] = '/';
        }
    }
    return 0;
}

int DirectoryCheck(const char *dir) {
    DIR *pdir = opendir(dir);
    if(pdir == NULL)
        return CreateDir(dir);
    else
        return closedir(pdir);   
}

void loopWhile(void) {
    APP_WARRING("exception loop while");

    while(1) {
        sleep(10);
    }
}

