#include "wpsnr.h"



long WPSNR::Registration(){

	// index initialize
	shiftX = shiftY = -10;
	

	printf("=====Registration=====\n");

#if 1
	while(FindSpatialShift()>0){		
		FindTemporalShift();
		calibration();
		FindTemporalShift();
	}
#else
	shiftX = -5;
	FindTemporalShift();
	calibration();
	FindTemporalShift();
#endif

	// process video re-read
	ReadDegradedVideo();
	printf("\n");
	
	return 1;
}

long WPSNR::FindSpatialShift(){

	long	re_val;
	long	shiftXtmp,shiftYtmp;
	long	shiftX_min,shiftY_min;
	double	min,SAE;

	shiftXtmp = shiftX;
	shiftYtmp = shiftY;
	min = 1e10;

	printf("---spatial shift...");
	printf("h:00/v:00");
	for(shiftY=-SpatialSR;shiftY<=SpatialSR;shiftY++){
		for(shiftX=-SpatialSR;shiftX<=SpatialSR;shiftX++){
			printf("\b\b\b\b\b\b\b\b\bh:%02d/v:%02d",shiftX,shiftY);
			SAE = CalcVideoMSE();
			if(SAE<min){
				min = SAE;
				shiftX_min = shiftX;
				shiftY_min = shiftY;
			}
		}
	}
	shiftX = shiftX_min;
	shiftY = shiftY_min;
	printf("\n");

	if(shiftXtmp==shiftX && shiftYtmp==shiftY)
		re_val = -1;
	else
		re_val = 1;
	
	return re_val;	
}

long WPSNR::FindTemporalShift(){

	long	i,j;
	long	st,en,min_index;
	long	SearchRange,add;
	double	min,MSE;

	add = abs(NumFrameDeg-NumFrameRef);
	SearchRange = TemporalSR + add;	

	printf("---temporal shift...");
	printf("000/%03d",NumFrameDeg);
	for(i=0;i<NumFrameDeg;i++){

		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);

		st = i-SearchRange;
		en = i+SearchRange;

		if(st<0)			st = 0;
		if(en>=NumFrameRef)	en = NumFrameRef-1;
		
		if(i>=NumFrameRef){
			MSE = CalcFrameMSE(RefImageA[en],DegImageA[i]);
			min = MSE, min_index = en;
		}
		else{
			MSE = CalcFrameMSE(RefImageA[i],DegImageA[i]);
			min = MSE, min_index = i;
		}
		for(j=st;j<=en;j++){
			MSE = CalcFrameMSE(RefImageA[j],DegImageA[i]);
			if(min>MSE){
				min = MSE;
				min_index = j;
			}
		}
		RegistrationIndex[i] = min_index;
	}
	printf("\n");

	return 1;
}

double WPSNR::CalcVideoMSE(){

	long	i;
	double	MSE;

	MSE = 0.;	
	for(i=0;i<NumFrameDeg;i++)
		MSE += CalcFrameMSE(RefImageA[RegistrationIndex[i]],DegImageA[i]);	
	MSE /= NumFrameDeg;	

	return MSE;
}


double WPSNR::CalcFrameMSE(byte *frame1, byte *frame2){
	
	long	i,j,jj;
	long	width_1,height_1;
	double	dtemp;
	double	MSE;
	byte	*p_byte1,*p_byte2;

	width_1 = width-marginX;
	height_1 = height-marginY;

	p_byte1 = frame1 + width*marginY;
	p_byte2 = frame2 + width*(marginY+shiftY);

	MSE = 0.;
	for(i=marginY;i<height_1;i++,p_byte1+=width,p_byte2+=width){
		for(j=marginX,jj=j+shiftX;j<width_1;j++,jj++){			
			dtemp = p_byte1[j]-p_byte2[jj];
			MSE += dtemp*dtemp;
		}
	}

	MSE /= (double)ImageSizeM;
	
	return MSE;
}


long WPSNR::calibration(){

	long	i,j;
	long	dataNum,ltemp1,ltemp2,ltemp3,offset_index;
	long	*index;
	byte	*data1,*data2;
	long	*p_index;
	byte	*p_data1,*p_data2;
	
	double	det;
	double	sumX,sumXX,sumXY,sumY;
	double	gain,offset;

	srand((unsigned int)time(NULL));		// random seed

	offset_index = width*shiftY + shiftX;	// offset calculation

	printf("---calibration...");

	// memory matching
	dataNum = NUMFTRFRAME*NumFrameDeg;
	data1 = reg_workspace;
	data2 = data1 + dataNum;
	index = (long *)(data2 + dataNum);

	// index initialize
	for(i=0;i<ImageSize;i++)
		index[i] = i;
	// index randomize
	for(i=0;i<ImageSize;i++){
		ltemp1 = (rand()*rand())%ImageSize;
		ltemp3 = index[i];
		index[i] = index[ltemp1];
		index[ltemp1] = ltemp3;		
	}
	for(i=0;i<ImageSize;i++){
		ltemp1 = (rand()*rand())%ImageSize;
		ltemp2 = (rand()*rand())%ImageSize;
		ltemp3 = index[ltemp2];
		index[ltemp2] = index[ltemp1];
		index[ltemp1] = ltemp3;	
	}
	// location check
	for(i=0,ltemp3=ImageSize-1;i<dataNum;i++){

		while(LocationCheck(index[i])<0){
			ltemp1 = index[i];
			index[i] = index[ltemp3];
			index[ltemp3] = ltemp1;			
			ltemp3--;
		}
	}

	// data read
	p_index = index;
	p_data1 = data1, p_data2 = data2;
	for(i=0;i<NumFrameDeg;i++,p_index+=NUMFTRFRAME,p_data1+=NUMFTRFRAME,p_data2+=NUMFTRFRAME){
		for(j=0;j<NUMFTRFRAME;j++){
			p_data1[j] = RefImageA[RegistrationIndex[i]][p_index[j]];
			p_data2[j] = DegImageA[i][p_index[j]+offset_index];
		}
	}

	// gain offset calculation
	for(i=0,sumX=sumXX=sumXY=sumY=0.;i<dataNum;i++){
		sumX += data2[i];
		sumXX += data2[i]*data2[i];
		sumXY += data1[i]*data2[i];
		sumY += data1[i];
	}
	det = dataNum*sumXX-sumX*sumX;
	gain = dataNum*sumXY - sumX*sumY;
	offset = sumXX*sumY - sumX*sumXY;
	gain /= det;
	offset /= det;

	printf("000/%03d",NumFrameDeg);
	for(i=0;i<NumFrameDeg;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		for(j=0;j<ImageSize;j++){
			det = gain*DegImageA[i][j]+offset;
			if(det<0)			DegImageA[i][j]=0;
			else if(det>255)	DegImageA[i][j]=255;
			else				DegImageA[i][j]=(byte)(det+0.5);
		}
	}
	printf("\n");

	return 1;
}

long WPSNR::LocationCheck(long location){

	long	x,y;

	x = location%width;
	y = location/width;

	if(x<marginX || x>=width-marginX)
		return -1;
	if(y<marginY || y>=height-marginY)
		return -1;	
	
	return 1;
}