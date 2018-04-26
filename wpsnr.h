// in file wpsnr.h
#pragma warning(disable:4996)	// version으로 인한 warning 해제
#define _CRT_SECURE_NO_WARNINGS

// includes
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "lossless.h"

typedef unsigned char byte;

#define MAXFRAME 650
#define NUMFTRFRAME	50	// for registration

long initialize_avi_read(char *infilename, long *dimx, long *dimy, long *NumFrame);
long read_oneframe_uyvy_avi(char *infilename, byte *oneframe);
long finalize_avi_read(char *infilename);

#define	AVI	0
#define	LSS 1
#define	YUV 2

#define	YUV420	0
#define	UYVY	1

class PST{
	public:
	short	x;
	short	y;
	short	val;
};

class WPSNR{

private:
	
	//////////////////////////////
	//     basic information    //
	//////////////////////////////
	long	width_in,height_in;
	long	width,height;
	long	NumFrameRef,NumFrameDeg;
	long	ImageSize,FrameSize;
	long	marginX,marginY;
	long	widthM,heightM;
	long	ImageSizeM;	
	// file names
	char	listFname[1024];
	char	outputFname[1024];
	char	refFname[512],degFname[512],refFnamePrev[512];
	long	refext,degext;

	long	yuvflag;


	//////////////////////////////
	//		tune parameters     //
	//////////////////////////////
	// wpsnr
	long	s_offset1,s_offset2;
	long	overnum1,overnum2;
	// epsnr
	double	epsnr_gain1,epsnr_offset1;
	double	epsnr_gain2,epsnr_offset2;
	// psnr selection
	double	psnr_gap;
	double	psnr_max,psnr_min;
	// smean
	double	smean_up,smean_down,smean_up_val,smean_down_val;
	// freeze
	double	frzout[3],frzgain[3],frzoffset[3];
	// edge ratio
	double	erth1,erth2,erval;
	// background
	double	plth11,plth12,plth21,plth22;
	double	pllb1,pllb2,plub1,plub2;
	double	plval1,plval2;
	// blocking
	double	blkoutth[4];
	double	blkcntth;
	double	blkth[4][2],blkvala[4][2];
	// fd, smean
	double	srcftrth,srcftrval;
	


	//////////////////////////////
	//			features		//
	//////////////////////////////
	// registration

	//---wpsnr
	long	VeryShortError;
	double	wpsnr;
	//---epsnr
	double	epsnr,epsnr1,epsnr2;
	//---reference feature
	double	smean;
	double	framediff;
	long	Rtfrz,Rmfrz,Rnfrz,Rtfrz1;
	//---blocking feature
	double	blocking,blk_cnt;
	//---false color
	double	FCvariance,FCpoint,FCframe;
	//---edge ratio
	double	EdgeRatio;
	//---background
	double	BGratio1,BGratio2,BGmse;
	//---freeze
	long	Dtfrz,Dmfrz,Dnfrz,Dtfrz1;
	// final
	double	vqm;
	double	vqm_nobnd;
	
	//////////////////////////////
	//		   variables		//
	//////////////////////////////
	// registration
	long	RegistrationIndex[MAXFRAME];
	long	shiftX,shiftY;
	long	TemporalSR,SpatialSR;	
	// wpsnr
	long	widthSub,heightSub;
	long	subSize,subSizeH;
	long	pstSub[25];
	// epsnr
	long	skip_frame;
	double	EdgeMSE;
	// false color
	double	FCvarframe;
	// edge ratio
	double	edgehigh,edgelow;
	// background
	long	BGpntnum1,BGpntnum2;
	
	
	//////////////////////////////
	// 		   memories 		//
	//////////////////////////////
	long	MemoryFlag;
	// common
	byte	*buffer,*ReadBuffer;
	byte	*RefImageA[MAXFRAME],*DegImageA[MAXFRAME];	
	//---registration	
	byte	*reg_workspace;
	//---wpsnr
	double	*SubMSE[MAXFRAME];
	//---epsnr
	byte	*Rfield1,*Rfield2;
	byte	*Dfield1,*Dfield2;
	byte	*EPSNR_workspace;
	//---refftr
	byte	*RefFreeze;
	byte	*RefWork;
	//---blocking
	short	*blkrefedge,*blkdegedge;
	byte	*blkImage1,*blkImage2;
	double	*blkval;
	long	*blkcnt;
	//---false color
	byte	*falsecolor_work;
	//---edge ratio
	byte	*ERfieldR1,*ERfieldR2;
	byte	*ERfieldD1,*ERfieldD2;
	short	*ERSobel;
	//---background
	byte	*BGMap;
	//---freeze
	byte	*DegFreeze;
	
