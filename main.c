#include "IT8951.h"

int main (int argc, char *argv[])
{
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
    uint8_t *buffer = (uint8_t*)malloc(2000*1500/2);
    memset(buffer, 0x00, 2000*1500/2);
	IT8951_Display4BppBuffer(buffer);
	printf("end");

	
	IT8951_Cancel();

	return 0;
}


