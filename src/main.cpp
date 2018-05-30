#include "mediaServer.h"

mediaServerParams g_mediaServerParams;

static void handle_pipe(int sig) {
    printf("capture SIGPIPE, pid:%d\n", getpid());
}

static int init(mediaServerParams *pMediaServerParams) {
    struct sigaction action;

    action.sa_handler = handle_pipe;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);

    //initParams((char *)NVR_CFG_FILE, &g_nvrParams);
    //pthread_mutex_init(&(g_nvrParams.mutex_camera), NULL);
    dbInit(pMediaServerParams);

    return 0;
}

static int destroy(mediaServerParams *pMediaServerParams) {
    dbDestroy(pMediaServerParams);
    //freeParams(&g_nvrParams);

    return 0;
}

static void child_quit_signal_handle(int sig) {
    int status;
    int quit_pid = wait(&status);
     mediaServerParams *pMediaServerParams = &g_mediaServerParams;

    if(!access("mediaServer.stop", F_OK)) {
        return;
    }
    APP_WARRING("child process %d quit, exit status %d", quit_pid, status);

    if(quit_pid == pMediaServerParams->pid_rest_api) {
    }
    else if(quit_pid == pMediaServerParams->pid_video_out) {
    }
    else if(quit_pid == pMediaServerParams->pid_alarm_video) {
    }
    else {
        //TODO
    }
}

static void camera_process(int cameraId, mediaServerParams *pMediaServerParams) {

    APP_DEBUG("pid:%d, cameraId:%d", getpid(), cameraId);

    while(pMediaServerParams->running) {
        if(!access("mediaServer.stop", F_OK)) {
            pMediaServerParams->running = 0;
            break;
        }
        sleep(2);
    }
}

static void alarmVideoprocess(mediaServerParams *pMediaServerParams) {

    APP_DEBUG("pid:%d", getpid());

    while(pMediaServerParams->running) {
        if(!access("mediaServer.stop", F_OK)) {
            pMediaServerParams->running = 0;
            break;
        }
        sleep(2);
    }
}

static void videoOutProcess(mediaServerParams *pMediaServerParams) {

    APP_DEBUG("pid:%d", getpid());

    while(pMediaServerParams->running) {
        if(!access("mediaServer.stop", F_OK)) {
            pMediaServerParams->running = 0;
            break;
        }
        sleep(2);
    }
}

static void restApiProcess(mediaServerParams *pMediaServerParams) {

    APP_DEBUG("pid:%d", getpid());

    while(pMediaServerParams->running) {
        if(!access("mediaServer.stop", F_OK)) {
            pMediaServerParams->running = 0;
            break;
        }
        sleep(2);
    }
}

static void alarmVideoMainProcess(mediaServerParams *pMediaServerParams) {
    pid_t pid = -1;

    pid = fork();
    if(pid == -1) {
        APP_ERR("fork failed");
        while(1) {
            sleep(2);
        }
    }
    else if(pid == 0) {
        alarmVideoprocess(pMediaServerParams);
    }
    else {
        pMediaServerParams->pid_alarm_video = pid;
        while(pMediaServerParams->running) {
            if(!access("mediaServer.stop", F_OK)) {
                g_mediaServerParams.running = 0;
                break;
            }
            sleep(3);
        }
        sleep(3);
    }
}

static void videoOutMainProcess(mediaServerParams *pMediaServerParams) {
    pid_t pid = -1;

    pid = fork();
    if(pid == -1) {
        APP_ERR("fork failed");
        while(1) {
            sleep(2);
        }
    }
    else if(pid == 0) {
        videoOutProcess(pMediaServerParams);
    }
    else {
        pMediaServerParams->pid_video_out = pid;
        alarmVideoMainProcess(pMediaServerParams);
    }
}

static void restApiMainProcess(mediaServerParams *pMediaServerParams) {
    pid_t pid = -1;

    pid = fork();
    if(pid == -1) {
        APP_ERR("fork failed");
        while(1) {
            sleep(2);
        }
    }
    else if(pid == 0) {
        restApiProcess(pMediaServerParams);
    }
    else {
        pMediaServerParams->pid_rest_api = pid;
        videoOutMainProcess(pMediaServerParams);
    }
}

int main(int argc, char *argv[]) {
    int i;
    pid_t pid = -1;
    CameraParams *p;

    if(DirectoryCheck("log") < 0) {
        APP_ERR("log is not exsit, and mkdir err");
        return -1;
    }

    APP_DEBUG("Built: %s %s, version:%s, mediaServer starting ...", __TIME__, __DATE__, MEDIA_SERVER_VERSION);

    memset(&g_mediaServerParams, 0, sizeof(mediaServerParams));
    init(&g_mediaServerParams);
    g_mediaServerParams.running = 1;

    for(i = 0; i < g_mediaServerParams.cameraNum; i ++) {
        p = g_mediaServerParams.pCameraParams + i;
        pid = fork();
        if(pid == 0 || pid == -1) {
            break;
        }
        else {
            p->pid = pid;
        }
    }

    if(pid == -1) {
        APP_ERR("fork failed");
        while(1) {
            sleep(2);
        }
    }
    else if(pid == 0) {
        camera_process(p->cameraId, &g_mediaServerParams);
    }
    else {
        APP_DEBUG("parent pid : %d", getpid());
        signal(SIGCHLD, child_quit_signal_handle);

        restApiMainProcess(&g_mediaServerParams);
    }

    destroy(&g_mediaServerParams);

    APP_DEBUG("pid %d, mediaServer run over", getpid());

    return 0;
}

