////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800 // DirectInput 버전 정의
#pragma comment(lib, "dinput8.lib") // DirectInput 라이브러리 링크
#pragma comment(lib, "dxguid.lib")  // GUID 라이브러리 링크

#include <dinput.h> // DirectInput 헤더


////////////////////////////////////////////////////////////////////////////////
// Class name: InputClass
////////////////////////////////////////////////////////////////////////////////
class InputClass
{
public:
	InputClass();
	InputClass(const InputClass&);
	~InputClass();

	bool Initialize(HINSTANCE, HWND, int screenWidth, int screenHeight);
	void Shutdown();

	bool Frame();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	bool IsKeyDown(unsigned int) const;

	void GetMouseLocation(int&, int&) const; // 현재 마우스 커서 위치 (화면 좌표, 필요하다면)
	void GetMouseDelta(long&, long&) const;   // 이전 프레임 대비 마우스 이동량

private:
	bool m_keys[256];

	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	DIMOUSESTATE m_mouseState;        // 현재 마우스 상태
	DIMOUSESTATE m_lastMouseState;    // 이전 프레임 마우스 상태 (델타 계산용, 예제 코드처럼) - 또는 직접 델타 계산
	long m_mouseXDelta;               // 마우스 X축 이동량
	long m_mouseYDelta;               // 마우스 Y축 이동량

	int m_screenWidth;
	int m_screenHeight;

	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

};

#endif