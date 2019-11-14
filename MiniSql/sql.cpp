#include "stdafx.h"
#include "sql.h"

BlockNode::BlockNode()
{
	fileName = "";
	offset = -1;
	pin = dirty = false;
	LRUtag=true;
	memset(BlockData,0,sizeof(BLOCK_SIZE)*sizeof(BYTE));
}

BlockNode::BlockNode(string fileName,int offset,bool pin)
{
    this->fileName=fileName;
    this->offset=offset;
    this->pin=pin;
    LRUtag=true;
    dirty=false;
}





