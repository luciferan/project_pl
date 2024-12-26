#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

//
#include "../_framework/network.h"
#include "../_framework/ExceptionReport.h"
#include "../_framework/util.h"
#include "../_framework/log.h"


//
//bool LoadConfig();

unsigned int WINAPI UpdateThread(void *p);
unsigned int WINAPI ProcessThread(void *p);
unsigned int WINAPI CommandThread(void *p);



//
#endif //__PROCESS_H__
