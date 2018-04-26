#include "wpsnr.h"

long WPSNR::CalcBlocking(){

	long	i;
	
	printf("=====Blocking=====\n");
	memset(blkval,0,sizeof(double)*NumFrameDeg);
	memset(blkcnt,0,sizeof(long)*NumFrameDeg);
	printf("---blocking: 000/%03d",NumFrameDeg);
	for(i=0;i<NumFrameDeg;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		CalcBlockingFrame(RefImageA[RegistrationIndex[i]],DegImageA[i],blkval[i],blkcnt[i]);
	}
	printf("\n\n");

	BLKCalcFTR();

	return 1;
}

long WPSNR::CalcBlockingFrame(byte *ref, byte *deg, double &blkframe, long &blkcnt){

	long	i,j,ltemp,th1,th2,cnt;
	long	Histo[256];
	byte	*p_byte1,*p_byte2;

	// low pass filtering
	Gaussian(ref,blkImage1);
	Gaussian(deg,blkImage2);

	// edge detection
	Sobel(blkImage1,blkrefedge,0);
	Sobel(blkImage2,blkdegedge,0);

	// make binary image
	for(i=0;i<ImageSize;i++){
		if(blkrefedge[i]>10)blkImage1[i]=255;
		else				blkImage1[i]=0;
	}

	// morphology
	p_byte1 = blkImage1, p_byte2 = blkImage2;
	for(i=0;i<height;i++,p_byte1+=width,p_byte2+=width){
		for(j=0;j<width;j++){

			if(p_byte1[j])
				p_byte2[j]=255;
			else{
				if(IsNearNonZero(p_byte1+j,5,j,i))
					p_byte2[j]=255;
				else
					p_byte2[j]=0;
			}			
		}
	}

	// binary image		
	for(i=0;i<ImageSize;i++){
		if(blkrefedge[i]<blkdegedge[i] && blkImage2[i]==0)
			blkImage1[i] = 255;
		else
			blkImage1[i] = 0;
	}

	// cdf calculation
	for(i=0;i<256;i++)
		Histo[i] = 0;
	blkcnt = 0;
	for(i=0;i<ImageSize;i++){
		if(blkImage1[i]){
			ltemp = blkdegedge[i]-blkrefedge[i];
			if(ltemp>=256)
				ltemp = 255;
			blkImage2[i] = (byte)ltemp;
						
			Histo[ltemp]++;	
			blkcnt++;
		}
	}

	th1 = blkcnt>>3;
	ltemp = 0;
	for(i=255;i>=0;i--){
		if(ltemp>th1)
			break;
		ltemp += Histo[i];
	}
	th2 = i;

	cnt = ltemp = 0;
	for(i=0;i<ImageSize;i++){
		if(blkImage1[i]){
			if(blkImage2[i]>=th2){
				ltemp += blkImage2[i];
				cnt ++;
			}
		}
	}
	
	blkframe = ltemp/(double)cnt;

	return 1;
}



