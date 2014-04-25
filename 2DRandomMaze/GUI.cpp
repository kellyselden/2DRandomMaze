#include "GUI.h"

GUI::GUI(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowTitle("2D Random Maze");

	char rows[10];
	sprintf(rows, "Rows: %i", STARTING_ROWS);
	ui.rowsLabel->setText(rows);
	ui.rowSlider->setValue(STARTING_ROWS);
	ui.rowSlider->setMaximum(STARTING_ROWS * 2);

	char columns[13];
	sprintf(columns, "Columns: %i", STARTING_COLUMNS);
	ui.columnsLabel->setText(columns);
	ui.columnSlider->setValue(STARTING_COLUMNS);
	ui.columnSlider->setMaximum(STARTING_COLUMNS * 2);

	connect(ui.rowSlider, SIGNAL(valueChanged(int)),
			SLOT(changeRowsLabel(int)));
	connect(ui.columnSlider, SIGNAL(valueChanged(int)),
			SLOT(changeColumnsLabel(int)));

	connect(ui.showRandomPath, SIGNAL(toggled(bool)),
			SLOT(toggleShowRandomPath(bool)));
	connect(ui.showShortestPath, SIGNAL(toggled(bool)),
			SLOT(toggleShowShortestPath(bool)));

	connect(ui.rowSlider, SIGNAL(valueChanged(int)),
			ui.glWin, SLOT(changeCellRows(int)));
	connect(ui.columnSlider, SIGNAL(valueChanged(int)),
			ui.glWin, SLOT(changeCellColumns(int)));

	connect(ui.randomPath, SIGNAL(toggled(bool)),
			ui.glWin, SLOT(toggleRandomPath(bool)));

	connect(ui.showRandomPath, SIGNAL(toggled(bool)),
			ui.glWin, SLOT(toggleShowRandomPath(bool)));
	connect(ui.showShortestPath, SIGNAL(toggled(bool)),
			ui.glWin, SLOT(toggleShowShortestPath(bool)));

	connect(ui.animate, SIGNAL(toggled(bool)),
			ui.glWin, SLOT(toggleAnimate(bool)));
}

GUI::~GUI() { }

void GUI::changeRowsLabel(int rows)
{
	char string[10];
	sprintf(string, "Rows: %i", rows);
	ui.rowsLabel->setText(string);
}

void GUI::changeColumnsLabel(int columns)
{
	char string[13];
	sprintf(string, "Columns: %i", columns);
	ui.columnsLabel->setText(string);
}

void GUI::toggleShowRandomPath(bool checked)
{
	if (checked && ui.showShortestPath->isChecked())
		ui.showShortestPath->setChecked(false);
}

void GUI::toggleShowShortestPath(bool checked)
{
	if (checked && ui.showRandomPath->isChecked())
		ui.showRandomPath->setChecked(false);
}
