#ifndef __SDL_EVENT_H__
#define __SDL_EVENT_H__

#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define enumdef(k) SDLK_##k,

enum SDL_Keys {
  SDLK_NONE = 0,
  _KEYS(enumdef)
};

enum SDL_EventType {
  SDL_KEYDOWN,
  SDL_KEYUP,
  SDL_USEREVENT,
};

#define SDL_EVENTMASK(ev_type) (1u << (ev_type))

enum SDL_EventAction {
  SDL_ADDEVENT,
  SDL_PEEKEVENT,
  SDL_GETEVENT,
};

typedef struct {
  uint8_t sym;
} SDL_keysym; 

typedef struct {
  uint8_t type;
  SDL_keysym keysym;//表示按下的键的符号（例如 SDLK_A 表示 ‘A’ 键）。
} SDL_KeyboardEvent;
//key 是一个 SDL_KeyboardEvent 结构体，它包含了与键盘事件相关的信息。
// 当 type 成员指示当前事件是一个键盘事件（例如 SDL_KEYDOWN 或 SDL_KEYUP）时，可以使用 key 成员来获取更多的细节，

typedef struct {
  uint8_t type;
  int code;//事件代码，可以用于区分不同的自定义事件。
  void *data1;//两个指针，可以用来传递任意数据
  void *data2;
} SDL_UserEvent;
//user 是一个 SDL_UserEvent 结构体，它用于自定义事件。当您需要发送自定义事件时，可以使用这个结构体

typedef union {
  uint8_t type;//type 是一个无符号的8位整数，用于存储事件类型。它决定了联合体中应该使用哪个成员来获取事件的具体信息。SDL 定义了许多不同的事件类型，例如 SDL_KEYDOWN、SDL_KEYUP、SDL_MOUSEMOTION 等。
  SDL_KeyboardEvent key;
  SDL_UserEvent user;
} SDL_Event;
//SDL_Event 是 Simple DirectMedia Layer (SDL) 库中的一个核心结构体，用于处理事件循环。
// SDL 是一个跨平台的开源库，用于处理游戏开发和其他多媒体应用程序中的音频、键盘、鼠标、操纵杆和图形硬件。
// SDL_Event 被定义为一个联合体（union），这意味着在这个联合体中的所有成员共享同一块内存空间。这意味着在任何时刻，只能使用联合体中的一个成员

int SDL_PushEvent(SDL_Event *ev);
int SDL_PollEvent(SDL_Event *ev);
int SDL_WaitEvent(SDL_Event *ev);
int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask);
uint8_t* SDL_GetKeyState(int *numkeys);

#endif
