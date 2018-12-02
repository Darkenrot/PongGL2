#include "stdafx.h"
#include "Enemy.h"
#include<freeglut.h>


SingleBlock::SingleBlock() {

	width = 100;
	height = 50;
	destroyed = false;
	
	}




void SingleBlock::EnemyDraw() {

	glRectd(0, 0, width, height);

	glLineWidth(1.0f);
	glColor3f(0.8, 0.8, 0.8);

	glBegin(GL_LINES);

	glVertex2d(0, 0);
	glVertex2d(width / 10, height / 5);

	glVertex2d(width / 10, height / 5);
	glVertex2d(width * 9 / 10, height / 5);

	glVertex2d(width * 9 / 10, height / 5);
	glVertex2d(width, 0);

	glVertex2d(0, height);
	glVertex2d(width / 10, height *4/ 5);

	glVertex2d(width / 10, height * 4 / 5 );
	glVertex2d(width *  9 / 10 , height * 4 / 5 );

	glVertex2d(width *  9 / 10 , height * 4 / 5 );
	glVertex2d(width, height);

	glVertex2d(width *  9 / 10 , height / 5);
	glVertex2d(width *  9 / 10 , height * 4 / 5 );

	glVertex2d(width  / 10 , height * 4 / 5 );
	glVertex2d(width / 10, height / 5 );

	glVertex2d(0, 0);
	glVertex2d(width, 0);

	glVertex2d(width, 0);
	glVertex2d(width, height);

	glVertex2d(width, height);
	glVertex2d(0, height);

	glVertex2d(0, height);
	glVertex2d(0, 0);

	glEnd();


	


	}




Bricks::Bricks() {
	stage = 1;
	}



void Bricks::CreateBricks() {
	
	int offsetX = 110;
	int offsetY = 100;
	int randValue;

	for (int i = 0; i < 2 + stage; i++) {
		offsetX += width / 2;
		offsetY += height;
		for (int j = 0; j < 15 - i; j++) {
			SingleBlock *Block = new SingleBlock;
			Block->posX = offsetX + j*width;
			Block->posY = offsetY;
			randValue = rand() % 100;
			Block->red = float(randValue) / 100;
			randValue = rand() % 100;
			Block->green = float(randValue) / 100;
			randValue = rand() % 100;
			Block->blue = float(randValue) / 100;
			
			BrickVec.push_back(Block);
			}
		}

	}

void Bricks::DestroyBrick(int i) {
	SingleBlock *destroyer;
	destroyer = BrickVec[i];
	BrickVec.erase(BrickVec.begin() + i);
	delete destroyer;

	}

BOOL Bricks::HitTest(int ballPosX, int ballPosY) {
	
	for (int i = 0; i < BrickVec.size(); i++) {
		if (ballPosX > BrickVec[i]->posX && ballPosX < BrickVec[i]->posX + BrickVec[i]->width) {
			if (ballPosY <= BrickVec[i]->posY + height && ballPosY > BrickVec[i]->posY-5) {
				DestroyBrick(i);
				return TRUE;
				}
			}
		}

	return FALSE;

	
	}

BOOL Bricks::BrickLeftTest() {
	if (BrickVec.empty()) {
		return FALSE;
		}

	return TRUE;
	}

void Bricks::DrawBricks() {

	

	for (int i = 0; i < BrickVec.size(); i++) {
	
		glLoadIdentity();
		glTranslated(BrickVec[i]->posX, BrickVec[i]->posY, 0);
		glColor3f(BrickVec[i]->red, BrickVec[i]->green, BrickVec[i]->blue);
		BrickVec[i]->EnemyDraw();
		}
	}
