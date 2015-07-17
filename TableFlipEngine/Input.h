/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#ifndef _INPUT_H__
#define _INPUT_H__
#include "Utilities.h"

//keystate bool array lookups
enum InputKey {
    /**  un-used characters, just here to fill out padding untill ascii starts at 32 **/
    KB_NULL=0, KB_SOH, KB_STX, KB_ETX, KB_EOT, KB_ENQ, KB_ACK, KB_BEL, KB_BS, KB_HT, KB_LF,
    KB_VT, KB_FF, KB_CR, KB_SO, KB_SI, KB_DLE, KB_DC1, KB_DC2, KB_DC3, KB_DC4, KB_NAK, KB_SYN,
    KB_ETB, KB_CAN, KB_EM, KB_SUB, KB_ESC27, KB_FS, KB_GS, KB_RS, KB_US,
    /**   start ascii input codes trying to maintain ascii keycodes, dont mind the extras**/
    KB_SPACE, KB_EXC,  KB_DBLQUOT,  KB_HASH,  KB_DOLLAR, KB_PERCENT,  KB_AND, KB_APOSTROPHE,  KB_LPAREN,  KB_RPAREN,
    KB_ASTERISK, KB_PLUS, KB_COMMA, KB_MINUS, KB_PERIOD, KB_SLASH, KB_0, KB_1, KB_2, KB_3, KB_4, KB_5, KB_6, KB_7, KB_8, KB_9,
    KB_COLON, KB_SEMICOLON, KB_LESS, KB_EQUAL, KB_GREATER, KB_QUESTION, KB_AT,
    KB_A, KB_B, KB_C, KB_D, KB_E, KB_F, KB_G, KB_H, KB_I, KB_J, KB_K, KB_L, KB_M, KB_N,
    KB_O, KB_P, KB_Q, KB_R, KB_S, KB_T, KB_U, KB_V, KB_W, KB_X, KB_Y, KB_Z,
    KB_LBRACKET, KB_BACKSLASH, KB_RBRACKET, KB_CARET, KB_UNDERSCORE, KB_GRAVE,
    /**   other key input codes **/
    KB_F1, KB_F2, KB_F3, KB_F4, KB_F5, KB_F6, KB_F7, KB_F8, KB_F9, KB_F10, KB_F11, KB_F12,
    KB_LSHIFT, KB_LCTRL, KB_UP, KB_DOWN, KB_LEFT, KB_RIGHT,
    KB_ESC, KB_RETURN, KB_BACKSPACE, KB_DELETE, NUMKEYS };

//how many unprocessed keys in ascii buffer
#define ASCII_BUFFER_SIZE 8 //dont see me really needing more than 8 bytes though
    
//how many cursors to track for multitouch
#if defined(PC_PLATFORM)
#define NUM_MULTITOUCH_CURSORS 1
#else
#define NUM_MULTITOUCH_CURSORS 4
#endif
//porting to android really confused this class, need to go through and possibly rename / re deesign one or two things
class Input
{
private:

    //disable functions for singleton use
    Input();
    ~Input();
    Input(const Input &) {}
    Input &operator = (const Input &) { return *this; }


    //the singletons instance.
    static Input *instance;
    
    //location on screen
    Vector2 touchCursors[NUM_MULTITOUCH_CURSORS];
    //pressed?
    bool    touchStates[NUM_MULTITOUCH_CURSORS];
    //last positions
    Vector2 touchLast[NUM_MULTITOUCH_CURSORS];
    
    int lastMouseX;
    int lastMouseY;
    
    //false is up, true is down
    bool lastMouseBtn1State;
    bool lastMouseBtn2State;
    
    //true = down, false = up
    bool keyLastState[NUMKEYS]; //keystate for previous frame;
    bool keyCurrentState[NUMKEYS]; //current in engine state
    
    // this is only polled every update call, for multithreaded input handling of "thisframe" key event functions
    //technically if you could press the key release and press again in under 17 millis @60fps,
    //the GetKeyUpThisFrame wouldnt register. oh well.
    bool keyState[NUMKEYS]; //current key states from OS
    
