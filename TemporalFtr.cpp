#include "wpsnr.h"

long WPSNR::CalcBackGround(){

	long	i,point;
	_int64	sse_temp,ltemp,sse;

	// intialize
	BGmse=BGratio2=0.;
	BGpntnum2=0;
	sse=0;
	
	printf("=====Background=====\n");
	if(strcmp(refFnamePrev,refFname))
		CalcBGratio1();		// BGratio1

	if(BGpntnum1<1000)
		return 1;

	printf("---bgcalc: 000/%03d",NumFrameRef);
	for(i=1;i<NumFrameDeg;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameRef);
		point=CalcBackgroundFrame(DegImageA[i],DegImageA[i-1],RefImageA[RegistrationIndex[i]],RefImageA[RegistrationIndex[i]-1],sse_temp);
		if(point)
			ltemp = sse_temp/point;
		else
			ltemp = 2000;

		if(ltemp<1000){
			BGpntnum2 += point;
			sse += sse_temp;			
		}		
	}
	printf("\n\n");

	BGmse = sse/(double)BGpntnum2;
	BGratio2 = BGpntnum2/(ImageSize*NumFrameDeg*0.01);

	return 1;
}

long WPSNR::CalcBGratio1(){

	long	i,j,ltemp;
	byte	*curr,*prev;

	BGpntnum1=0;
	printf("---bgratio1: 000/%03d",NumFrameRef);
	for(i=1;i<NumFrameRef;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameRef);

		curr = RefImageA[i];
		prev = RefImageA[i-1];

		for(j=0;j<ImageSize;j++){

			ltemp = abs(curr[j]-prev[j]);
			if(ltemp<4)
				BGpntnum1++;
		}
	}
	printf("\n");

	BGratio1 = BGpntnum1/(ImageSize*NumFrameRef*0.01);
	
	return 1;
}

long WPSNR::CalcBackgroundFrame(byte *curr, byte *prev, byte *currR, byte *prevR, _int64 &sse){

	long	i,j,ltemp;
	long	point;
	byte	*p_byte1,*p_byte2,*p_byte3;

	// background map
	for(i=0;i<ImageSize;i++){
		ltemp = abs(currR[i]-prevR[i]);
		if(ltemp<4)	BGMap[i]=255;
		else		BGMap[i]=0;
	}

	point=0;
	sse=0;

	p_byte1 = curr + width*(shiftY+marginY) + shiftX;
	p_byte2 = prev + width*(shiftY+marginY) + shiftX;
	p_byte3 = BGMap + width*marginY;

	for(i=marginY;i<heightM;i++,p_byte1+=width,p_byte2+=width,p_byte3+=width){
		for(j=marginX;j<widthM;j++){

			if(p_byte3[j]){

				ltemp = p_byte1[j]-p_byte2[j];
				ltemp = ltemp*ltemp;

				if(ltemp>10 && IsNearbyZero(p_byte3+j,5)==0){
					sse += ltemp;
					point++;					
				}
			}			
		}
	}

	return point;
}


long WPSNR::IsNearbyZero(byte *map_pst, long radius){

	long	i,j;
	long	RadiusSq;
	long	dist1,dist2;
	byte	*p_byte1;

	RadiusSq = radius*radius;

	p_byte1 = map_pst - radius*width;
	for(i=-radius;i<=radius;i++,p_byte1+=width){

		dist1 = i*i;
		for(j=-radius;j<=radius;j++){
			
			dist2 = dist1 + j*j;
			if(dist2>RadiusSq)
				continue;
			if(p_byte1[j]==0)
				return 1;			
		}
	}

	
	return 0;
}


long WPSNR::CalcFreeze(){

	long	i;

	memset(DegFreeze,0,NumFrameDeg);
	printf("=====Freeze=====\n");
	printf("---SearchFreeze: 000/%03d",NumFrameDeg);
	for(i=1;i<NumFrameDeg;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		DegFreeze[i] = IsSame(DegImageA[i],DegImageA[i-1]);
	}
	printf("\n\n");
	FreezeInfoDeg(DegFreeze);

	
	return 1;
}


long WPSNR::FreezeInfoDeg(byte *freeze){

	long	i,frz_len;
	
	Dtfrz=Dmfrz=Dnfrz=Dtfrz1=0;
	frz_len=0;
	for(i=1;i<NumFrameDeg;i++){
		
		if(freeze[i]){			
			if(frz_len==0)
				Dnfrz++;
			Dtfrz++;
			frz_len++;			
		}
		else{
			if(frz_len){
				if(frz_len>1)
					Dtfrz1 += frz_len;
				if(frz_len>Dmfrz)
					Dmfrz = frz_len;
				frz_len=0;
			}
		}
	}

	if(frz_len){
		if(frz_len>1)
			Dtfrz1 += frz_len;
		if(frz_len>Dmfrz)
			Dmfrz = frz_len;
	}
	
	return 1;
}