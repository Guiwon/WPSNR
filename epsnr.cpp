#include "wpsnr.h"

long WPSNR::CalcEPSNR(){
	
	long	i;
	
	EdgeMSE = 0;
	skip_frame = 10;

	printf("=====EPSNR=====\n");
	printf("---epsnr: 000/%03d",NumFrameDeg);
	for(i=5;i<NumFrameDeg-5;i++){
		printf("\b\b\b\b\b\b\b%03d/%03d",i+1,NumFrameDeg);
		CalcEPSNRFrame(RefImageA[RegistrationIndex[i]],DegImageA[i]);
	}
	printf("\n\n");

	EdgeMSE /= (NumFrameDeg-skip_frame);
	epsnr2 = 10*log10(65025/EdgeMSE);

	return 1;
}

long WPSNR::CalcEPSNRFrame(byte *Ref, byte *Deg){

	double	MSE1,MSE2,MSE3;
	double	SobelMSE1,SobelMSE2,SobelMSE3;

	DivideField(Ref,Rfield1,Rfield2);
	DivideField(Deg,Dfield1,Dfield2);

	MSE1 = CalcEPSNRImage(Ref,Deg,500,SobelMSE1,0);
	MSE2 = CalcEPSNRImage(Rfield1,Dfield1,250,SobelMSE2,1);
	MSE3 = CalcEPSNRImage(Rfield2,Dfield2,250,SobelMSE3,1);

	if(SobelMSE1<SobelMSE2 && SobelMSE1<SobelMSE3){
		if(MSE1<0)
			skip_frame++;
		else
			EdgeMSE += MSE1;			
	}
	else if(SobelMSE2<SobelMSE1 && SobelMSE2<SobelMSE3){
		if(MSE2<0)
			skip_frame++;
		else
			EdgeMSE += MSE2;
	}
	else{
		if(MSE3<0)
			skip_frame++;
		else
			EdgeMSE += MSE3;
	}
	
	return 1;
}


double WPSNR::CalcEPSNRImage(byte *Ref, byte *Deg, long NumPST, double &SobelMSE, long interlaced){

	long	i,j,k,ltemp,NumPST1,Size,Size1;
	long	width_1,height_1,marginYY;
	short	*SobelR,*SobelD;
	short	*p_SR,*p_SD;
	double	mse;
	PST		*CalcPST;
	PST		tmpPST;

	// size calculation
	if(interlaced){
		Size = width*(height>>1);
		width_1 = width-marginX;
		height_1 = (height-marginY)>>1;
		marginYY = marginY>>1;
		Size1 = (width_1-marginX)*(height_1-marginYY);
	}
	else{
		Size = width*height;
		width_1 = width-marginX;
		height_1 = height-marginY;
		marginYY = marginY;
		Size1 = (width_1-marginX)*(height_1-marginYY);
	}

	// memory
	SobelR = (short *)EPSNR_workspace;
	SobelD = SobelR + Size;
	CalcPST = (PST *)(SobelD + Size);

	// Initialize
	memset(CalcPST,0,sizeof(PST)*NumPST);

	// sobel filtering	
	Sobel(Ref,SobelR,interlaced);
	Sobel(Deg,SobelD,interlaced);

	// SobelMSE calculation
	SobelMSE=0.;
	p_SR = SobelR + width*marginYY;
	p_SD = SobelD + width*(marginYY+shiftY)+shiftX;
	for(i=marginYY;i<height_1;i++,p_SR+=width,p_SD+=width){
		for(j=marginX;j<width_1;j++){
			ltemp = p_SR[j]-p_SD[j];
			SobelMSE += ltemp*ltemp;
		}
	}	
	SobelMSE /= Size1;

	NumPST1 = 0;
	// Sobel Sorting
	for(i=marginYY,p_SR=SobelR+width*marginYY;i<height_1;i++,p_SR+=width){
		for(j=marginY;j<width_1;j++){
			if(p_SR[j]<10 || p_SR[j]<CalcPST[0].val)
				continue;

			NumPST1++;
			CalcPST[0].val=p_SR[j];
			CalcPST[0].x=(short)j, CalcPST[0].y=(short)i;

			// sorting
			for(k=1;k<NumPST;k++){
				if(CalcPST[k].val<CalcPST[k-1].val){
					tmpPST = CalcPST[k];
					CalcPST[k] = CalcPST[k-1];
					CalcPST[k-1] = tmpPST;
				}
				else
					break;
			}
		}
	}
	if(NumPST1>NumPST)	NumPST1 = NumPST;
	if(NumPST1==0)		return -1;

	ltemp = shiftY*width + shiftX;
	for(mse=0.,i=NumPST-NumPST1;i<NumPST;i++){

		j = CalcPST[i].y*width + CalcPST[i].x;
		//k = Ref[j]-Deg[j+ltemp];
		k = FilteringPixel(Ref+j)-FilteringPixel(Deg+j+ltemp);
		mse += k*k;
	}
	mse /= NumPST1;
	
	return mse;
}

