// This program is based on the online example of DirectXTutorial.com and was edited by
// Katharina Olze in 2016 to a simple visual stimulation via flickering chessboard patterns
// for SSVEP applications.
// The program shows a flickering chessboard pattern with the number of frames BLACK_CNT for the non-inverse 
// and WHITE_CNT for the inverse pattern.
// The size of the chessboard is defined by rectwidth and rectheight for each chessboard box and chessrows and chesscols.
// The colors of the chessboard are defined via rgb-Code in the update-graphics function (at line 238).
// The flickering has to be stopped manual, it will not end automatically.
//
// Katharina Olze, 2017 - 03 - 05


// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

// read the screen resolution automatically // or #define the screen resolution
int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN); //#define SCREEN_WIDTH  1920
int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN); //#define SCREEN_HEIGHT 1080

#define BLACK_CNT 1 
#define WHITE_CNT 1

#define RECTWIDTH 7.5f
#define RECTHEIGHT 7.5f


#define CHESSROWS 10
#define CHESSCOLS 10

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declarations
struct d3dconstruct {
	LPDIRECT3D9 d3d; // the pointer to our Direct3D interface
	LPDIRECT3DDEVICE9 d3ddev; // the pointer to the device class
	LPDIRECT3DVERTEXBUFFER9 v_buffer; // the pointer to the vertex buffer
	LPDIRECT3DVERTEXBUFFER9 v_buffer_inverted; // the pointer to the rect with inverted color
	int v_buffer_primitiveCNT;
	int v_buffer_vertexCNT;
} typedef d3dconstruct;


// define our custom vertex format
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// define our vertex for this format
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // from the D3DFVF_XYZRHW flag
	DWORD color;    // from the D3DFVF_DIFFUSE flag
} typedef CUSTOMVERTEX;

						  // function prototypes
void initD3D(HWND hWnd, d3dconstruct* constructp); // sets up and initializes Direct3D
void render_frame(d3dconstruct* constructp, int r, int g, int b, bool invert); // renders a single frame
void cleanD3D(d3dconstruct* constructp); // closes Direct3D and releases memory
void init_graphics(d3dconstruct* constructp);    // 3D declarations
void update_graphics(d3dconstruct* constructp, D3DCOLOR, D3DCOLOR);    // 3D declarations

					 // the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	// wc.hbrBackground = (HBRUSH)COLOR_WINDOW;    // not needed any more
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		L"WindowClass",
		L"Our Direct3D Program",
		WS_EX_TOPMOST | WS_POPUP,    // fullscreen values
		0, 0,    // the starting x and y positions should be 0
		SCREEN_WIDTH, SCREEN_HEIGHT,    // set the window to fullscreen (or defined size)
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	d3dconstruct construct;

	// set up and initialize Direct3D
	initD3D(hWnd, &construct);

	// enter the main loop:
	int renderBWcnt = 0;
	bool renderBLACK = false;

	MSG msg;

	while (TRUE)
	{
		
		renderBWcnt--;

		if (renderBWcnt <= 0) {
			renderBLACK = !renderBLACK;
			if (renderBLACK) {
				renderBWcnt = BLACK_CNT; // Divisor of black
			}
			else {
				renderBWcnt = WHITE_CNT; // Divisor of white
			}
		}


		
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		if (renderBLACK) {
			//render_frame(&construct, 0, 0, 0, renderBLACK);
			render_frame(&construct, 0, 0, 0, renderBLACK);
		}
		else {
			//render_frame(&construct, 255, 255, 255, renderBLACK);
			render_frame(&construct, 0, 0, 0, renderBLACK);
		}
		
		
		

		
	}

	// clean up DirectX and COM
	cleanD3D(&construct);

	return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd, d3dconstruct* constructp)
{
	
	(*constructp).d3d = Direct3DCreate9(D3D_SDK_VERSION); // create the Direct3D interface
	(*constructp).v_buffer = NULL; // the pointer to the vertex buffer

	UINT uiDispModeCount = (*constructp).d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
	D3DDISPLAYMODE* dispModeList = new D3DDISPLAYMODE[uiDispModeCount];
	boolean* dispModeListValid = new boolean[uiDispModeCount];

	for (UINT k = 0; k < uiDispModeCount; k++) {
		if ((*constructp).d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, k, &dispModeList[k]) == D3D_OK) {
			// this is a valid configuration.
			dispModeListValid[k] = true;
		} else {
			dispModeListValid[k] = false;
		}


	}

	D3DPRESENT_PARAMETERS d3dpp; // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = false;    // program fullscreen, not windowed
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = SCREEN_WIDTH;    // set the width of the buffer
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;    // set the height of the buffer
	//d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//d3dpp.EnableAutoDepthStencil = TRUE;    // automatically run the z-buffer for us
	//d3dpp.AutoDepthStencilFormat = D3DFMT_D16;    // 16-bit pixel format for the z-buffer

	bool bVsyncEnabled = true;

	if (bVsyncEnabled)
	{
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	}
	else
	{
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}


											   // create a device class using this information and the info from the d3dpp stuct
	(*constructp).d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		//D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		&((*constructp).d3ddev));

	init_graphics(constructp);    // call the function to initialize the triangle
	update_graphics(constructp, D3DCOLOR_XRGB(0, 0, 0), D3DCOLOR_XRGB(255, 255, 255)); // colors of the chessboard

	(*constructp).d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);    // turn off the 3D lighting
	(*constructp).d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);    // both sides of the triangles
	(*constructp).d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);    // turn on the z-buffer
}


