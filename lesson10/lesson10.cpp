//
// This Code Was Created By Lionel Brits / Jeff Molofee '99
//
// Visit NeHe Productions At www.demonews.com/hosted/nehe
//

#include <windows.h>  // Header File For Windows
#include <gl\gl.h>	// Header File For The OpenGL32 Library
#include <gl\glu.h>   // Header File For The GLu32 Library
#include <gl\glaux.h> // Header File For The GLaux Library

#include <stdio.h>  // Standard Input/Output Header File
#include <stdlib.h> // Standard Library Files Header File
#include <math.h>   // Math Library Header File
#include <string.h> // String Header
#include "model.h"  // Model Header (custom)

static HGLRC hRC; // Permanent Rendering Context
static HDC hDC;   // Private GDI Device Context

char *worldfile = "data\\world.txt";

BOOL keys[256]; // Array Used For The Keyboard Routine
BOOL light;		// Lighting ON/OFF
BOOL blend;		// Blending ON/OFF
BOOL bp;		// B Pressed?
BOOL lp;		// L Pressed?
BOOL fp;		// F Pressed?

GLfloat xrot;   // X Rotation
GLfloat yrot;   // Y Rotation
GLfloat xspeed; // X Rotation Speed
GLfloat yspeed; // Y Rotation Speed

GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;

GLfloat lookupdown = 0.0f;
const float piover180 = 0.0174532925f;

float heading, xpos, zpos;

GLfloat camx = 0, camy = 0, camz = 0; // Camera Location
GLfloat therotate;

GLfloat z = 0.0f; // Depth Into The Screen

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};  // Ambient Light
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  // Diffuse Light
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f}; // Light Position

GLuint filter;	 // Which Filter To Use
GLuint texture[3]; // Storage for 3 textures

// Our Model Goes Here:
SECTOR sector1;

// Converts Degrees To Radians. There Are 2 PI Radians In 360 Degrees.
float rad(float angle)
{
	return angle * piover180;
}

void readstr(FILE *f, char *string)
{
	do
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
	return;
}

void SetupWorld()
{
	float x, y, z, u, v;
	int numtriangles;
	FILE *filein;
	char oneline[255];
	filein = fopen(worldfile, "rt");

	readstr(filein, oneline);
	sscanf(oneline, "NUMPOLLIES %d\n", &numtriangles);

	sector1.triangle = new TRIANGLE[numtriangles];
	sector1.numtriangles = numtriangles;
	for (int loop = 0; loop < numtriangles; loop++)
	{
		for (int vert = 0; vert < 3; vert++)
		{
			readstr(filein, oneline);
			sscanf(oneline, "%f %f %f %f %f %f %f", &x, &y, &z, &u, &v);
			sector1.triangle[loop].vertex[vert].x = x;
			sector1.triangle[loop].vertex[vert].y = y;
			sector1.triangle[loop].vertex[vert].z = z;
			sector1.triangle[loop].vertex[vert].u = u;
			sector1.triangle[loop].vertex[vert].v = v;
		}
	}
	fclose(filein);
	return;
}

// Load Bitmaps And Convert To Textures
GLvoid LoadGLTextures()
{
	// Load Texture
	AUX_RGBImageRec *texture1;

	texture1 = auxDIBImageLoad("Data/mud.bmp");
	if (!texture1)
	{
		exit(1);
	}

	// Create Nearest Filtered Texture
	glGenTextures(3, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture1->sizeX, texture1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1->data);

	// Create Linear Filtered Texture
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture1->sizeX, texture1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1->data);

	// Create MipMapped Texture
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture1->sizeX, texture1->sizeY, GL_RGB, GL_UNSIGNED_BYTE, texture1->data);
};

GLvoid InitGL(GLsizei Width, GLsizei Height) // This Will Be Called Right After The GL Window Is Created
{
	LoadGLTextures();		 // Load The Texture(s)
	glEnable(GL_TEXTURE_2D); // Enable Texture Mapping

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Set The Blending Function For Translucency
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // This Will Clear The Background Color To Black
	glClearDepth(1.0);					  // Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);				  // The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);			  // Enables Depth Testing
	glShadeModel(GL_SMOOTH);			  // Enables Smooth Color Shading

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // Reset The Projection Matrix

	gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f); // Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);
}

GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)
{
	if (Height == 0) // Prevent A Divide By Zero If The Window Is Too Small
		Height = 1;

	glViewport(0, 0, Width, Height); // Reset The Current Viewport And Perspective Transformation

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

GLvoid DrawGLScene(GLvoid)
{
	GLfloat x_m, y_m, z_m, u_m, v_m;
	GLfloat xtrans, ztrans, ytrans;
	GLfloat sceneroty;

	xtrans = -xpos;
	ztrans = -zpos;
	ytrans = -walkbias - 0.25f;

	sceneroty = 360.0f - yrot;

	int numtriangles;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View

	glRotatef(lookupdown, 1.0f, 0, 0);
	glRotatef(sceneroty, 0, 1.0f, 0);

	glTranslatef(xtrans, ytrans, ztrans);
	glBindTexture(GL_TEXTURE_2D, texture[filter]);

	numtriangles = sector1.numtriangles;

	// Process Each Triangle

	for (int loop_m = 0; loop_m < numtriangles; loop_m++)
	{
		glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 0.0f, 1.0f);
		x_m = sector1.triangle[loop_m].vertex[0].x;
		y_m = sector1.triangle[loop_m].vertex[0].y;
		z_m = sector1.triangle[loop_m].vertex[0].z;
		u_m = sector1.triangle[loop_m].vertex[0].u;
		v_m = sector1.triangle[loop_m].vertex[0].v;
		glTexCoord2f(u_m, v_m);
		glVertex3f(x_m, y_m, z_m);

		x_m = sector1.triangle[loop_m].vertex[1].x;
		y_m = sector1.triangle[loop_m].vertex[1].y;
		z_m = sector1.triangle[loop_m].vertex[1].z;
		u_m = sector1.triangle[loop_m].vertex[1].u;
		v_m = sector1.triangle[loop_m].vertex[1].v;
		glTexCoord2f(u_m, v_m);
		glVertex3f(x_m, y_m, z_m);

		x_m = sector1.triangle[loop_m].vertex[2].x;
		y_m = sector1.triangle[loop_m].vertex[2].y;
		z_m = sector1.triangle[loop_m].vertex[2].z;
		u_m = sector1.triangle[loop_m].vertex[2].u;
		v_m = sector1.triangle[loop_m].vertex[2].v;
		glTexCoord2f(u_m, v_m);
		glVertex3f(x_m, y_m, z_m);
		glEnd();
	}
}

