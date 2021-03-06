﻿#include "Table.h"

Table::Table()
{
	maxRow = 1;

	rows = new Row(maxRow);
}

int Table::getMaxRow() const
{
	return maxRow;
}
const char* Table::getCellOnRow(int row, int col) const
{
	return rows[row].getCellStr(col);
}
Row* Table::getRows() const
{
	return rows;
}

void Table::addRow(int numOfRow)
{
	if (maxRow <= numOfRow)
	{
		Row* helper = new Row[numOfRow + 1];

		for (int i = 0; i < maxRow; i++)
		{
			helper[i] = rows[i];
		}
		delete[] rows;

		rows = new Row[numOfRow + 1];
		rows = helper;

		maxRow = numOfRow + 1;

		rows[numOfRow] = *new Row(numOfRow);
	}

	fillTheEmptyCells();
}

void Table::edit(const String& value, int row, int col)
{
	if (maxRow <= row)
	{
		addRow(row);
		rows[row].addOrEditCell(value, col);

		maxRow = row + 1;
	}
	else
		rows[row].addOrEditCell(value, col);

	if (maxCol < rows[row].getCapacity())
		maxCol = rows[row].getCapacity();
}

void Table::setLhsValue(const char*& cell, int& start, int& finish, int& lhsRow, int& lhsCol, int& digit, char& Operator)
{
	for (int i = start; cell[i] >= '0' && cell[i] <= '9'; i++)
	{
		digit = cell[i] - '0';
		lhsRow = lhsRow * 10 + digit;

		start = i;
	}
	digit = 0;
	for (int i = start + 2; cell[i] >= '0' && cell[i] <= '9'; i++)
	{
		digit = cell[i] - '0';
		lhsCol = lhsCol * 10 + digit;

		finish = i;
	}
	digit = 0;
	start = finish + 5;

	Operator = cell[finish + 2];
}
void Table::setRhsValue(const char*& cell, int& finish, int& start, int& rhsRow, int& rhsCol, int& digit, int& len)
{
	for (int i = start + 5; cell[i] >= '0' && cell[i] <= '9'; i++)
	{
		digit = cell[i] - '0';
		rhsRow = rhsRow * 10 + digit;

		finish = i;
	}
	digit = 0;
	for (int i = finish + 2; i < len; i++)
	{
		digit = cell[i] - '0';
		rhsCol = rhsCol * 10 + digit;
	}
}

void Table::checkIfRhsValueIsReference(const char*& cell, int& lastDigPosBefOp, int& len, bool& isRhsFormula)
{
	for (int i = 0; i < len; i++)
	{
		if (cell[i] == 'R')
		{
			isRhsFormula = true;
			lastDigPosBefOp = i - 4;
			break;
		}
	}
}

bool Table::setLhsValueAsNum(const char*& cell, double& lhsCell, int& digit, char& Operator)
{
	bool isValid = true;
	int countOfDots = 0;
	for (int i = 2; (cell[i] >= '0' && cell[i] <= '9') || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
			countOfDots++;
	}
	if (countOfDots >= 2 || cell[2] == '.')
		cout << "Invalid number. The dots can't be more than one or a number can't start with a dot." << endl;

	int posOfDot = 0;
	bool dot = false;
	for (int i = 2; (cell[i] >= '0' && cell[i] <= '9') || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
		{
			dot = true;
			posOfDot = i;
			break;
		}
		digit = cell[i] - '0';
		lhsCell = lhsCell * 10 + digit;
		Operator = cell[i + 2];
	}
	if (dot)
	{
		for (int i = posOfDot + 1, j = 1; cell[i] != ' '; i++, j++)
		{
			digit = cell[i] - '0';
			lhsCell = lhsCell + (digit / (pow(10, j)));

			Operator = cell[i + 2];
		}
	}
	return isValid ? true : false;
}
bool Table::setRhsValueAsNum(const char*& cell, int& start, double& rhsCell, int& digit, int& len, bool& isRhsNum)
{
	bool isValid = true;
	int countOfDots = 0;
	for (int i = start + 4; (cell[i] >= '0' && cell[i] <= '9') || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
			countOfDots++;
	}
	if (countOfDots >= 2 || cell[start + 4] == '.')
	{
		isValid = false;
		cout << "Invalid number. The dots can't be more than one or a number can't start with a dot." << endl;
	}

	isRhsNum = true;
	int posOfDot = 0;
	bool dot = false;
	for (int i = start + 4; (cell[i] >= '0' && cell[i] <= '9') || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
		{
			dot = true;
			posOfDot = i;
			break;
		}
		digit = cell[i] - '0';
		rhsCell = rhsCell * 10 + digit;
	}
	if (dot)
	{
		for (int i = posOfDot + 1, j = 1; i < len; i++, j++)
		{
			digit = cell[i] - '0';
			rhsCell = rhsCell + (digit / (pow(10, j)));
		}
	}
	return isValid ? true : false;
}

