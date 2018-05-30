/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
/* Copyright (C) 2011 Zack Xue <zackxue@163.com> */

#ifndef __PLAYER_API__
#define __PLAYER_API__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (* player_data_cb) (unsigned char * p_buf, int size, void * param);

typedef struct __player_params{
    unsigned char url[256];
    char threadDoneFlag;
    int streamUsingTCP;
    void *phandle;
    void *arg; //cameraParams
    player_data_cb cb;
}player_params;

void * player_create(unsigned char * url);

int player_set_callback(void * handle, player_data_cb cb, void * param);

void player_run(void * handle, char *threadDoneFlag, int streamUsingTCP);


int player_start_play(player_data_cb cb, void* param);
void  player_stop_play(void **pphandle);


#ifdef __cplusplus
};
#endif
#endif
