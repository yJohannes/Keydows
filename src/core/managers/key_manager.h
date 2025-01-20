#pragma once
#include <unordered_map>


template <typename Key>
struct KeyInfo
{
    Key key;
    bool down;
};

template <typename KeyID, typename Key>
class KeyManager
{
    std::unordered_map<KeyID>, KeyInfo<Key> m_keybinds;
};