long WPSNR::Gaussian(byte *InImage, byte *OutImage){

	long	i,j;
	long	width_1,height_1;
	long	ltemp,ltemp2;
	byte	*p_byte1[7],*p_byte11[7],*p_byte2;

	width_1 = width-3;
	height_1 = height-3;

	p_byte11[0] = InImage;	
	for(i=1;i<7;i++)
		p_byte11[i] = p_byte11[i-1] + width;
	p_byte2 = OutImage + width*3;

	for(i=3;i<height_1;i++,p_byte2+=width){

		for(j=0;j<7;j++)
			p_byte1[j] = p_byte11[j];

		for(j=3;j<width_1;j++,p_byte1[0]++,p_byte1[1]++,p_byte1[2]++,p_byte1[3]++,p_byte1[4]++,p_byte1[5]++,p_byte1[6]++){

			ltemp = 0;
			ltemp = p_byte1[0][0] + p_byte1[0][6] + p_byte1[6][0] + p_byte1[6][6];	// for 1: 4 item(4)
			
			ltemp2 = p_byte1[0][1] + p_byte1[0][5] + p_byte1[1][0] + p_byte1[1][6]
			       + p_byte1[5][0] + p_byte1[5][6] + p_byte1[6][1] + p_byte1[6][5];	// for 3: 8 item(12)
			ltemp += ltemp2*3;
			
			ltemp2 = p_byte1[0][2] + p_byte1[0][4] + p_byte1[2][0] + p_byte1[2][6]
			       + p_byte1[4][0] + p_byte1[4][6] + p_byte1[6][2] + p_byte1[6][4];	// for 6: 8 item(20)
			ltemp += ltemp2*6;

			ltemp2 = p_byte1[0][3] + p_byte1[3][0] + p_byte1[3][6] + p_byte1[6][3];	// for 7: 4 item(24)
			ltemp += ltemp2*7;

			ltemp2 = p_byte1[1][1] + p_byte1[1][5] + p_byte1[5][1] + p_byte1[5][5];	// for 9: 4 item(28)
			ltemp += ltemp2*9;

			ltemp2 = p_byte1[1][2] + p_byte1[1][4] + p_byte1[2][1] + p_byte1[2][5]
			       + p_byte1[4][1] + p_byte1[4][5] + p_byte1[5][2] + p_byte1[5][4];	// for 18: 8 item(36)
			ltemp += ltemp2*18;

			ltemp2 = p_byte1[1][3] + p_byte1[3][1] + p_byte1[3][5] + p_byte1[5][3];	// for 22: 4 item(40)
			ltemp += ltemp2*22;

			ltemp2 = p_byte1[2][2] + p_byte1[2][4] + p_byte1[4][2] + p_byte1[4][4];	// for 35: 4 item(44)
			ltemp += ltemp2*35;

			ltemp2 = p_byte1[2][3] + p_byte1[3][2] + p_byte1[3][4] + p_byte1[4][3];	// for 44: 4 item(48)
			ltemp += ltemp2*44;

			ltemp += p_byte1[3][3]*55;

			p_byte2[j] = (byte)(ltemp/743.+0.5);

		}
		for(j=0;j<7;j++)
			p_byte11[j] += width;
	}

	return 1;
}

long WPSNR::IsNearNonZero(byte *point, long radius, long x, long y){

	long	i,j,ltemp;
	long	radiusSq;
	double	distance1,distance2;
	byte	*p_byte1;
		
	radiusSq = radius*radius;

	p_byte1 = point - radius*width;	
	for(i=-radius;i<=radius;i++,p_byte1+=width){

		ltemp = i+y;
		if(ltemp<0 || ltemp>=height)
			continue;
		distance1 = i*i;

		for(j=-radius;j<=radius;j++){

			ltemp = j+x;
			if(ltemp<0 || ltemp>=width)
				continue;
			
			distance2 = distance1+j*j;
			if(distance2>radiusSq)
				continue;

			if(p_byte1[j])
				return 1;
		}
	}
	
	return 0;
}


long WPSNR::BLKCalcFTR(){

	long	i,j,Num,ltemp;
	double	dtemp;
	double	temp[5];
	long	tempIdx[5];

	Num = 5;

	for(i=0;i<Num;i++){
		temp[i] = 0;
		tempIdx[i] = i;
	}

	for(i=0;i<NumFrameDeg;i++){
		if(blkval[i]>temp[0]){
			temp[0] = blkval[i];
			tempIdx[0] = i;

			for(j=1;j<Num;j++){
				if(temp[j]<temp[j-1]){
					dtemp = temp[j];
					temp[j] = temp[j-1];
					temp[j-1] = dtemp;

					ltemp = tempIdx[j];
					tempIdx[j] = tempIdx[j-1];
					tempIdx[j-1] = ltemp;
				}
			}
		}
	}

	blocking = 0;
	blk_cnt = 0;
	for(i=0;i<Num;i++){
		blocking += temp[i];
		blk_cnt += blkcnt[tempIdx[i]];
	}	
	blocking /= Num;
	blk_cnt /= Num;

	return 1;
}


