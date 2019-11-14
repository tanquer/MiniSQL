// MiniSql.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "API.h"

int main()
{
	API api;
	while (1)
	{
		api.getInstruction();
		clock_t start = clock();
		api.InstructionExcute();
		clock_t end = clock();
		printf("time elapsed:¡¡%lfs¡¡\n", ((double)(end - start) / CLOCKS_PER_SEC));
		api.ShowResult();
	}
    return 0;
}

