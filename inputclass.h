////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#define DIRECTINPUT_VERSION 0x0800 // DirectInput ���� ����
#pragma comment(lib, "dinput8.lib") // DirectInput ���̺귯�� ��ũ
#pragma comment(lib, "dxguid.lib")  // GUID ���̺귯�� ��ũ

#include <dinput.h> // DirectInput ���


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

	void GetMouseLocation(int&, int&) const; // ���� ���콺 Ŀ�� ��ġ (ȭ�� ��ǥ, �ʿ��ϴٸ�)
	void GetMouseDelta(long&, long&) const;   // ���� ������ ��� ���콺 �̵���

private:
	bool m_keys[256];

	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	DIMOUSESTATE m_mouseState;        // ���� ���콺 ����
	DIMOUSESTATE m_lastMouseState;    // ���� ������ ���콺 ���� (��Ÿ ����, ���� �ڵ�ó��) - �Ǵ� ���� ��Ÿ ���
	long m_mouseXDelta;               // ���콺 X�� �̵���
	long m_mouseYDelta;               // ���콺 Y�� �̵���

	int m_screenWidth;
	int m_screenHeight;

	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

};

#endif