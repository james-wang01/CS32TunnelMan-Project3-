#include "StudentWorld.h"
#include "GraphObject.h"
#include "Actor.h"
#include <string>
#include <list>
#include <stack>
#include <cmath>
#include <queue>

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

bool StudentWorld::checkValid(int x, int y) const {
	if (!checkEarth(x, y) || !checkEarth(x + 3, y + 3))
		return false;

	if (m_objects.empty())
		return true;

	list<Object*>::const_iterator it;
	it = m_objects.begin();

	while (it != m_objects.end()) {
		if (sqrt(pow((*it)->getX() - x, 2) + pow((*it)->getY() - y, 2)) <= 6)
			return false;
		it++;
	}
	return true;
}

int StudentWorld::init() {
	m_ticks = 0;

	m_tunnelman = new TunnelMan(this);
	
	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < 60; y++) {
			if (x < 30 || x > 33 || y < 4) {
				Earth* ptr = new Earth(this, x, y);
				m_earth[x][y] = ptr;
				m_earthBool[x][y] = true;
			}
			else
				m_earthBool[x][y] = false;
		}
	}

	int level = getLevel();

	m_oilLeft = min(2 + level, 21);
	m_chanceToAddGoodies = level * 25 + 300;
	m_targetNumOfProtester = min(15, (int)(2 + level * 1.5));
	m_timeAddNewProtester = max(25, 200 - level);
	
	m_probOfHard = min(90, level * 10 + 30);
	int boulder = min(level / 2 + 2, 9);
	int gold = max(5 - level / 2, 2);

	for (int i = 0; i < boulder; i++) {
		int x = rand() % 61;
		int y = 20 + rand() % 37;
		if (checkValid(x, y)) {
			addObject(new Boulder(this, x, y));
			removeEarth(x, y);
		}
		else
			i--;
	}

	for (int i = 0; i < gold; i++) {
		int x = rand() % 61;
		int y = rand() % 57;
		if (checkValid(x, y))
			addObject(new Nugget(this, x, y, false, false, false));
		else
			i--;
	}

	for (int i = 0; i < m_oilLeft; i++) {
		int x = rand() % 61;
		int y = rand() % 57;
		if (checkValid(x, y))
			addObject(new Oil(this, x, y));
		else
			i--;
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::addNewActors()
{
	if (m_ticks == 1) {
		if ((int)(1 + rand() % 100) <= min(90, (int)getLevel() * 10 + 30))
			addObject(new HardProtester(this));
		else
			addObject(new RegProtester(this));
		m_targetNumOfProtester--;
	}

	else if (m_ticks % m_timeAddNewProtester == 0 && m_targetNumOfProtester > 0) {
		if (m_probOfHard >= (int)(1 + rand() % 100))
			addObject(new HardProtester(this));
		else
			addObject(new RegProtester(this));
		m_targetNumOfProtester--;
	}

	if ((rand() % m_chanceToAddGoodies) == 1) {
		if ((rand() % 5) == 1) {
			addObject(new Sonar(this, 0, 60));
			return;
		}
		int x = rand() % 61;
		int y = rand() % 61;
		while (!checkMove(x, y, GraphObject::none, x, y, false, nullptr, true)) {
			x = rand() % 61;
			y = rand() % 61;
		}
		addObject(new Water(this, x, y));
	}
}

int StudentWorld::move() {
	m_ticks++;

	setDisplayText();

	addNewActors();

	m_tunnelman->doSomething();

	makeMap(m_moveMapExit);
	
	bool mapMade = false;
	for (list<Object*>::iterator it = m_objects.begin(); it != m_objects.end(); it++) {
		if (!mapMade && (*it)->isProtester() && !(*it)->isReg()) {
			makeMap(m_moveMapTunnelMan, m_tunnelman->getX(), m_tunnelman->getY());
			mapMade = true;
		}
		(*it)->doSomething();
	}

	if (m_tunnelman->isDead())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	for (list<Object*>::iterator it = m_objects.begin(); it != m_objects.end();) {
		if ((*it)->isDead()) {
			delete(*it);
			it = m_objects.erase(it);
		}
		else {
			it++;
		}
	}

	if (m_oilLeft <= 0) {
		playSound(SOUND_FINISHED_LEVEL);
		
		return GWSTATUS_FINISHED_LEVEL;
	}
	
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
	delete m_tunnelman;

	for (int x = 0; x < VIEW_WIDTH; x++) {
		for (int y = 0; y < VIEW_HEIGHT; y++) {
			if (m_earthBool[x][y]) {
				delete m_earth[x][y];
				m_earthBool[x][y] = false;
			}
		}
	}

	for (list<Object*>::iterator it = m_objects.begin(); it != m_objects.end();) {
		delete *it;
		it = m_objects.erase(it);
	}
}

void StudentWorld::removeEarth(int x, int y) {
	bool dig = false;
	for (int i = 0; i < 4; i++) {
		for (int r = 0; r < 4; r++) {
			if (m_earthBool[x + i][y + r]) {
				dig = true;
				delete m_earth[x + i][y + r];
				m_earthBool[x + i][y + r] = false;
			}
		}
	}
	if (dig && m_ticks != 0)
		playSound(SOUND_DIG);
}
	


bool StudentWorld::checkEarth(int x, int y) const {
	return m_earthBool[x][y];
}

bool StudentWorld::checkBoulder(int x, int y, Object* object) const {
	for (list<Object*>::const_iterator it = m_objects.begin(); it != m_objects.end(); it++) {
		if ((*it)->isBoulder()) {
			if (*it != object && abs((*it)->getX() - x) <= 3 && abs((*it)->getY() - y) <= 3)
				return true;
		}
	}
	return false;
}

bool StudentWorld::annoyObjects(int x, int y, int amount, int radius, bool isSquirt) {
	bool state = false;
	for (list<Object*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if (sqrt(pow((*it)->getX() - x, 2) + pow((*it)->getY() - y, 2)) <= radius) {
			if ((*it)->loseHealth(amount)) {
				if (isSquirt)
					return true;
				state = true;
			}
		}
	}
	return state;
}

bool StudentWorld::annoyTunnelMan(int x, int y, int amount, int radius) {
	if (sqrt(pow(m_tunnelman->getX() - x, 2) + pow(m_tunnelman->getY() - y, 2)) <= radius) {
		m_tunnelman->loseHealth(amount);
		return true;
	}
	return false;
}

void StudentWorld::makeMap(int map[][VIEW_HEIGHT], int startX, int startY) {
	for (int i = 0; i < VIEW_WIDTH; i++)
		for (int j = 0; j < VIEW_HEIGHT; j++)
			map[i][j] = -1;
	queue<Coord> steps;
	Coord start(startX, startY);
	steps.push(start);
	Coord curr(0, 0);
	int a, b;
	while (!steps.empty()) {
		curr = steps.front();
		steps.pop();
		int x = curr.x();
		int y = curr.y();
		if (checkMove(x, y, GraphObject::right, a, b, false, nullptr, true) && map[x + 1][y] == -1) {
			steps.push(Coord(x + 1, y));
			map[x + 1][y] = LEFT;
		}
		if (checkMove(x, y, GraphObject::left, a, b, false, nullptr, true) && map[x - 1][y] == -1) {
			steps.push(Coord(x - 1, y));
			map[x - 1][y] = RIGHT;
		}
		if (checkMove(x, y, GraphObject::up, a, b, false, nullptr, true) && map[x][y + 1] == -1) {
			steps.push(Coord(x, y + 1));
			map[x][y + 1] = DOWN;
		}
		if (checkMove(x, y, GraphObject::down, a, b, false, nullptr, true) && map[x][y - 1] == -1) {
			steps.push(Coord(x, y - 1));
			map[x][y - 1] = UP;
		}
	}
}
bool StudentWorld::checkMove(int x, int y, GraphObject::Direction dir, int& a, int& b, bool isBoulder, Object* object, bool isObject) {
	switch (dir)
	{
	case GraphObject::right:
		x++;
		break;
	case GraphObject::left:
		x--;
		break;
	case GraphObject::down:
		y--;
		break;
	case GraphObject::up:
		y++;
		break;
	default:        
		break;
	}

	if (x < 0 || x > 60 || y < 0 || y > 60)
		return false;

	if (isObject) {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (checkEarth(x + i, y + j))
					return false;
	}

	if (isBoulder) {
		if (checkEarth(x, y) || checkEarth(x + 1, y) || checkEarth(x + 2, y) || checkEarth(x + 3, y))
			return false;
	}

	else {
		if (checkBoulder(x, y, object))
			return false;
	}
	 
	a = x;
	b = y;
	return true;
}

void StudentWorld::sonarReveal(int x, int y, int radius) {
	for (list<Object*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if (sqrt(pow((*it)->getX() - x, 2) + pow((*it)->getY() - y, 2)) <= radius)
			(*it)->setVisible(true);
	}
}

//AUXILIARY FUNCTIONS
string StudentWorld::convertToString(int value, int length, string leading) {
	string s = to_string(value);
	while (s.length() != length)
		s = leading + s;
	return s;
}

void StudentWorld::setDisplayText()
{
	int level = getLevel();
	int lives = getLives();
	int health = m_tunnelman->getHealth() * 10;
	int squirts = m_tunnelman->getWater();
	int gold = m_tunnelman->getNugget();
	int barrelsLeft = getOil();
	int sonar = m_tunnelman->getSonar();
	int score = getScore();
	// Next, create a string from your statistics, of the form:
	// Lvl: 52 Lives: 3 Hlth: 80% Wtr: 20 Gld: 3 Oil Left: 2 Sonar: 1 Scr: 321000
	
	string s = ("Lvl:" + convertToString(level, 2, " ") + " Lives:" + convertToString(lives, 1, "0") + " Hlth: "
		+ convertToString(health, 3, " ") + "% Wtr: " + convertToString(squirts, 2, " ") + " Gld: "
		+ convertToString(gold, 2, " ") + " Oil Left: " + convertToString(barrelsLeft, 2, " ") + " Sonar: "
		+ convertToString(sonar, 2, " ") + " Scr: " + convertToString(score, 6, "0"));
		
	// Finally, update the display text at the top of the screen with your
	// newly created stats
	setGameStatText(s); // calls our provided GameWorld::setGameStatText
}

bool StudentWorld::findTunnelMan(int x, int y, int radius) {
	if (sqrt((m_tunnelman->getX() - x)*(m_tunnelman->getX() - x) + (m_tunnelman->getY() - y)*(m_tunnelman->getY() - y)) <= radius)
		return true;
	return false;
}

Object* StudentWorld::findProtester(int x, int y, int radius) {
	for (list<Object*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if ((*it)->isProtester() && sqrt(pow((*it)->getX() - x, 2) + pow((*it)->getY() - y, 2)) <= radius)
			return *it;
	}
	return nullptr;
}

bool StudentWorld::facingTunnelMan(Object* object) {
	if (m_tunnelman->getX() == object->getX() && m_tunnelman->getY() == object->getY())
		return true;
	switch (object->getDirection())
	{
	case GraphObject::up:
		return m_tunnelman->getY() > object->getY();
		break;
	case GraphObject::down:
		return m_tunnelman->getY() < object->getY();
		break;
	case GraphObject::left:
		return m_tunnelman->getX() < object->getX();
		break;
	case GraphObject::right:
		return m_tunnelman->getX() > object->getX();
		break;
	default:
		return false;
		break;
	}
}

GraphObject::Direction StudentWorld::lineOfSightToTunnelMan(int x, int y) {
	if (m_tunnelman->getX() == x)
	{
		if (m_tunnelman->getY() > y) {
			while (y != m_tunnelman->getY() && checkMove(x, y, GraphObject::up, x, y, false, nullptr, true)) {}
			if (y == m_tunnelman->getY())
				return GraphObject::up;
			return GraphObject::none;
		}
		if (m_tunnelman->getY() < y) {
			while (y != m_tunnelman->getY() && checkMove(x, y, GraphObject::down, x, y, false, nullptr, true)) {}
			if (y == m_tunnelman->getY())
				return GraphObject::down;
			return GraphObject::none;
		}
		else
			return GraphObject::none;
	}
	if (y == m_tunnelman->getY())
	{
		if (m_tunnelman->getX() < x) {
			while (x != m_tunnelman->getX() && checkMove(x, y, GraphObject::left, x, y, false, nullptr, true)) {}
			if (x == m_tunnelman->getX())
				return GraphObject::left;
			return GraphObject::none;
		}
		if (m_tunnelman->getX() > x) {
			while (x != m_tunnelman->getX() && checkMove(x, y, GraphObject::right, x, y, false, nullptr, true)) {}
			if (x == m_tunnelman->getX())
				return GraphObject::right;
			return GraphObject::none;
		}
		else
			return GraphObject::none;
	}
	return GraphObject::none;
}

int StudentWorld::getDistanceTunnelMan(int x, int y) const {
	if (x == m_tunnelman->getX() && y == m_tunnelman->getY())
		return 0;
	switch (m_moveMapTunnelMan[x][y]) {
	case UP:
		return 1 + getDistanceTunnelMan(x, y + 1);
	case DOWN:
		return 1 + getDistanceTunnelMan(x, y - 1);
	case LEFT:
		return 1 + getDistanceTunnelMan(x - 1, y);
	case RIGHT:
		return 1 + getDistanceTunnelMan(x + 1, y);
	default:
		return 10000;
	}
}