void Table::settingFinalCellValue(int& row, int& col, double& cell, bool& rowOrColExist)
{
	if (row > maxRow - 1 || col > maxCol - 1)
	{
		rowOrColExist = false;
		cell = 0;
	}
	else
	{
		if (rows[row].isFormula(col))
			calculateFormulaCellsReference(row, col, cell);
		else 
			cell = rows[row].getCellValue(col); // set SECOND AS FORMULA
	}
}

double Table::arithmeticOperations(char& Operator, double& lhsCell, double& rhsCell)
{
	if (Operator == '+')
		return lhsCell + rhsCell;
	else if (Operator == '-')
		return lhsCell - rhsCell;
	else if (Operator == '*')
		return lhsCell * rhsCell;
	else if (Operator == '/')
		return lhsCell / rhsCell;
	else if (Operator == '^')
		return pow(lhsCell, rhsCell);
}

bool Table::calculateStandartFormula(int row, int col, double& lhsNum, double& rhsNum, char& Operator)
{
	int digit = 0;
	lhsNum = 0;
	rhsNum = 0;

	int len = strlen(rows[row].getCellStr(col));
	const char* cell = rows[row].getCellStr(col);

	int posOfOp = 0;
	int posOfDot = 0;
	bool dot = false;

	bool isValid = true;
	int countOfDots = 0;
	for (int i = 2; (cell[i] >= '0' && cell[i] <= '9') || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
			countOfDots++;
	}
	if (countOfDots >= 2 || cell[2] == '.')
	{
		cout << "Invalid number. The dots can't be more than one or a number can't start with a dot." << endl;
		isValid = false;
	}
	countOfDots = 0;

	for (int i = 2; (cell[i] >= '0' && cell[i] <= '9') || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
		{
			dot = true;
			posOfDot = i;
			break;
		}

		digit = cell[i] - '0';
		lhsNum = lhsNum * 10 + digit;
		posOfOp = i + 2;
	}
	if (dot)
	{
		for (int i = posOfDot + 1, j = 1; cell[i] >= '0' && cell[i] <= '9'; i++, j++)
		{
			digit = cell[i] - '0';
			lhsNum = lhsNum + (digit / (pow(10, j)));

			posOfOp = i + 2;
		}
	}

	digit = 0;
	posOfDot = 0;
	dot = false;

	for (int i = posOfOp + 2; i < len || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
			countOfDots++;
	}
	if (countOfDots >= 2 || cell[posOfOp + 2] == '.')
	{
		cout << "Invalid number. The dots can't be more than one or a number can't start with a dot." << endl;
		isValid = false;
	}

	for (int i = posOfOp + 2; i < len || (cell[i] == '.'); i++)
	{
		if (cell[i] == '.')
		{
			dot = true;
			posOfDot = i;
			break;
		}

		digit = cell[i] - '0';
		rhsNum = rhsNum * 10 + digit;
	}
	Operator = cell[posOfOp];
	if (dot)
	{
		for (int i = posOfDot + 1, j = 1; i < len; i++, j++)
		{
			digit = cell[i] - '0';
			rhsNum = rhsNum + (digit / (pow(10, j)));

			posOfOp = i + 2;
		}
	}
	return isValid ? true : false;
}
bool Table::calculateFormulaCellsReference(int row, int col, double& res)
{
	int len = strlen(rows[row].getCellStr(col));
	const char* cell = rows[row].getCellStr(col);

	int digit = 0;
	int lhsRow = 0;
	int lhsCol = 0;
	int rhsRow = 0;
	int rhsCol = 0;

	int lastDigPosBefCol = 0;
	int lastDigPosBefOp = 0;

	double lhsCell = 0;
	double rhsCell = 0;

	bool isRhsNum = false;
	bool isRhsFormula = false;
	bool rowOrColExist = true;

	char Operator;

	if (cell[2] >= '0' && cell[2] <= '9') // check if first is number
	{
		checkIfRhsValueIsReference(cell, lastDigPosBefOp, len, isRhsFormula); // check if second is reference
		if (isRhsFormula) 
		{
			setRhsValue(cell, lastDigPosBefCol, lastDigPosBefOp, rhsRow, rhsCol, digit, len);
			settingFinalCellValue(rhsRow, rhsCol, rhsCell, rowOrColExist); // set second as reference
			if (!setLhsValueAsNum(cell, lhsCell, digit, Operator)) // set first as number
				return false;
		}

		else if ((cell[2] >= '0' && cell[2] <= '9') && (!isRhsFormula)) // check if both are numbers
		{
			if (!calculateStandartFormula(row, col, lhsCell, rhsCell, Operator)) // set both as numbers
				return false;
		}		
	}

	else if (cell[2] == 'R') // first is reference
	{
		lastDigPosBefCol += 3;
		lastDigPosBefOp += 5;

		setLhsValue(cell, lastDigPosBefCol, lastDigPosBefOp, lhsRow, lhsCol, digit, Operator);
		settingFinalCellValue(lhsRow, lhsCol, lhsCell, rowOrColExist); // set first as reference
		if (rowOrColExist)
		{
			if (rows[lhsRow].getCells()->checkIfStringIsValidNumber(rows[lhsRow].getCells()[lhsCol].getValue()) == -1)
				lhsCell = 0;
		}

		if (cell[lastDigPosBefOp + 4] >= '0' && cell[lastDigPosBefOp + 4] <= '9') // check if second is number
		{		
			if (!setRhsValueAsNum(cell, lastDigPosBefOp, rhsCell, digit, len, isRhsNum)) // set second as number
				return false;
		}

		if (!isRhsNum) // second is reference
		{
			setRhsValue(cell, lastDigPosBefCol, lastDigPosBefOp, rhsRow, rhsCol, digit, len);
			settingFinalCellValue(rhsRow, rhsCol, rhsCell, rowOrColExist); // set second as reference
			if (rowOrColExist)
			{
				if (rows[rhsRow].getCells()->checkIfStringIsValidNumber(rows[rhsRow].getCells()[rhsCol].getValue()) == -1)
					rhsCell = 0;
			}
		}		
	}
	else // invalid number
		return false;

	res = arithmeticOperations(Operator, lhsCell, rhsCell);

	if (Operator == '/' && rhsCell == 0)
		return false;
	return true;
}