void init_graphics(d3dconstruct* constructp)
{
	// create three vertices using the CUSTOMVERTEX struct built earlier
	/*CUSTOMVERTEX vertices[] =
	{
		{ 320.0f, 50.0f, 0.5f, D3DCOLOR_XRGB(0, 0, 255), },
		{ 520.0f, 400.0f, 0.5f, D3DCOLOR_XRGB(0, 255, 0), },
		{ 120.0f, 400.0f, 0.5f, D3DCOLOR_XRGB(255, 0, 0), },
	};
	(*constructp).v_buffer_primitiveCNT = 1; // the number of primitives we have specified
	(*constructp).v_buffer_vertexCNT = 3; // the number of vertices we have specified
	
	*/

	// FLOAT x, y, z, rhw; DWORD color;    // format of custom vertex

	// rectangle
	float topleftx = -RECTWIDTH/2;
	float toplefty = -RECTHEIGHT / 2;

	float width  = RECTWIDTH;
	float height = RECTHEIGHT;

	CUSTOMVERTEX vertices[] =
	{
		{ topleftx, toplefty+height, 0.5f, D3DCOLOR_XRGB(255, 0, 0), },
		{ topleftx, toplefty, 0.5f,  D3DCOLOR_XRGB(0, 0, 255), },
		{ topleftx + width, toplefty + height, 0.5f,  D3DCOLOR_XRGB(0, 255, 0), },
		{ topleftx + width, toplefty, 0.5f, D3DCOLOR_XRGB(0, 255, 255), },
		
	};
	(*constructp).v_buffer_primitiveCNT = 2; // the number of primitives we have specified
	(*constructp).v_buffer_vertexCNT = 4; // the number of vertices we have specified

	// create the vertex and store the pointer into v_buffer, which is created globally
	(*constructp).d3ddev->CreateVertexBuffer((*constructp).v_buffer_vertexCNT * sizeof(CUSTOMVERTEX),
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&((*constructp).v_buffer),
		NULL);

	VOID* pVoid;    // the void pointer
	(*constructp).v_buffer->Lock(0, 0, (void**)&pVoid, 0);    // lock the vertex buffer
	memcpy(pVoid, vertices, sizeof(vertices));    // copy the vertices to the locked buffer: destination, source, size
	(*constructp).v_buffer->Unlock();    // unlock the vertex buffer


	(*constructp).d3ddev->CreateVertexBuffer((*constructp).v_buffer_vertexCNT * sizeof(CUSTOMVERTEX),
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&((*constructp).v_buffer_inverted),
		NULL);

	(*constructp).v_buffer_inverted->Lock(0, 0, (void**)&pVoid, 0);    // lock the vertex buffer
	memcpy(pVoid, vertices, sizeof(vertices));    // copy the vertices to the locked buffer: destination, source, size
	(*constructp).v_buffer_inverted->Unlock();    // unlock the vertex buffer
}

