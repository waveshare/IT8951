#include "IT8951.h"

int main (int argc, char *argv[])
{
	/*
	printf("ReadReg = 0x%x\n",IT8951ReadReg(LISAR));
	IT8951WriteReg(LISAR,0x1234);
	printf("ReadReg = 0x%x\n",IT8951ReadReg(LISAR));
	*/

	if(IT8951_Init())
	{
		printf("IT8951_Init error \n");
		return 1;
	}
	
	
	if (argc != 2)
	{
		printf("Error: argc!=2.\n");
		exit(1);
	}

	printf("begin");
	IT8951_BMP_Example(argv[1]);
	printf("end");

	
	IT8951_Cancel();

	return 0;
}


