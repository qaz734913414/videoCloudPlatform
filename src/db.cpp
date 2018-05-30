#include "mediaServer.h"

    /*
int jsonCallBack(node_common *p, void *arg) {
    cameraParams *pCamera = (cameraParams *)p->arg;
    cJSON *rootArry = (cJSON *)arg;
    cJSON *fld;

    //printf("cameraId:%d,recRunning:%d,cameraType:%s,rtspAddr:%s\n", pCamera->cameraId, pCamera->recRunning, 
    //        pCamera->cameraType, ((rtspParams *)(pCamera->pparams))->playerParams.url);
    cJSON_AddItemToArray(rootArry, fld = cJSON_CreateObject());
    cJSON_AddNumberToObject(fld, "cameraId", pCamera->cameraId);
    cJSON_AddNumberToObject(fld, "recRunning", pCamera->recRunning);
    cJSON_AddStringToObject(fld, "cameraType", pCamera->cameraType);
    if(!strncmp(pCamera->cameraType, "rtsp", 32)) {
        rtspParams *pRtsp = (rtspParams *)(pCamera->pparams);
        cJSON_AddStringToObject(fld, "rtspAddr", (char *)(pRtsp->playerParams.url));
    }
    else {
        APP_WARRING("unsupport cameraType : %s", pCamera->cameraType);
    }

    return 0;
}

static int writeCfgByJson(mediaServerParams *pNvrParams) {
    cJSON *root =  cJSON_CreateObject();
    cJSON *rootArry =  cJSON_CreateArray();
    if(rootArry != NULL && root != NULL) {
        pthread_mutex_lock(&(pNvrParams->mutex_camera));
        traverseQueue(&(pNvrParams->queueCamera), rootArry, jsonCallBack);
        pthread_mutex_unlock(&(pNvrParams->mutex_camera));
        cJSON_AddItemToObject(root, "camera", rootArry);
        char *out = cJSON_Print(root);
        cJSON_Delete(root);
        //printf("json:%s\n", out);

        pthread_mutex_lock(&(pNvrParams->mutex_camera_json));
        FILE *fp = fopen(CAMERA_CFG_FILE, "wb");
        if(fp != NULL) {
            fwrite(out, 1, strlen(out), fp);
            fclose(fp);
        }
        else {
            APP_ERR("fopen %s failed", CAMERA_CFG_FILE);
        }
        pthread_mutex_unlock(&(pNvrParams->mutex_camera_json));
        free(out);
    }
    else {
        APP_ERR("create json failed");
        return -1;
    }

    return 0;
}
    */

static int paraseJson(char *buf, mediaServerParams *pMediaServerParams) {
    int i;
    char name[256];
    cJSON *pSub, *pArray, *p;
    CameraParams *pCamera;

    cJSON * root = cJSON_Parse(buf);
    if(root == NULL) {
        APP_ERR("err, buf:%s", buf);
        goto err;
    }
    strncpy(name, "camera", 256);
    pSub = cJSON_GetObjectItem(root, name);
    if(pSub == NULL) {
        APP_ERR("get json failed, %s", name);
        goto err;
    }
    pMediaServerParams->cameraNum = cJSON_GetArraySize(pSub);
    if(pMediaServerParams->cameraNum <= 0 || pMediaServerParams->cameraNum > 500) {
        APP_ERR("err cameraNum:%d", pMediaServerParams->cameraNum);
        goto err;
    }
    pMediaServerParams->pCameraParams = (CameraParams *)malloc(sizeof(CameraParams)*pMediaServerParams->cameraNum);
    if(pMediaServerParams->pCameraParams == NULL) {
        APP_ERR("malloc failed");
        goto err;
    }
    memset(pMediaServerParams->pCameraParams, 0, sizeof(CameraParams)*pMediaServerParams->cameraNum);

    for(i = 0; i < pMediaServerParams->cameraNum; i ++) {
        pCamera = pMediaServerParams->pCameraParams + i;
        pArray = cJSON_GetArrayItem(pSub, i);
        if(pArray != NULL) {
            strncpy(name, "cameraId", 256);
            p = cJSON_GetObjectItem(pArray, name);
            if(p != NULL) {
                //APP_DEBUG("%s:%d", name, p->valueint);
                pCamera->cameraId = p->valueint;
            }
            else {
                APP_ERR("get json failed, %s", name);
                loopWhile();
            }
            strncpy(name, "running", 256);
            p = cJSON_GetObjectItem(pArray, name);
            if(p != NULL) {
                //APP_DEBUG("%s:%d", name, p->valueint);
                pCamera->running = p->valueint;
            }
            else {
                APP_ERR("get json failed, %s", name);
                loopWhile();
            }
            strncpy(name, "cameraType", 256);
            p = cJSON_GetObjectItem(pArray, name);
            if(p != NULL) {
                //APP_DEBUG("%s:%s", name, p->valuestring);
                strncpy(pCamera->cameraType, p->valuestring, 32);
            }
            else {
                APP_ERR("get json failed, %s", name);
                loopWhile();
            }
        }
    }
    
err:
    if(root != NULL) {
        cJSON_Delete(root);
    }

    return 0;
}

int dbInit(void *arg) {
    int cfgSize;
    char *buf = NULL;;
    mediaServerParams *pMediaServerParams = (mediaServerParams *)arg;
    pthread_mutex_init(&(pMediaServerParams->mutex_camera_json), NULL);

    if(access(CAMERA_CFG_FILE, F_OK) != 0) {
        return 0;
    }

    FILE *fp = fopen(CAMERA_CFG_FILE, "rb");
    if(fp == NULL) {
        APP_ERR("fopen %s failed", CAMERA_CFG_FILE);
        goto err;
    }
    fseek(fp, 0L, SEEK_END);
    cfgSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    buf = (char *)malloc(cfgSize/1024*1024 + 1024);
    if(buf == NULL) {
        APP_ERR("malloc failed");
        goto err;
    }
    if(fread(buf, 1, cfgSize, fp)){
    }
    paraseJson(buf, pMediaServerParams);
    
err:
    if(buf != NULL) {
        free(buf);
    }
    if(fp != NULL) {
        fclose(fp);
    }

    return 0;
}

int dbDestroy(void *arg) {
    mediaServerParams *pMediaServerParams = (mediaServerParams *)arg;
    pthread_mutex_destroy(&(pMediaServerParams->mutex_camera_json));
    if(pMediaServerParams->pCameraParams != NULL) {
        free(pMediaServerParams->pCameraParams);
        pMediaServerParams->pCameraParams = NULL;
    }

    return 0;
}

int writeToDb(char *cmd, void *arg) {
    int ret = 0;
    /*
    nvrParams *pNvrParams = (nvrParams *)arg;

    if(!strncmp(pNvrParams->jsonParam.cfgType, "json", 32)) {
        ret = writeCfgByJson(pNvrParams);
    }
    else {
        APP_WARRING("unsupport cfgType : %s", pNvrParams->jsonParam.cfgType);
        ret = -1;
    }
    */

    return ret;
}

