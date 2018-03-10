
#include <stdlib.h> //- for exit()
#include <stdio.h>  //- for sprintf()
#include <string.h> //- for memset()
#include <math.h>	//- for pow()
#include <unistd.h>

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
	glPixelZoom(1,-1);
	glRasterPos2i(0, FRAME_HIGH-1);
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

class Point {
    public:
        int x;
        int y;

        Point() {
            x = 0; 
            y = 0;
        }

        Point(int x1, int y1) {
            x = x1;
            y = y1;
        }

        double slope(Point p2) {
            return p2.y == y ? 0 : (p2.x - x) / (double) (p2.y - y);
        }
};

//
// Draw Pixel Function
//

void SetPixel(BYTE *frame, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	int fbPel = (x + FRAME_WIDE * y) * 3;
	frame[fbPel] = r;
	frame[fbPel + 1] = g;
	frame[fbPel + 2] = b;
}

//
// Draw Line Function (DDA Method)
//
#define ROUND(x) ((int)(x+0.5))
void DrawLine(BYTE *frame, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	double x_inc = dx / (double)steps;
	double y_inc = dy / (double)steps;
	double x = x1;
	double y = y1;
	SetPixel(frame, ROUND(x), ROUND(y), r, g, b);
	for ( int i = 0; i < steps; i++, x+=x_inc, y+=y_inc ) {
		SetPixel(frame, ROUND(x), ROUND(y), r, g, b);
	} 
}
//
// Draw Shaded Line Function (DDA Method)
//
void DrawShadedLine(BYTE *frame, int x1, int y1, int x2, int y2, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	double x_inc = dx / (double)steps;
	double y_inc = dy / (double)steps;
	double x = x1;
	double y = y1;
	int length = pow((pow(dx,2) + pow(dy,2)), 0.5);
	double rdiff = (r2 - r1) / (double)length;
	double gdiff = (g2 - g1) / (double)length;
	double bdiff = (b2 - b1) / (double)length;
	double r = (double)r1;
	double g = (double)g1;
	double b = (double)b1;
	SetPixel(frame, ROUND(x), ROUND(y), r1, g1, b1);
	for ( int i = 0; i < steps; i++, x+=x_inc, y+=y_inc, r+=rdiff, g+=gdiff, b+=bdiff) {
		SetPixel(frame, ROUND(x), ROUND(y), (int)ceil(r), (int)ceil(g), (int)ceil(b));
	} 
}

//
// Draw Filled Triangle
//
#define ROUND_LEFT(x) ((int)ceil(x))
#define CEIL(x) ((int)ceil(x))
#define ROUND_RIGHT(x) ((int)ceil(x-1))
void Triangle(BYTE *frame, int x1, int y1, int x2, int y2, int x3, int y3, unsigned char r, unsigned char g, unsigned char b)
{
	Point p1 = new Point(x1, y1);
	Point p2 = new Point(x2, y2);
	Point p3 = new Point(x3, y3);
	
	// Special case flat-top
	// get highest point y value
	int p1Y = y1 < y2 && y1 < y3 ? y1 : y2 < y1 && y2 < y3 ? y2 : y3;
	// find its x equivalent
	int p1X = p1Y == y1 ? x1 : p1Y == y2 ? x2 : x3;

	// arbitrarily set p2 and p3
	// if p1 is y1, p2 becomes y2
	// otherwise p2 is y1
	// p3 is just whats left
	int p2Y = y1 == p1Y ? y2 : y1;
	int p2X = p2Y == y1 ? x1 : p2Y == y2 ? x2 : x3;
	int p3Y = y1 != p1Y && y1 != p3Y ? y1 : y2 != p1Y && y2 != p3Y ? y2 : y3;
	int p3X = p3Y == y1 ? x1 : p3Y == y2 ? x2 : x3;

	// now figure out which is on which side using angles

	double m1 = (p2X - p1X) / (double)(p2Y - p1Y);
	double m2 = (p3X - p1X) / (double)(p3Y - p1Y);
	int pRY, pRX, pLY, pLX;
	if (atan(m1) < atan(m2)) {
		pRY = p2Y, pRX = p2X, pLY = p3Y, pLX = p3X;
	} else {
		pRY = p3Y, pRX = p3X, pLY = p2Y, pLX = p2X;
	}

	// now we have our hights point: p1 and our left and right edge points pL pR

	double mL = (pLX - p1X) / (double)(pLY - p1Y);
	double mR = (pRX - p1X) / (double)(pRY - p1Y);

	double xL = p1X, xR = p1X;
	int hL = pLY - p1Y, hR = pRY - p1Y;
	int h = (hL > hR) ? hL : hR;

	for (int y = 0; y < h; y++) {
		DrawLine(frame, xL, p1Y + y, xR, p1Y + y, r, g, b);
		if (y == hL) mL = (pRX - pLX) / (double) (pRY - pLY);
		if (y == hR) mR = (pLX - pRX) / (double) (pLY - pRY);
		xL += mL;
		xR += mR;
	}
	
}

////////////////////////////////////////////////////////
// Drawing Function
////////////////////////////////////////////////////////

void BuildFrame(BYTE *pFrame, int view)
{
	BYTE *screen = (BYTE *)pFrame; // use copy of screen pointer for safety

	// for (int i = 0; i < 1000; i++)
	// {
	// 	SetPixel(screen, rand() % FRAME_WIDE, rand() % FRAME_HIGH, rand() % 255, rand() % 255, rand() % 255);
	// }

	// DrawLine(screen, 0, 0, 100, 100, rand() % 255, rand() % 255, rand() % 255);
	// DrawShadedLine(screen, rand() % FRAME_WIDE, rand() % FRAME_HIGH, rand() % FRAME_WIDE, rand() % FRAME_HIGH, 0, 0, 0, 255, 255, 255);
	// DrawShadedLine(screen, rand() % FRAME_WIDE, rand() % FRAME_HIGH, rand() % FRAME_WIDE, rand() % FRAME_HIGH, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255, rand() % 255);

	// Triangle(screen, 100, 100, 200, 100, 100, 200, 255, 255, 255);
	// Triangle(screen, 
	// 	rand() % FRAME_WIDE, rand() % FRAME_HIGH, 
	// 	rand() % FRAME_WIDE, rand() % FRAME_HIGH, 
	// 	rand() % FRAME_WIDE, rand() % FRAME_HIGH, 
	// 	255, 255, 255
	// );
	Triangle(screen, 
		50, 150,
		150, 150,
		100, 100,
		255, 255, 255
	);
	// sleep(1000);
}
