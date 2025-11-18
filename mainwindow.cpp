#include "mainwindow.h"
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <cstdlib>
#include <cmath>
#include <QToolButton>
#include <QPixmap>
using namespace std;

class Animal : public QObject, public QGraphicsRectItem {
public:
    int health;
    QString type;

    Animal(QString t, QColor color, int h) //sets animal type, color and health
        : QGraphicsRectItem(0, 0, 10, 10) {
        type = t;
        health = h;
        setBrush(color);
        setPen(QPen(Qt::black));
    }

    int randMove() {
        return (arc4random() % 3 - 1) * 10; // -10, 0, +10
    }

    void move() {
        int newX = x() + randMove();
        int newY = y() + randMove();
        newX = qMax(0, qMin(790, newX));
        newY = qMax(0, qMin(490, newY));
        setPos(newX, newY);
        health = qMax(0, health - 1); //decreases health every move
    }

    int calculateDistance(Animal &target) { //calculates distance between itself and other animals
        int dx = int(x() - target.x());
        int dy = int(y() - target.y());
        return int(sqrt(dx*dx + dy*dy));
    }

    void eat(Animal &target, int hp, int max_hp) {
        if (health >= max_hp) return; // cannot eat if already full
        int chance = arc4random() % 100;
        if (chance < int(100 * (1.0 - float(health) / max_hp))) { //the more hungry the animal is the greter chance of eating
            target.health = 0;
            health += hp;
        }
    }

    virtual void act(QList<Animal*>& animals, //triggers specific animal behavior
                     QList<Animal*>& newAnimals,
                     QGraphicsScene* scene) = 0; //pure virtual
};

class Fish : public Animal { //Fish Derived Class
public:
    Fish() : Animal("Fish", Qt::yellow, 30) {}

    void huntPlankton(QList<Animal*> &animals) {
        for (Animal *target : animals) {
            if (target->type == "Plankton" && target->health > 0) {
                int distance = calculateDistance(*target);
                if (distance <= 20) { //if taget is at least two squares away the fish may eat and gain 10 HP.
                    eat(*target, 10, 30);
                    break; // stop after eating one
                }
            }
        }
    }

    void act(QList<Animal*>& animals,QList<Animal*>&,QGraphicsScene*) override
    {
        huntPlankton(animals); //overrides the virtual fuction with the specific behavior for the fish
    }
};

class Shark : public Animal {
public:
    Shark() : Animal("Shark", Qt::blue, 50) {}

    void attack(QList<Animal*> &animals) { //attacks any other animal that is not a shark and eats them
        for (Animal *target : animals) {
            if (target == this) continue;
            if ((target->type != "Shark") && target->health > 0) {
                if (calculateDistance(*target) <= 30){
                    if (target->type == "Plankton") //plankton only increases shark's HP by 5
                        eat(*target, 5, 50);
                    else eat(*target, 20, 50);
                    target->health = 0;
                    break; // stop after attacking one
                }
            }
        }
    }

    void act(QList<Animal*>& animals, QList<Animal*>&, QGraphicsScene*) override
    {
        attack(animals);
    }
};

class PufferFish : public Animal {
public:
    PufferFish() : Animal("Pufferfish", QColor(150,0,150), 30) {}

    void prick(QList<Animal*> &animals) {
        for (Animal *target : animals) {
            if (target == this) continue;
            int distance = calculateDistance(*target);
            if (distance <= 20 && (arc4random() % 100) <= 30) {
                target->health -= 10;
            }
            if (target->type == "Plankton" && distance <= 20) {
                eat(*target, 10, 30);
                break; // stop after eating one plankton
            }
        }
    }

    void act(QList<Animal*>& animals, QList<Animal*>&, QGraphicsScene*) override
    {
        prick(animals);
    }
};

class Plankton : public Animal {
private:
    int reproduceCooldown;

public:
    Plankton() : Animal("Plankton", Qt::green, 100), reproduceCooldown(0) {}


    // Called each simulation step; appends newborns to newAnimals (main loop will add them to scene)
    void reproduce(QList<Animal*> &animals, QList<Animal*>& newAnimals) {
        // Respect an overall plankton cap (use the global maxPlankton const)
        if (reproduceCooldown > 0) { --reproduceCooldown; return; }

        int planktonCount = 0;
        for (Animal *a : animals) {
            if (a->type == "Plankton" && a->health > 0) ++planktonCount;
        }
        if (planktonCount >= 40) return;

        // Find a nearby plankton neighbor and attempt to spawn one baby (only 1 per call)
        for (Animal *other : animals) {
            if (other == this || other->type != "Plankton") continue;
            if (calculateDistance(*other) <= 20) {
                if ((arc4random() % 100) < 15) {  // 15% chance when near another plankton
                    Plankton *baby = new Plankton();
                    baby->setPos(x() + 10, y() + 10);
                    newAnimals.append(baby);
                    reproduceCooldown = 5;
                }
                break; // only one baby attempt per frame
            }
        }
    }

