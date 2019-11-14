#include "stdafx.h"
#include "Structures.h"

Attribute::Attribute()
{

}

Attribute::Attribute(string name, int type, int key_type)
{
	this->name = name;
	this->type = type;
	this->key_type = key_type;
	switch (type)
	{
	case INT_ATTR: width = 4; break;
	case FLOAT_ATTR: width = 4; break;
	default:width = type; break;
	}
	//spacelen = to_string(id).length() + 1 + name.length() + 1 + 4;
}


int Attribute::typeSize()
{
	switch (type)
	{
	case 0:
	case -1:
		return 4;

	default:
		return type;
	}
}


void Attribute::print(ostream& out) 
{
	out << name << " ";
	switch (type)
	{
	case INT_ATTR:out << "int "; break;
	case FLOAT_ATTR:out << "float "; break;
	default:out << "char(" << to_string(type) << ") "; break;
	}
	if (key_type == NULL_KEY) out << "NULL_key" << endl;
	else if (key_type == PRIMARY_KEY) out << "PRIMARY_key" << endl;
	else if (key_type ==UNIQUE_KEY) out << "UNIQUE_key" << endl;
	else out << "ORDINARY_key" << endl;
	return;
}


Table::Table()
{
	blocknumber = 0;
}

Table::~Table()
{

}

int Table::AttributeID(string attributeName)
{
	for (int i = 0; i < MAX_ATTRIBUTE_NUM; i++) {
		if (attributes[i].name == attributeName) {
			return i;
		}
	}
	return -1;
}




