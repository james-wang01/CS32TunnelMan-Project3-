#include "Actor.h"
#include "StudentWorld.h"
#include <queue>
#include <vector>

using namespace std;

//OBJECT
Object::Object(StudentWorld* world, int imageID, bool canSee, unsigned int depth, int x, int y, double size, GraphObject::Direction dir) 
	: GraphObject(imageID, x, y, dir, size, depth)
{
	m_world = world;
	m_dead = false;
	setVisible(canSee);
}

GraphObject::Direction Object::convertIntToDir(int dir) {
	switch (dir) {
	case UP:
		return up;
	case DOWN:
		return down;
	case LEFT:
		return left;
	case RIGHT:
		return right;
	default:
		return none;
	}
}
//Disappearing Object
DisappearingObject::DisappearingObject(StudentWorld* world, int imageID, bool visible, unsigned int depth, int x, int y)
	:Object(world, imageID, visible, depth, x, y) {}

void DisappearingObject::checkDead() {
	if (getTemporary() && getTimer() >= getLifespan()) {
		setDead();
		return;
	}
	if (getTemporary())
		incTimer();
}
//Nugget
Nugget::Nugget(StudentWorld* world, int x, int y, bool visible, bool pickedUpByPlayer, bool temporary) 
	: DisappearingObject(world, TID_GOLD, visible, 2, x, y)
{
	setTemporary(temporary);
	m_pickedUpByPlayer = pickedUpByPlayer;
	setLifespan(max(100, 300 - 10 * (int)getWorld()->getLevel()));
}

void Nugget::doSomething() {
	if (isDead())
		return;
	if (!isVisible() && !getPickedUpByPlayer() && getWorld()->findTunnelMan(getX(), getY(), 4)) {
		setVisible(true);
		return;
	}
	if (isVisible() && !getPickedUpByPlayer() && getWorld()->findTunnelMan(getX(), getY(), 3)) {
		setDead();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(10);
		getWorld()->getTunnelMan()->addNugget();
		return;
	}
	Object* ptr = getWorld()->findProtester(getX(), getY(), 3);
	if (getPickedUpByPlayer() &&  ptr != nullptr) {
		setDead();
		ptr->foundGold();
	}
	checkDead();
}

//Sonar
Sonar::Sonar(StudentWorld* world, int x, int y)
	: DisappearingObject(world, TID_SONAR, true, 2, x, y) {
	setTemporary(true);
	setLifespan(max(100, 300 - 10 * (int)getWorld()->getLevel()));
}

void Sonar::doSomething() {
	if (isDead())
		return;
	if (getWorld()->findTunnelMan(getX(), getY(), 3)) {
		setDead();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->getTunnelMan()->addSonar();
		getWorld()->increaseScore(75);
	}
	checkDead();
}

//Water
Water::Water(StudentWorld* world, int x, int y) 
	: DisappearingObject(world, TID_WATER_POOL, true, 2, x, y) {
	setTemporary(true);
	setLifespan(max(100, 300 - 10 * (int)getWorld()->getLevel()));
}

void Water::doSomething() {
	if (isDead())
		return;
	if (getWorld()->findTunnelMan(getX(), getY(), 3)) {
		setDead();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->getTunnelMan()->addWater(5);
		getWorld()->increaseScore(100);
	}
	checkDead();
}

//Boulder
Boulder::Boulder(StudentWorld* world, int x, int y) 
	: Object(world, TID_BOULDER, true, 1, x, y, 1.0, down)
{
	getWorld()->removeEarth(x, y);
	m_stable = true;
	m_falling = false;
	m_ticks = 0;
}

bool Boulder::boulderFall() {
	if (getWorld()->checkEarth(getX(), getY() - 1) || getWorld()->checkEarth(getX() + 1, getY() - 1) || getWorld()->checkEarth(getX() + 2, getY() - 1) || getWorld()->checkEarth(getX() + 3, getY() - 1))
		return false;
	return true;
}

void Boulder::doSomething() {
	if (isDead())
		return;
	if (m_stable) {
		if (!boulderFall())
			return;
		else {
			m_stable = false;
			m_ticks = 0;
		}
	}
	else if(!m_falling) {
		if (m_ticks >= 30) {
			getWorld()->playSound(SOUND_FALLING_ROCK);
			m_falling = true;
		}
		m_ticks++;
	}
	else if(m_falling) {
		getWorld()->annoyTunnelMan(getX(), getY(), 100, 3);
		getWorld()->annoyObjects(getX(), getY(), 100, 3);
		int a = 0;
		int b = 0;
		if(getWorld()->checkMove(getX(), getY(), down, a, b, true))
			moveTo(a, b);
		else {
			setDead();
			return;
		}
	}
}

//Squirt
Squirt::Squirt(StudentWorld* world, int x, int y, GraphObject::Direction dir)
	: Object(world, TID_WATER_SPURT, true, 1, x, y, 1.0, dir) 
{
	if (getWorld()->checkEarth(x, y) || getWorld()->checkBoulder(x, y)) {
		setDead();
		return;
	}
	m_distanceTravelled = 0;
}

