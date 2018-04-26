#include "wpsnr.h"

long WPSNR::CalcWPSNR(){

	long	i,j;

	// initialize
	for(i=0;i<NumFrameDeg;i++){
		for(j=0;j<25;j++)
			SubMSE[i][j] = 0;
	}

	// sub-psnr position calculation
	widthSub = (width-(marginX<<1))>>2;
	heightSub = (height-(marginY<<1))>>2;	
	subSize = widthSub*heightSub;
	subSizeH = subSize>>1;
	j = heightSub*width;
	pstSub[24]=marginY*width+marginX;pstSub[13]=pstSub[24]+widthSub;pstSub[14]=pstSub[13]+widthSub;	pstSub[15]=pstSub[14]+widthSub;
	pstSub[23]=pstSub[24]+j;		pstSub[4]=pstSub[23]+widthSub;	pstSub[1]=pstSub[4]+widthSub;	pstSub[16]=pstSub[1]+widthSub;
	pstSub[22]=pstSub[23]+j;		pstSub[3]=pstSub[22]+widthSub;	pstSub[2]=pstSub[3]+widthSub;	pstSub[17]=pstSub[2]+widthSub;
	pstSub[21]=pstSub[22]+j;		pstSub[20]=pstSub[21]+widthSub;	pstSub[19]=pstSub[20]+widthSub;	pstSub[18]=pstSub[19]+widthSub;

	pstSub[12]=(marginY+(heightSub>>1))*width + marginX+(widthSub>>1);	pstSub[5]=pstSub[12]+widthSub;	pstSub[6]=pstSub[5]+widthSub;
	pstSub[11]=pstSub[12]+j;											pstSub[0]=pstSub[11]+widthSub;	pstSub[7]=pstSub[0]+widthSub;
	pstSub[10]=pstSub[11]+j;											pstSub[9]=pstSub[10]+widthSub;	pstSub[8]=pstSub[9]+widthSub;

	printf("=====WPSNR=====\n");
	printf("---wpsnr: 000/%03d",NumFrameDeg);	
	for(i=5;i<NumFrameDeg-5;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		CalcWPSNRFrame(RefImageA[RegistrationIndex[i]],DegImageA[i],SubMSE[i]);
	}
	printf("\n");
	SelectWPSNR();
	printf("\n");

		
	return 1;
}

long WPSNR::CalcWPSNRFrame(byte *Ref, byte *Deg, double *FrameMSE){

	long	i,j,k,offset,ltemp1,ltemp2;
	double	progressive,interlaced1,interlaced2;
	byte	*p_byte1,*p_byte2;

	offset = shiftY*width + shiftX;

	for(i=0;i<25;i++){

		p_byte1 = Ref + pstSub[i];
		p_byte2 = Deg + pstSub[i] + offset;

		progressive = interlaced1 = interlaced2 = 0.;
		for(j=0;j<heightSub;j++,p_byte1+=width,p_byte2+=width){

			if(j%2){
				for(k=0;k<widthSub;k++){
					ltemp1 = p_byte1[k]-p_byte2[k];
					ltemp2 = ltemp1*ltemp1;
					progressive += ltemp2;
					interlaced2 += ltemp2;
				}
			}
			else{
				for(k=0;k<widthSub;k++){
					ltemp1 = p_byte1[k]-p_byte2[k];
					ltemp2 = ltemp1*ltemp1;
					progressive += ltemp2;
					interlaced1 += ltemp2;
				}
			}
		}
		progressive /= subSize;
		interlaced1 /= subSizeH;
		interlaced2 /= subSizeH;

		FrameMSE[i] = MIN3(progressive,interlaced1,interlaced2);
	}
	
	return 1;
}

long WPSNR::SelectWPSNR(){

	long	i,j,k;
	long	FrameNum=78;
	double	dtemp1,dtemp2;
	double	FrameMax,VideoMax[78];

	long	OverFrameNum;
	double	wpsnr1,wpsnr2;
	double	wpsnr_ave;

	VeryShortError = 0;

	for(i=0;i<FrameNum;i++)
		VideoMax[i] = 0;

	for(i=0;i<NumFrameDeg;i++){

		// initialize
		FrameMax = 0;		
		for(j=0;j<25;j++){			
			if(SubMSE[i][j]>FrameMax)
				FrameMax = SubMSE[i][j];
		}
		
		if(FrameMax>VideoMax[0]){
			VideoMax[0] = FrameMax;
			for(k=1;k<FrameNum;k++){
				if(VideoMax[k]<VideoMax[k-1]){
					dtemp1 = VideoMax[k];
					VideoMax[k] = VideoMax[k-1];
					VideoMax[k-1] = dtemp1;
				}
				else
					break;
			}
		}
	}

	// average wpsnr
	for(i=0,dtemp1=0.;i<FrameNum;i++)
		dtemp1 += 10*log10(65025/VideoMax[i]);
	dtemp1 /= FrameNum;
	wpsnr_ave = dtemp1;

	// mse average
	for(i=0,dtemp1=0;i<FrameNum;i++)
		dtemp1 += VideoMax[i];
	dtemp1 /= FrameNum;

	// overframe num
	OverFrameNum=0;
	for(i=0;i<FrameNum;i++){
		if(dtemp1<VideoMax[i])
			OverFrameNum++;
	}

	dtemp2=0.;
	for(i=0;i<FrameNum;i++){
		if(dtemp1>=VideoMax[i])
			dtemp2 += VideoMax[i];
	}
	dtemp2 /= (FrameNum-OverFrameNum);

	if(dtemp2>0.0001)
		wpsnr2 = 10*log10(65025/dtemp2);
	else
		wpsnr2 = 99.;


	if(dtemp1>0.0001)
		wpsnr1 = 10*log10(65025/dtemp1);
	else
		wpsnr1 = 99.;

	// wpsnr setting
	if(wpsnr_ave>wpsnr1+s_offset1 && OverFrameNum<overnum1){
		VeryShortError=1;
		wpsnr = wpsnr2;
	}
	else if(wpsnr_ave>wpsnr1+s_offset2 && OverFrameNum<overnum2)
		wpsnr = wpsnr_ave;
	else
		wpsnr = wpsnr1;

	
	return 1;
}