    //multithreaded fix, motion that happens before update stored here,
    //then update transfers it over to delta, and zeros out motion
    float mouseXMotion;
    float mouseYMotion;

    char asciiBuffer[ASCII_BUFFER_SIZE]; //for typing input, gets cleared each frame
    int asciiReadPos;
    int asciiWritePos;

    float pinchDelta;
    void UpdateGestures();


public:


    //if set true, no input events will be registered for keys A-Z,
    //ascii buffer will be still be filled for current frame
    bool textGrabMode;

    float mouseClickX;
    float mouseClickY;
    float mouseXDelta;
    float mouseYDelta;
    float mouseXSensitivity;
    float mouseYSensitivity;
    
    float mouseXCoords;
    float mouseYCoords;
    
    bool mouseBtn1Down;
    bool mouseBtn2Down;
    
    //inlines
    inline static Input *GetInstance() { return instance; }

    //init the singleton instance
    static void InitInstance();

    //delete the singelton instance
    static void DeleteInstance();

    //initialize the render system
    bool Init();

    
    //notify the input system that these events have occured
    //SETS  -- TODO rename these i suppose..
    void PointerAcceleration(void *data);
    void PointerMoved(void *data);
    void ButtonPressed(unsigned int btn);
    void ButtonReleased(unsigned int btn);
    void KeyPressed(InputKey key);
    void KeyReleased(InputKey key);
    void TouchDown(int id, Vector2 pos);
    void TouchUp(int id, Vector2 pos);
    void TouchMove(int id, Vector2 pos);

    
    //update some things, like mouse delta to 0
    void Update();
    
    //read input devices
    // i guess 1 pixel shall be 100 units. whatever that means..
    inline bool  GetMouseBtn1Down() { return mouseBtn1Down; }
    inline bool  GetMouseBtn2Down() { return mouseBtn2Down; }
    
    //returns true if the button state was just changed this frame
    inline bool  GetMouseBtn1JustPressed()  { return (GetMouseBtn1Down()  && !lastMouseBtn1State); }
    inline bool  GetMouseBtn2JustPressed()  { return (GetMouseBtn2Down()  && !lastMouseBtn2State); }
    inline bool  GetMouseBtn1JustReleased() { return (!GetMouseBtn1Down() && lastMouseBtn1State); }
    inline bool  GetMouseBtn2JustReleased() { return (!GetMouseBtn2Down() && lastMouseBtn2State); }
    
    inline float GetXAxis() { return mouseXDelta  * mouseXSensitivity; }
    inline float GetYAxis() { return mouseYDelta  * mouseYSensitivity; }
    inline float GetXRaw() { return mouseXDelta; }
    inline float GetYRaw() { return mouseYDelta; }
    inline float GetXCoords(int id) { return touchCursors[id].x; }
    inline float GetYCoords(int id) { return touchCursors[id].y; }
    
    inline Vector2 GetCoords(int id) { return Vector2(touchCursors[id].x, touchCursors[id].y); }
    ///returns normalized mouse coords between 0.0 and 1000.0
    //Vector2 GetNormalizedCoords(int id);

    inline float GetPinchDelta() { return abs(pinchDelta) < 1000 ? pinchDelta : 0; }
    
    bool GetKeyDown(InputKey k);
    //returns true only if key was pressed THIS frame
    bool GetKeyDownThisFrame(InputKey k);
    bool GetKeyUpThisFrame(InputKey k);
    
    inline bool GetTouchDown(int id) { 
        if (id < 0 || id >= NUM_MULTITOUCH_CURSORS)
            return false; 
        return touchStates[id]; 
        
    }

    inline void SetTouchCoords(int id, Vector2 coords) { 
        if (id < 0 || id >= NUM_MULTITOUCH_CURSORS) 
            return;
        touchLast[id] = touchCursors[id];
        touchCursors[id] = coords;
        mouseXCoords = coords.x;
        mouseYCoords = coords.y;
        
    }

    
    //returns characters in ascii buffer
    char GetNextAsciiChar();
    void AddAsciiChar(char c);
    void Valgrind() { LogInfo("Valgrind toggle function called"); }
    
    void Shutdown();
};

#endif
