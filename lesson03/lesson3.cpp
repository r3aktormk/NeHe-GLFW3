//
// This code was created by Jeff Molofee '99
//
// If you've found this code useful, please let me know.
//
// Visit me at www.demonews.com/hosted/nehe
//

#include <stdio.h>      // Standard Input/Output Header File
#include <stdlib.h>     // Standard Library Files Header File

#include <gl\gl.h>		// Header File For The OpenGL32 Library
#include <gl\glu.h>		// Header File For The GLu32 Library
#include <GLFW\glfw3.h>

BOOL	keys[256];		// Array Used For The Keyboard Routine

GLvoid InitGL(GLsizei Width, GLsizei Height)	// This Will Be Called Right After The GL Window Is Created
{
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);		// This Will Clear The Background Color To *dark Blue
    glClearDepth(1.0);							// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);						// The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);					// Enables Depth Testing
    glShadeModel(GL_SMOOTH);					// Enables Smooth Color Shading

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();							// Reset The Projection Matrix

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

    glMatrixMode(GL_MODELVIEW);
}

GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)
{
    if (Height==0)								// Prevent A Divide By Zero If The Window Is Too Small
        Height=1;

    glViewport(0, 0, Width, Height);			// Reset The Current Viewport And Perspective Transformation

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
}

GLvoid DrawGLScene(GLvoid)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
    glLoadIdentity();										// Reset The View

    glTranslatef(-1.5f,0.0f,-6.0f);							// Move Into The Screen

    glBegin(GL_POLYGON);									// Start Drawing A Polygon
    glColor3f(1.0f,0.0f,0.0f);							// Set Top Point Of Polygon To Red
    glVertex3f( 0.0f, 1.0f, 0.0f);						// First Point Of The Polygon (Triangle)
    glColor3f(0.0f,1.0f,0.0f);							// Set Left Point Of Polygon To Green
    glVertex3f(-1.0f,-1.0f, 0.0f);						// Second Point Of The Polygon
    glColor3f(0.0f,0.0f,1.0f);							// Set Right Point Of Polygon To Blue
    glVertex3f( 1.0f,-1.0f, 0.0f);						// Third Point Of The Polygon
    glEnd();												// Done Drawing The Polygon

    glTranslatef(3.0f,0.0f,0.0f);							// Move Right Three Units

    glColor3f(0.5f,0.5f,1.0f);								// Set The Color To A Nice Blue Shade
    glBegin(GL_QUADS);										// Start Drawing A Quad
    glVertex3f(-1.0f, 1.0f, 0.0f);						// Top Left Of The Quad
    glVertex3f( 1.0f, 1.0f, 0.0f);						// Top Right Of The Quad
    glVertex3f( 1.0f,-1.0f, 0.0f);						// Bottom Right Of The Quad
    glVertex3f(-1.0f,-1.0f, 0.0f);						// Bottom Left Of The Quad
    glEnd();												// Done Drawing The Quad
}




static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

}


int main(int argc, char *argv[])
{

    GLFWwindow* window;

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

    while (!glfwWindowShouldClose(window))
    {

        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);


        DrawGLScene();


        glfwSwapBuffers(window);
        glfwPollEvents();

        //glClearColor(0.3f, 0.0f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
