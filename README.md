# Keydows
Keydows allows using your keyboard to navigate on Windows.

The program is and will be a WIP for a long time unless I finish it. Hence anyone reading this may not find any use for it. Additionally there is no proper GUI for keybinding, styling, etc.

[Test site](https://www.tandfonline.com/doi/full/10.1080/15548627.2015.1100356#d1e57487)

## Goals
- To learn more about the Windows API.
- To learn more about C/C++.
- To build a tool that could save me having to reach for my mouse.

## Implementation
One main app that has hooks for listening to events.
Tools are compiled to dlls and use an interface to interact with the main app.

#### Main app keybinds
`Ctrl + Alt + Q` - closes the app.

## Tools
At the moment there are two tools.
1. Smooth Navigate
2. Overlay 

### 1. Smooth Navigate
Contains
- smooth scrolling with keyboard
- smooth movement for the cursor.

#### Keybinds
To activate/deactivate the navigation the user has to click Alt + §.
```cpp
ACTIVATE,      220 // § key,
ACTIVATE_MOD,  MENU // Alt key,
```

Other keybinds
```cpp
LEFT_CLICK,    SPACE,
RIGHT_CLICK,   X
MOVE_UP,       W
MOVE_DOWN,     S
MOVE_LEFT,     A
MOVE_RIGHT,    D
SCROLL_UP,     Q
SCROLL_DOWN,   E
SCROLL_LEFT,   J
SCROLL_RIGHT,  L
INCREASE,      R // Base speed control
DECREASE,      F // Base speed control
FAST_MODE,     C
SLOW_MODE,     V
```


### 2. Overlay
An overlay grid of character coordinates the user can type to perform a click or other actions.

#### Keybinds
To open the overlay, `Ctrl + .`.

While the overlay is open the user may
```cpp
HIDE,           ESCAPE  // Hide the overlay
REMOVE,        BACKSPACE  // Removes a typed character
CLEAR,         RETURN // Reset typed characters
```
When the user has entered two (2) characters they may
click at the entered coordinates by pressing (almost) any key on their keyboard or ...
```cpp
MOVE,          C // Move the cursor to the location
DOUBLE_CLICK,  V
TRIPLE_CLICK,  N
QUAD_CLICK,    M
```

### 3. To be added
- A way to move and resize windows similar to [Alt Drag](https://stefansundin.github.io/altdrag/) and utilize the smooth behaviour.

## Dependencies
- Windows
- CMake

In the build folder
- libstdc++-6.dll
- libgcc_s_seh-1.dll
- libwinpthread-1.dll

## Building
Run CMake Build on a Windows computer in the project root.

## License
[MIT](https://choosealicense.com/licenses/mit/)