long WPSNR::CalcFalseColor(){

	long	i;

	skip_frame=10;
	FCpoint=0;
	FCvariance=0;
	
	printf("=====FalseColor=====\n");
	printf("---blocking: 000/%03d",NumFrameDeg);
	for(i=5;i<NumFrameDeg-5;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		CalcFalseColorFrame(RefImageA[RegistrationIndex[i]],DegImageA[i]);

		if(FCvarframe<0)
			skip_frame++;
		else
			FCvariance += FCvarframe;
	}
	printf("\n\n");

	FCframe = NumFrameDeg-skip_frame;
	if(FCframe)
		FCvariance /= FCframe;
	else
		FCvariance = -1;
	
	return 1;
}

long WPSNR::CalcFalseColorFrame(byte *ref, byte *deg){

	long	i,j,cnt;
	byte	*DiffImage,*DiffMap,*DiffMap2;
	byte	*p_byte1,*p_byte2,*p_byte3;
	short	*SobelRef,*p_short1;
	long	*ErrorHisto;
	long	HistoTh,ErrorTh;
	long	SparseFlag;
	double	density,dtemp;
	byte	btemp,btemp2;

	// memory matching		
	DiffImage = falsecolor_work;
	DiffMap = DiffImage + ImageSize;
	DiffMap2 = DiffMap + ImageSize;
	SobelRef = (short *)(DiffMap2 + ImageSize);
	ErrorHisto = (long *)(SobelRef + ImageSize);

	// calculate histogram
	p_byte1 = ref + width*marginY;
	p_byte2 = deg + width*(marginY+shiftY) + shiftX;
	p_byte3 = DiffImage + width*marginY;
	memset(ErrorHisto,0,sizeof(long)*256);
	memset(DiffImage,0,ImageSize);
	for(i=marginY;i<heightM;i++,p_byte1+=width,p_byte2+=width,p_byte3+=width){
		for(j=marginX;j<widthM;j++){
			if(p_byte1[j]>p_byte2[j])
				btemp = p_byte1[j]-p_byte2[j];
			else
				btemp = p_byte2[j]-p_byte1[j];

			p_byte3[j] = btemp;
			ErrorHisto[btemp]++;
		}
	}

	// Calculate CDF
	for(i=254;i>=0;i--)
		ErrorHisto[i] += ErrorHisto[i+1];
	HistoTh = (long)(ImageSizeM*0.1 + 0.5);

	for(i=0;i<256;i++){
		if(ErrorHisto[i]<HistoTh)
			break;
	}
	ErrorTh = i-1;

	// Sobel filtering
	Sobel(ref,SobelRef,0);

	// Difference Map setting
	memset(DiffMap,0,ImageSize);
	p_byte1 = DiffImage + width*marginY;
	p_byte2 = DiffMap + width*marginY;
	p_short1 = SobelRef + width*marginY;	
	for(i=marginY;i<heightM;i++,p_byte1+=width,p_byte2+=width,p_short1+=width){
		for(j=marginX;j<widthM;j++){
			if(p_byte1[j]>ErrorTh && p_short1[j]<10)
				p_byte2[j] = 255;
		}
	}

	cnt = 0;
	memset(DiffMap2,0,ImageSize);
	p_byte1 = DiffMap + width*marginY;
	p_byte2 = DiffMap2 + width*marginY;
	for(i=marginY;i<heightM;i++,p_byte1+=width,p_byte2+=width){
		for(j=marginX;j<widthM;j++){

			if(p_byte1[j]==0)
				continue;

			density = CalculateDensity(p_byte1+j,5);
			if(density>0.5){
				p_byte2[j] = 255;
				cnt++;
			}

		}
	}

	if(cnt<10000){
		FCvarframe = -1;
		return 1;
	}

	// merge image
	SparseFlag=0;
	memset(ErrorHisto,0,sizeof(long)*256);
	btemp = 1;
	p_byte1 = DiffMap2 + width*marginY;
	for(i=marginY;i<heightM;i++,p_byte1+=width){
		for(j=marginX;j<widthM;j++){

			if(p_byte1[j]){

				btemp2 = FindCircle(p_byte1+j,50,j,i);

				if(btemp2){
					p_byte1[j] = btemp2;
					ErrorHisto[btemp2]++;
				}
				else{
					p_byte1[j] = btemp;
					ErrorHisto[btemp]++;
					btemp++;
					if(btemp==0){
						SparseFlag=1;
						btemp++;
					}
				}
			}
		}
	}

	double	sumXX,sumX;
	if(SparseFlag){
		sumXX = sumX = 0.;
		for(i=0;i<ImageSize;i++){
			
			if(DiffMap2[i]){
				sumXX += DiffImage[i]*DiffImage[i];
				sumX += DiffImage[i];
			}
		}		
		sumXX /= cnt;
		sumX /= cnt;
		FCvarframe = sumXX - sumX*sumX;
	}
	else{
		FCvarframe=0.;
		for(i=1;i<btemp;i++){
			sumXX = sumX = 0.;
			for(j=0;j<ImageSize;j++){
				if(DiffMap2[j]==(byte)i){
					sumXX += DiffImage[j]*DiffImage[j];
					sumX += DiffImage[j];
				}				
			}
			sumXX /= ErrorHisto[i];
			sumX /= ErrorHisto[i];
			dtemp = sumXX-sumX*sumX;

			if(dtemp>0.00001)
				FCvarframe += dtemp*ErrorHisto[i]/cnt;
		}
	}

	FCpoint += cnt;

	return 1;
}


