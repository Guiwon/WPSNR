#include "wpsnr.h"

long WPSNR::ProcessList(){

	FILE	*flist,*fout;
	long	st1,st2,en;

	// previous reference name setting
	refFnamePrev[0] = NULL;

	// list file open
	flist = fopen(listFname,"rt");
	if(flist==NULL){
		printf("List file \"%s\" does not exist!\n",listFname);
		return -1;
	}
	
	// output file open
	fout = fopen(outputFname,"wt");
	if(fout==NULL){
		printf("Output file \"%s\" cannot be open!\n",outputFname);
		fclose(flist);
		return -1;
	}

	// process all video
	st1 = clock();
	while(fscanf(flist,"%s",refFname)!=EOF){
		fscanf(flist,"%s",degFname);

		st2 = clock();
		ProcessVideo();			
		en = clock();
		
		printf("Final VQM of \"%s\": %.3f\n",degFname,vqm);
		printf("Processing Time (%.1f sec/Total: %.1f sec)\n\n",(en-st2)/1000.,(en-st1)/1000.);
#if 0
		fprintf(fout,"%s\t%f\t%f\n",degFname,vqm,vqm_nobnd);
#elif 1
		fprintf(fout,"%s\t%f\t%f\n",degFname,psnr,ssim);
#else
		fprintf(fout,"%s\t%f\t",degFname,vqm);
		fprintf(fout,"%f\t%f\t%f\t",wpsnr,epsnr1,epsnr2);
		fprintf(fout,"%f\t",smean);
		fprintf(fout,"%f\t",framediff);
		fprintf(fout,"%d\t%d\t",Rtfrz1,Dtfrz1);
		fprintf(fout,"%f\t",EdgeRatio);
		fprintf(fout,"%f\t%f\t%f\t",FCvariance,FCpoint,FCframe);
		fprintf(fout,"%f\t%f\t%f\t",BGratio1,BGratio2,BGmse);
		fprintf(fout,"%f\t%f\n",blocking,blk_cnt);

#endif
		
		fflush(fout);

		strcpy(refFnamePrev,refFname);
	}

	fclose(flist);
	fclose(fout);

	
	return 1;
}

long WPSNR::ProcessVideo(){

	long	pst1,pst2;
	if(InitializeVideo()<0)
		return -1;

	
	//NumFrameRef = NumFrameDeg = 11;
	ReadVideo();
	Registration();

	pst1 = clock();
	PSNR_SSIM();
	//return 1;
	
	// epsnr and wpsnr
	pst2 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst2-pst1)/1000.);


	CalcWPSNR();	// WPSNR
	pst1 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst1-pst2)/1000.);
	RunEPSNR();		// EPSNR1	
	pst2 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst2-pst1)/1000.);
	CalcEPSNR();	// EPSNR2
	pst1 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst1-pst2)/1000.);
	CalcWPSNR();	// WPSNR
	pst2 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst2-pst1)/1000.);
	// spatial features
	CalcBlocking();	// Blocking
	pst1 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst1-pst2)/1000.);
	CalcFalseColor();	// FalseColor
	pst2 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst2-pst1)/1000.);
	CalcEdgeRatio();	// EdgeRatio
	pst1 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst1-pst2)/1000.);
	// temporal features
	CalcBackGround();	// Background
	pst2 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst2-pst1)/1000.);
	CalcFreeze();		// Freeze
	pst1 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst1-pst2)/1000.);
	// source features
	if(strcmp(refFnamePrev,refFname))
		CalcRefFeature();	// RefFeature
	pst2 = clock();
	printf("=====SubProcessingTime (%.5f sec)\n",(pst2-pst1)/1000.);


	PostProcess();	

	return 1;
}

long WPSNR::ReadVideo(){

	printf("=====Video Read=====\n");

	// reference read
	if(strcmp(refFnamePrev,refFname))
		ReadReferenceVideo();
	ReadDegradedVideo();
	
	printf("\n");
	
	return 1;
}

