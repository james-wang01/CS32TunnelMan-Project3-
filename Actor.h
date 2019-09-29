#ifndef ACTOR_H_
#define ACTOR_H_

#include "StudentWorld.h"
#include "GraphObject.h"

class Object : public GraphObject {
public:
	Object(StudentWorld* world, int imageID, bool canSee, unsigned int depth = 0, int x = 0, int y = 0, double size = 1.0, GraphObject::Direction dir = right);
	virtual ~Object() {}			//doesnt do anything
	virtual void doSomething() = 0;	//pure virtual
	virtual bool loseHealth(int amount) { return false; }
	virtual bool isBoulder() const { return false; }
	virtual bool isProtester() const { return false; }
	virtual void setStunned() { return; }
	virtual void foundGold() { return; }
	//ACCESSORS
	virtual bool isReg() const { return false; }
	StudentWorld* getWorld() const { return m_world; }
	bool isDead() const { return m_dead; }
	//MUTATORS
	void setDead() { m_dead = true; }
	//HELPER
	GraphObject::Direction convertIntToDir(int dir);
private:
	StudentWorld* m_world;
	bool m_dead;
};

class DisappearingObject : public Object {
public:
	DisappearingObject(StudentWorld* world, int imageID, bool visible, unsigned int depth, int x, int y);
	virtual ~DisappearingObject() {}
	virtual void doSomething() = 0;
	virtual bool loseHealth(int amount) { return false; }
	//ACCESSORS
	int getTimer() const { return m_ticks; }
	int getLifespan() const { return m_lifespan; }
	bool getTemporary() const { return m_temporary; }
	//MUTATORS
	void incTimer() { m_ticks++; }
	void setLifespan(int time) { m_lifespan = time; }
	void setTemporary(bool temporary) { m_temporary = temporary; }
	//Other functions
	void checkDead();
private:
	bool m_temporary;
	int m_ticks;
	int m_lifespan;
};

class Nugget : public DisappearingObject {
public:
	Nugget(StudentWorld* world, int x, int y, bool visible, bool pickedUpByPlayer, bool temporary);
	~Nugget() {}
	virtual void doSomething();
	bool getPickedUpByPlayer() { return m_pickedUpByPlayer; }
private:
	bool m_pickedUpByPlayer;
};

class Sonar : public DisappearingObject {
public:
	Sonar(StudentWorld* world, int x, int y);
	~Sonar() {}
	virtual void doSomething();
};

class Water : public DisappearingObject {
public:
	Water(StudentWorld* world, int x, int y);
	~Water() {}
	virtual void doSomething();
};

class Earth : public Object {
public:
	Earth(StudentWorld* world, int x, int y) : Object(world, TID_EARTH, true, 3, x, y, 0.25) {}
	virtual void doSomething() {}	//does nothing
	virtual ~Earth() {}				//no dynamic memory
};

class Boulder : public Object { 
public:
	Boulder(StudentWorld* world, int x, int y);
	~Boulder() {}
	virtual void doSomething();
	virtual bool isBoulder() const { return true; }
	bool boulderFall();
private:
	bool m_stable;
	bool m_falling;
	int m_ticks;
};

class Squirt : public Object {
public:
	Squirt(StudentWorld* world, int x, int y, GraphObject::Direction dir);
	~Squirt() {}
	virtual void doSomething();
	//ACCESSORS
	int getDistanceTravelled() { return m_distanceTravelled; }
	//MUTATORS
	void incDistanceTravelled() { m_distanceTravelled++; }
private:
	int m_distanceTravelled;
};

class Oil : public Object {
public:
	Oil(StudentWorld* world, int x, int y);
	~Oil() {}
	virtual void doSomething();
};

class CanBeAnnoyed : public Object {
public:
	CanBeAnnoyed(StudentWorld* world, int imageID, bool canSee, int depth, int x, int y, int size = 1.0, Direction dir = right);
	virtual ~CanBeAnnoyed() {}
	virtual void doSomething() = 0;
	virtual bool loseHealth(int amount) { return true; }
	//ACCESSORS
	int getHealth() const { return m_health; }
	//MUTATORS
	void setHealth(int amount) { m_health = amount; }
private:
	int m_health;
};

class TunnelMan : public CanBeAnnoyed {
public:
	TunnelMan(StudentWorld* world);
	~TunnelMan() {}
	void doSomething();
	//ACCESSORS
	int getNugget() const { return m_nugget; }
	int getWater() const { return m_water; }
	int getSonar() const { return m_sonar; }
	//MUTATORS
	bool loseHealth(int amount);
	void addNugget() { m_nugget++; }
	void decNugget() { m_nugget--; }
	void addSonar() { m_sonar += 2; }
	void decSonar() { m_sonar--; }
	void addWater(int amount) { m_water += amount; }
	void decWater() { m_water--; }
private:
	int m_water;
	int m_sonar;
	int m_nugget;
};

class Protester : public CanBeAnnoyed {
public:
	Protester(StudentWorld* world, int imageID, bool canSee);
	virtual void doSomething();
	virtual void foundGold() = 0;
	virtual ~Protester() {}
	//ACCESSORS
	virtual bool isReg() const { return false; }
	virtual bool isProtester() const { return true; }
	virtual int computeM() { return 0; }
	bool isLeaving() const { return m_leaving; }
	bool isStunned() const { return m_stunned; }
	int getRestingTick() const { return m_restingTick; }
	int getTicksToWaitBetweenMoves() const { return m_ticksToWaitBetweenMoves; }
	int getSquaresToMove() const { return m_squaresToMove; }
	int getTicksSinceLastShout() const { return m_ticksSinceLastShout; }
	int getTicksToWaitIfStunned() const { return m_ticksToWaitIfStunned; }
	//MUTATORS
	virtual bool loseHealth(int amount);
	void setStunned(bool stunned) { m_stunned = stunned; }
	void setLeaving() { m_leaving = true; m_ticksToWaitBetweenMoves = 0; m_stunned = false; }
	void incTicksSincePerpTurn() { m_ticksSincePerpTurn++; }
	void resetTicksSincePerpTurn() { m_ticksSincePerpTurn = 0; }
	void incTicksSinceLastShout() { m_ticksSinceLastShout++; }
	void resetTicksSinceLastShout() { m_ticksSinceLastShout = 0; }
	void incRestingTick() { m_restingTick++; }
	void resetRestingTick() { m_restingTick = 0; }
	void setSquaresToMove(int amount) { m_squaresToMove = amount; }
	//EXTRA
	int numSquareToMove() const;
	void setTicksToWaitBetweenMoves();
	void setTicksToWaitIfStunned();
	GraphObject::Direction getNewDirection();
	GraphObject::Direction canTurnPerp();
private:
	bool m_stunned;
	bool m_leaving;
	int m_restingTick;
	int m_ticksSincePerpTurn;
	int m_ticksToWaitBetweenMoves;
	int m_ticksToWaitIfStunned;
	int m_squaresToMove;
	int m_ticksSinceLastShout;
};

class RegProtester : public Protester {
public:
	RegProtester(StudentWorld* world);
	~RegProtester() {}
	virtual bool isReg() const { return true; }
	virtual void foundGold();
};

class HardProtester : public Protester {
public:
	HardProtester(StudentWorld* world);
	~HardProtester() {}
	virtual bool isReg() const { return false; }
	virtual void foundGold();
	virtual int computeM() { return 16 + getWorld()->getLevel() * 2; }
};

#endif // ACTOR_H_
