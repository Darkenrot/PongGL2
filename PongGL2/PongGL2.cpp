// PongGL.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "PongGL2.h"
#include <Windows.h>
#include<freeglut.h>
#include <stdio.h>
#include <SOIL.h>
#include<iostream>
#include <math.h>
#include<stdarg.h>
#include<chrono>
#include<thread>
#include <mmsystem.h>
#include "Enemy.h"
#include<vector>

#define PI 3.14159265

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;

BOOL	keys[256];		// Array Used For The Keyboard Routine
BOOL active = TRUE;
BOOL fullscreen = TRUE;
BOOL done = FALSE;


GLuint texture[2];
GLuint base;

GLUquadricObj *quadratic;
int playerX = 800;
float ballAngle;
int ballPosX, ballPosY;
float ballSpeed, playerSpeed;
BOOL playerMoves, playerMovesRight;

Bricks *Block;
int score=0;
int lives = 3;
BOOL liveLost = false;


auto t1 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
auto t2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
float time_passed;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


BOOL LoadGLTextures() {

	int width, height;

	unsigned char *image = SOIL_load_image(
		"../Data/Font.bmp",
		&width,
		&height,
		0, SOIL_LOAD_RGB
	);



	glGenTextures(2, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	SOIL_free_image_data(image);


	return true;
	}

GLvoid BuildFont() {

	base = glGenLists(256);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	for (int i = 0; i < 256; i++) {
		float cx = float(i % 16) / 16.0f;
		float cy = float(i / 16) / 16.0f;
		glNewList(base + i, GL_COMPILE);               // Start Building A List
		glBegin(GL_QUADS);                  // Use A Quad For Each Character
		glTexCoord2f(cx, -( 1.0f - cy - 0.0625f ));       // Texture Coord (Bottom Left)
		glVertex2d(0, 16);               // Vertex Coord (Bottom Left)
		glTexCoord2f(cx + 0.0625f, -( 1.0f - cy - 0.0625f ));   // Texture Coord (Bottom Right)
		glVertex2i(16, 16);              // Vertex Coord (Bottom Right)
		glTexCoord2f(cx + 0.0625f, -( 1.0f - cy ));       // Texture Coord (Top Right)
		glVertex2i(16, 0);               // Vertex Coord (Top Right)
		glTexCoord2f(cx, -( 1.0f - cy ));           // Texture Coord (Top Left)
		glVertex2i(0, 0);                // Vertex Coord (Top Left)
		glEnd();                        // Done Building Our Quad (Character)
		glTranslated(15, 0, 0);                   // Move To The Right Of The Character
		glEndList();
		}

	}

GLvoid KillFont() {
	glDeleteLists(base, 256);
	}

GLvoid glPrint(GLint x, GLint y, int set, GLfloat scaleX, GLfloat scaleY, const char* fmt, ...) {

	char text[256];
	va_list ap;
	if (fmt == NULL) {
		return;
		}

	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	if (set > 1) {
		set = 1;
		}
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();
	glTranslated(x, y, 0);
	glListBase(base - 32 + ( 128 * set ));
	glScalef(scaleX, scaleY, 1.0f);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glDisable(GL_TEXTURE_2D);

	}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
	{


	if (height == 0)										// Prevent A Divide By Zero By
		{
		height = 1;										// Making Height Equal One
		}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix


	glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);


	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	}

GLvoid BallSetup() {
	ballAngle = 300;
	ballSpeed = 10;
	ballPosX = 500;
	ballPosY = 500;
	}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
	{
	LoadGLTextures();
	BuildFont();
	Block = new Bricks;
	Block->CreateBricks();

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glClearStencil(0);									// Stencil Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
														//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	quadratic = gluNewQuadric();          // Create A Pointer To The Quadric Object ( NEW )
	gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals ( NEW )
	gluQuadricTexture(quadratic, GL_TRUE);

	BallSetup();

	return TRUE;

	}