long WPSNR::PostProcess(){

	double	dtemp1,dtemp2;


	// epsnr selection
	dtemp1 = epsnr_offset1 + epsnr1*epsnr_gain1;
	dtemp2 = epsnr_offset2 + epsnr2*epsnr_gain2;
	if(dtemp1>dtemp2)	epsnr = dtemp1;
	else				epsnr = dtemp2;

	// psnr selection
	if(epsnr<wpsnr+psnr_gap && VeryShortError==0)
		dtemp1 = epsnr;
	else
		dtemp1 = wpsnr;

	// psnr scailing
	vqm_nobnd = (dtemp1-psnr_min)*4/(psnr_max-psnr_min)+1;
	if(dtemp1>psnr_max)			dtemp1=psnr_max;
	else if(dtemp1<psnr_min)	dtemp1=psnr_min;	
	vqm = (dtemp1-psnr_min)*4/(psnr_max-psnr_min) + 1;
	

	
	// source feature (spatial mean)
	if(smean>smean_up){
		vqm += smean_up_val;
		vqm_nobnd += smean_up_val;
	}
	if(smean<smean_down){
		vqm -= smean_down_val;
		vqm_nobnd += smean_down_val;
	}

	if(NumFrameDeg<NumFrameRef+75){
	// freeze
	dtemp1 = (Dtfrz1-Rtfrz1);
	if(dtemp1>0){
		if(vqm>frzout[0])		dtemp2 = frzgain[0]*dtemp1+frzoffset[0];
		else if(vqm>frzout[1])	dtemp2 = frzgain[1]*dtemp1+frzoffset[1];
		else if(vqm>frzout[2])	dtemp2 = frzgain[2]*dtemp1+frzoffset[2];
		else					dtemp2 = 0;
		vqm -= dtemp2;
		vqm_nobnd -= dtemp2;
	}
	}
	
	// edge ratio
	if(EdgeRatio>erth1 && vqm<erth2){
		vqm+=erval;
		vqm_nobnd+=erval;
	}

	// plane percent
	if(BGratio1>plth11){
		if(BGratio2>plth21 && BGmse>pllb1 && BGmse<plub1){
			vqm -= plval1;
			vqm_nobnd -= plval1;
		}
	}
	else if(BGratio1>plth12){
		if(BGratio2>plth22 && BGmse>pllb2 && BGmse<plub2){
			vqm -= plval2;
			vqm_nobnd -= plval2;
		}
	}

	// mpeg
	if(FCvariance>0 && FCvariance<10 && FCframe>50 /*&& plane_percent1<80*/ && vqm<2.95 && vqm>2.8 /*&& tfrz_1==0*/){
		vqm += 0.9;
		vqm_nobnd += 0.9;
	}

	// frame diff and smean
	if(framediff>1000 && smean>50){
		if(vqm>srcftrth){
			vqm+=srcftrval;
			vqm_nobnd+=srcftrval;
		}
	}
	
	// blocking
	dtemp1 = 0;
	if(blk_cnt>blkcntth){
		// high output
		if(vqm>=blkoutth[0]){
			if(blocking>blkth[0][0])
				dtemp1 = blkvala[0][0];			
			else if(blocking>blkth[0][1])
				dtemp1 = blkvala[0][1];			
		}
		else if(vqm>=blkoutth[1]){
			if(blocking>blkth[1][0])
				dtemp1 = blkvala[1][0];
			else if(blocking>blkth[1][1])
				dtemp1 = blkvala[1][1];
		}
		else if(vqm>=blkoutth[2]){
			if(blocking>blkth[2][0])
				dtemp1 = blkvala[2][0];
			else if(blocking>blkth[2][1])
				dtemp1 = blkvala[2][1];				
		}
		else if(vqm>=blkoutth[3]){
			if(blocking>blkth[3][0])
				dtemp1 = blkvala[3][0];
			else if(blocking>blkth[3][1])
				dtemp1 = blkvala[3][1];			
		}
		vqm -= dtemp1;
		vqm_nobnd -= dtemp1;
	}

	// final upper and lower bound
	if(vqm<1)	vqm = 1;
	if(vqm>5)	vqm = 5;

	return 1;
}