	double	ssim;
	double	psnr;

public:

	// Initialize.cpp
	long	Initialize(long argc, char *argv[]);
	long	GetSizeInformation(char *SizeInfo);
	long	Finalize();
	long	InitializeVideo();
	long	MemoryAllocation();

	// process.cpp
	long	ProcessList();
	long	ProcessVideo();
	long	ReadVideo();
	long	PostProcess();

	// subfunctions.cpp
	long	ExtractLuma(byte *output);
	double	MIN3(double m1, double m2, double m3);
	byte	IsSame(byte *frame1, byte *frame2);
	long	FindExtension();
	long	CalcNumFrame(char *fname, long dimx, long dimy);
	long	ReadReferenceVideo();	
	long	ReadDegradedVideo();	

	// registration.cpp
	long	Registration();
	long	FindSpatialShift();
	long	FindTemporalShift();
	double	CalcVideoMSE();
	double	CalcFrameMSE(byte *frame1, byte *frame2);
	long	calibration();
	long	LocationCheck(long location);

	// wpsnr.cpp
	long	CalcWPSNR();
	long	CalcWPSNRFrame(byte *Ref, byte *Deg, double *FrameMSE);
	long	SelectWPSNR();

	// epsnr.cpp
	long	CalcEPSNR();
	long	CalcEPSNRFrame(byte *Ref, byte *Deg);
	double	CalcEPSNRImage(byte *Ref, byte *Deg, long NumPST, double &SobelMSE, long interlaced);
	long	DivideField(byte *frame, byte *field1, byte *field2);
	long	Sobel(byte *Image, short *SobelImage, long interlaced);
	byte	FilteringPixel(byte *pst);
	long	RunEPSNR();

	// refftr.cpp
	long	CalcRefFeature();
	double	CalcSpatialInformation(byte *image);
	double	CalcFrameDifference(byte *frame1, byte *frame2);
	long	FreezeInfoRef(byte *freeze);

	// spatialftr.cpp
	long	CalcBlocking();
	long	CalcBlockingFrame(byte *ref, byte *deg, double &blkframe, long &blkcount);
	long	Gaussian(byte *InImage, byte *OutImage);
	long	IsNearNonZero(byte *point, long radius, long x, long y);
	long	BLKCalcFTR();

	long	CalcFalseColor();
	long	CalcFalseColorFrame(byte *ref, byte *deg);
	double	CalculateDensity(byte *Point, long Radius);
	byte	FindCircle(byte *point, long radius, long x, long y);

	long	CalcEdgeRatio();
	long	CalcEdgeRatioFrame(byte *ref, byte *deg);
	double	CalcEdgeRatioImage(byte *ref, byte *deg, double &high, double &low, long interlaced);


	// temporalftr.cpp
	long	CalcBackGround();
	long	CalcBGratio1();
	long	CalcBackgroundFrame(byte *curr, byte *prev, byte *currR, byte *prevR, _int64 &sse);
	long	IsNearbyZero(byte *map_pst, long radius);

	long	CalcFreeze();
	long	FreezeInfoDeg(byte *freeze);
	

	// PSNR.cpp
	long	PSNR_SSIM();
	double	CalcPSNRFrame(byte *ref, byte *deg);
	double	CalcSSIMFrame(byte *ref, byte *deg);

	



};