#include "wpsnr.h"


int main(long argc, char *argv[]){

	WPSNR	object;

	object.Initialize(argc,argv);
	object.ProcessList();
	object.Finalize();	
	
	return 1;
}