long WPSNR::DivideField(byte *frame, byte *field1, byte *field2){

	long	i;
	byte	*p_fr,*p_f1,*p_f2;

	p_fr = frame, p_f1 = field1, p_f2 = field2;
	for(i=0;i<height;i++,p_fr+=width){

		if(i%2){
			memcpy(p_f2,p_fr,width);
			p_f2 += width;
		}
		else{
			memcpy(p_f1,p_fr,width);
			p_f1 += width;
		}
	}

	return 1;
}

long WPSNR::Sobel(byte *Image, short *SobelImage, long interlaced){

	long	i,j,j1,j2;
	long	width_1,height_1;
	short	horizontal,vertical;
	byte	*p_u,*p_l,*p_c;
	short	*p_sobel;

	memset(SobelImage,0,sizeof(short)*ImageSize);

	// pointer setting
	p_u = Image;
	p_c = p_u + width;
	p_l = p_c + width;
	p_sobel = SobelImage + width;

	width_1 = width-1;
	if(interlaced)
		height_1 = (height>>1)-1;
	else
		height_1 = height-1;
	
	for(i=1;i<height_1;i++,p_u+=width,p_c+=width,p_l+=width,p_sobel+=width){
		for(j=1,j1=0,j2=2;j<width_1;j++,j1++,j2++){
			
			horizontal = p_u[j2]-p_u[j1]+((p_c[j2]-p_c[j1])<<1)+p_l[j2]-p_l[j1];
			vertical = p_u[j1]-p_l[j1]+((p_u[j]-p_l[j])<<1)+p_u[j2]-p_l[j2];

			p_sobel[j] = (short)(sqrt((double)horizontal*horizontal + vertical*vertical)+0.5);
		}
	}
	return 1;
}

byte WPSNR::FilteringPixel(byte *pst){

	byte	*p1,*p2,*p3;
	long	ltemp;

	p1 = pst - width;
	p2 = pst;
	p3 = pst + width;

	/*
	3	9	18	22	18	9	3
	7	22	44	55	44	22	7
	3	9	18	22	18	9	3		
		*/

	ltemp = (p1[-3]+p1[3]+p3[-3]+p3[3])*3;
	ltemp += (p1[-2]+p1[2]+p3[-2]+p3[2])*9;
	ltemp += (p1[-1]+p1[1]+p3[-1]+p3[1])*18;
	ltemp += (p1[0]+p3[0]+p2[-2]+p2[2])*22;
	ltemp += (p2[-3]+p2[3])*7;
	ltemp += (p2[-1]+p2[1])*44;
	ltemp += p2[0]*55;
	
	return (byte)(ltemp/365.+0.5);
}

long WPSNR::RunEPSNR(){

	long	i;
	FILE	*fp;
	char	ctemp[128];
		
	fp = fopen("temp_list.txt","wt");
	fprintf(fp,"%s\t%s\tprogressive\n",refFname,degFname);
	fclose(fp);

	system("epsnr.exe temp_list.txt HFR");
	
	fp = fopen("temp_list.txt_hfr_hyoutput.txt","rt");
	for(i=0;i<7;i++)
		fscanf(fp,"%s",ctemp);
	fclose(fp);

	epsnr1 = atof(ctemp);

	remove("temp_list.txt");
	remove("temp_list.txt_hfr_hyoutput.txt");

	return 1;
}