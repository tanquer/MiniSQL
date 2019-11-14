#define UNKNOWN 0
#include "Structures.h"
#include "sql.h"
#include <sstream>

struct ParseTree {
	int opcode;
	string table_name;
	string index_name;
	string attr_name;
	string primarykey;
	string filename;
	vector<Attribute> attributes;
	vector<Conditionp> conditions;
	RowData rowdata;
	Table tableinfo;
	Index indexinfo;
};

class Interpreter
{
public:
	Interpreter()
	{
		parsetree.opcode = UNKNOWN;
		parsetree.table_name = "";
		parsetree.index_name = "";
		parsetree.attr_name = "";
		parsetree.attributes.clear();
		parsetree.conditions.clear();
		parsetree.rowdata.attribute.clear();
	}
	~Interpreter();
	string GetInstruction();
	string GetInstruction(fstream &);
	RC HandleInstruction(string instruction);
	ParseTree GetParseTree();
	void CleanInstruction();
private:
	ParseTree parsetree;
	bool InstructionEnd(string& instruction);
};