void Squirt::doSomething() {
	if (isDead())
		return;
	int a = 0, b = 0;
	if (getWorld()->annoyObjects(getX(), getY(), 2, 3, true)) {
		setDead();
	}
	else if (getDistanceTravelled() == 4) {
		setDead();
	}
	else if (getWorld()->checkMove(getX(), getY(), getDirection(), a, b, false, nullptr, true)) {
		moveTo(a, b);
		incDistanceTravelled();
	}
	else {
		setDead();
	}
}

Oil::Oil(StudentWorld* world, int x, int y) 
	: Object(world, TID_BARREL, false, 2, x, y) {}

void Oil::doSomething() {
	if (isDead())
		return;
	if (!isVisible() && getWorld()->findTunnelMan(getX(), getY(), 4)) {
		setVisible(true);
		return;
	}
	if (isVisible() && getWorld()->findTunnelMan(getX(), getY(), 3)) {
		setDead();
		getWorld()->playSound(SOUND_FOUND_OIL);
		getWorld()->increaseScore(1000);
		getWorld()->decOil();
	} 
}

//CanBeAnnoyed
CanBeAnnoyed::CanBeAnnoyed(StudentWorld* world, int imageID, bool canSee, int depth, int x, int y, int size, Direction dir)
	: Object(world, imageID, canSee, depth, x, y, size, dir) {}

//TUNNELMAN
TunnelMan::TunnelMan(StudentWorld* world) : CanBeAnnoyed(world, TID_PLAYER, true, 1, 30 , 60) {
	setHealth(10);
	m_water = 5;
	m_sonar = 1;
	m_nugget = 0;
}

bool TunnelMan::loseHealth(int amount) {
	setHealth(getHealth() - amount);
	if (getHealth() <= 0) {
		setDead();
		getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
	}
	return true;
}

void TunnelMan::doSomething() {
	if (getHealth() <= 0) {
		setDead();
		return;
	}
	int input;
	if (getWorld()->getKey(input) == true) {
		int a = 0, b = 0;
		switch (input) {
		case KEY_PRESS_UP:
			if (getDirection() != up)
				setDirection(up);
			else if (getWorld()->checkMove(getX(), getY(), up, a, b))
				moveTo(a, b);
			break;
		case KEY_PRESS_DOWN:
			if (getDirection() != down)
				setDirection(down);
			else if (getWorld()->checkMove(getX(), getY(), down, a, b))
				moveTo(a, b);
			break;
		case KEY_PRESS_LEFT:
			if (getDirection() != left)
				setDirection(left);
			else if (getWorld()->checkMove(getX(), getY(), left, a, b))
				moveTo(a, b);
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() != right)
				setDirection(right);
			else if (getWorld()->checkMove(getX(), getY(), right, a, b))
				moveTo(a, b);
			break;
		case KEY_PRESS_ESCAPE:
			setDead();
			break;
		case KEY_PRESS_SPACE:
			if (getWater() <= 0)
				break;
			switch (getDirection()) {
			case up:
				getWorld()->addObject(new Squirt(getWorld(), getX(), getY() + 4, up));
				break;
			case down:
				getWorld()->addObject(new Squirt(getWorld(), getX(), getY() - 4, down));
				break;
			case left:
				getWorld()->addObject(new Squirt(getWorld(), getX() - 4, getY(), left));
				break;
			case right:
				getWorld()->addObject(new Squirt(getWorld(), getX() + 4, getY(), right));
				break;
			default:
				break;
			}
			decWater();
			getWorld()->playSound(SOUND_PLAYER_SQUIRT);
			break;
		case 'Z':
		case 'z':
			if (getSonar() <= 0)
				break;
			getWorld()->sonarReveal(getX(), getY(), 12);
			getWorld()->playSound(SOUND_SONAR);
			decSonar();
			break;
		case KEY_PRESS_TAB:
			if (getNugget() <= 0)
				break;
			getWorld()->addObject(new Nugget(getWorld(), getX(), getY(), true, true, true));
			decNugget();
		default:
			break;
		}
		getWorld()->removeEarth(getX(), getY());
	}
}

Protester::Protester(StudentWorld* world, int imageID, bool canSee) 
	: CanBeAnnoyed(world, imageID, canSee, 0, 60, 60, 1, left) {
	m_stunned = false;
	m_leaving = false;
	m_restingTick = 0;
	resetTicksSincePerpTurn();
	setSquaresToMove(numSquareToMove());
	setTicksToWaitBetweenMoves();
	setTicksToWaitIfStunned();
}

