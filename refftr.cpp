#include "wpsnr.h"

long WPSNR::CalcRefFeature(){

	long	i;
	double	SmeanFrame;
	double	FramediffFrame;

	// initialize
	memset(RefFreeze,0,NumFrameRef);
	smean = framediff = 0;

	printf("=====Reference Features=====\n");
	printf("---refftr: 000/%03d",NumFrameRef);
	for(i=0;i<NumFrameRef;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameRef);
		SmeanFrame = CalcSpatialInformation(RefImageA[i]);
		smean += SmeanFrame;

		if(i){
			FramediffFrame = CalcFrameDifference(RefImageA[i],RefImageA[i-1]);
			framediff += FramediffFrame;
			RefFreeze[i] = IsSame(RefImageA[i],RefImageA[i-1]);
		}
	}
	printf("\n\n");
	smean /= NumFrameRef;
	framediff /= NumFrameRef-1;
	FreezeInfoRef(RefFreeze);

	return 1;
}

double WPSNR::CalcSpatialInformation(byte *image){

	long	i,j,width_1,height_1;
	short	vertical,horizontal;
	double	dtemp;
	double	output;
	double	*sobel,*p_sobel;
	

	byte	*u,*c,*b;

	sobel = (double *)RefWork;						// ImageSize

	width_1 = width-1;
	height_1 = height-1;

	u = image;
	c = u + width;
	b = c + width;
	memset(sobel,0,sizeof(double)*ImageSize);
	p_sobel = sobel + width;
	for(i=1;i<height_1;i++,u+=width,c+=width,b+=width,p_sobel+=width){
		for(j=1;j<width_1;j++){
			horizontal = -u[j-1] + u[j+1] - 2*c[j-1] + 2*c[j+1] - b[j-1] + b[j+1];
			vertical = u[j-1] + 2*u[j] + u[j+1] - b[j-1] - 2*b[j] - b[j+1];
			p_sobel[j] = sqrt((double)(horizontal*horizontal + vertical*vertical));			
		}
	}

	dtemp=0.;
	for(i=0;i<ImageSize;i++)
		dtemp += sobel[i];

	output = dtemp/ImageSize;
	
	return output;
}


double WPSNR::CalcFrameDifference(byte *frame1, byte *frame2){

	long	i;
	double	fd;

	fd=0.;
	for(i=0;i<ImageSize;i++)
		fd += (frame1[i]-frame2[i])*(frame1[i]-frame2[i]);
	fd /= ImageSize;

	return fd;
}


long WPSNR::FreezeInfoRef(byte *freeze){

	long	i,frz_len;
	
	Rtfrz=Rmfrz=Rnfrz=Rtfrz1=0;
	frz_len=0;
	for(i=1;i<NumFrameRef;i++){
		
		if(freeze[i]){			
			if(frz_len==0)
				Rnfrz++;
			Rtfrz++;
			frz_len++;			
		}
		else{
			if(frz_len){
				if(frz_len>1)
					Rtfrz1 += frz_len;
				if(frz_len>Rmfrz)
					Rmfrz = frz_len;
				frz_len=0;
			}
		}
	}

	if(frz_len){
		if(frz_len>1)
			Rtfrz1 += frz_len;
		if(frz_len>Rmfrz)
			Rmfrz = frz_len;
	}
	
	return 1;
}