LRESULT CALLBACK WndProc(HWND hWnd,
						 UINT message,
						 WPARAM wParam,
						 LPARAM lParam)
{
	RECT Screen; // Used Later On To Get The Size Of The Window
	GLuint PixelFormat;
	static PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
			1,							   // Version Number (?)
			PFD_DRAW_TO_WINDOW |		   // Format Must Support Window
				PFD_SUPPORT_OPENGL |	   // Format Must Support OpenGL
				PFD_DOUBLEBUFFER,		   // Must Support Double Buffering
			PFD_TYPE_RGBA,				   // Request An RGBA Format
			16,							   // Select A 16Bit Color Depth
			0, 0, 0, 0, 0, 0,			   // Color Bits Ignored (?)
			0,							   // No Alpha Buffer
			0,							   // Shift Bit Ignored (?)
			0,							   // No Accumulation Buffer
			0, 0, 0, 0,					   // Accumulation Bits Ignored (?)
			16,							   // 16Bit Z-Buffer (Depth Buffer)
			0,							   // No Stencil Buffer
			0,							   // No Auxiliary Buffer (?)
			PFD_MAIN_PLANE,				   // Main Drawing Layer
			0,							   // Reserved (?)
			0, 0, 0						   // Layer Masks Ignored (?)
		};

	switch (message) // Tells Windows We Want To Check The Message
	{
	case WM_CREATE:
		hDC = GetDC(hWnd);							// Gets A Device Context For The Window
		PixelFormat = ChoosePixelFormat(hDC, &pfd); // Finds The Closest Match To The Pixel Format We Set Above

		if (!PixelFormat)
		{
			MessageBox(0, "Can't Find A Suitable PixelFormat.", "Error", MB_OK | MB_ICONERROR);
			PostQuitMessage(0); // This Sends A 'Message' Telling The Program To Quit
			break;				// Prevents The Rest Of The Code From Running
		}

		if (!SetPixelFormat(hDC, PixelFormat, &pfd))
		{
			MessageBox(0, "Can't Set The PixelFormat.", "Error", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
			break;
		}

		hRC = wglCreateContext(hDC);
		if (!hRC)
		{
			MessageBox(0, "Can't Create A GL Rendering Context.", "Error", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
			break;
		}

		if (!wglMakeCurrent(hDC, hRC))
		{
			MessageBox(0, "Can't activate GLRC.", "Error", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
			break;
		}

		GetClientRect(hWnd, &Screen);
		InitGL(Screen.right, Screen.bottom);
		break;

	case WM_DESTROY:
	case WM_CLOSE:
		ChangeDisplaySettings(NULL, 0);

		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);
		ReleaseDC(hWnd, hDC);

		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		keys[wParam] = TRUE;
		break;

	case WM_KEYUP:
		keys[wParam] = FALSE;
		break;

	case WM_SIZE:
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));
		break;

	default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (0);
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	MSG msg;	 // Windows Message Structure
	WNDCLASS wc; // Windows Class Structure Used To Set Up The Type Of Window
	HWND hWnd;   // Storage For Window Handle

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL WinClass";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "Failed To Register The Window Class.", "Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	hWnd = CreateWindow(
		"OpenGL WinClass",
		"Jeff Molofee's GL Code Tutorial ... NeHe '99", // Title Appearing At The Top Of The Window

		WS_POPUP |
			WS_CLIPCHILDREN |
			WS_CLIPSIBLINGS,

		0, 0,	 // The Position Of The Window On The Screen
		640, 480, // The Width And Height Of The WIndow

		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
	{
		MessageBox(0, "Window Creation Error.", "Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	DEVMODE dmScreenSettings; // Developer Mode

	memset(&dmScreenSettings, 0, sizeof(DEVMODE));			  // Clear Room To Store Settings
	dmScreenSettings.dmSize = sizeof(DEVMODE);				  // Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = 640;						  // Screen Width
	dmScreenSettings.dmPelsHeight = 480;					  // Screen Height
	dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT; // Pixel Mode
	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN); // Switch To Full Screen

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	wglMakeCurrent(hDC, hRC);

	SetupWorld();

	while (1)
	{
		// Process All Messages
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				return TRUE;
			}
		}

		DrawGLScene();
		SwapBuffers(hDC);
		if (keys[VK_ESCAPE])
			SendMessage(hWnd, WM_CLOSE, 0, 0);

		if (keys['B'] && !bp)
		{
			bp = TRUE;
			blend = !blend;
			if (!blend)
			{
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
			}
			else
			{
				glEnable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);
			}
		}
		if (!keys['B'])
		{
			bp = FALSE;
		}

		if (keys['F'] && !fp)
		{
			fp = TRUE;
			filter += 1;
			if (filter > 2)
			{
				filter = 0;
			}
		}
		if (!keys['F'])
		{
			fp = FALSE;
		}

		if (keys['L'] && !lp)
		{
			lp = TRUE;
			light = !light;
			if (!light)
			{
				glDisable(GL_LIGHTING);
			}
			else
			{
				glEnable(GL_LIGHTING);
			}
		}
		if (!keys['L'])
		{
			lp = FALSE;
		}

		if (keys[VK_PRIOR])
		{
			z -= 0.02f;
		}

		if (keys[VK_NEXT])
		{
			z += 0.02f;
		}

		if (keys[VK_UP])
		{

			xpos -= (float)sin(heading * piover180) * 0.05f;
			zpos -= (float)cos(heading * piover180) * 0.05f;
			if (walkbiasangle >= 359.0f)
				walkbiasangle = 0.0f;
			else
				walkbiasangle += 10;

			walkbias = (float)sin(walkbiasangle * piover180) / 20.0f;
		}

		if (keys[VK_DOWN])
		{
			xpos += (float)sin(heading * piover180) * 0.05f;
			zpos += (float)cos(heading * piover180) * 0.05f;
			if (walkbiasangle <= 1.0f)
				walkbiasangle = 359.0f;
			else
				walkbiasangle -= 10;
			walkbias = (float)sin(walkbiasangle * piover180) / 20.0f;
		}

		if (keys[VK_RIGHT])
		{
			heading -= 1.0f;
			yrot = heading;
		}

		if (keys[VK_LEFT])
		{
			heading += 1.0f;
			yrot = heading;
		}

		if (keys[VK_PRIOR])
		{
			lookupdown -= 1.0f;
		}

		if (keys[VK_NEXT])
		{
			lookupdown += 1.0f;
		}
	}
}
