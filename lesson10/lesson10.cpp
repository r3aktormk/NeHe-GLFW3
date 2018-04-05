//
// This Code Was Created By Lionel Brits / Jeff Molofee '99
//
// Visit NeHe Productions At www.demonews.com/hosted/nehe
//

#include <stdio.h>  // Standard Input/Output Header File
#include <stdlib.h> // Standard Library Files Header File
#include <math.h>   // Math Library Header File
#include <string.h> // String Header

#include <gl\gl.h>   // Header File For The OpenGL32 Library
#include <gl\glu.h>  // Header File For The GLu32 Library

#include "model.h"  // Model Header (custom)

#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static HGLRC hRC; // Permanent Rendering Context
static HDC hDC;   // Private GDI Device Context

const char *worldfile = "data\\world.txt";

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
	int w, h, comp;
	unsigned char* image =
	stbi_load("Data/mud.bmp", &w, &h, &comp, STBI_rgb);

	// Create Nearest Filtered Texture
	glGenTextures(3, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	// Create Linear Filtered Texture
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	// Create MipMapped Texture
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, image);
};


GLvoid InitGL(GLsizei Width, GLsizei Height) // This Will Be Called Right After The GL Window Is Created
{
	LoadGLTextures();							// Load The Texture(s)
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

static void error_callback(int error, const char *description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

int main(int argc, char *argv[])
{

	GLFWwindow *window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	window = glfwCreateWindow(640, 480, "NeHe GLFW3", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	InitGL(640, 480);
	SetupWorld();

	while (!glfwWindowShouldClose(window))
	{

		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);

		glClearColor(0.3f, 0.0f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawGLScene();

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

		if (keys[GLFW_KEY_W])
		{

			xpos -= (float)sin(heading * piover180) * 0.05f;
			zpos -= (float)cos(heading * piover180) * 0.05f;
			if (walkbiasangle >= 359.0f)
				walkbiasangle = 0.0f;
			else
				walkbiasangle += 10;

			walkbias = (float)sin(walkbiasangle * piover180) / 20.0f;
		}

		if (keys[GLFW_KEY_S])
		{
			xpos += (float)sin(heading * piover180) * 0.05f;
			zpos += (float)cos(heading * piover180) * 0.05f;
			if (walkbiasangle <= 1.0f)
				walkbiasangle = 359.0f;
			else
				walkbiasangle -= 10;
			walkbias = (float)sin(walkbiasangle * piover180) / 20.0f;
		}

		if (keys[GLFW_KEY_D])
		{
			heading -= 1.0f;
			yrot = heading;
		}

		if (keys[GLFW_KEY_A])
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

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
}
