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

	// DirectInput �������̽� ����
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Ű���� ��ġ ����
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Ű���� ������ ���� ����
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// Ű���� ���� ���� ���� (���׶���, ����)
	// DISCL_NONEXCLUSIVE: �ٸ� ���ø����̼ǵ� Ű���� �Է��� ���� �� �ְ� ��
	// DISCL_FOREGROUND: ���ø����̼� â�� Ȱ��ȭ�Ǿ� ���� ���� �Է� ����
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}
	m_keyboard->Acquire();

	// ���콺 ��ġ ����
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// ���콺 ������ ���� ����
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		return false;
	}

	// ���콺 ���� ���� ���� (���׶���, ����)
	// DISCL_EXCLUSIVE: �� ���ø����̼Ǹ� ���콺 �Է��� ���������� ����
	// DISCL_NOWINKEY: Windows Ű ��Ȱ��ȭ (���� ����)
	// DISCL_FOREGROUND: ���ø����̼� â�� Ȱ��ȭ�Ǿ� ���� ���� �Է� ����
	// FPS ī�޶󿡼��� ���콺 Ŀ���� ����� ���� ���� ����ϴ� ���� �Ϲ����Դϴ�.
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(result))
	{
		return false;
	}

	// �ʱ� ���콺 ���� �б� (m_lastMouseState �ʱ�ȭ ����)
	if (m_mouse)
	{
		m_mouse->Acquire(); // ���콺 ��ġ ��� ����
		m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_lastMouseState);
	}


	return true;
}

void InputClass::Shutdown()
{
	// ���콺 ��ġ ����
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = nullptr;
	}

	// Ű���� ��ġ ����
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = nullptr;
	}

	// DirectInput �������̽� ����
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = nullptr;
	}
}

bool InputClass::Frame()
{
	bool result;

	// Ű���� ���� �б�
	result = ReadKeyboard();
	if (!result)
	{
		return false;
	}

	// ���콺 ���� �б�
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
		return true; // �Ǵ� false�� ó���Ͽ� �Է� ���и� �˸�
	}

	// GetDeviceState ���� �ÿ��� m_keys ������Ʈ
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

	// ���콺 ��ġ ���� �б�
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
		// Acquire �Ŀ��� �ٷ� ���¸� �� ���� �� �����Ƿ� ��Ÿ�� 0���� ó��
		m_mouseXDelta = 0;
		m_mouseYDelta = 0;
		return true; // �Ǵ� false�� ó���Ͽ� �Է� ���и� �˸�
	}


	m_mouseXDelta = m_mouseState.lX; // lX, lY�� �̹� ��� �̵�����
	m_mouseYDelta = m_mouseState.lY;


	return true;
}

bool InputClass::IsMouseButtonDown(int button) const
{
	// m_mouseState.rgbButtons �迭�� ���콺 ��ư ���¸� ��� �ֽ��ϴ�.
	// button 0: ��Ŭ��, 1: ��Ŭ��, 2: ��Ŭ��
	if (button < 0 || button > 3) return false;

	// 0x80 ��Ʈ�� ���� ������ ��ư�� ���� �����Դϴ�.
	return (m_mouseState.rgbButtons[button] & 0x80) ? true : false;
}

bool InputClass::IsMouseButtonPressed(int button) const
{
	if (button < 0 || button > 3) return false;

	// ����� �����ְ�(true), ���� �����ӿ��� �� �����־��ٸ�(false) -> "������ ����"
	bool current_state = (m_mouseState.rgbButtons[button] & 0x80);
	bool last_state = (m_lastMouseState.rgbButtons[button] & 0x80);

	return current_state && !last_state;
}