GLvoid PlayerDraw() {

	glLoadIdentity();
	glTranslated(playerX, 950, 0);
	glColor3f(1.0f, 0.0f, 0.0f);
	glRectd(0, 0, 200, 50);
	
	

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2d(200, 50);
	glVertex2d(0, 0);

	glVertex2d(200, 0);
	glVertex2d(0, 50);

	glVertex2d(0, 0);
	glVertex2d(200, 0);

	glVertex2d(0, 0);
	glVertex2d(0, 50);

	glVertex2d(200, 0);
	glVertex2d(200, 50);

	glVertex2d(0, 50);
	glVertex2d(200, 50);

	glEnd();

	}

GLvoid BricksDraw() {



	glLineWidth(2.0f);
	for (int i = 5; i >0; i--) {

		glTranslated(550 + ( 5 - i ) * 50, 150 + ( 5 - i ) * 50, 0);
		for (int j = 0; j <i; j++) {

			glTranslated(100, 0, 0);

			glColor3f(0.0f, 1.0f, 0.0f);
			glRectd(0, 0, 100, 50);

			glColor3f(0.0f, 0.0f, 0.0f);
			glBegin(GL_LINES);
			glVertex2d(100, 50);
			glVertex2d(0, 0);

			glVertex2d(100, 0);
			glVertex2d(0, 50);

			glVertex2d(0, 0);
			glVertex2d(100, 0);

			glVertex2d(0, 0);
			glVertex2d(0, 50);

			glVertex2d(100, 0);
			glVertex2d(100, 50);

			glVertex2d(0, 50);
			glVertex2d(100, 50);

			glEnd();

			}
		glLoadIdentity();
		}


	}

int BallBounceX(int angle) {

	if (angle < 180) {
		angle = 180 - angle;
		}
	if (angle > 180) {
		angle = 540 - angle;

		if (angle == 180) {
			angle = 175;
			}
		return angle;
		}

	}
int BallBounceY(int angle) {

	
	angle = 360 - angle;
	return angle;
	}

int CollisionAng(float angle, float pos, float off) {
	
	if (angle > 180) {
		angle = 22;
		}
	int angleres;
	angleres = angle - angle*off*pos*((angle*off / 180) + 1);

	return angleres;

	}

BOOL Collision() {

	int posDifference=200;
	float vecXChange;
	float offset = 0.5;
	float ballres;

	if (ballPosY >= 930) {
		posDifference = ballPosX - playerX - 100;
		if (posDifference >-100 && posDifference<100) {
			vecXChange = float(posDifference) / 100;
			
			ballAngle = BallBounceY(ballAngle);
			if (vecXChange > 0) {
				ballAngle = CollisionAng(ballAngle, vecXChange, offset);
				}
			if (vecXChange < 0) {
				ballAngle = 180-CollisionAng(180 - ballAngle, -vecXChange, offset);
				}

			ballPosY = 929;
			return TRUE;
			}
		else {		
			
			return FALSE;
			}
		}
	return FALSE;
	}

GLvoid FailDraw() {

	glLoadIdentity();
	glColor3f(1, 0, 0);
	glTranslated(820, 150, 0);

	glBegin(GL_QUADS);
	glVertex2d(160, 0);
	glVertex2d(0, 0);
	glVertex2d(0, 600);
	glVertex2d(160, 600);
	glEnd();
	glTranslated(80, 700, 0);
	gluPartialDisk(quadratic, 0, 80, 32, 32, 0, 360);

	}


BOOL FailTest() {
	if (ballPosY > 1000) {
		lives--;
		ballPosX = 700;
		ballPosY = 500;
		ballAngle = 200;
		

		return TRUE;
		}
	return FALSE;
	}


GLvoid BallLogic() {

	
	if (ballPosX > 1780) {
		ballPosX = 1780;
		ballAngle = BallBounceX(ballAngle);
		}
	if (ballPosX < 120) {
		ballPosX = 120;
		ballAngle = BallBounceX(ballAngle);
		}

	if (ballPosY > 1030) {
		ballPosY = 1030;
		ballAngle = BallBounceY(ballAngle);
		}
	if (ballPosY < 150) {
		ballPosY = 150;
		ballAngle = BallBounceY(ballAngle);
		}

	Collision();

	
	ballPosX += cos(ballAngle*PI/180)*ballSpeed;
	ballPosY += -sin(ballAngle*PI/180)*ballSpeed;

	ballSpeed += 0.002;
	



	}


