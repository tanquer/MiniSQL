#pragma once
#include <string>
#include <iostream>
#include <time.h>
#include <fstream>
using namespace std;



//每个表一个文件，offset是在文件里的偏移量
//文件里对应若干个block，根据offset定位

#define SUCCESS 1
#define SYNTAXERR 2

#define EMPTY 0
#define STARTOPCODE 10
#define INSERT (STARTOPCODE + 1)
#define DELETE (STARTOPCODE + 2)
#define DROPINDEX (STARTOPCODE + 3)
#define DROPTABLE (STARTOPCODE + 4)
#define CREINDEX (STARTOPCODE + 5)
#define CRETABLE (STARTOPCODE + 6)
#define SELECT (STARTOPCODE + 7)
#define QUIT (STARTOPCODE + 8)
#define EXECFILE (STARTOPCODE + 9)
#define SHOWTABLE (STARTOPCODE + 10)
#define CLEARBUFFER (STARTOPCODE + 11)

#define TYPECHAR 2
#define TYPEINT 0
#define TYPEFLOAT -1

#define TYPEUNIQUE 2
#define TYPENULL 0
#define TYPEPRIMARY 1

#define MAX_BLOCK_NUM 128
#define INDEX_POINTER_SIZE 5
#define MAX_FILE_NUM 40
#define MAX_FILENAME_SIZE 100
#define BLOCK_SIZE 4096
#define MAX_TABLE_LENGTH 100

#define CATALOG_OPCODE_START 300
#define CATALOG_TABLE_NOSPACE (CATALOG_OPCODE_START + 1)
#define CATALOG_TABLE_NAMECONFLICT (CATALOG_OPCODE_START + 2)
#define CATALOG_TABLE_CANTOPEN (CATALOG_OPCODE_START + 3)
#define CATALOG_INDEX_CANTOPEN (CATALOG_OPCODE_START + 4)
#define CATALOG_INDEX_NAMECONFLICT (CATALOG_OPCODE_START + 5)
#define CATALOG_INDEX_NO_SUCH_TABLE (CATALOG_OPCODE_START + 6)
#define CATALOG_INDEX_NO_SUCH_ATTR (CATALOG_OPCODE_START + 7)

#define RECORD_OPCODE_START 600
#define RECORD_NO_SUCH_RECORD (RECORD_OPCODE_START + 1)
#define RECORD_PRIMARY_CONFLICT (RECORD_OPCODE_START + 2)

typedef int RC;
typedef unsigned char BYTE;

class BlockNode
{
public:
	BlockNode();
	BlockNode(string fileName,int offset,bool pin);
	string fileName;//in which file
	int offset;//offset from the head of the file
	BYTE BlockData[BLOCK_SIZE];
	bool pin;
	bool LRUtag;//true-- reference=1, false -- reference=0
	bool dirty;

	BlockNode *preBlock;
	BlockNode *nextBlock;

	char *address;//the content address
	size_t usingSize; // the byte size that the block have used. The total size of the block is BLOCK_SIZE . 
};




