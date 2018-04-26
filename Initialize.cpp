#include "wpsnr.h"

long WPSNR::Initialize(long argc, char *argv[]){

	long	i;
	
	// yuv flag setting
	yuvflag = YUV420;	

	// Initialize
	MemoryFlag = 0;
	strcpy(listFname,"list.txt");
	strcpy(outputFname,"output.txt");
	TemporalSR = 60;
	SpatialSR = 0;

	width_in = 1920;
	height_in = 1080;

	// tune parameter
	s_offset1 = 6, overnum1 = 5;
	s_offset2 = 6, overnum2 = 13;
	epsnr_offset1 = -7.48, epsnr_gain1 = 0.986;
	epsnr_offset2 = -6.75, epsnr_gain2 = 0.944;
	// psnr selection
	psnr_gap = 0.27;
	psnr_max = 32.54, psnr_min = 13.84;
	// smean
	smean_up = 40.5, smean_up_val = 0.143;
	smean_down = 6.5, smean_down_val = 0.113;
	// freeze	
	frzout[0]=4.17,		frzout[1]=3.74,		frzout[2]=1.8;
	frzgain[0]=0.01888,	frzgain[1]=0.01959,	frzgain[2]=0.01343;
	frzoffset[0]=1.09140,	frzoffset[1]=0.2413,	frzoffset[2]=0.2069;
	// edge ratio
	erth1 = 1.6, erth2 = 3.79, erval = 0.51;
	// background
	plth11=93,	plth12=88,	plth21=0.05,	plth22=0.09;
	pllb1=45,	pllb2=44,	plub1=200,		plub2=400;
	plval1=1.72,	plval2=1.04;
	// fd & smean
	srcftrth = 2.89, srcftrval = 0.69;
	// blocking
	blkcntth=79000;
	blkoutth[0]=4.64,	blkoutth[1]=4.3,	blkoutth[2]=3.1,	blkoutth[3]=1.96;
	blkth[0][0]=3.92,	blkth[1][0]=6.43,	blkth[2][0]=9.26,	blkth[3][0]=7.0;
	blkth[0][1]=3.66;	blkth[1][1]=3.95,	blkth[2][1]=4.03,	blkth[3][1]=5.4;
	blkvala[0][0]=0.54,	blkvala[1][0]=0.93,	blkvala[2][0]=0.5,	blkvala[3][0]=0.28;
	blkvala[0][1]=0.25;	blkvala[1][1]=0.27;	blkvala[2][1]=0.11,	blkvala[3][1]=0.25;	

	for(i=1;i<argc;i++){
		strlwr(argv[i]);	// make lower case

		if(argv[i][0]=='-'){
			
			// list file name setting
			if(strcmp(argv[i]+1,"i")==0){
				if(i==argc-1)	return -1;
				strcpy(listFname,argv[++i]);
			}
			
			// output file name setting
			if(strcmp(argv[i]+1,"o")==0){
				if(i==argc-1)	return -1;
				strcpy(outputFname,argv[++i]);
			}

			if(strcmp(argv[i]+1,"s")==0){
				if(i==argc-1)	return -1;
				if(GetSizeInformation(argv[++i])<0)
					return -1;				
			}

		}	// if
	}	// for
	
	return 1;
}


long WPSNR::GetSizeInformation(char *SizeInfo){
	
	byte	i,length;

	strlwr(SizeInfo);
	length = strlen(SizeInfo);
	// Size information WidthxHeight form
	width_in = atoi(SizeInfo);

	for(i=0;i<length;){
		if(SizeInfo[i++]=='x')
			break;
	}
	
	if(i==length)
		return -1;

	height_in = atoi(SizeInfo+i);

	return 1;
}

long WPSNR::Finalize(){

	free(buffer);

	return 1;
}


long WPSNR::InitializeVideo(){

	long	i,ltemp;
	long	width1,height1,width2,height2;
	readlss	lss;

	FindExtension();


	// information check (reference)
	// - reference
	if(refext==AVI){
		initialize_avi_read(refFname,&width1,&height1,&NumFrameRef);
		finalize_avi_read(refFname);
	}
	else if(refext==LSS){
		ltemp = lss.lss_info(refFname);
		if(ltemp<0){
			printf("Video file \"%s\" does not exist!\n",refFname);
			return -1;
		}
		width1 = lss.WhatIsWidth();
		height1 = lss.WhatIsHeight();
		NumFrameRef = lss.WhatIsNumFrame();
	}
	else if(refext==YUV){
		width1 = width_in;
		height1 = height_in;
		NumFrameRef = CalcNumFrame(refFname,width1,height1);
	}
	else
		return -1;
	// information check (degraded)
	// - degraded
	if(degext==AVI){
		initialize_avi_read(degFname,&width2,&height2,&NumFrameDeg);
		finalize_avi_read(degFname);
	}
	else if(degext==LSS){
		ltemp = lss.lss_info(degFname);
		if(ltemp<0){
			printf("Video file \"%s\" does not exist!\n",degFname);
			return -1;
		}
		width2 = lss.WhatIsWidth();
		height2 = lss.WhatIsHeight();
		NumFrameDeg = lss.WhatIsNumFrame();
	}
	else if(degext==YUV){
		width2 = width_in;
		height2 = height_in;
		NumFrameDeg = CalcNumFrame(degFname,width2,height2);
	}
	else
		return -1;

	// information check (LSS) - end

	
	// common
	if((width1!=width2)||(height1!=height2)){
		printf("Information does not match between reference and degraded!\n");
		return -1;
	}
	
	width = width1, height = height1;

	// margin initialize
	if(width<=852)
		marginX = marginY = 12;
	else
		marginX = 32, marginY = 24;

	
	// basic infomation calculation
	ImageSize = width*height;
	FrameSize = ImageSize<<1;		// (uyvy)

	widthM = width-marginX;
	heightM = height-marginY;
	ImageSizeM = (widthM-marginX)*(heightM-marginY);

	if(MemoryAllocation()<0){
		printf("Memory allocation fail!\n");
		return -1;
	}

	// spatial-temporal information intialize
	shiftX = shiftY = 0;
	for(i=0;i<NumFrameDeg;i++){
		if(i>=NumFrameRef)
			RegistrationIndex[i] = NumFrameRef-1;
		else
			RegistrationIndex[i] = i;
	}

	return 1;
}

