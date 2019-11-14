#include "stdafx.h"
#include "Interpreter.h"

Interpreter::~Interpreter()
{
}

string Interpreter::GetInstruction()
{
	bool stopcommand = false;
	string command, linecommand;
	std::cout << "MiniSQL>> ";
	while (!stopcommand)
	{
		getline(cin, linecommand);
		if (InstructionEnd(linecommand))
			stopcommand = true;
		command += " " + linecommand;
	}
	return command;
}
string Interpreter::GetInstruction(fstream& iff)
{
	bool stopcommand = false;
	string command, linecommand;
	while (!stopcommand)
	{
		getline(iff, linecommand);
		if (InstructionEnd(linecommand))
			stopcommand = true;
		command += " " + linecommand;
	}
	return command;
}



bool Interpreter::InstructionEnd(string& instruction)
{
	for (int i = 0; i < instruction.length(); ++i)
	{
		if (instruction[i] == ';')
		{
			instruction = instruction.substr(0, i);
			return true;
		}
	}
	return false;
}

RC Interpreter::HandleInstruction(string instruction)
{
	bool iscreate = false, isinsert = false, isdelete = false, isdrop = false;
	stringstream ss(instruction);
	string word;
	ss >> word;
	if (word == "create")
	{
		iscreate = true;
		ss >> word;
		if (word == "index")
		{
			parsetree.opcode = CREINDEX;
			ss >> word;
			parsetree.index_name = word;
			ss >> word;
			if (word != "on") return SYNTAXERR;
			else
			{
				ss >> word;
				parsetree.table_name = word;
				ss >> word;
				if (word == "(")
				{
					ss >> word;
					if (word.find(')') == string::npos)
						parsetree.attr_name = word;
					else {
						word.erase(word.find(')'), 1);
						parsetree.attr_name = word;
					}
				}
				else
				{
					if (word.find('(') == string::npos) return SYNTAXERR;
					else word.erase(word.find('('), 1);
					if (word.find(')') == string::npos) return SYNTAXERR;
					else word.erase(word.find(')'), 1);
					parsetree.attr_name = word;
				}
			}
		}
		else if (word == "table")
		{
			parsetree.opcode = CRETABLE;
			ss >> word;
			parsetree.table_name = word;
			ss >> word;
			if (word != "(") return SYNTAXERR;
			else
				while (word != ")")
				{
					Attribute tmpattribute;
					ss >> word;
					if (word == "primary")
					{
						ss >> word;
						if (word.substr(0, 3) != "key") return SYNTAXERR;
						if (word.find('(') == string::npos || word.find(')') == string::npos)
							return SYNTAXERR;
						string tmpname = word.substr(word.find('(') + 1, word.find(')') - word.find('(') - 1 );
						bool findprimary = false;
						for (vector<Attribute>::iterator iter = parsetree.attributes.begin(); iter != parsetree.attributes.end(); iter++)
							if (iter->name == tmpname)
							{
								iter->key_type = TYPEPRIMARY;
								parsetree.primarykey = tmpname;
								findprimary = true;
							}
						if (!findprimary) return SYNTAXERR;
						ss >> word;
						continue;
					}
//					if (word.find(',') != string::npos)
//						return SYNTAXERR;
					tmpattribute.name = word;
					ss >> word;
					if (word.substr(0, 4) == "char")
					{
						if (word.find('(') == string::npos || word.find(')') == string::npos)
							return SYNTAXERR;
						string tmpint = word.substr(word.find('(') + 1, word.find(')') - word.find('(') - 1);
						stringstream check(tmpint);
						int length; 
						check >> length;
						tmpattribute.width = length;
						tmpattribute.type = length;
					}
					else if (word.substr(0, 3) == "int")
						tmpattribute.type = TYPEINT;
					else if (word.substr(0, 5) == "float")
						tmpattribute.type = TYPEFLOAT;
					else return SYNTAXERR;
					/* name char(10), */
					if (word.find(',') != string::npos) {
						tmpattribute.key_type = TYPENULL;
						parsetree.attributes.push_back(tmpattribute);
						//ss >> word;
						continue;
					}
					ss >> word;
					if (word == "unique,")
						tmpattribute.key_type = TYPEUNIQUE;
					else return SYNTAXERR;
					parsetree.attributes.push_back(tmpattribute);
					// ss >> word;
				}
		}
		else return SYNTAXERR;
	}
	else if (word == "insert")
	{
		parsetree.opcode = INSERT;
		isinsert = true;
		ss >> word;
		if (word != "into") return SYNTAXERR;
		ss >> word;
		parsetree.table_name = word;
		ss >> word;
		if (word.substr(0, 6) != "values") return SYNTAXERR;
		else
		{
			word = word.substr(word.find('(') + 1, word.length());
			while (word.find(',') != string::npos || word.find(')') != string::npos)
			{
				if (word.find(',') != string::npos)
				{
					string value = word.substr(0, word.find(','));
					while (value.find('\'') != string::npos)
						value.erase(value.find('\''), 1);
					parsetree.rowdata.attribute.push_back(value);
					word = word.substr(word.find(',') + 1, word.length());
				}
				else
				{
					string value = word.substr(0, word.find(')'));
					while (value.find('\'') != string::npos)
						value.erase(value.find('\''), 1);
					parsetree.rowdata.attribute.push_back(value);
					word = word.substr(word.find(')') + 1, word.length());
				}
			}
		}
	}
	else if (word == "delete")
	{
		parsetree.opcode = DELETE;
		isdelete = true;
		ss >> word;
		if (word != "from") return SYNTAXERR;
		ss >> word;
		parsetree.table_name = word;
		ss >> word;
		if (word == "where")
		{
			do {
				ss >> word;
				Conditionp c;
				if (word.find('<') == string::npos && word.find('>') == string::npos && word.find('=') == string::npos)
					return SYNTAXERR;
				if (word.find("<=") != string::npos)
				{
					string cond = word.substr(0, word.find("<="));
					string constant = word.substr(word.find("<=" )+2, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::LE;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find(">=") != string::npos)
				{
					string cond = word.substr(0, word.find(">="));
					string constant = word.substr(word.find(">=" )+2, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::GE;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find("<>") != string::npos)
				{
					string cond = word.substr(0, word.find("<>"));
					string constant = word.substr(word.find("<>" )+2, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::NE;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find("=") != string::npos)
				{
					string cond = word.substr(0, word.find("="));
					string constant = word.substr(word.find("=" )+1, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::E;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find("<") != string::npos)
				{
					string cond = word.substr(0, word.find("<"));
					string constant = word.substr(word.find("<" )+1, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::L;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find(">") != string::npos)
				{
					string cond = word.substr(0, word.find(">"));
					string constant = word.substr(word.find(">" )+1, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::G;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else return SYNTAXERR;
				parsetree.conditions.push_back(c);
				ss >> word;
			} while (word == "and");
		}
	}
	else if (word == "drop")
	{
		isdrop = true;
		ss >> word;
		if (word == "index")
		{
			parsetree.opcode = DROPINDEX;
			ss >> word;
			parsetree.index_name = word;
			ss >> word;
		}
		else if (word == "table")
		{
			parsetree.opcode = DROPTABLE;
			ss >> word;
			parsetree.table_name = word;
			ss >> word;
		}
	}
	else if (word == "select")
	{
		parsetree.opcode = SELECT;
		ss >> word;
		if (word != "*") return SYNTAXERR;
		ss >> word;
		if (word != "from") return SYNTAXERR;
		ss >> word;
		parsetree.table_name = word;
		ss >> word;
		if (word == "where")
		{
			do {
				ss >> word;
				Conditionp c;
				if (word.find('<') == string::npos && word.find('>') == string::npos && word.find('=') == string::npos)
					return SYNTAXERR;
				if (word.find("<=") != string::npos)
				{
					string cond = word.substr(0, word.find("<="));
					string constant = word.substr(word.find("<=")+2, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::LE;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find(">=") != string::npos)
				{
					string cond = word.substr(0, word.find(">="));
					string constant = word.substr(word.find(">=")+2, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::GE;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find("<>") != string::npos)
				{
					string cond = word.substr(0, word.find("<>"));
					string constant = word.substr(word.find("<>")+2, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::NE;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find("=") != string::npos)
				{
					string cond = word.substr(0, word.find("="));
					string constant = word.substr(word.find("=")+1, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::E;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find("<") != string::npos)
				{
					string cond = word.substr(0, word.find("<"));
					string constant = word.substr(word.find("<")+1, word.length());
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::L;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else if (word.find(">") != string::npos)
				{
					string cond = word.substr(0, word.find(">"));
					string constant = word.substr(word.find(">") + 1, word.length());
					//cout << constant << endl;
					while (constant.find('\'') != string::npos)
						constant.erase(constant.find('\''), 1);
					c.comparator = Conditionp::G;
					c.Attribute_value = cond;
					c.Constant_value = constant;
				}
				else return SYNTAXERR;
				parsetree.conditions.push_back(c);
				ss >> word;
			} while (word == "and");
		}
	}
	else if (word == "quit")
	{
		parsetree.opcode = QUIT;
	}
	else if (word == "execfile")
	{
		ss >> word;
		parsetree.filename = word;
		parsetree.opcode = EXECFILE;
	}
	else if (word == "show")
	{
		ss >> word;
		if (word == "tables")
			parsetree.opcode = SHOWTABLE;
	}
	else if (word == "clear")
	{
		ss >> word;
		if (word == "buffer")
			parsetree.opcode = CLEARBUFFER;
	}
	else return SYNTAXERR;
	return SUCCESS;
}

void Interpreter::CleanInstruction()
{
	parsetree.opcode = UNKNOWN;
	parsetree.table_name = "";
	parsetree.index_name = "";
	parsetree.attr_name = "";
	parsetree.attributes.clear();
	parsetree.conditions.clear();
	parsetree.rowdata.attribute.clear();
}

ParseTree Interpreter::GetParseTree()
{
	return parsetree;
}