GLvoid BallDraw() {

	glLoadIdentity();
	glTranslated(ballPosX, ballPosY, 0);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluPartialDisk(quadratic, 0, 20, 32, 32, 0, 360);

	}

GLvoid LivesDraw(int livesNr) {

	int size = 15;

	glLoadIdentity();
	glTranslated(230, 105, 0);

	for (int i = 0; i < livesNr; i++) {

		glTranslated(4 * size, 0, 0);

		glColor3f(1, 0, 0);
		glRectd(0, 0, 3 * size, size);

		glColor3f(0, 0, 0);
		glBegin(GL_LINES);
		glVertex2d(0, 0);
		glVertex2d(3 * size, 0);

		glVertex2d(3 * size, 0);
		glVertex2d(3 * size, size);

		glVertex2d(3 * size, size);
		glVertex2d(0, size);

		glVertex2d(0, size);
		glVertex2d(0, 0);

		glVertex2d(0, 0);
		glVertex2d(3 * size, size);

		glVertex2d(3 * size, 0);
		glVertex2d(0, size);

		glEnd();


		}

	}

GLvoid PlayerLogic() {

	glLoadIdentity();

	if (playerMoves) {
		if (playerMovesRight) {
			if (playerSpeed > 0) {
				playerSpeed += 0.04;
				}
			else {
				playerSpeed = 1;
				}
			}
		else {
			if (playerSpeed < 0) {
				playerSpeed -= 0.04;
				}
			else {
				playerSpeed = -1;
				}

			}
		playerX += playerSpeed * 10;
		playerMoves = FALSE;
		playerMovesRight = FALSE;
		}



	if (playerX > 1600) playerX = 1600;
	if (playerX < 100) playerX = 100;

	
	
	
	}

bool DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
	{

	if (liveLost) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
			liveLost = false;

		}

	if (lives == 0) {
		std::this_thread::sleep_for(std::chrono::seconds(3));
		done = TRUE;
		}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -10.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos2f(5.5f, 3.5f);

	do {
		t1 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		time_passed = t1 - t2;
		time_passed = time_passed / 1000000;
		} while (time_passed < 16.59);
		t2 = t1;

		int FPS = 1000 / time_passed;

		glPrint(1600, 50, 0, 2.0f, 1.5f, "FPS:%2.0i", FPS);
		glColor3f(0.0f, 1.0f, 1.0f);
		glPrint(600, 50, 1, 4.0f, 3.0f, "PONG SUPREME");
		glColor3f(0.0f, 1.0f, 0.0f);
		glPrint(100, 50, 0, 2.0f, 1.5f, "Score:%5.0i", score);
		glPrint(100, 100, 0, 2.0f, 1.5f, "Lives:");


		glLoadIdentity();
		glColor3f(0.1f, 0.3f, 0.3f);
		glRectd(100, 130, 1800, 1050);
		BallLogic();
		PlayerLogic();
		if (FailTest()) {
			liveLost = TRUE;
			}

		if (Block->HitTest(ballPosX, ballPosY)) {
			ballAngle=BallBounceY(ballAngle);
			score += 100;
			if (!Block->BrickLeftTest()) {
				Block->stage += 1;
				Block->CreateBricks();
				score += 1000;
				lives++;
				}
			}

		LivesDraw(lives);
		BallDraw();
		PlayerDraw();
		Block->DrawBricks();
		if (liveLost&& lives>0) {
			FailDraw();
			}
	
		if (lives == 0) {
			glColor3f(1, 0, 0);
			glPrint(200, 500, 0, 12.0f, 12.0f, "You SUCK");
			}

		return true;
	}

void ProcessKeyboard() {

	int step = 5;

	if (keys[VK_LEFT]) {
		playerMoves = TRUE;
		playerMovesRight = FALSE;
		}
	if (keys[VK_RIGHT]) {
		playerMoves = TRUE;
		playerMovesRight = TRUE;
		}

	}



GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
	{



	if (fullscreen)										// Are We In Fullscreen Mode?
		{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
		}

	if (hRC)											// Do We Have A Rendering Context?
		{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
			{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
			}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
			{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
			}
		hRC = NULL;										// Set RC To NULL
		}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
		{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
		}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
		{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
		}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
		{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
		}
	KillFont();

	}


BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag) {
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
		{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
		}

	if (fullscreen)												// Attempt Fullscreen Mode?
		{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {

			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
				}
			else {
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
				}

			}
		}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
		{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
		}
	else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
		}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

																	// Create The Window
	if (!( hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL) ))								// Dont Pass Anything To WM_CREATE
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
		{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		1,											// Use Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
		};

	if (!( hDC = GetDC(hWnd) ))							// Did We Get A Device Context?
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	if (!( PixelFormat = ChoosePixelFormat(hDC, &pfd) ))	// Did Windows Find A Matching Pixel Format?
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	if (!( hRC = wglCreateContext(hDC) ))				// Are We Able To Get A Rendering Context?
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}

	return TRUE;									// Success
	}
LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
	{
	// NOT MINE
	switch (uMsg)									// Check For Windows Messages
		{
			case WM_ACTIVATE:							// Watch For Window Activate Message
				{
				if (!HIWORD(wParam))					// Check Minimization State
					{
					active = TRUE;						// Program Is Active
					}
				else {
					active = FALSE;						// Program Is No Longer Active
					}

				return 0;								// Return To The Message Loop
				}

			case WM_SYSCOMMAND:							// Intercept System Commands
				{
				switch (wParam)							// Check System Calls
					{
						case SC_SCREENSAVE:					// Screensaver Trying To Start?
						case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
							return 0;							// Prevent From Happening
					}
				break;									// Exit
				}

			case WM_CLOSE:								// Did We Receive A Close Message?
				{
				PostQuitMessage(0);						// Send A Quit Message
				return 0;								// Jump Back
				}

			case WM_KEYDOWN:							// Is A Key Being Held Down?
				{
				keys[wParam] = TRUE;					// If So, Mark It As TRUE
				return 0;								// Jump Back
				}

			case WM_KEYUP:								// Has A Key Been Released?
				{
				keys[wParam] = FALSE;					// If So, Mark It As FALSE
				return 0;								// Jump Back
				}

			case WM_SIZE:								// Resize The OpenGL Window
				{
				ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
				return 0;								// Jump Back
				}
		}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}


int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
	{
	MSG		msg;									// Windows Message Structure
	done = FALSE;								// Bool Variable To Exit Loop

														// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO) {
		fullscreen = FALSE;							// Windowed Mode
		}

	// Create Our OpenGL Window
	if (!CreateGLWindow("Lionel Brits & NeHe's 3D World Tutorial", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), GetDeviceCaps(GetWindowDC(GetDesktopWindow()), BITSPIXEL), fullscreen)) {
		return 0;									// Quit If Window Was Not Created
		}



	while (!done)									// Loop That Runs While done=FALSE
		{





		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
			{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
				{
				done = TRUE;							// If So done=TRUE
				}
			else									// If Not, Deal With Window Messages
				{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
				}
			}
		else										// If There Are No Messages
			{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active && keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
				{
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
				}
			else									// Not Time To Quit, Update Screen
				{
				DrawGLScene();
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
				ProcessKeyboard();


				}

			if (keys[VK_F1])						// Is F1 Being Pressed?
				{
				keys[VK_F1] = FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
														// Recreate Our OpenGL Window
				if (!CreateGLWindow("bosco & NeHe's Waving Texture Tutorial", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), GetDeviceCaps(GetWindowDC(GetDesktopWindow()), BITSPIXEL), fullscreen)) {
					return 0;						// Quit If Window Was Not Created
					}
				}
			}

		}

	// Shutdown

	KillGLWindow();										// Kill The Window
	return ( msg.wParam );								// Exit The Program
	}
