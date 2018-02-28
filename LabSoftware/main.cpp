
#include <stdlib.h> //- for exit()
#include <stdio.h>  //- for sprintf()
#include <string.h> //- for memset()

#ifdef _WIN32
#include "libs/glut.h"
#include <windows.h>
#pragma comment(lib, "winmm.lib") //- not required but have it in anyway
#pragma comment(lib, "libs/glut32.lib")
#elif __APPLE__
#include <GLUT/glut.h>
#elif __unix__ // all unices including  __linux__
#include <GL/glut.h>
#endif

//====== Macros and Defines =========

#define FRAME_WIDE 1000
#define FRAME_HIGH 600

//====== Structs & typedefs =========
typedef unsigned char BYTE;
struct POINT2D
{
	int x, y;
};

//====== Global Variables ==========
BYTE pFrameL[FRAME_WIDE * FRAME_HIGH * 3];
BYTE pFrameR[FRAME_WIDE * FRAME_HIGH * 3];
int shade = 0;
POINT2D xypos = {0, 0};
int stereo = 0;
int eyes = 10;

//===== Forward Declarations ========
void ClearScreen();
void DrawFrame();
void Interlace(BYTE *pL, BYTE *pR);
void PlaySoundEffect(char *filename);
void BuildFrame(BYTE *pFrame, int view);
void OnIdle(void);
void OnDisplay(void);
void reshape(int w, int h);
void OnMouse(int button, int state, int x, int y);
void OnKeypress(unsigned char key, int x, int y);

////////////////////////////////////////////////////////
// Program Entry Poinr
////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	//-- setup GLUT --
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); //GLUT_3_2_CORE_PROFILE |
	glutInitWindowSize(FRAME_WIDE, FRAME_HIGH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	/*
#ifdef WIN32
	//- eliminate flashing --
	typedef void (APIENTRY * PFNWGLEXTSWAPCONTROLPROC) (int i);
	PFNWGLEXTSWAPCONTROLPROC wglSwapControl = NULL;
	wglSwapControl = (PFNWGLEXTSWAPCONTROLPROC) wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapControl != NULL) wglSwapControl(1); 
#endif
*/

	//--- set openGL state --
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//-- register call back functions --
	glutIdleFunc(OnIdle);
	glutDisplayFunc(OnDisplay);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(OnKeypress);
	glutMouseFunc(OnMouse);

	//-- run the program
	glutMainLoop();
	return 0;
}

////////////////////////////////////////////////////////
// Event Handers
////////////////////////////////////////////////////////

void OnIdle(void)
{
	DrawFrame();
	glutPostRedisplay();
}

void OnDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2i(0, 0);
	glDrawPixels(FRAME_WIDE, FRAME_HIGH, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte *)pFrameR);
	glutSwapBuffers();
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OnMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		PlaySoundEffect("Laser.wav");
		if (++shade > 16)
			shade = 0;
	}
}

void OnKeypress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ' ':
		xypos.x = xypos.y = 0;
		break;
	case 's':
		stereo ^= 1, eyes = 10;
		break;
	case ']':
		eyes++;
		break;
	case '[':
		eyes--;
		break;
	case 27:
		exit(0);
	}
	PlaySoundEffect("Whoosh.wav");
}

////////////////////////////////////////////////////////
// Utility Functions
////////////////////////////////////////////////////////

void ClearScreen()
{
	memset(pFrameL, 0, FRAME_WIDE * FRAME_HIGH * 3);
	memset(pFrameR, 0, FRAME_WIDE * FRAME_HIGH * 3);
}

void Interlace(BYTE *pL, BYTE *pR)
{
	int rowlen = 3 * FRAME_WIDE;
	for (int y = 0; y < FRAME_HIGH; y += 2)
	{
		for (int x = 0; x < rowlen; x++)
			*pR++ = *pL++;
		pL += rowlen;
		pR += rowlen;
	}
}

void DrawFrame()
{
	ClearScreen();

	if (!stereo)
		BuildFrame(pFrameR, 0);
	else
	{
		BuildFrame(pFrameL, -eyes);
		BuildFrame(pFrameR, +eyes);
		Interlace((BYTE *)pFrameL, (BYTE *)pFrameR);
	}
}

void PlaySoundEffect(char *filename)
{
#ifdef _WIN32
	PlaySound(filename, NULL, SND_ASYNC | SND_FILENAME);
#else
	char command[80];
#ifdef __APPLE__
	sprintf(command, "afplay %s &", filename);
#else
	sprintf(command, "play %s &", filename);
#endif
	system(command);
#endif
}

//
// Draw Pixel Function
//

void SetPixel(BYTE *frame, int x, int y, char r, char g, char b)
{
	int fbPel = (x + FRAME_WIDE * y) * 3;
	frame[fbPel] = r;
	frame[fbPel + 1] = g;
	frame[fbPel + 2] = b;
}

////////////////////////////////////////////////////////
// Drawing Function
////////////////////////////////////////////////////////

void BuildFrame(BYTE *pFrame, int view)
{
	BYTE *screen = (BYTE *)pFrame; // use copy of screen pointer for safety

	for (int i = 0; i < 1000; i++)
	{
		SetPixel(screen, rand() % FRAME_WIDE, rand() % FRAME_HIGH, rand() % 255, rand() % 255, rand() % 255);
	}
}
