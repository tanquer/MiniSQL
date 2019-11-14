#pragma once
#include <iostream>
#include <ctime>
#include <vector>
#include "BufferManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"
#include "Interpreter.h"
#include "Structures.h"
#include "RecordManager.h"
#include "sql.h"

class API
{
public:
	API();
	~API();
public:
	void getInstruction();
	void getInstruction(fstream & iff);
	void InstructionExcute();
	void ShowResult();
private:
	void HandleException(int);
	void Displaydata(outputdata data, Table);
	ParseTree tree;
	RC validinstruction;

	
};

