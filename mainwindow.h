#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QToolButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QList>
#include <QString>
#include <QColor>

class Animal; // Forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void selectFish();
    void selectShark();
    void selectPufferFish();
    void selectPlankton();
    int countPlanktons();

    void oceanClicked(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    void updateSimulation();


private:
    QGraphicsView *oceanView;
    QGraphicsScene *oceanScene;
    QLabel *infoLabel;
    QWidget *selectorPanel;
    QTimer *updateTimer;

    QString selectedType;
    QList<Animal*> animals;

    void setupUI();

    QLabel *infoImageLabel;
    QLabel *infoHealthLabel;
    Animal *currentAnimal;

    QToolButton *fishBtn;
    QToolButton *sharkBtn;
    QToolButton *pufferFishBtn;
    QToolButton *planktonBtn;

    void updateInfoPanel(Animal *a);
};

#endif
