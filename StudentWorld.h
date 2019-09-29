#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int NONE = 4;

class Earth;
class TunnelMan;
class Object;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir) : GameWorld(assetDir) {}
	virtual ~StudentWorld() {}
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	//HELPER
	bool checkMove(int x, int y, GraphObject::Direction dir, int& a, int& b, bool isBoulder = false, Object* object = nullptr, bool isObject = false);
	void sonarReveal(int x, int y, int radius);
	void addObject(Object* add) { m_objects.push_back(add); }
	//ACCESSORS
	TunnelMan* getTunnelMan() const { return m_tunnelman; }
	int getOil() const { return m_oilLeft; }
	int getDirExit(int x, int y) const { return m_moveMapExit[x][y]; }				//for protester when leaving
	int getDirTunnelMan(int x, int y) const { return m_moveMapTunnelMan[x][y]; }	//for hard protester when finding tunnelman
	int getDistanceTunnelMan(int x, int y) const;									//for hard protester to see how far from tunnelman
	bool checkEarth(int x, int y) const;
	bool checkBoulder(int x, int y, Object* object = nullptr) const;
	bool checkValid(int x, int y) const;											//only used in init
	//MUTATORS
	void removeEarth(int x, int y);
	void decOil() { m_oilLeft--; }
	void decProtester() { m_targetNumOfProtester++; }
	bool annoyObjects(int x, int y, int amount, int radius, bool isSquirt = false);
	bool annoyTunnelMan(int x, int y, int amount, int radius);
	//AUXILIARY 
	void makeMap(int map[][VIEW_HEIGHT], int startX = 60, int startY = 60);
	std::string convertToString(int value, int length, std::string leading);
	void setDisplayText();
	void addNewActors();
	bool findTunnelMan(int x, int y, int radius);
	bool facingTunnelMan(Object* object);
	Object* findProtester(int x, int y, int radius);
	GraphObject::Direction lineOfSightToTunnelMan(int x, int y);
private:
	int m_ticks;
	int m_oilLeft;
	int m_timeAddNewProtester;
	int m_targetNumOfProtester;
	int m_chanceToAddGoodies;
	int m_probOfHard;
	Earth* m_earth[VIEW_WIDTH][VIEW_HEIGHT];
	bool m_earthBool[VIEW_WIDTH][VIEW_HEIGHT];
	int m_moveMapExit[VIEW_WIDTH][VIEW_HEIGHT];
	int m_moveMapTunnelMan[VIEW_WIDTH][VIEW_HEIGHT];
	std::list<Object*> m_objects;
	TunnelMan* m_tunnelman;

	class Coord {
	public:
		Coord(int x, int y) : m_x(x), m_y(y) {}
		int x() const { return m_x; }
		int y() const { return m_y; }
	private:
		int m_x;
		int m_y;
	};
};

#endif // STUDENTWORLD_H_