    void act(QList<Animal*>& animals, QList<Animal*>& newAnimals, QGraphicsScene*) override {
        reproduce(animals, newAnimals);
    }

};
//Main Window
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI()
{
    //Ocean Scene
    oceanScene = new QGraphicsScene(this);
    oceanScene->setSceneRect(0, 0, 800, 500);
    oceanScene->setBackgroundBrush(QColor(30, 200, 255));

    oceanView = new QGraphicsView(oceanScene);
    oceanView->setRenderHint(QPainter::Antialiasing);
    oceanView->setFixedSize(820, 520);

    // Radar grid
    QPen gridPen(Qt::lightGray);
    for (int x = 0; x <= 810; x += 10)
        oceanScene->addLine(x, 0, x, 500, gridPen);
    for (int y = 0; y <= 510; y += 10)
        oceanScene->addLine(0, y, 800, y, gridPen);

    // Info Panel
    infoImageLabel = new QLabel;
    infoImageLabel->setAlignment(Qt::AlignCenter);
    infoImageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    infoImageLabel->setScaledContents(true);

    infoHealthLabel = new QLabel("Health: ");
    infoHealthLabel->setAlignment(Qt::AlignCenter);
    infoHealthLabel->setFixedHeight(10);

    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(infoImageLabel);
    infoLayout->addWidget(infoHealthLabel);

    QWidget *infoPanel = new QWidget;
    infoPanel->setLayout(infoLayout);

    //Selector Panel
    selectorPanel = new QWidget;
    QGridLayout *selectorLayout = new QGridLayout(selectorPanel);
    selectorLayout->setSpacing(10);
    selectorLayout->setAlignment(Qt::AlignTop);

    // Fish Button
    fishBtn = new QToolButton;
    fishBtn->setText("Goldfish");
    fishBtn->setFixedSize(160, 60);

    // Shark Button
    sharkBtn = new QToolButton;
    sharkBtn->setText("Shark");
    sharkBtn->setFixedSize(160, 60);

    // Puffer Button
    pufferFishBtn = new QToolButton;
    pufferFishBtn->setText("Pufferfish");
    pufferFishBtn->setFixedSize(160, 60);

    // Plankton Button
    planktonBtn = new QToolButton;
    planktonBtn->setText("Plankton");
    planktonBtn->setFixedSize(160, 60);


    // Button layout
    selectorLayout->addWidget(fishBtn, 0, 0);
    selectorLayout->addWidget(sharkBtn, 0, 1);
    selectorLayout->addWidget(pufferFishBtn, 1, 0);
    selectorLayout->addWidget(planktonBtn, 1, 1);

    // Connect buttons
    connect(fishBtn, &QToolButton::clicked, this, &MainWindow::selectFish);
    connect(sharkBtn, &QToolButton::clicked, this, &MainWindow::selectShark);
    connect(pufferFishBtn, &QToolButton::clicked, this, &MainWindow::selectPufferFish);
    connect(planktonBtn, &QToolButton::clicked, this, &MainWindow::selectPlankton);

    // Right Panels
    QWidget *rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(infoPanel);
    rightLayout->addWidget(selectorPanel);

    //Main Layout
    QWidget *central = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->addWidget(oceanView);
    mainLayout->addWidget(rightPanel);
    setCentralWidget(central);
    setWindowTitle("Survival Aquarium");
    resize(1250, 600);
    setFixedSize(1250,600);

    updateInfoPanel(nullptr); // Main Screen

    // Mouse click handling
    oceanView->viewport()->installEventFilter(this);

    // Frame rate
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    updateTimer->start(1500); // every 1.5 seconds
}

// Events
bool MainWindow::eventFilter(QObject *obj, QEvent *event){
    if (obj == oceanView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        oceanClicked(mouseEvent);
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}
// Clear board
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_K)
    {
        for (int i = animals.size() - 1; i >= 0; --i) { // Remove all animals by pressing "K"
            Animal* a = animals[i];

            if (a == currentAnimal)
                currentAnimal = nullptr;

            oceanScene->removeItem(a);
            delete a;
            animals.removeAt(i);
        }

        updateInfoPanel(nullptr); // Reset panel
    }
}

