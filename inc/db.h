#ifndef ___MEDIA_SERVER_DB_H__
#define ___MEDIA_SERVER_DB_H__

int dbInit(void *arg);
int dbDestroy(void *arg);
int writeToDb(char *cmd, void *arg);

#endif

