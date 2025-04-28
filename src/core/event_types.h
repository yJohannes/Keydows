#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

#ifdef DOUBLE_CLICK     // windows.h
    #undef DOUBLE_CLICK
#endif

// namespace keydows
// {

enum class Event
{
    QUIT,
    HIDE,
    REMOVE,
    CLEAR,
    INCREASE,
    DECREASE,
    RESET,

    ACTIVATE,
    ACTIVATE_MOD,

    FAST_MODE,
    SLOW_MODE,

    MOVE,
    SCROLL,
    CLICK,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    QUAD_CLICK,

    LEFT_CLICK,
    RIGHT_CLICK,

    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    
    SCROLL_UP,
    SCROLL_DOWN,
    SCROLL_LEFT,
    SCROLL_RIGHT
};

// } // namespace keydows

#endif // EVENT_TYPES_H