// Button pressing
void MainWindow::selectFish()
{
    if (selectedType == "Fish") {
        selectedType.clear();
        updateInfoPanel(nullptr);
    } else {
        selectedType = "Fish";
        static Fish previewFish;
        updateInfoPanel(&previewFish);
    }
}

void MainWindow::selectShark()
{
    if (selectedType == "Shark") {
        selectedType.clear();
        updateInfoPanel(nullptr);
    } else {
        selectedType = "Shark";
        static Shark previewShark;
        updateInfoPanel(&previewShark);
    }
}

void MainWindow::selectPufferFish()
{
    if (selectedType == "Pufferfish") {
        selectedType.clear();
        updateInfoPanel(nullptr);
    } else {
        selectedType = "Pufferfish";
        static PufferFish previewPuffer;
        updateInfoPanel(&previewPuffer);
    }
}

void MainWindow::selectPlankton()
{
    if (selectedType == "Plankton") {
        selectedType.clear();
        updateInfoPanel(nullptr);
    } else {
        selectedType = "Plankton";
        static Plankton previewPlankton;
        updateInfoPanel(&previewPlankton);
    }
}

//Counts plankton for control
int MainWindow::countPlanktons() {
    int count = 0;
    for (Animal *a : animals) {
        if (a->type == "Plankton" && a->health > 0) count++;
    }
    return count;
}

// Clicks withing Oocean window
void MainWindow::oceanClicked(QMouseEvent *event)
{
    QPointF scenePos = oceanView->mapToScene(event->pos());

    // Check if clicking on existing animal
    QList<QGraphicsItem*> items = oceanScene->items(scenePos);
    for (QGraphicsItem* item : items) {
        Animal *a = dynamic_cast<Animal*>(item);
        if (a) {
            updateInfoPanel(a);
            return;
        }
    }

    // Add new animal if a type is selected
    if (selectedType.isEmpty()) return;

    Animal *animal = nullptr;
    if (selectedType == "Fish") animal = new Fish();
    else if (selectedType == "Shark") animal = new Shark();
    else if (selectedType == "Pufferfish") animal = new PufferFish();
    else if (selectedType == "Plankton") animal = new Plankton();

    if (animal) {
        animal->setPos(scenePos.x(), scenePos.y());
        oceanScene->addItem(animal);
        animals.append(animal);
    }
}

// Info Panel
void MainWindow::updateInfoPanel(Animal *a)
{
    // If no animal is selected, show default ocean image
    if (!a) {
        infoImageLabel->setPixmap(QPixmap(":/images/maintitle.png").scaled(infoImageLabel->size(), Qt::KeepAspectRatio));
        infoHealthLabel->setText("Select an Animal!");
        return;
    }
    if (animals.contains(a))
        currentAnimal = a;
    else
        currentAnimal = nullptr;

    // Show corresponding animal image
    if (a->type == "Fish")
        infoImageLabel->setPixmap(QPixmap(":/images/fish.png").scaled(infoImageLabel->size(), Qt::KeepAspectRatio));
    else if (a->type == "Shark")
        infoImageLabel->setPixmap(QPixmap(":/images/shark.png").scaled(infoImageLabel->size(), Qt::KeepAspectRatio));
    else if (a->type == "Pufferfish")
        infoImageLabel->setPixmap(QPixmap(":/images/puffer.png").scaled(infoImageLabel->size(), Qt::KeepAspectRatio));
    else if (a->type == "Plankton")
        infoImageLabel->setPixmap(QPixmap(":/images/plankton.png").scaled(infoImageLabel->size(), Qt::KeepAspectRatio));

    // Health displayed at bottom
    infoHealthLabel->setText(QString(a->type) + "'s Health: " + QString::number(a->health));
}

QMap<Animal*, int> planktonCooldowns;

void MainWindow::updateSimulation()
{
    QList<Animal*> newAnimals;
    QList<Animal*> tempAnimals = animals;

    for (Animal* a : tempAnimals) {
        if (a->health <= 0) continue;
        a->move();
        a->act(animals, newAnimals, oceanScene);
    }

    for (Animal* baby : newAnimals) {
        if (!baby) continue;
        animals.append(baby);
        oceanScene->addItem(baby);
    }

    // Remove dead animals
    for (int i = animals.size() - 1; i >= 0; --i) {
        Animal* a = animals[i];
        if (a->health <= 0) {
            if (a == currentAnimal) currentAnimal = nullptr;
            if (planktonCooldowns.contains(a)) planktonCooldowns.remove(a);

            oceanScene->removeItem(a);
            delete a;
            animals.removeAt(i);
        }
    }
    // Refresh the info panel
    if (currentAnimal && currentAnimal->health > 0) {
        updateInfoPanel(currentAnimal);
    }
}
