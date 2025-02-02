#pragma once
#ifndef __COMMON_VARIABLE_H__
#define __COMMON_VARIABLE_H__

enum eResultCode
{
	RESULT_SUCC = 0,
	RESULT_FAIL = 1,

	RESULT_INVALID_PACKET,
	RESULT_SOCKET_DISCONNECTED,

	RESULT_PENDING,
};


enum eNetwork
{
	MAX_THREAD_COUNT = 1,

	MAX_LEN_IP4_STRING = 16,
	MAX_LEN_DOMAIN_STRING = 1024,

	MAX_LOG_BUFFER_SIZE = 1024 * 10,
};

#define SAFE_DELETE(ptr) { if(ptr) delete ptr; ptr = nullptr; }


#endif //__COMMON_VARIABLE_H__