void update_graphics(d3dconstruct* constructp, D3DCOLOR color, D3DCOLOR color2) {
	// rectangle
	/*
	float topleftx = -RECTWIDTH / 2;
	float toplefty = -RECTHEIGHT / 2;

	float width = RECTWIDTH;
	float height = RECTHEIGHT;
	*/
	float topleftx = -RECTWIDTH / 2;
	float toplefty = -RECTHEIGHT / 2;

	float width = RECTWIDTH;
	float height = RECTHEIGHT;
	

	// this must have the same number of primitives and vertices as in create_graphics
	CUSTOMVERTEX vertices[] =
	{
		{ topleftx, toplefty + height, 0.5f, color, },
		{ topleftx, toplefty, 0.5f, color, },
		{ topleftx + width, toplefty + height, 0.5f, color, },
		{ topleftx + width, toplefty, 0.5f, color, },

	};


	CUSTOMVERTEX vertices_inverted[] =
	{
		{ topleftx, toplefty + height, 0.5f, color2, },
		{ topleftx, toplefty, 0.5f, color2, },
		{ topleftx + width, toplefty + height, 0.5f, color2, },
		{ topleftx + width, toplefty, 0.5f, color2, },

	};

	VOID* pVoid;    // the void pointer
	(*constructp).v_buffer->Lock(0, 0, (void**)&pVoid, 0);    // lock the vertex buffer
	memcpy(pVoid, vertices, sizeof(vertices));    // copy the vertices to the locked buffer: destination, source, size
	(*constructp).v_buffer->Unlock();    // unlock the vertex buffer

//	VOID* pVoid;    // the void pointer
	(*constructp).v_buffer_inverted->Lock(0, 0, (void**)&pVoid, 0);    // lock the vertex buffer
	memcpy(pVoid, vertices_inverted, sizeof(vertices_inverted));    // copy the vertices to the locked buffer: destination, source, size
	(*constructp).v_buffer_inverted->Unlock();    // unlock the vertex buffer
	
}


