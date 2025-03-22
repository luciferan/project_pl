#pragma once
#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "../_framework/character_base.h"
#include "../_lib/object_pool_mgr.h"

#include <map>

using namespace std;

//
class CharacterMgr : public ObjectMgrBase<Character>
{
private:
    map<INT64, Character*> _characterMap{};

    Character* _playerCharacter;
    INT64 _biUpdateTime{0};

    //
private:
    CharacterMgr() { _playerCharacter = GetFreeObject(); }
public:
    virtual ~CharacterMgr() {}
    static CharacterMgr& GetInstance()
    {
        static CharacterMgr* pInstance = new CharacterMgr;
        return *pInstance;
    }

    Character* EnterCharacter(INT64 token);
    void LeaveCharacter(INT64 token);

    Character* GetPlayerCharacter();
    bool IsPlayerCharacter(INT64 token);
    bool IsPlayerCharacter(Character* character);

    Character* GetCharacter(INT64 token);
    Character* GetCharacter(SafeLock&, INT64 token);
};

//
#endif //__CHARACTER_H__