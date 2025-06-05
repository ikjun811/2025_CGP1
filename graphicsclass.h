////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "lightshaderclass.h"
#include "lightclass.h"

#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "textclass.h"


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass()
	{
		m_D3D = 0;
		m_Camera = 0;

		m_Model[0] = 0;
		m_Model[1] = 0;
		m_Model[2] = 0;
		m_Model[3] = 0;
		m_Model[4] = 0;
		m_Model[5] = 0;
		m_Model[6] = 0;
		m_Model[7] = 0;
		m_Model[8] = 0;
		m_Model[9] = 0;
		m_Model[10] = 0;
		m_Model[11] = 0;
		m_Model[12] = 0;
		m_Model[13] = 0;
		m_Model[14] = 0;

		m_LightShader = 0;
		m_Light1 = 0;
		m_Light2 = 0;
		m_Light3 = 0;


		m_TextureShader = 0;
		m_Bitmap = 0;

		m_Text = 0;

		//직선이동
		m_ObjectStartPosition = 50.0f;
		m_ObjectZOffset = 0.0f;
		m_ObjectSpeed = 0.05f;
		m_ObjectMovingForward = true;
	}
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(int fps, int cpu, CameraClass* gameCamera);


private:
	bool Render(float rotation, CameraClass* gameCamera);

	float m_ObjectSpeed;
	float m_ObjectZOffset;
	bool m_ObjectMovingForward;
	float m_ObjectStartPosition;

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model[15];

	TextureShaderClass* m_TextureShader;
	BitmapClass* m_Bitmap;

	TextClass* m_Text;

	LightShaderClass* m_LightShader;
	LightClass* m_Light1, * m_Light2, * m_Light3, * m_Light4;
};

#endif