#ifndef GUI_H
#define GUI_H

#include <QtGui/QMainWindow>
#include "ui_GUI.h"

class GUI : public QMainWindow
{
    Q_OBJECT
public:
    GUI(QWidget *parent = 0);
    ~GUI();
    static const int STARTING_ROWS = 25;
    static const int STARTING_COLUMNS = 25;
public slots:
	void changeRowsLabel(int);
	void changeColumnsLabel(int);
	void toggleShowRandomPath(bool);
	void toggleShowShortestPath(bool);
private:
    Ui::GUIClass ui;
};

#endif // GUI_H