int Table::lenOfNum(int num)
{
	int numOfDigits = 0;
	while (num)
	{
		++numOfDigits;
		num /= 10;
	}
	return numOfDigits;
}
int Table::findMaxLenOfCellInCols()
{
	int len = 0;
	int maxLen = 0;
	double res = 0;
	lenOfMaxCols = new int[maxCol];

	for (int col = 1; col < maxCol; col++)
	{
		len = 0;
		maxLen = 0;
		for (int row = 1; row < maxRow; row++)
		{
			if (rows[row].getIsEmpty())
				len = 0;

			else if (rows[row].isFormula(col))
			{
				if (!calculateFormulaCellsReference(row, col, res))
				{
					len = 5;
					continue;
				}
				calculateFormulaCellsReference(row, col, res);
				if (!((int)res == res))
				{
					res = round(res * 100 + 0.5) / 100;
					len = lenOfNum((int)res) + 2;
				}
				else
					len = lenOfNum((int)res);
			}

			else
			{
				len = strlen(rows[row].getCellStr(col));
				if (rows[row].getCellStr(col)[0] == '"' && rows[row].getCellStr(col)[len - 1] == '"')
				{
					if (rows[row].getCellStr(col)[2] == '"' && rows[row].getCellStr(col)[len - 2] == '"' &&
						rows[row].getCellStr(col)[1] == '\\' && rows[row].getCellStr(col)[len - 3] == '\\')
						len = len - 4;
					else
						len = len - 2;
				}
			}
			if (maxLen < len)
				maxLen = len;
		}
		lenOfMaxCols[col] = maxLen;
	}
	return maxLen;
}
void Table::fillTheEmptyCells()
{
	for (int i = 0; i < maxRow; i++)
	{
		if (rows[i].getIsEmpty())
			continue;
		for (int j = rows[i].getCapacity(); j < maxCol; j++)
			rows[i].addOrEditCell(Cell().getValue(), j);
	}
}

void Table::print()
{
	int len = 0;

	fillTheEmptyCells();
	findMaxLenOfCellInCols();

	for (int i = 1; i < maxRow; i++)
	{
		for (int j = 1; j < maxCol; j++)
		{
			double res = 0;

			if (rows[i].getIsEmpty())
				break;

			if (rows[i].isFormula(j))
			{
				if (!calculateFormulaCellsReference(i, j, res))
				{
					cout << "ERROR | ";
					len = 5;
					continue;
				}
				calculateFormulaCellsReference(i, j, res);
				if (!((int)res == res))
				{
					res = round(res * 100 + 0.5) / 100;
					len = lenOfNum((int)res) + 2;
				}
				else
					len = lenOfNum((int)res);
				cout << res;
			}
			else
			{
				rows[i].printCell(j);
				len = strlen(rows[i].getCellStr(j));
			}
			for (int k = 0; k < lenOfMaxCols[j] - len; k++)
			{
				cout << ' ';
			}
			if (j != maxCol - 1)
				cout << " | ";
		}
		cout << endl;
	}
}