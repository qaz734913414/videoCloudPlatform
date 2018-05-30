#ifndef ___MEDIA_SERVER_H__
#define ___MEDIA_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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
#include <ctype.h>
#include "cJSON.h"
#include "share.h"
#include "playerapi.h"
#include "db.h"

#define MEDIA_SERVER_VERSION            "V0.01"

#define MAGIC_STRING                    "$CV#"
#define MEDIA_SERVER_CFG_FILE           "config.json"
#define CAMERA_CFG_FILE                 "camera.json"

enum STREAMTYPE  {
    LIVE_VIDEO_RTSP_TYPE = 1,
    LIVE_VIDEO_GB28181,
    LIVE_VIDEO_HIK_SDK,
    LIVE_VIDEO_DH_SDK,
    LIVE_VIDEO_HLS = 8,
    LIVE_VIDEO_HTTP_FLV,
    UNKNOWN_LIVE_VIDEO_TYPE
};

enum H264_FRAME_TYPE {
    I_FRAME_TYPE = 1,
    P_FRAME_TYPE,
    B_FRAME_TYPE
};

typedef struct {
    int t1;
    int t2;
    int t3;
    int t4;
    int t5;
    int t6;
    int t7;
    int t8;
} probeParams;

typedef struct {
    void *lastFramePtr;
    int offset;
    char reserved[4];
} __attribute__((packed)) shareMemHeadParams;
#define SHARE_MEM_HEAD_SIZE     (int)(sizeof(shareMemHeadParams))

typedef struct {
    char consNID[4];
    int type;
    int frameId;
    int frameSize;
    long int timestamp;
    void *prev;
    void *next;
    char reserved[4];
} __attribute__((packed)) frameHeadParams;
#define FRAME_HEAD_SIZE     (int)(sizeof(frameHeadParams))

typedef struct {
    pid_t pid;
    int cameraId;
    char cameraType[32];
    void *cameraArgs;
    int status;
    int statusErrCnt;
    int restartInterval;
    int running;
    void *arg; //mediaServerParams
} CameraParams;

typedef struct {
    char h264Path[256];
    char mp4Path[256];
    int alarmRecordDays;
    int totalFrames;
    int alarmOffsetSeconds;
    int useSnapTimestamp;
} alarmParams;

typedef struct {
    int recordDays;
} nvrParams;

typedef struct {
    int restApiPort;
    int streamOutputPort;
    int videoShareMemSize;
    int previewType;
    char localIp[128];
    int httpPort;

    int cameraNum;
    CameraParams *pCameraParams;

    int alarmVideoEnable;
    alarmParams alarmParam;

    int nvrEnable;
    nvrParams nvrParam;

    pthread_mutex_t mutex_camera_json;

    pid_t pid_rest_api;
    pid_t pid_video_out;
    pid_t pid_alarm_video;

    probeParams probeParam;
    probeParams lastProbeParam;

    int running;
} mediaServerParams;

#endif