void Protester::doSomething() {
	if (isDead())
		return;
	if ((isStunned() && getRestingTick() <= getTicksToWaitIfStunned()) || (!isStunned() && getRestingTick() <= getTicksToWaitBetweenMoves())) {
		incRestingTick();
		return;
	}

	int a, b;
	setStunned(false);
	resetRestingTick();
	incTicksSinceLastShout();
	incTicksSincePerpTurn();

	if (isLeaving()) {
		if (getX() == 60 && getY() == 60) {
			setDead();
			return;
		}
		else {
			int dir = getWorld()->getDirExit(getX(), getY());
			setDirection(convertIntToDir(dir));
			if (getWorld()->checkMove(getX(), getY(), getDirection(), a, b, false, nullptr, true))
				moveTo(a, b);
			return;
		}
	}
	else {
		if (getWorld()->findTunnelMan(getX(), getY(), 4) && getWorld()->facingTunnelMan(this) && getTicksSinceLastShout() > 15) {
			getWorld()->playSound(SOUND_PROTESTER_YELL);
			getWorld()->getTunnelMan()->loseHealth(2);
			resetTicksSinceLastShout();
			return;
		}
		else {
			if (!isReg()) {
				int M = computeM();
				int dir = getWorld()->getDirTunnelMan(getX(), getY());
				if (dir != -1 && getWorld()->getDistanceTunnelMan(getX(), getY()) <= M) {
					setDirection(convertIntToDir(dir));
					if (getWorld()->checkMove(getX(), getY(), getDirection(), a, b, false, this, true))
						moveTo(a, b);
					return;
				}
			}
			GraphObject::Direction dir = getWorld()->lineOfSightToTunnelMan(getX(), getY());
			if (dir != none) {
				setDirection(dir);
				if (getWorld()->checkMove(getX(), getY(), dir, a, b, false, nullptr, true))
					moveTo(a, b);
				setSquaresToMove(0);
				return;
			}
			else {
				if (getSquaresToMove() <= 0) {
					getNewDirection();
					setSquaresToMove(numSquareToMove());
				}
				else {
					if (canTurnPerp() != none && m_ticksSincePerpTurn > 200) {
						setDirection(canTurnPerp());
						resetTicksSincePerpTurn();
					}
					if (getWorld()->checkMove(getX(), getY(), getDirection(), a, b, false, nullptr, true)) {
						moveTo(a, b);
						incTicksSincePerpTurn();
						return;
					}
					else {
						setSquaresToMove(0);
						return;
					}
				}
			}
		}
	}
}


GraphObject::Direction Protester::canTurnPerp() {
	int a, b;
	Direction dir[4] = { up, down, left, right };
	switch (getDirection()) {
	case GraphObject::up:
	case GraphObject::down:
		if (getWorld()->checkMove(getX(), getY(), left, a, b, false, nullptr, true) && getWorld()->checkMove(getX(), getY(), right, a, b, false, nullptr, true))
			return dir[2 + rand() % 2];
		else if (getWorld()->checkMove(getX(), getY(), left, a, b, false, nullptr, true))
			return GraphObject::left;
		else if (getWorld()->checkMove(getX(), getY(), right, a, b, false, nullptr, true))
			return GraphObject::right;
		return none;
		break;
	case GraphObject::left:
	case GraphObject::right:
		if (getWorld()->checkMove(getX(), getY(), up, a, b, false, nullptr, true) && getWorld()->checkMove(getX(), getY(), down, a, b, false, nullptr, true))
			return dir[0 + rand() % 2];
		else if (getWorld()->checkMove(getX(), getY(), up, a, b, false, nullptr, true))
			return GraphObject::up;
		else if (getWorld()->checkMove(getX(), getY(), down, a, b, false, nullptr, true))
			return GraphObject::down;
		return none;
		break;
	default:
		return none;
	}
}
GraphObject::Direction Protester::getNewDirection() {
	int a, b;
	Direction dir[4] = { up, down, left, right };
	while (!getWorld()->checkMove(getX(), getY(), getDirection(), a, b, false, nullptr, true)) {
		setDirection(dir[rand() % 4]);
	}
	return getDirection();
}

bool Protester::loseHealth(int amount) {
	if (isLeaving())
		return false;
	setHealth(getHealth() - amount);
	if (getHealth() <= 0) {
		setLeaving();
		getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
		getWorld()->decProtester();
		if (getWorld()->checkBoulder(getX(), getY(), this))
			getWorld()->increaseScore(500);
		else if (isReg())
			getWorld()->increaseScore(100);
		else
			getWorld()->increaseScore(250);
	}
	else {
		getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
		setStunned(true);
	}
	return true;	
}

int Protester::numSquareToMove() const {
	return rand() % 52 + 8;
}

void Protester::setTicksToWaitBetweenMoves() { 
	m_ticksToWaitBetweenMoves = max(0, 3 - (int)getWorld()->getLevel() / 4);
}

void Protester::setTicksToWaitIfStunned() {
	m_ticksToWaitIfStunned = (max(50, 100 - (int)getWorld()->getLevel() * 10));
}

RegProtester::RegProtester(StudentWorld* world) : Protester(world, TID_PROTESTER, true) {
	setHealth(5);
}

void RegProtester::foundGold() {
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	getWorld()->increaseScore(25);
	setLeaving();
}

HardProtester::HardProtester(StudentWorld* world) : Protester(world, TID_HARD_CORE_PROTESTER, true) {
	setHealth(20);
}

void HardProtester::foundGold() {
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	getWorld()->increaseScore(50);
	setStunned(true);
	resetRestingTick();
}
