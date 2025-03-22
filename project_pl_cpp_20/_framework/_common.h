#pragma once
#include "stdafx.h"

#ifndef __COMMON_H__
#define __COMMON_H__

enum class eResultCode : int
{
    succ = 0,
    fail = 1,

    invalid_packet,
    socket_disconnected,
    pending,
};

namespace NetworkConst
{
const int MAX_THREAD_COUNT = 1;
const int MAX_LEN_IP4_STRING = 16;
const int MAX_LEN_DOMAIN_STRING = 1024;
const int MAX_LOG_BUFFER_SIZE = 1024 * 10;
};

#define SAFE_DELETE(ptr) { if(ptr) delete ptr; ptr = nullptr; }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) delete[] ptr; ptr = nullptr; }

//
#endif //__COMMON_H__