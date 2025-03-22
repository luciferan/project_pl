#include "./character.h"

//
Character* CharacterMgr::EnterCharacter(INT64 token)
{
    SafeLock lock(_lock);
    if (auto character = GetCharacter(lock, token)) {
        return character;
    }

    if( auto* character = GetFreeObject(lock) ){
        character->SetToken(token);
        _characterMap.insert({token, character});

        return character;
    }

    return nullptr;
}

void CharacterMgr::LeaveCharacter(INT64 token)
{
    SafeLock lock(_lock);
    if( auto it = _characterMap.find(token); it != _characterMap.end() ){
        _characterMap.erase(it);
    }
}

Character* CharacterMgr::GetPlayerCharacter()
{
    return _playerCharacter;
}

bool CharacterMgr::IsPlayerCharacter(INT64 token)
{
    return _playerCharacter->GetToken() == token;
}

bool CharacterMgr::IsPlayerCharacter(Character* character)
{
    return _playerCharacter == character;
}

Character* CharacterMgr::GetCharacter(INT64 token)
{
    SafeLock lock(_lock);
    return GetCharacter(lock, token);
}

Character* CharacterMgr::GetCharacter(SafeLock&, INT64 token)
{
    if (_playerCharacter->GetToken() == token) {
        return _playerCharacter;
    } else if (auto it = _characterMap.find(token); it != _characterMap.end()) {
        return it->second;
    }

    return nullptr;
}