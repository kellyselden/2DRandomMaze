/*
 * GLPanel.cpp
 *
 *  Created on: Jan 14, 2009
 *      Author: kelly
 */

#include "GLPanel.h"
#include "GUI.h"
#include "time.h"

GLPanel::GLPanel(QWidget *parent) : QGLWidget(parent)
{
	cellRows = GUI::STARTING_ROWS;
	cellColumns = GUI::STARTING_COLUMNS;
	randomPath = true;
	showRandomPath = showShortestPath = false;
	animate = false;
	firstPass = false;
	srand(time(NULL));
}

GLPanel::~GLPanel() { }

void GLPanel::initializeGL()
{
	//use black background color
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void GLPanel::resizeGL(int width, int height)
{
	//use the entire available drawing area
	glViewport(0, 0, width, height);

	generating = true;

	generateCells();

	generating = false;
}

void GLPanel::paintGL()
{
	//clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//color: PeachPuff (HexCode: FF DA B9)
	glColor3ub(0xFF, 0xDA, 0xB9);

	glBegin(GL_LINES);

	for (int column = 0; column < cellColumns; column++)
		for (int row = 0; row < cellRows; row++)
		{
			cell_t cell = cells[column][row];

			if (cell.bottom)
			{
				glVertex2f(column, row);
				glVertex2f(column + 1, row);
			}

			if (cell.right)
			{
				glVertex2f(column + 1, row);
				glVertex2f(column + 1, row + 1);
			}

			if (cell.top)
			{
				glVertex2f(column + 1, row + 1);
				glVertex2f(column, row + 1);
			}

			if (cell.left)
			{
				glVertex2f(column, row + 1);
				glVertex2f(column, row);
			}
		}

	glEnd();

	if (firstPass)
	{
		glColor3ub(0xFF, 0x00, 0x00);

		glBegin(GL_QUADS);

		for (int column = 0; column < cellColumns; column++)
			for (int row = 0; row < cellRows; row++)
			{
				cell_t cell = buffer[column][row];

				if (cell.path && showShortestPath && !generating)
				{
					glVertex2f(column + .1, row + .1);
					glVertex2f(column + .9, row + .1);
					glVertex2f(column + .9, row + .9);
					glVertex2f(column + .1, row + .9);
				}
			}

		glEnd();
	}

	glColor3ub(0x4D, 0x4D, 0xFF);

	glBegin(GL_LINES);

	for (int column = 0; column < cellColumns; column++)
		for (int row = 0; row < cellRows; row++)
		{
			cell_t cell = cells[column][row];

			if (cell.path && (showRandomPath || showShortestPath) && !generating)
			{
				displayPath(row, column, cell.in);
				displayPath(row, column, cell.out);
			}
		}

	glEnd();
}

void GLPanel::displayPath(int row, int column, int side)
{
	switch (side)
	{
	case TOP:
		glVertex2f(column + .5, row + 1);
		glVertex2f(column + .5, row + .5);
		break;
	case BOTTOM:
		glVertex2f(column + .5, row);
		glVertex2f(column + .5, row + .5);
		break;
	case LEFT:
		glVertex2f(column, row + .5);
		glVertex2f(column + .5, row + .5);
		break;
	case RIGHT:
		glVertex2f(column + 1, row + .5);
		glVertex2f(column + .5, row + .5);
		break;
	}
}

void GLPanel::changeCellRows(int rows)
{
	generating = true;

	clearCells(cells);
	cellRows = rows;
	generateCells();

	calculating = generating;
	generating = false;

	if (showRandomPath)
	{
		resetCells();
		calculateRandomPath(enter);
	}
	if (showShortestPath)
	{
		resetCells();
		calculateShortestPath();
	}
	updateGL();

	calculating = false;
}

void GLPanel::changeCellColumns(int columns)
{
	generating = true;

	clearCells(cells);
	cellColumns = columns;
	generateCells();

	calculating = generating;
	generating = false;

	if (showRandomPath)
	{
		resetCells();
		calculateRandomPath(enter);
	}
	if (showShortestPath)
	{
		resetCells();
		calculateShortestPath();
	}
	updateGL();

	calculating = false;
}

void GLPanel::toggleRandomPath(bool checked)
{
	generating = true;

	clearCells(cells);
	randomPath = checked;
	generateCells();

	calculating = generating;
	generating = false;

	if (showRandomPath)
	{
		resetCells();
		calculateRandomPath(enter);
	}
	if (showShortestPath)
	{
		resetCells();
		calculateShortestPath();
	}
	updateGL();

	calculating = false;
}

void GLPanel::toggleShowRandomPath(bool checked)
{
	calculating = true;

	if ((showRandomPath = checked) && !generating)
	{
		showShortestPath = false;
		resetCells();
		calculateRandomPath(enter);
	}
	updateGL();

	calculating = false;
}

void GLPanel::toggleShowShortestPath(bool checked)
{
	calculating = true;

	if ((showShortestPath = checked) && !generating)
	{
		showRandomPath = false;
		resetCells();
		calculateShortestPath();
	}
	updateGL();

	calculating = false;
}

void GLPanel::toggleAnimate(bool checked)
{
	animate = checked;
}

void GLPanel::clearCells(cell_t **cells)
{
	for (int i = 0; i < cellColumns; i++)
		free(cells[i]);
	free(cells);
}

struct GLPanel::cell_t **GLPanel::duplicateCells()
{
	cell_t **newCells = (cell_t **)malloc(cellColumns * sizeof(cell_t *));
	for (int i = 0; i < cellColumns; i++)
	{
		newCells[i] = (cell_t *)malloc(cellRows * sizeof(cell_t));
		memcpy(newCells[i], cells[i], cellRows * sizeof(cell_t));
	}

	return newCells;
}

void GLPanel::generateCells()
{
	//use 2D orthographic projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1, cellColumns + 1, -1, cellRows + 1);

	//always set the matrix mode back to ModelView
	glMatrixMode(GL_MODELVIEW);

	cells = (cell_t **)malloc(cellColumns * sizeof(cell_t *));
	for (int i = 0; i < cellColumns; i++)
		cells[i] = (cell_t *)malloc(cellRows * sizeof(cell_t));

	//populate all sides
	for (int column = 0; column < cellColumns; column++)
		for (int row = 0; row < cellRows; row++)
		{
			cell_t *cell = &cells[column][row];
			cell->column = column;
			cell->row = row;
			cell->bottom = cell->right = cell->top = cell->left = true;
			cell->path = false;
		}

	//create entrance and exit
	int side = removePerimeterSide(&enter);
	enter->in = side;
	side = removePerimeterSide(&exit);
	exit->out = side;

	if (randomPath)
		removeRandomSides();
	else
		removeSequentialSides(enter);
}

int GLPanel::removePerimeterSide(cell_t **cell)
{
	int row, column, side;
	bool isValid = false;

	while (!isValid)
		switch (side = rand() % 4)
		{
		case TOP:
			column = rand() % cellColumns;
			if (!cells[column][cellRows - 1].top)
				continue;
			if (column == 0)
			{
				if (!cells[column][cellRows - 1].left)
					continue;
			}
			else if (!cells[column - 1][cellRows - 1].top)
				continue;
			if (column == cellColumns - 1)
			{
				if (!cells[column][cellRows - 1].right)
					continue;
			}
			else if (!cells[column + 1][cellRows - 1].top)
				continue;
			*cell = &cells[column][cellRows - 1];
			(*cell)->top = false;
			isValid = true;
			break;
		case BOTTOM:
			column = rand() % cellColumns;
			if (!cells[column][0].bottom)
				continue;
			if (column == 0)
			{
				if (!cells[column][0].left)
					continue;
			}
			else if (!cells[column - 1][0].bottom)
				continue;
			if (column == cellColumns - 1)
			{
				if (!cells[column][0].right)
					continue;
			}
			else if (!cells[column + 1][0].bottom)
				continue;
			*cell = &cells[column][0];
			(*cell)->bottom = false;
			isValid = true;
			break;
		case LEFT:
			row = rand() % cellRows;
			if (!cells[0][row].left)
				continue;
			if (row == 0)
			{
				if (!cells[0][row].bottom)
					continue;
			}
			else if (!cells[0][row - 1].left)
				continue;
			if (row == cellRows - 1)
			{
				if (!cells[0][row].top)
					continue;
			}
			else if (!cells[0][row + 1].left)
				continue;
			*cell = &cells[0][row];
			(*cell)->left = false;
			isValid = true;
			break;
		case RIGHT:
			row = rand() % cellRows;
			if (!cells[cellColumns - 1][row].right)
				continue;
			if (row == 0)
			{
				if (!cells[cellColumns - 1][row].bottom)
					continue;
			}
			else if (!cells[cellColumns - 1][row - 1].right)
				continue;
			if (row == cellRows - 1)
			{
				if (!cells[cellColumns - 1][row].top)
					continue;
			}
			else if (!cells[cellColumns - 1][row + 1].right)
				continue;
			*cell = &cells[cellColumns - 1][row];
			(*cell)->right = false;
			isValid = true;
			break;
		}

	return side;
}

void GLPanel::removeRandomSides()
{
	QApplication::processEvents();

	if (!generating)
		return;

	if (animate)
		updateGL();

	if (traverseCells(enter))
		return;

	resetCells();

	int row, column, side;
	do
	{
		row = rand() % cellRows;
		column = rand() % cellColumns;
		side = rand() % 4;
	}
	while (!isValidSide(row, column, side));

	switch (side)
	{
	case TOP:
		(&cells[column][row])->top = (&cells[column][row + 1])->bottom = false;
		break;
	case BOTTOM:
		(&cells[column][row])->bottom = (&cells[column][row - 1])->top = false;
		break;
	case LEFT:
		(&cells[column][row])->left = (&cells[column - 1][row])->right = false;
		break;
	case RIGHT:
		(&cells[column][row])->right = (&cells[column + 1][row])->left = false;
		break;
	}

	removeRandomSides();
}

void GLPanel::removeSequentialSides(cell_t *cell)
{
	QApplication::processEvents();

	if (!generating)
		return;

	if (animate)
		updateGL();

	cell->path = true;

	int sidesRemaining = 4;
	int *sides = (int *)malloc(sidesRemaining * sizeof(int));
	for (int i = 0; i < sidesRemaining; i++)
		sides[i] = i;

	while (sidesRemaining)
	{
		if (!generating)
			break;

		int random = rand() % sidesRemaining;
		cell_t *nextCell;
		switch (sides[random])
		{
		case TOP:
			if (cell->row != cellRows -1 && !(nextCell = &cells[cell->column][cell->row + 1])->path)
			{
				cell->top = nextCell->bottom = false;
				removeSequentialSides(nextCell);
			}
			break;
		case BOTTOM:
			if (cell->row != 0 && !(nextCell = &cells[cell->column][cell->row - 1])->path)
			{
				cell->bottom = nextCell->top = false;
				removeSequentialSides(nextCell);
			}
			break;
		case LEFT:
			if (cell->column != 0 && !(nextCell = &cells[cell->column - 1][cell->row])->path)
			{
				cell->left = nextCell->right = false;
				removeSequentialSides(nextCell);
			}
			break;
		case RIGHT:
			if (cell->column != cellColumns - 1 && !(nextCell = &cells[cell->column + 1][cell->row])->path)
			{
				cell->right = nextCell->left = false;
				removeSequentialSides(nextCell);
			}
			break;
		}

		removeSideIndex(&sides, sidesRemaining, random);
		sidesRemaining--;
	}
	free(sides);
}

void GLPanel::removeSideIndex(int **sides, int size, int indexToRemove)
{
	int *newSides = (int *)malloc((size - 1) * sizeof(int));

	for (int i = 0; i < size; i++)
		if (i == indexToRemove)
			continue;
		else
			newSides[i < indexToRemove ? i : i - 1] = (*sides)[i];

	free(*sides);
	*sides = newSides;
}

//checks that every cell is connected
bool GLPanel::traverseCells(cell_t *cell)
{
	cell->path = true;

	cell_t *nextCell;
	if ((!cell->top && cell->row != cellRows - 1 && !(nextCell = &cells[cell->column][cell->row + 1])->path && traverseCells(nextCell))
			|| (!cell->bottom && cell->row != 0 && !(nextCell = &cells[cell->column][cell->row - 1])->path && traverseCells(nextCell))
			|| (!cell->left && cell->column != 0 && !(nextCell = &cells[cell->column - 1][cell->row])->path && traverseCells(nextCell))
			|| (!cell->right && cell->column != cellColumns - 1 && !(nextCell = &cells[cell->column + 1][cell->row])->path && traverseCells(nextCell)))
		return true;

	for (int column = 0; column < cellColumns; column++)
		for (int row = 0; row < cellRows; row++)
			if (!cells[column][row].path)
				return false;
	return true;
}

void GLPanel::resetCells()
{
	if (!generating && !calculating)
		return;

	for (int column = 0; column < cellColumns; column++)
		for (int row = 0; row < cellRows; row++)
			(&cells[column][row])->path = false;
}

bool GLPanel::isValidSide(int row, int column, int side)
{
	switch (side)
	{
	case TOP:
		if (row == cellRows - 1)
			return false;
		if (column != 0)
			if (!cells[column][row].left
					&& !cells[column][row + 1].left
					&& !cells[column - 1][row].top)
				return false;
		if (column != cellColumns - 1)
			if (!cells[column][row].right
					&& !cells[column][row + 1].right
					&& !cells[column + 1][row].top)
				return false;
		break;
	case BOTTOM:
		if (row == 0)
			return false;
		if (column != 0)
			if (!cells[column][row].left
					&& !cells[column][row - 1].left
					&& !cells[column - 1][row].bottom)
				return false;
		if (column != cellColumns - 1)
			if (!cells[column][row].right
					&& !cells[column][row - 1].right
					&& !cells[column + 1][row].bottom)
				return false;
		break;
	case LEFT:
		if (column == 0)
			return false;
		if (row != 0)
			if (!cells[column][row].bottom
					&& !cells[column - 1][row].bottom
					&& !cells[column][row - 1].left)
				return false;
		if (row != cellRows - 1)
			if (!cells[column][row].top
					&& !cells[column - 1][row].top
					&& !cells[column][row + 1].left)
				return false;
		break;
	case RIGHT:
		if (column == cellColumns - 1)
			return false;
		if (row != 0)
			if (!cells[column][row].bottom
					&& !cells[column + 1][row].bottom
					&& !cells[column][row - 1].right)
				return false;
		if (row != cellRows - 1)
			if (!cells[column][row].top
					&& !cells[column + 1][row].top
					&& !cells[column][row + 1].right)
				return false;
		break;
	}
	return true;
}

bool GLPanel::calculateRandomPath(cell_t *cell)
{
	QApplication::processEvents();

	if (!calculating)
		return false;

	if (animate)
		updateGL();

	cell->path = true;

	if (cell == exit)
		return true;

	int sidesRemaining = 4;
	int *sides = (int *)malloc(sidesRemaining * sizeof(int));
	for (int i = 0; i < sidesRemaining; i++)
		sides[i] = i;

	while (sidesRemaining)
	{
		if (!calculating)
			break;

		int random = rand() % sidesRemaining;
		cell_t *nextCell;
		switch (sides[random])
		{
		case TOP:
			if (!cell->top && cell->row != cellRows - 1 && !(nextCell = &cells[cell->column][cell->row + 1])->path)
			{
				cell->out = TOP;
				nextCell->in = BOTTOM;
				if (calculateRandomPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		case BOTTOM:
			if (!cell->bottom && cell->row != 0 && !(nextCell = &cells[cell->column][cell->row - 1])->path)
			{
				cell->out = BOTTOM;
				nextCell->in = TOP;
				if (calculateRandomPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		case LEFT:
			if (!cell->left && cell->column != 0 && !(nextCell = &cells[cell->column - 1][cell->row])->path)
			{
				cell->out = LEFT;
				nextCell->in = RIGHT;
				if (calculateRandomPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		case RIGHT:
			if (!cell->right && cell->column != cellColumns - 1 && !(nextCell = &cells[cell->column + 1][cell->row])->path)
			{
				cell->out = RIGHT;
				nextCell->in = LEFT;
				if (calculateRandomPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		}
		removeSideIndex(&sides, sidesRemaining, random);
		sidesRemaining--;
	}
	free(sides);

	if (!calculating)
		return false;

	return cell->path = false;
}

void GLPanel::calculateShortestPath()
{
	buffer = duplicateCells();
	(&buffer[enter->column][enter->row])->path = true;
	cell_t **queue = (cell_t **)malloc(sizeof(cell_t *));
	queue[0] = &buffer[enter->column][enter->row];
	firstPass = true;
	free(travelFirstPass(&queue, 1));
	firstPass = false;
	reconstructPath(enter);
	if (calculating)
		clearCells(buffer);
}

GLPanel::cell_t **GLPanel::travelFirstPass(cell_t ***queue, int queueSize)
{
	QApplication::processEvents();

	if (!calculating)
		return *queue;

	if (animate)
		updateGL();

	cell_t *cell = dequeueCell(queue, queueSize);
	queueSize--;

	if (cell == &buffer[exit->column][exit->row])
		return *queue;

	int sidesRemaining = 4;
	int *sides = (int *)malloc(sidesRemaining * sizeof(int));
	for (int i = 0; i < sidesRemaining; i++)
		sides[i] = i;

	while (sidesRemaining)
	{
		if (!calculating)
			break;

		int random = rand() % sidesRemaining;
		cell_t *nextCell;
		switch (sides[random])
		{
		case TOP:
			if (!cell->top && cell->row != cellRows - 1 && !(nextCell = &buffer[cell->column][cell->row + 1])->path)
			{
				nextCell->path = true;
				enqueueCell(nextCell, queue, queueSize);
				queueSize++;
			}
			else
				cell->top = true;
			break;
		case BOTTOM:
			if (!cell->bottom && cell->row != 0 && !(nextCell = &buffer[cell->column][cell->row - 1])->path)
			{
				nextCell->path = true;
				enqueueCell(nextCell, queue, queueSize);
				queueSize++;
			}
			else
				cell->bottom = true;
			break;
		case LEFT:
			if (!cell->left && cell->column != 0 && !(nextCell = &buffer[cell->column - 1][cell->row])->path)
			{
				nextCell->path = true;
				enqueueCell(nextCell, queue, queueSize);
				queueSize++;
			}
			else
				cell->left = true;
			break;
		case RIGHT:
			if (!cell->right && cell->column != cellColumns - 1 && !(nextCell = &buffer[cell->column + 1][cell->row])->path)
			{
				nextCell->path = true;
				enqueueCell(nextCell, queue, queueSize);
				queueSize++;
			}
			else
				cell->right = true;
			break;
		}
		removeSideIndex(&sides, sidesRemaining, random);
		sidesRemaining--;
	}
	free(sides);

	if (!calculating)
		return *queue;

	return travelFirstPass(queue, queueSize);
}

void GLPanel::enqueueCell(cell_t *cell, cell_t ***queue, int queueSize)
{
	cell_t **newQueue = (cell_t **)malloc((queueSize + 1) * sizeof(cell_t *));

	for (int i = 0; i < queueSize; i++)
		newQueue[i] = (*queue)[i];

	newQueue[queueSize] = cell;

	free(*queue);
	*queue = newQueue;
}

GLPanel::cell_t *GLPanel::dequeueCell(cell_t ***queue, int queueSize)
{
	cell_t *cell = (*queue)[0];

	cell_t **newQueue = (cell_t **)malloc((queueSize - 1) * sizeof(cell_t *));

	for (int i = 1; i < queueSize; i++)
		newQueue[i - 1] = (*queue)[i];

	free(*queue);
	*queue = newQueue;

	return cell;
}

bool GLPanel::reconstructPath(cell_t *cell)
{
	QApplication::processEvents();

	if (!calculating)
		return false;

	if (animate)
		updateGL();

	cell->path = true;

	if (cell == exit)
		return true;

	int sidesRemaining = 4;
	int *sides = (int *)malloc(sidesRemaining * sizeof(int));
	for (int i = 0; i < sidesRemaining; i++)
		sides[i] = i;

	while (sidesRemaining)
	{
		if (!calculating)
			return false;

		int random = rand() % sidesRemaining;
		cell_t *nextCell;
		switch (sides[random])
		{
		case TOP:
			if (!(&buffer[cell->column][cell->row])->top && cell->row != cellRows - 1 && !(nextCell = &cells[cell->column][cell->row + 1])->path && (&buffer[nextCell->column][nextCell->row])->path)
			{
				cell->out = TOP;
				nextCell->in = BOTTOM;
				if (reconstructPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		case BOTTOM:
			if (!(&buffer[cell->column][cell->row])->bottom && cell->row != 0 && !(nextCell = &cells[cell->column][cell->row - 1])->path && (&buffer[nextCell->column][nextCell->row])->path)
			{
				cell->out = BOTTOM;
				nextCell->in = TOP;
				if (reconstructPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		case LEFT:
			if (!(&buffer[cell->column][cell->row])->left && cell->column != 0 && !(nextCell = &cells[cell->column - 1][cell->row])->path && (&buffer[nextCell->column][nextCell->row])->path)
			{
				cell->out = LEFT;
				nextCell->in = RIGHT;
				if (reconstructPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		case RIGHT:
			if (!(&buffer[cell->column][cell->row])->right && cell->column != cellColumns - 1 && !(nextCell = &cells[cell->column + 1][cell->row])->path && (&buffer[nextCell->column][nextCell->row])->path)
			{
				cell->out = RIGHT;
				nextCell->in = LEFT;
				if (reconstructPath(nextCell))
				{
					free(sides);
					return true;
				}
			}
			break;
		}
		removeSideIndex(&sides, sidesRemaining, random);
		sidesRemaining--;
	}
	free(sides);

	if (!calculating)
		return false;

	return cell->path = false;
}
