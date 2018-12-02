#pragma once
#include<vector>

class SingleBlock {
public:
	SingleBlock();

	int width, height;
	float red, green, blue;
	int posX, posY;
	BOOL destroyed;


	void EnemyDraw();


	};


class Bricks : SingleBlock {
public:

	Bricks();
	

	std::vector<SingleBlock*>BrickVec = std::vector<SingleBlock*>();
	int stage;

	void CreateBricks();
	void DestroyBrick(int i);
	BOOL HitTest(int ballPosX, int ballPosY);
	BOOL BrickLeftTest();
	void DrawBricks();

private:
	
	};