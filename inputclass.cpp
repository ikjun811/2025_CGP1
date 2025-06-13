////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "inputclass.h"


InputClass::InputClass()
{
	m_directInput = nullptr;
	m_keyboard = nullptr;
	m_mouse = nullptr;

	m_mouseXDelta = 0;
	m_mouseYDelta = 0;
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


bool InputClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;
	int i;
	
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Initialize all the keys to being released and not pressed.
	for(i=0; i<256; i++)
	{
		m_keys[i] = false;
	}

	// DirectInput 인터페이스 생성
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 키보드 장치 생성
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 키보드 데이터 형식 설정
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// 키보드 협력 수준 설정 (포그라운드, 비독점)
	// DISCL_NONEXCLUSIVE: 다른 애플리케이션도 키보드 입력을 받을 수 있게 함
	// DISCL_FOREGROUND: 애플리케이션 창이 활성화되어 있을 때만 입력 받음
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}
	m_keyboard->Acquire();

	// 마우스 장치 생성
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 마우스 데이터 형식 설정
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// 마우스 협력 수준 설정 (포그라운드, 독점)
	// DISCL_EXCLUSIVE: 이 애플리케이션만 마우스 입력을 독점적으로 받음
	// DISCL_NOWINKEY: Windows 키 비활성화 (선택 사항)
	// DISCL_FOREGROUND: 애플리케이션 창이 활성화되어 있을 때만 입력 받음
	// FPS 카메라에서는 마우스 커서를 숨기고 독점 모드로 사용하는 것이 일반적입니다.
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(result))
	{
		return false;
	}

	// 초기 마우스 상태 읽기 (m_lastMouseState 초기화 목적)
	if (m_mouse)
	{
		m_mouse->Acquire(); // 마우스 장치 사용 시작
		m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_lastMouseState);
	}


	return true;
}

void InputClass::Shutdown()
{
	// 마우스 장치 해제
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = nullptr;
	}

	// 키보드 장치 해제
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = nullptr;
	}

	// DirectInput 인터페이스 해제
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = nullptr;
	}
}

bool InputClass::Frame()
{
	bool result;

	// 키보드 상태 읽기
	result = ReadKeyboard();
	if (!result)
	{
		return false;
	}

	// 마우스 상태 읽기
	result = ReadMouse();
	if (!result)
	{
		return false;
	}


	return true;
}

void InputClass::KeyDown(unsigned int input)
{
	// If a key is pressed then save that state in the key array.
	m_keys[input] = true;
	return;
}


void InputClass::KeyUp(unsigned int input)
{
	// If a key is released then clear that state in the key array.
	m_keys[input] = false;
	return;
}


bool InputClass::IsKeyDown(unsigned int key) const
{
	return m_keys[key];
}

void InputClass::GetMouseLocation(int& mouseX, int& mouseY) const
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	mouseX = cursorPos.x; 
	mouseY = cursorPos.y;
}

void InputClass::GetMouseDelta(long& deltaX, long& deltaY) const
{
	deltaX = m_mouseXDelta;
	deltaY = m_mouseYDelta;
}


bool InputClass::ReadKeyboard()
{
	HRESULT result;
	BYTE keyboardState[256] = { 0 };

	result = m_keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		
		for (int i = 0; i < 256; i++) { m_keys[i] = false; }
		return true; // 또는 false로 처리하여 입력 실패를 알림
	}

	// GetDeviceState 성공 시에만 m_keys 업데이트
	for (int i = 0; i < 256; i++)
	{
		m_keys[i] = (keyboardState[i] & 0x80) ? true : false;
	}
	return true;
}

bool InputClass::ReadMouse()
{
	HRESULT result;

	m_lastMouseState = m_mouseState;

	// 마우스 장치 상태 읽기
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
		// Acquire 후에도 바로 상태를 못 읽을 수 있으므로 델타는 0으로 처리
		m_mouseXDelta = 0;
		m_mouseYDelta = 0;
		return true; // 또는 false로 처리하여 입력 실패를 알림
	}


	m_mouseXDelta = m_mouseState.lX; // lX, lY는 이미 상대 이동량임
	m_mouseYDelta = m_mouseState.lY;


	return true;
}

bool InputClass::IsMouseButtonDown(int button) const
{
	// m_mouseState.rgbButtons 배열은 마우스 버튼 상태를 담고 있습니다.
	// button 0: 좌클릭, 1: 우클릭, 2: 휠클릭
	if (button < 0 || button > 3) return false;

	// 0x80 비트가 켜져 있으면 버튼이 눌린 상태입니다.
	return (m_mouseState.rgbButtons[button] & 0x80) ? true : false;
}

bool InputClass::IsMouseButtonPressed(int button) const
{
	if (button < 0 || button > 3) return false;

	// 현재는 눌려있고(true), 이전 프레임에는 안 눌려있었다면(false) -> "눌리는 순간"
	bool current_state = (m_mouseState.rgbButtons[button] & 0x80);
	bool last_state = (m_lastMouseState.rgbButtons[button] & 0x80);

	return current_state && !last_state;
}