// this is the function used to render a single frame
void render_frame(d3dconstruct* constructp, int r, int g, int b, bool invert)
{
	LPDIRECT3DDEVICE9 d3ddev = (*constructp).d3ddev;
	
	// clear the window to a deep blue
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(r, g, b), 1.0f, 0);
	d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	//d3ddev->GetBackBuffer()

	d3ddev->BeginScene();    // begins the 3D scene

	// do 3D rendering on the back buffer here


	// update the vertex color with the inverse of the background: todo: maybe there is a simpler way than recreating all vertices
	// update_graphics(constructp, D3DCOLOR_XRGB(255 - r, 255 - g, 255 - b));

	// select which vertex format we are using
	d3ddev->SetFVF(CUSTOMFVF);


	// set the view transform
	D3DXMATRIX matView;    // the view transform matrix
	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3(0.0f, 0.0f, 100.0f),   // the camera position
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),    // the look-at position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));    // the up direction
	d3ddev->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView


												   // set the projection transform
	D3DXMATRIX matProjection;    // the projection transform matrix
	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(45),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
		1.0f,    // the near view-plane
		100.0f);    // the far view-plane
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);     // set the projection



	

	// copy the vertex buffer to the back buffer
	//d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, (*constructp).v_buffer_primitiveCNT);

	
	D3DXMATRIX matTranslateX;
	D3DXMATRIX matTranslateY;
	D3DXMATRIX matTranslateCombined;
	D3DXMATRIX matRotateY;    // a matrix to store the rotation for each triangle
	UINT yidx, xidx;

	D3DXMatrixRotationY(&matRotateY, 0);    // the front side

	// the rect is centered.
	// the total width of the cessboard is CHESSCOLS * RECTWIDTH, of which 1/RECTWIDTH is right of the center, and (CHESSCOLS-1/2) * RECTWIDTH is left.
	// center by moving it to the right by (CHESSCOLS-1)/2 * RECTWIDTH
	float xoffset = - (((float)(CHESSCOLS - 1))/2.0f)*RECTWIDTH;
	float yoffset = -(((float)(CHESSROWS - 1)) / 2.0f)*RECTHEIGHT;

	// create a chessboard:
	for (yidx = 0; yidx < CHESSROWS; yidx++) { // iterates the cols
		for (xidx = 0; xidx < CHESSCOLS; xidx++) { // iterates the rows for every column index
			
			//D3DXMatrixTranslation(&matTranslateX, xoffset+((k*2)+ (m % 2)) *2* RECTWIDTH, 0.0f, 0.0f);
			//D3DXMatrixTranslation(&matTranslateY, 0.0f, yoffset+m*2*RECTHEIGHT, 0.0f);

			//d3ddev->SetTransform(D3DTS_WORLD, &(matTranslateX + matTranslateY));

			// draw only if 
			// a) x index is even and y index is even
			// b) x index is odd and y index is odd
			if (   (((xidx % 2) == 0) && ((yidx % 2) == 0))
				|| (((xidx % 2) == 1) && ((yidx % 2) == 1))) {

				// select the vertex buffer to display
				if (!invert) {
					d3ddev->SetStreamSource(0, (*constructp).v_buffer, 0, sizeof(CUSTOMVERTEX));
				} else {
					d3ddev->SetStreamSource(0, (*constructp).v_buffer_inverted, 0, sizeof(CUSTOMVERTEX));
				}

				D3DXMatrixTranslation(&matTranslateX, xoffset + ((float)(1 * xidx))* RECTWIDTH, 0.0f, 0.0f);
				D3DXMatrixTranslation(&matTranslateY, 0.0f, yoffset + ((float)(1 * yidx)) * RECTHEIGHT, 0.0f);

				matTranslateCombined = matTranslateX * matTranslateY;

				d3ddev->SetTransform(D3DTS_WORLD, &(matTranslateCombined));

				d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, (*constructp).v_buffer_primitiveCNT); // draw the transformed primitive
			} else {

				// select the vertex buffer to display
				if (invert) {
					d3ddev->SetStreamSource(0, (*constructp).v_buffer, 0, sizeof(CUSTOMVERTEX));
				} else {
					d3ddev->SetStreamSource(0, (*constructp).v_buffer_inverted, 0, sizeof(CUSTOMVERTEX));
				}

				D3DXMatrixTranslation(&matTranslateX, xoffset + ((float)(1 * xidx))* RECTWIDTH, 0.0f, 0.0f);
				D3DXMatrixTranslation(&matTranslateY, 0.0f, yoffset + ((float)(1 * yidx)) * RECTHEIGHT, 0.0f);

				matTranslateCombined = matTranslateX * matTranslateY;

				d3ddev->SetTransform(D3DTS_WORLD, &(matTranslateCombined));

				d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, (*constructp).v_buffer_primitiveCNT); // draw the transformed primitive
			}
			
		}
	}
	


	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen

	(*constructp).d3ddev = d3ddev;
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(d3dconstruct* constructp)
{
	(*constructp).d3ddev->Release(); // close and release the 3D device
	(*constructp).d3d->Release(); // close and release Direct3D
}