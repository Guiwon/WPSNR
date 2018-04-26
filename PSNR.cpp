#include "wpsnr.h"

long WPSNR::PSNR_SSIM(){

	long	i;
	
	ssim = psnr = 0;
	printf("=====PSNR_SSIM=====\n");
	printf("---psnr: 000/%03d",NumFrameDeg);
	for(i=5;i<NumFrameDeg-5;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		psnr+=CalcPSNRFrame(RefImageA[RegistrationIndex[i]],DegImageA[i]);
		ssim+=CalcSSIMFrame(RefImageA[RegistrationIndex[i]],DegImageA[i]);
	}
	printf("\n");

	ssim /= (NumFrameDeg-10);
	psnr /= (NumFrameDeg-10);

	return 1;
}

double WPSNR::CalcPSNRFrame(byte *ref, byte *deg){

	long	i,j;
	double	psnr,mse;
	byte	*p_byte1,*p_byte2;

	FILE *fp;
	fp = fopen("1.raw","wb");
	fwrite(ref,1,ImageSize,fp);
	fwrite(deg,1,ImageSize,fp);
	
	fclose(fp);

	mse = 0;
	p_byte1 = ref, p_byte2 = deg;
	for(i=0;i<height;i++,p_byte1+=width,p_byte2+=width){
		for(j=0;j<width;j++){
			mse += (p_byte1[j]-p_byte2[j])*(p_byte1[j]-p_byte2[j]);
		}
	}

	mse /= ImageSize;
	if(mse==0)
		psnr = 80;
	else
		psnr = 10*log10(65025/mse);

	
	return psnr;
}

double WPSNR::CalcSSIMFrame(byte *ref, byte *deg){

	long	i,j;
	double	ssim;
	double	x,xx,y,yy,xy;
	double	varx,vary,covxy;
	double c1,c2;
	
	byte	*p_byte1,*p_byte2;

	x = xx = y = yy = xy = 0;
	p_byte1 = ref, p_byte2 = deg;
	for(i=0;i<height;i++,p_byte1+=width,p_byte2+=width){
		for(j=0;j<width;j++){
			x += p_byte2[j];
			xx += p_byte2[j]*p_byte2[j];
			y += p_byte1[j];
			yy += p_byte1[j]*p_byte1[j];
			xy += p_byte1[j]*p_byte2[j];
		}
	}

	x /= ImageSize;
	y /= ImageSize;
	xx /= ImageSize;
	yy /= ImageSize;
	xy /= ImageSize;

	varx = xx - x*x;
	vary = yy - y*y;
	covxy = xy - x*y;

	c1 = 0.01*65025*0.01*65025;
	c2 = 0.03*65025*0.03*65025;

	ssim = (2*x*y + c1)*(2*covxy + c2)/((x*x+y*y+c1)*(varx+vary+c2));


	return ssim;

}