double WPSNR::CalculateDensity(byte *Point, long Radius){

	long	i,j;
	long	cnt,cntAll;
	long	RadiusSq;
	double	distance1,distance2;
	double	density;
	byte	*p_byte1;

	cnt = cntAll = 0;
	RadiusSq = Radius*Radius;
	p_byte1 = Point - Radius*width;
	for(i=-Radius;i<=Radius;i++,p_byte1+=width){
		distance1 = i*i;
		for(j=-Radius;j<=Radius;j++){
			
			distance2 = distance1+j*j;
			if(distance2>RadiusSq)
				continue;

			cntAll++;
			if(p_byte1[j])
				cnt++;		
		}
	}

	density = cnt/(double)cntAll;	
	
	return density;
}

byte WPSNR::FindCircle(byte *point, long radius, long x, long y){

	long	i,j,ii,jj;
	long	RadiusSq;
	long	dist1,dist2;
	byte	*p_byte1;

	RadiusSq = radius*radius;

	p_byte1 = point - radius*width;
	for(i=-radius,ii=i+y;i<=radius;i++,ii++,p_byte1+=width){

		if(ii<0 || ii>=height)
			continue;
		
		dist1 = i*i;
		for(j=-radius,jj=j+x;j<=radius;j++,jj++){
			if(jj<0 || jj>=width)
				continue;
			dist2 = dist1 + j*j;
			if(dist2>RadiusSq)
				continue;
			if(p_byte1[j]==0)
				continue;

			if(p_byte1[j]!=255)
				return p_byte1[j];
		}
	}
	return 0;
}

