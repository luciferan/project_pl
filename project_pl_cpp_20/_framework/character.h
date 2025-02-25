#pragma once
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <windows.h>

class Character
{
public:
    int id{0};
    INT64 token{0};

    int pos_x{0};
    int pos_y{0};

public:
    Character() {}
    ~Character() {}
};

#endif //__CHARACTER_H__