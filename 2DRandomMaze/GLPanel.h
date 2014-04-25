/*
 * GLPanel.h
 *
 *  Created on: Jan 14, 2009
 *      Author: kelly
 */

#ifndef GLPANEL_H_
#define GLPANEL_H_

#include <qgl.h>

const int TOP = 0;
const int BOTTOM = 1;
const int LEFT = 2;
const int RIGHT = 3;

class GLPanel : public QGLWidget {
	Q_OBJECT
public:
	GLPanel(QWidget *);
	virtual ~GLPanel();
public slots:
	void changeCellRows(int);
	void changeCellColumns(int);
	void toggleRandomPath(bool);
	void toggleShowRandomPath(bool);
	void toggleShowShortestPath(bool);
	void toggleAnimate(bool);
protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
private:
	struct cell_t {
		int row, column;
		int in, out;
		bool top, bottom, left, right;
		bool path;
	};

	cell_t **cells, **buffer;
	int cellRows, cellColumns;
	cell_t *enter, *exit;
	bool generating, calculating, firstPass;
	bool showRandomPath, showShortestPath, randomPath, animate;

	void clearCells(cell_t **);
	struct GLPanel::cell_t **duplicateCells();
	void generateCells();
	int removePerimeterSide(cell_t **);
	void removeRandomSides();
	void removeSequentialSides(cell_t *);
	void removeSideIndex(int **sides, int size, int indexToRemove);
	bool traverseCells(cell_t *);
	void resetCells();
	bool isValidSide(int row, int column, int side);
	bool calculateRandomPath(cell_t *);
	void calculateShortestPath();
	GLPanel::cell_t **travelFirstPass(cell_t ***queue, int queueSize);
	void enqueueCell(cell_t *cell, cell_t ***queue, int queueSize);
	GLPanel::cell_t *dequeueCell(cell_t ***queue, int queueSize);
	bool reconstructPath(cell_t *cell);
	void displayPath(int row, int column, int side);
};

#endif /* GLPANEL_H_ */
