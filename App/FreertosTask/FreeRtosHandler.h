//
// Created by 114514 on 2026/11/11.
//

#ifndef LORACOPY_FREERTOSHANDLER_H
#define LORACOPY_FREERTOSHANDLER_H
void CreateInitAllTask();
void CreateWatchDogTask();
void WatchDogTask(void *argument);
void InitLWIPTask(void *argument);
#endif //LORACOPY_FREERTOSHANDLER_H
