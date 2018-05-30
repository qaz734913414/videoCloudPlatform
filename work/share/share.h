#ifndef __MEDIA_SHARE__H__
#define __MEDIA_SHARE__H__

#define DEBUG_LOG_FILE                  "log/mediaServer.log"
#define DEBUG_LOG_BAK_FILE              "log/mediaServer.log.bak"
#define LOG_SIZE_MAX                    (1024*1024*20)

//#define SYS_LOG_DEBUG
#ifndef SYS_LOG_DEBUG

#define APREFIX_NONE   "\033[0m"
#define APREFIX_RED    "\033[0;31m"
#define APREFIX_GREEN  "\033[0;32m"
#define APREFIX_YELLOW "\033[1;33m"

#define APP_DEBUG(format, args...) \
    printf(APREFIX_GREEN"DEBUG : FILE -> %s, LINE -> %d, %s, "  format APREFIX_NONE"\n", __FILE__, __LINE__, __func__, ## args); \
    do { \
        char bufLog[1024]; \
        struct timeval tv; \
        struct tm _time; \
        gettimeofday(&tv, NULL); \
        localtime_r(&tv.tv_sec, &_time); \
        int len = snprintf(bufLog, 1024, "%d-%02d-%02d %02d:%02d:%02d, debug, %s:%d,%s " format "\n", \
                _time.tm_year + 1900, _time.tm_mon + 1, _time.tm_mday, _time.tm_hour, _time.tm_min, \
                _time.tm_sec, __FILE__, __LINE__, __func__, ##args); \
        FILE *fp = fopen(DEBUG_LOG_FILE, "a+"); \
        if(fp != NULL) { \
            fwrite(bufLog, 1, len, fp); \
            fclose(fp); \
        } \
        struct stat f_stat; \
        if(stat(DEBUG_LOG_FILE, &f_stat) == 0) { \
            if(f_stat.st_size > LOG_SIZE_MAX) { \
				rename(DEBUG_LOG_FILE, DEBUG_LOG_BAK_FILE);\
            } \
        } \
    } while(0)
#define APP_WARRING(format, args...) \
    printf(APREFIX_YELLOW"WARRING : FILE -> %s, LINE -> %d, %s, "  format APREFIX_NONE"\n", __FILE__, __LINE__, __func__, ## args); \
    do { \
        char bufLog[1024]; \
        struct timeval tv; \
        struct tm _time; \
        gettimeofday(&tv, NULL); \
        localtime_r(&tv.tv_sec, &_time); \
        int len = snprintf(bufLog, 1024, "%d-%02d-%02d %02d:%02d:%02d, warring, %s:%d,%s " format "\n", \
                _time.tm_year + 1900, _time.tm_mon + 1, _time.tm_mday, _time.tm_hour, _time.tm_min, \
                _time.tm_sec, __FILE__, __LINE__, __func__, ##args); \
        FILE *fp = fopen(DEBUG_LOG_FILE, "a+"); \
        if(fp != NULL) { \
            fwrite(bufLog, 1, len, fp); \
            fclose(fp); \
        } \
        struct stat f_stat; \
        if(stat(DEBUG_LOG_FILE, &f_stat) == 0) { \
            if(f_stat.st_size > LOG_SIZE_MAX) { \
				rename(DEBUG_LOG_FILE, DEBUG_LOG_BAK_FILE);\
            } \
        } \
    } while(0)
#define APP_ERR(format, args...) \
    printf(APREFIX_RED"ERR : FILE -> %s, LINE -> %d, %s, "  format APREFIX_NONE"\n", __FILE__, __LINE__, __func__, ## args); \
    do { \
        char bufLog[1024]; \
        struct timeval tv; \
        struct tm _time; \
        gettimeofday(&tv, NULL); \
        localtime_r(&tv.tv_sec, &_time); \
        int len = snprintf(bufLog, 1024, "%d-%02d-%02d %02d:%02d:%02d, error, %s:%d,%s " format "\n", \
                _time.tm_year + 1900, _time.tm_mon + 1, _time.tm_mday, _time.tm_hour, _time.tm_min, \
                _time.tm_sec, __FILE__, __LINE__, __func__, ##args); \
        FILE *fp = fopen(DEBUG_LOG_FILE, "a+"); \
        if(fp != NULL) { \
            fwrite(bufLog, 1, len, fp); \
            fclose(fp); \
        } \
        struct stat f_stat; \
        if(stat(DEBUG_LOG_FILE, &f_stat) == 0) { \
            if(f_stat.st_size > LOG_SIZE_MAX) { \
				rename(DEBUG_LOG_FILE, DEBUG_LOG_BAK_FILE);\
            } \
        } \
    } while(0)

#else

#define APP_DEBUG(format, args...) syslog(LOG_EMERG, "DEBUG : FILE -> %s, LINE -> %d, %s, " format "\n", __FILE__, __LINE__, __func__, ## args)
#define APP_WARRING(format, args...) syslog(LOG_EMERG, "WARRING : FILE -> %s, LINE -> %d, %s, " format "\n", __FILE__, __LINE__, __func__, ## args)
#define APP_ERR(format, args...) syslog(LOG_EMERG, "ERR : FILE -> %s, LINE -> %d, %s, " format "\n", __FILE__, __LINE__, __func__, ## args)

#endif

#define COMMON_QUEUE_MAX_LEN    300

typedef struct nodecommon {
    char name[256];
    int val;
    void *arg;
    struct nodecommon *next;
} node_common;

typedef struct {
	node_common *head;
	node_common *tail;
	int queLen;
} queue_common;

int blockSend(unsigned int connfd, char *src, int size);
int blockRecv(unsigned int connfd, char *dst, int size);
int putToQueue(queue_common *queue, node_common *new_node);
int getFromQueue(queue_common *queue, node_common **new_p);
int delFromQueue(queue_common *queue, void *arg, node_common **new_p, int (*condition)(node_common *p, void *arg));
int searchFromQueue(queue_common *queue, void *arg, node_common **new_p, int (*condition)(node_common *p, void *arg));
int traverseQueue(queue_common *queue, void *arg, int (*callBack)(node_common *p, void *arg));
int freeQueue(queue_common *queue);

int popenRun(char *cmd, int timeout_100ms);
int DirectoryCheck(const char *dir);
void loopWhile(void);

#endif //__MEDIA_SHARE__H__
