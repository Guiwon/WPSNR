#include "wpsnr.h"

// luminance extraction
long WPSNR::ExtractLuma(byte *output){
	
	long	i;
	byte	*p_byte;

	for(i=0,p_byte=ReadBuffer+1;i<ImageSize;i++,p_byte+=2)
		output[i] = *p_byte;

	return 1;
}

double WPSNR::MIN3(double m1, double m2, double m3){

	if(m1<m2 && m1<m3)
		return m1;
	else if(m2<m1 && m2<m3)
		return m2;
	else
		return m3;
}


#define	AVE_DIFF_TH		0.90
#define	CNT_DIFFF_TH	300
#define	SIZE_DIFF_TH	50

byte WPSNR::IsSame(byte *frame1, byte *frame2){

	long	i,j,diff_cnt,pix_diff;
	long	max_i,max_j,min_i,min_j;
	long	size_i,size_j;
	double	diff;
	byte	*p_frame1,*p_frame2;

	diff = 0.;
	diff_cnt = 0;
	max_i=max_j=-1,min_i=height,min_j=width;			
	for(i=0,p_frame1=frame1,p_frame2=frame2;i<height;i++,p_frame1+=width,p_frame2+=width){
		for(j=0;j<width;j++){
			pix_diff = p_frame1[j]-p_frame2[j];
			if(pix_diff!=0){
				diff_cnt++;
				if(j>max_j)	max_j=j;
				if(j<min_j)	min_j=j;
				if(i>max_i)	max_i=i;
				if(i<min_i)	min_i=i;
			}

			if(pix_diff<0)	diff -= pix_diff;
			else			diff += pix_diff;

		}
	}

	size_i = max_i-min_i+1, size_j = max_j-min_j+1;
	diff /= (double)ImageSize;

	if(diff<AVE_DIFF_TH && diff_cnt<CNT_DIFFF_TH && size_i<SIZE_DIFF_TH && size_j<SIZE_DIFF_TH)
		return 1;
	return 0;	
}

long WPSNR::FindExtension(){

	long	i,len;
	char	ext[512];

	refext = degext = -1;

	// reference	
	len = strlen(refFname);
	for(i=len-1;i>=0;i--){
		if(refFname[i]=='.')
			break;
	}
	strcpy(ext,refFname+i);
	strlwr(ext);
	if(strcmp(ext,".avi")==0)		refext = AVI;
	else if(strcmp(ext,".lss")==0)	refext = LSS;
	else if(strcmp(ext,".yuv")==0)	refext = YUV;
	else							refext = -1;

	// degraded
	len = strlen(degFname);
	for(i=len-1;i>=0;i--){
		if(degFname[i]=='.')
			break;
	}
	strcpy(ext,degFname+i);
	strlwr(ext);
	if(strcmp(ext,".avi")==0)		degext = AVI;
	else if(strcmp(ext,".lss")==0)	degext = LSS;
	else if(strcmp(ext,".yuv")==0)	degext = YUV;
	else							degext = -1;

	return 1;
}

long WPSNR::CalcNumFrame(char *fname, long dimx, long dimy){

	FILE	*fp;
	long	ltemp;
	long	divider;

	fp = fopen(fname,"rb");
	fseek(fp,0,SEEK_END);
	ltemp = ftell(fp);
	fclose(fp);

	if(yuvflag==UYVY){
		divider = dimx*dimy*2;	// only consider uyvy
		ltemp /= divider;
	}
	else if(yuvflag==YUV420){
		divider = dimx*dimy*3/2;
		ltemp /= divider;
	}
	else
		return -1;

	return ltemp;
}

long WPSNR::ReadReferenceVideo(){

	long	i,ltemp;
	long	t1,t2,t3;
	readlss	lss;
	FILE	*fp;

	printf("---reference image read...000");

	if(refext==AVI){
		initialize_avi_read(refFname,&t1,&t2,&t3);
		for(i=0;i<NumFrameRef;i++){
			printf("\b\b\b%03d",i);
			read_oneframe_uyvy_avi(refFname,ReadBuffer);
			ExtractLuma(RefImageA[i]);
		}
		finalize_avi_read(refFname);
		printf("\n");
	}
	else if(refext==LSS){
		lss.initialize_lss_read(refFname);
		for(i=0;i<NumFrameRef;i++){
			printf("\b\b\b%03d",i);
			lss.read_oneframe_uyvy_lss(ReadBuffer,i);
			ExtractLuma(RefImageA[i]);			
		}
		lss.finalize_lss_read();
		printf("\n");
	}
	else if(refext==YUV){
		fp = fopen(refFname,"rb");
		if(yuvflag==UYVY){
			for(i=0;i<NumFrameRef;i++){
				printf("\b\b\b%03d",i);
				fread(ReadBuffer,sizeof(byte),FrameSize,fp);
				ExtractLuma(RefImageA[i]);
			}			
		}
		else if(yuvflag==YUV420){
			for(i=0;i<NumFrameRef;i++){
				printf("\b\b\b%03d",i);
				ltemp = ImageSize*3/2;
				fread(ReadBuffer,sizeof(byte),ltemp,fp);
				memcpy(RefImageA[i],ReadBuffer,ImageSize);				
			}
		}
		fclose(fp);
		printf("\n");
	}
	else
		return -1;
	return 1;
}

long WPSNR::ReadDegradedVideo(){

	long	i,ltemp;
	long	t1,t2,t3;
	readlss	lss;
	FILE	*fp;

	printf("---degraded image read...000");

	if(degext==AVI){
		initialize_avi_read(degFname,&t1,&t2,&t3);
		for(i=0;i<NumFrameDeg;i++){
			printf("\b\b\b%03d",i);
			read_oneframe_uyvy_avi(degFname,ReadBuffer);
			ExtractLuma(DegImageA[i]);
		}
		finalize_avi_read(degFname);
		printf("\n");
	}
	else if(degext==LSS){
		lss.initialize_lss_read(degFname);
		for(i=0;i<NumFrameDeg;i++){
			printf("\b\b\b%03d",i);
			lss.read_oneframe_uyvy_lss(ReadBuffer,i);
			ExtractLuma(DegImageA[i]);			
		}
		lss.finalize_lss_read();
		printf("\n");
	}
	else if(degext==YUV){
		fp = fopen(degFname,"rb");
		if(yuvflag==UYVY){
			for(i=0;i<NumFrameDeg;i++){
				printf("\b\b\b%03d",i);
				fread(ReadBuffer,sizeof(byte),FrameSize,fp);
				ExtractLuma(DegImageA[i]);
			}			
		}
		else if(yuvflag==YUV420){
			for(i=0;i<NumFrameDeg;i++){
				printf("\b\b\b%03d",i);
				ltemp = ImageSize*3/2;
				fread(ReadBuffer,sizeof(byte),ltemp,fp);
				memcpy(DegImageA[i],ReadBuffer,ImageSize);
			}
		}
		fclose(fp);
		printf("\n");
	}	
	else
		return -1;
	return 1;
}