long WPSNR::CalcEdgeRatio(){

	long	i;

	skip_frame = 10;
	edgehigh = edgelow = 0;

	printf("=====EdgeRatio=====\n");
	printf("---epsnr: 000/%03d",NumFrameDeg);
	for(i=5;i<NumFrameDeg-5;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		CalcEdgeRatioFrame(RefImageA[RegistrationIndex[i]],DegImageA[i]);

	}
	printf("\n\n");

	edgehigh /= (NumFrameDeg-skip_frame);
	edgehigh = 10*log10(65025/edgehigh);
	edgelow /= (NumFrameDeg-skip_frame);
	edgelow = 10*log10(65025/edgelow);

	EdgeRatio = edgelow/edgehigh;
	
	return 1;
}

long WPSNR::CalcEdgeRatioFrame(byte *ref, byte *deg){

	double	MSE[3];
	double	MSEhigh[3],MSElow[3];
	
	DivideField(ref,ERfieldR1,ERfieldR2);
	DivideField(deg,ERfieldD1,ERfieldD2);

	MSE[0] = CalcEdgeRatioImage(ref,deg,MSEhigh[0],MSElow[0],0);
	MSE[1] = CalcEdgeRatioImage(ERfieldR1,ERfieldD1,MSEhigh[1],MSElow[1],1);
	MSE[2] = CalcEdgeRatioImage(ERfieldR1,ERfieldD1,MSEhigh[2],MSElow[2],1);
	
	// adding
	if(MSE[0]<MSE[1] && MSE[0]<MSE[2])
		edgehigh+=MSEhigh[0],edgelow+=MSElow[0];
	else if(MSE[1]<MSE[0] && MSE[1]<MSE[2])
		edgehigh+=MSEhigh[1],edgelow+=MSElow[1];
	else
		edgehigh+=MSEhigh[2],edgelow+=MSElow[2];

	return 1;
}

double WPSNR::CalcEdgeRatioImage(byte *ref, byte *deg, double &high, double &low, long interlaced){

	long	i,j,k,ltemp,Size1;
	long	width_1,height_1,marginYY;
	short	edge_max,edge_min;
	short	threshold[10];
	short	*p_short;
	byte	*p_byte1,*p_byte2;
	long	subSize[10];
	double	output[10];
	double	mse;
		
	// size calculation
	if(interlaced){		
		width_1 = width-marginX;
		height_1 = (height-marginY)>>1;
		marginYY = marginY>>1;		
		Size1 = (width_1-marginX)*(height_1-marginYY);
	}
	else{
		width_1 = width-marginX;
		height_1 = height-marginY;
		marginYY = marginY;		
		Size1 = (width_1-marginX)*(height_1-marginYY);
	}

	for(i=0;i<10;i++){
		subSize[i] = 0;
		output[i] = 0;
	}

	// sobel filtering
	Sobel(ref,ERSobel,interlaced);

	edge_max = 0, edge_min = 10000;
	for(i=0;i<ImageSize;i++){
		if(edge_max<ERSobel[i])
			edge_max = ERSobel[i];
		if(edge_min>ERSobel[i])
			edge_min = ERSobel[i];
	}		

	for(i=1;i<=9;i++)
		threshold[i-1] = (short)((edge_max-edge_min)*i*0.1+0.5);
	threshold[i-1] = edge_max+1;

	p_byte1 = ref + width*marginYY;
	p_byte2 = deg + width*(marginYY+shiftY) + shiftX;
	p_short = ERSobel + width*marginYY;
		
	mse = 0.;
	for(i=marginYY;i<height_1;i++,p_byte1+=width,p_byte2+=width,p_short+=width){
		for(j=marginX;j<width_1;j++){			
			ltemp = FilteringPixel(p_byte1+j)-FilteringPixel(p_byte2+j);
			mse += ltemp*ltemp;			
			for(k=0;k<10;k++){
				if(p_short[j]<threshold[k]){					
					output[k] += ltemp*ltemp;
					subSize[k]++;
					break;
				}
			}
		}
	}

	for(k=0;k<10;k++){
		if(subSize[k])
			output[k] /= subSize[k];
	}
	mse /= Size1;

	high = 	output[9];
	low = output[0];
	
	return mse;
}