#ifndef ANIMALS_H
#define ANIMALS_H

#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

class Animal {
public:
    int x, y, health;

    Animal() {
        x = arc4random() % 10;
        y = arc4random() % 10;
    }

    int calculateDistance(Animal& targetAnimal) {
        return int(sqrt(pow(x - targetAnimal.x, 2) + pow(y - targetAnimal.y, 2)));
    }
};

class Fish : public Animal {
public:
    Fish() { health = 5; }

    void move() {
        int moveX = arc4random() % 3 - 1;
        int moveY = arc4random() % 3 - 1;
        x = fmax(0, fmin(9, x + moveX));
        y = fmax(0, fmin(9, y + moveY));
    }
};

class Shark : public Fish {
public:
    Shark() { health = 10; }

    void attack(Fish& fishTarget) {
        int distance = calculateDistance(fishTarget);
        if (distance <= 3) {
            int chance = arc4random() % 100;
            if (chance > 50) {
                fishTarget.health = 0;
            }
        }
    }
};

class Urchin : public Animal {
public:
    Urchin() { health = 3; }

    void prick(Animal& target) {
        int distance = calculateDistance(target);
        if (distance <= 1) {
            int chance = arc4random() % 100;
            if (chance > 30) {
                target.health -= 3;
            }
        }
    }
};

#endif // ANIMALS_H