#define ModuleNum 2

long WPSNR::MemoryAllocation(){

	long	i,BufferSize;
	long	MaxSize,BufferSizeModule;
	byte	*common_end;

	// initialize	
	MaxSize = 0;

	//////////////////////////////////////////////////////////////////////
	//						Memory size setting							//
	//////////////////////////////////////////////////////////////////////		
	// common size
	BufferSize = FrameSize;					// readbuffer	
	BufferSize += ImageSize*NumFrameRef;	// reference
	BufferSize += ImageSize*NumFrameDeg;	// degraded

	//---registration
	BufferSizeModule = 0;
	BufferSizeModule += ((NUMFTRFRAME*NumFrameDeg)<<1);
	BufferSizeModule += (ImageSize<<2);
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---wpsnr
	BufferSizeModule = 0;
	BufferSizeModule += ((NumFrameDeg*25)<<3);
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---epsnr
	BufferSizeModule = 0;
	BufferSizeModule += (ImageSize<<1);
	BufferSizeModule += (ImageSize<<2);
	BufferSizeModule += sizeof(PST)*500;
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---refftr
	BufferSizeModule = 0;
	BufferSizeModule += NumFrameRef;
	BufferSizeModule += (ImageSize<<3);
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---blocking
	BufferSizeModule = 0;
	BufferSizeModule += (ImageSize<<1);	
	BufferSizeModule += (ImageSize<<2);	
	BufferSizeModule += (NumFrameDeg<<2);
	BufferSizeModule += (NumFrameDeg<<3);
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---false color
	BufferSizeModule = 0;
	BufferSizeModule += (ImageSize*6);	
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---edge ratio
	BufferSizeModule = 0;
	BufferSizeModule += (ImageSize<<2);	
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---background
	BufferSizeModule = 0;
	BufferSizeModule += ImageSize;	
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;

	//---freeze
	BufferSizeModule = 0;
	BufferSizeModule += NumFrameDeg;	
	if(MaxSize<BufferSizeModule)
		MaxSize = BufferSizeModule;
	
	BufferSize += MaxSize;
	
	//////////////////////////////////////////////////////////////////////
	//						Memory allocation							//
	//////////////////////////////////////////////////////////////////////	
	if(MemoryFlag==0){
		buffer = (byte*)malloc(sizeof(byte)*BufferSize);
		MemoryFlag = BufferSize;
	}
	else if(MemoryFlag<BufferSize){
		free(buffer);
		buffer = (byte*)malloc(sizeof(byte)*BufferSize);
		MemoryFlag = BufferSize;
	}

	//////////////////////////////////////////////////////////////////////
	//							Memory matching							//
	//////////////////////////////////////////////////////////////////////
	// common
	ReadBuffer = buffer;								// FrameSize
	RefImageA[0] = ReadBuffer + FrameSize;				// ImageSize
	for(i=1;i<NumFrameRef;i++)
		RefImageA[i] = RefImageA[i-1] + ImageSize;		// ImageSize
	DegImageA[0] = RefImageA[NumFrameRef-1] + ImageSize;// ImageSize
	for(i=1;i<NumFrameDeg;i++)
		DegImageA[i] = DegImageA[i-1] + ImageSize;		// ImageSize
	common_end = DegImageA[NumFrameDeg-1] + ImageSize;

	// registration	
	reg_workspace = common_end;

	// wpsnr	
	SubMSE[0] = (double *)(common_end);	// 25
	for(i=1;i<NumFrameDeg;i++)
		SubMSE[i] = SubMSE[i-1] + 25;	// 25

	// epsnr
	Rfield1 = common_end;
	Rfield2 = Rfield1 + (ImageSize>>1);
	Dfield1 = Rfield2 + (ImageSize>>1);
	Dfield2 = Dfield1 + (ImageSize>>1);
	EPSNR_workspace = Dfield2 + (ImageSize>>1);

	// refftr
	RefFreeze = common_end;
	RefWork = RefFreeze + NumFrameRef;

	// blocking
	blkImage1 = common_end;
	blkImage2 = blkImage1 + ImageSize;
	blkrefedge = (short *)(blkImage2+ImageSize);
	blkdegedge = blkrefedge + ImageSize;
	blkval = (double *)(blkdegedge + ImageSize);
	blkcnt = (long *)(blkval + NumFrameDeg);

	// false color
	falsecolor_work = common_end;

	// edge ratio
	ERfieldR1 = common_end;
	ERfieldR2 = ERfieldR1 + (ImageSize>>1);
	ERfieldD1 = ERfieldR2 + (ImageSize>>1);
	ERfieldD2 = ERfieldD1 + (ImageSize>>1);
	ERSobel = (short *)(ERfieldD2 + (ImageSize>>1));

	// background
	BGMap = common_end;

	// freeze
	DegFreeze = common_end;
	
	return 1;
}