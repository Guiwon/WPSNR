#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include"read_avi.h"

#define UYVY601 2
#define RGB24	3
#define RGB16	2


long initialize_avi_read(char *infilename, long *dimx, long *dimy, long *NumFrame);
long read_oneframe_uyvy_avi(char *infilename, BYTE *oneframe);
long finalize_avi_read(char *infilename);

/**************************************************
 ***
 *** FUNCTION: initialize_avi_read, 
 *** DESCRIPTION:
 ***
 *** HDSP Lab. Yonsei University, Apr. 08
 **************************************************/

AVIMAINHEADER avih;
AVISTREAMHEADER avisV;
BITMAPINFO binfo;
int instream;
long cumulative_cnt;
__int64 cumulative_size;
DWORD videosize;

typedef struct _avi_extension{
	long current_riff;
	long number_of_riff;
	long riff_filesize[100];
} AVI_EXTENSION;

AVI_EXTENSION avi_ext;

long debug_flag = -1;
FILE *fpdebug = NULL;
FILE *fphdrl = NULL;
FILE *fptxt = NULL;
DWORD junk_dword = 0xffffffff;
DWORD for_i;
DWORD PrevchunkSize;

long initialize_avi_read(char *infilename, long *dimx, long *dimy, long *NumFrame){
	
	avi_ext.current_riff = 0;
	avi_ext.number_of_riff = 0;
	avi_ext.riff_filesize[0] = 0;
	
  	if((instream = _open(infilename,_O_RDONLY|_O_BINARY))<0){	
		printf("Error!! Can't open input file...\n");
		return -1;
	}

	if(debug_flag>0)
		fpdebug = fopen("debug.bin", "wb");
	if(debug_flag>0)
		fphdrl = fopen("hdrl.bin", "wb");
	if(debug_flag>0)
		fptxt = fopen("debug.txt", "wt");
	
	
	char RIFF[5], fileType[5];
	RIFF[4]='\0', fileType[4]='\0';
	DWORD fileSize;
	
	//	listSize listType	
	char LIST[5], listType[5];
	listType[4]='\0', LIST[4]='\0';
	DWORD listSize;

	//chunkSize chunkType	
	char chunkID[5], chunkType[5];
	chunkID[4]=chunkType[4]='\0';
	DWORD chunkSize;
	
	DWORD imgsize;

	// RIFF header
	_read(instream, RIFF, 4);					//RIFF
	_read(instream, &fileSize, sizeof(DWORD)*1);//filesize
	_read(instream, fileType, 4);				//filetype==avi
	avi_ext.riff_filesize[ avi_ext.number_of_riff ] = fileSize;
	avi_ext.number_of_riff += 1;

	
	cumulative_size = fileSize + 8;
	__int64 return_value;
	for(;;){
		return_value = _lseeki64(instream, fileSize -4, SEEK_CUR);
	
		if(return_value<0 || return_value > cumulative_size)
			break;
		_read(instream, RIFF, 4);					//RIFF
		return_value = _read(instream, &fileSize, sizeof(DWORD)*1);//filesize
		if(return_value <1)
			//fileSize = 0;
			break;
		_read(instream, fileType, 4);				//filetype==avi		
		avi_ext.riff_filesize[ avi_ext.number_of_riff ] = fileSize;
		avi_ext.number_of_riff += 1;
		
		cumulative_size += fileSize + 8;
	}
	
	_lseeki64(instream, 0, SEEK_SET);

	// RIFF header
	_read(instream, RIFF, 4);					//RIFF
	if(debug_flag>0)	fwrite(RIFF, 4, 1, fpdebug);
	if(debug_flag>0)	fprintf(fptxt, "+%s[%dBytes]\n", RIFF, 4);
	_read(instream, &fileSize, sizeof(DWORD)*1);//filesize
	if(debug_flag>0)	fwrite(&fileSize, sizeof(DWORD), 1, fpdebug);
	if(debug_flag>0)	fprintf(fptxt, "+%d[%dBytes]\n", fileSize, sizeof(DWORD));
	_read(instream, fileType, 4);				//filetype==avi
	if(debug_flag>0)	fwrite(fileType, sizeof(DWORD), 1, fpdebug);
	if(debug_flag>0)	fprintf(fptxt, "+%s[%dBytes]\n", fileType, sizeof(DWORD));


	cumulative_cnt = 0;

	while(!!strcmp(LIST,"idx1")){

		_read(instream, LIST, 4);						//LIST
		if(debug_flag>0)	fwrite(LIST, 4, 1, fpdebug);
		if(debug_flag>0)	fwrite(LIST, 4, 1, fphdrl);
		if(debug_flag>0)	fprintf(fptxt, "\t+%s[%dBytes]\n", LIST, 4);
		_read(instream, &listSize, sizeof(DWORD)*1);	//size
		if(debug_flag>0)	fwrite(&listSize, sizeof(DWORD), 1, fpdebug);
		if(debug_flag>0)	fwrite(&listSize, sizeof(DWORD), 1, fphdrl);
		if(debug_flag>0)	fprintf(fptxt, "\t+%d[%dBytes]\n", listSize, sizeof(DWORD));		

		if(strcmp(LIST,"JUNK")==0){
			if(debug_flag>0){
				for(for_i=0; for_i<listSize/4; for_i++){
					_read(instream, &junk_dword, sizeof(DWORD)*1);
					fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
				}
			}
			else
				_lseeki64(instream, (listSize), SEEK_CUR);
			
			continue;
		}
		else if(strcmp(LIST,"idx1")==0){
			break;
		}
		else {
			_read(instream, listType, 4);				//type==avih
			if(debug_flag>0)	fwrite(listType, 4, 1, fpdebug);
			if(debug_flag>0)	fprintf(fptxt, "\t+%s[%dBytes]\n", listType, 4);
		}
		
		if(strcmp(listType, "hdrl")==0) {	// fphdrl
			// avih chunk;		
			_read(instream, &avih, sizeof(AVIMAINHEADER));	// avih(64)
			if(debug_flag>0)	fwrite(&avih, sizeof(AVIMAINHEADER), 1, fpdebug);
			if(debug_flag>0)	fwrite(&avih, sizeof(AVIMAINHEADER), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", "avih", sizeof(AVIMAINHEADER));
			_read(instream, chunkID, 4);					// LIST
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkID, 4);
			_read(instream, &chunkSize, sizeof(DWORD)*1);	// 32372
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fpdebug);
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(DWORD));
			
			//read video stream header
			_read(instream, chunkType, 4);					// strl
			if(debug_flag>0)	fwrite(chunkType, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkType, 4, 1, fpdebug);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkType, 4);
			_read(instream, &avisV, sizeof(AVISTREAMHEADER)); // strh (64)
			if(debug_flag>0)	fwrite(&avisV, sizeof(AVISTREAMHEADER), 1, fpdebug);
			if(debug_flag>0)	fwrite(&avisV, sizeof(AVISTREAMHEADER), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", "strh", sizeof(AVISTREAMHEADER));
			_read(instream, chunkID, 4);					// strf
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkID, 4);
			_read(instream, &chunkSize, sizeof(DWORD));		
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fpdebug);
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(DWORD));
			//bitmap info header
			_read(instream, &binfo, sizeof(BITMAPINFO));
			if(debug_flag>0)	fwrite(&binfo, sizeof(BITMAPINFO), 1, fpdebug);
			if(debug_flag>0)	fwrite(&binfo, sizeof(BITMAPINFO), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", "bmph", sizeof(BITMAPINFO));
			//junk buffer
			imgsize = avih.dwHeight * avih.dwWidth;
			videosize = imgsize * 2 * avih.dwTotalFrames;

			*dimx = avih.dwWidth;
			*dimy = avih.dwHeight;
			*NumFrame = avih.dwTotalFrames;
			
			_read(instream, chunkID, 4);
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkID, 4);

			_read(instream, &chunkSize, sizeof(DWORD));
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fpdebug);
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(DWORD));
			if(chunkSize>videosize){
				_lseeki64(instream, -8, SEEK_CUR);
			}
			else{
				if(debug_flag>0){
					for(for_i=0; for_i<chunkSize/4; for_i++){
						_read(instream, &junk_dword, sizeof(DWORD)*1);
						fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
						fwrite(&junk_dword, sizeof(DWORD), 1, fphdrl);
					}
				}
				else
					_lseeki64(instream, chunkSize, SEEK_CUR);	
			}
		}
		else if(strcmp(listType,"strl")==0){				//read audio stream header
				if(debug_flag>0){
					for(for_i=0; for_i<(listSize-4)/4; for_i++){
						_read(instream, &junk_dword, sizeof(DWORD)*1);
						fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
						fwrite(&junk_dword, sizeof(DWORD), 1, fphdrl);
					}
				}
				else
					_lseeki64(instream, (listSize-4), SEEK_CUR);	//skip audio header 
		}
		else if(strcmp(listType,"odml")==0){				//OpenDML header??
			if(debug_flag>0){
				for(for_i=0; for_i<(listSize-4)/4; for_i++){
					_read(instream, &junk_dword, sizeof(DWORD)*1);
					fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
				}
			}
			else
				_lseeki64(instream, (listSize-4), SEEK_CUR);		//skip 
		}
		else if(strcmp(listType, "movi")==0) {
			cumulative_size = 0;

			return 1;

		}	// listType == movi
		else {
			printf("Invalid File format.....\n");
			return -1;
		}
	}	//while LIST

	return -1;
}

long jump_to_movi(){
	
	char RIFF[5], fileType[5];
	RIFF[4]='\0', fileType[4]='\0';
	DWORD fileSize;
	
	//	listSize listType	
	char LIST[5], listType[5];
	listType[4]='\0', LIST[4]='\0';
	DWORD listSize;

	//chunkSize chunkType	
	char chunkID[5], chunkType[5];
	chunkID[4]=chunkType[4]='\0';
	DWORD chunkSize;
	
	__int64 return_value;
	long i;
	_lseeki64(instream, 0, SEEK_SET);
	for(i=0; i< avi_ext.current_riff + 1; i++){
		// RIFF header
		_read(instream, RIFF, 4);					//RIFF
		_read(instream, &fileSize, sizeof(DWORD)*1);//filesize
		_read(instream, fileType, 4);				//filetype==avi
		return_value = _lseeki64(instream, avi_ext.riff_filesize[ i ] -4, SEEK_CUR);
		if(return_value<0)
			return -1;
	}	

	avi_ext.current_riff += 1;

	if(debug_flag>0){
		junk_dword = 0x8fffffff;
		for(for_i=0; for_i<10; for_i++){
			fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
		}
	}
	
	// RIFF header
	_read(instream, RIFF, 4);					//RIFF
	if(debug_flag>0)	fwrite(RIFF, 4, 1, fpdebug);
	if(debug_flag>0)	fprintf(fptxt, "+%s[%dBytes]\n", RIFF, 4);
	_read(instream, &fileSize, sizeof(DWORD)*1);//filesize
	if(debug_flag>0)	fwrite(&fileSize, sizeof(DWORD), 1, fpdebug);
	if(debug_flag>0)	fprintf(fptxt, "+%d[%dBytes]\n", fileSize, sizeof(DWORD));
	_read(instream, fileType, 4);				//filetype==avi
	if(debug_flag>0)	fwrite(fileType, sizeof(DWORD), 1, fpdebug);
	if(debug_flag>0)	fprintf(fptxt, "+%s[%dBytes]\n", fileType, sizeof(DWORD));

	cumulative_cnt = 0;

	while(!!strcmp(LIST,"idx1")){

		_read(instream, LIST, 4);						//LIST
		if(debug_flag>0)	fwrite(LIST, 4, 1, fpdebug);
		if(debug_flag>0)	fprintf(fptxt, "\t+%s[%dBytes]\n", LIST, 4);
		_read(instream, &listSize, sizeof(DWORD)*1);	//size
		if(debug_flag>0)	fwrite(&listSize, sizeof(DWORD), 1, fpdebug);
		if(debug_flag>0)	fprintf(fptxt, "\t+%d[%dBytes]\n", listSize, sizeof(DWORD));

		if(strcmp(LIST,"JUNK")==0){
			if(debug_flag>0){
				for(for_i=0; for_i<listSize/4; for_i++){
					_read(instream, &junk_dword, sizeof(DWORD)*1);
					fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
				}
			}
			else
				_lseeki64(instream, (listSize), SEEK_CUR);
			
			continue;
		}
		else if(strcmp(LIST,"idx1")==0){
			break;
		}
		else {
			_read(instream, listType, 4);				//type==avih
			if(debug_flag>0)	fwrite(listType, 4, 1, fpdebug);
			if(debug_flag>0)	fprintf(fptxt, "\t+%s[%dBytes]\n", listType, 4);
		}
		
		if(strcmp(listType, "hdrl")==0) {
			// avih chunk;		
			_read(instream, &avih, sizeof(AVIMAINHEADER));	// avih(64)
			if(debug_flag>0)	fwrite(&avih, sizeof(AVIMAINHEADER), 1, fpdebug);
			if(debug_flag>0)	fwrite(&avih, sizeof(AVIMAINHEADER), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", "avih", sizeof(AVIMAINHEADER));
			_read(instream, chunkID, 4);					// LIST
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkID, 4);
			_read(instream, &chunkSize, sizeof(DWORD)*1);	// 32372
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fpdebug);
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(DWORD));
			
			//read video stream header
			_read(instream, chunkType, 4);					// strl
			if(debug_flag>0)	fwrite(chunkType, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkType, 4, 1, fpdebug);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkType, 4);
			_read(instream, &avisV, sizeof(AVISTREAMHEADER)); // strh (64)
			if(debug_flag>0)	fwrite(&avisV, sizeof(AVISTREAMHEADER), 1, fpdebug);
			if(debug_flag>0)	fwrite(&avisV, sizeof(AVISTREAMHEADER), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", "strh", sizeof(AVISTREAMHEADER));
			_read(instream, chunkID, 4);					// strf
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fpdebug);
			if(debug_flag>0)	fwrite(chunkID, 4, 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkID, 4);
			_read(instream, &chunkSize, sizeof(DWORD));		
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fpdebug);
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(DWORD));
			//bitmap info header
			_read(instream, &binfo, sizeof(BITMAPINFO));
			if(debug_flag>0)	fwrite(&binfo, sizeof(BITMAPINFO), 1, fpdebug);
			if(debug_flag>0)	fwrite(&binfo, sizeof(BITMAPINFO), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", "bmph", sizeof(BITMAPINFO));
			//junk buffer
			
			_read(instream, &chunkSize, sizeof(DWORD));
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fpdebug);
			if(debug_flag>0)	fwrite(&chunkSize, sizeof(DWORD), 1, fphdrl);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(DWORD));
			if(chunkSize>videosize){
				_lseeki64(instream, -8, SEEK_CUR);
			}
			else{
				if(debug_flag>0){
					for(for_i=0; for_i<chunkSize/4; for_i++){
						_read(instream, &junk_dword, sizeof(DWORD)*1);
						fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
						fwrite(&junk_dword, sizeof(DWORD), 1, fphdrl);
					}
				}
				else
					_lseeki64(instream, chunkSize, SEEK_CUR);	
			}
		}
		else if(strcmp(listType,"strl")==0){				//read audio stream header
				if(debug_flag>0){
					for(for_i=0; for_i<(listSize-4)/4; for_i++){
						_read(instream, &junk_dword, sizeof(DWORD)*1);
						fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
						fwrite(&junk_dword, sizeof(DWORD), 1, fphdrl);
					}
				}
				else
					_lseeki64(instream, (listSize-4), SEEK_CUR);	//skip audio header 
		}
		else if(strcmp(listType,"odml")==0){				//OpenDML header??
			if(debug_flag>0){
				for(for_i=0; for_i<(listSize-4)/4; for_i++){
					_read(instream, &junk_dword, sizeof(DWORD)*1);
					fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
				}
			}
			else
				_lseeki64(instream, (listSize-4), SEEK_CUR);		//skip 
		}
		else if(strcmp(listType, "movi")==0) {
			cumulative_size = 0;
			return 1;

		}	// listType == movi
		else {
			printf("Invalid File format.....\n");
			return -1;
		}
	}	//while LIST

	return -1;
}

long read_oneframe_uyvy_avi(char *infilename, BYTE *oneframe){
	
	BYTE *data = oneframe;
	char chunkID[5], chunkType[5];
	chunkID[4]=chunkType[4]='\0';
	DWORD chunkSize;
	__int64 return_value;

		
		//image variables setting
		long frame_cnt=0;
		long total_frame=avih.dwTotalFrames;
		long image_size=binfo.bmiHeader.biSizeImage; //buffered image size for one chunk
		long total_image_size=image_size*total_frame;			//
		unsigned long Compression=binfo.bmiHeader.biCompression;
		long BitCount=binfo.bmiHeader.biBitCount;

		
		while(frame_cnt<1 && cumulative_cnt<total_frame){
		
			return_value = _read(instream, chunkType, 4);				//read chunkType
			if(debug_flag>0)	fwrite(chunkType,  4, 1, fpdebug);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%s[%dBytes]\n", chunkType, 4);
			if(return_value<0)	break;
			cumulative_size += return_value;
			if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
				if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
					jump_to_movi();
					continue;
				}
				else break;
			}			
			return_value = _read(instream, &chunkSize, sizeof(long));	//read chunkSize
			if(debug_flag>0)	fwrite(&chunkSize,  sizeof(long), 1, fpdebug);
			if(debug_flag>0)	fprintf(fptxt, "\t\t+%d[%dBytes]\n", chunkSize, sizeof(long));
			if(return_value<0)	break;
			cumulative_size += return_value;
			if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
				if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
					jump_to_movi();
					continue;
				}
				else break;
			}

			if(strcmp(chunkType, "JUNK")==0){

					if(chunkSize> videosize )
						_lseeki64(instream, -8, SEEK_CUR);
					else{
						if(debug_flag>0){
							for(for_i=0; for_i<chunkSize/4; for_i++){
								_read(instream, &junk_dword, sizeof(DWORD));
								fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
							}
							return_value = chunkSize;
						}
						else
							return_value = _lseeki64(instream, chunkSize, SEEK_CUR);
					}
				//return_value = (long)_lseeki64(instream, (chunkSize), SEEK_CUR);
					
				if(return_value<0)	break;
				cumulative_size += chunkSize;
				if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
					if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
						jump_to_movi();
						continue;
					}
					else break;
				}
			}
			else if(strcmp(chunkType, "00dc")==0) {			//video data [00dc == compressed]
				return_value = _read(instream, data, chunkSize);		//read video frame
				if(return_value<0)	break;
				PrevchunkSize = chunkSize;
				cumulative_size += return_value;
				if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
					if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
						jump_to_movi();
						continue;
					}
					else break;
				}
				frame_cnt++;
				cumulative_cnt++;
			}
			else if(strcmp(chunkType, "01wb")==0) {		//audio data
				return_value = _lseeki64(instream, chunkSize, SEEK_CUR);	//skip audio frame
				if(return_value<0)	break;
				cumulative_size += chunkSize;
				if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
					if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
						jump_to_movi();
						continue;
					}
					else break;
				}
			}
			else if(strcmp(chunkType, "00db")==0) {		//video data [00db == uncompressed]
				return_value = _read(instream, data, chunkSize);		//read video frame
				if(return_value<0)	break;
				PrevchunkSize = chunkSize;
				cumulative_size += return_value;
				if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
					if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
						jump_to_movi();
						continue;
					}
					else break;
				}
				frame_cnt++;
				cumulative_cnt++;
			}
			else{
				_read(instream, data, PrevchunkSize);
				frame_cnt++;
				cumulative_cnt++;
			/*	if(debug_flag>0){
					//FILE *fpdebug2 = fopen("debug2.bin", "ab");

					for(for_i=0; for_i<chunkSize/4; for_i++){
						_read(instream, &junk_dword, sizeof(DWORD));
					//	fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug2);
						fwrite(&junk_dword, sizeof(DWORD), 1, fpdebug);
					}
					return_value = chunkSize;
					//fclose(fpdebug2);
				}
				else
					return_value = _lseeki64(instream, chunkSize, SEEK_CUR);
				
				//return_value = _lseeki64(instream, (chunkSize), SEEK_CUR);
				if(return_value<0)	break;
				cumulative_size += chunkSize;
				if(cumulative_size > avi_ext.riff_filesize[ avi_ext.current_riff ]){
					if(avi_ext.current_riff + 1 < avi_ext.number_of_riff){
						jump_to_movi();
						continue;
					}
					else break;
				}
				*/
			}
		}	//while frame_cnt

		if(frame_cnt<1)
			return -1;
		else
			return 1;

	return -1;
}


long finalize_avi_read(char *infilename){
	
	_close(instream);
	if(debug_flag>0)
		fclose(fpdebug);

	return 1;
}

static const char avi_headers[][8] = {
    { 'R', 'I', 'F', 'F',    'A', 'V', 'I', ' ' },
    { 'R', 'I', 'F', 'F',    'A', 'V', 'I', 'X' },
    { 'R', 'I', 'F', 'F',    'A', 'V', 'I', 0x19},
    { 'O', 'N', '2', ' ',    'O', 'N', '2', 'f' },
    { 'R', 'I', 'F', 'F',    'A', 'M', 'V', ' ' },
    { 0 }
};

long get_riff(char *RIFF, char *fileType, DWORD *fileSize, int instream)
{
	char header[8];
	long i;
	
	_read(instream, header, 4);
	_read(instream, &fileSize, sizeof(DWORD)*1);
	_read(instream, header+4, 4);
	
	for(i=0; i<4; i++){
		*(RIFF+i) = *(header+i);
		*(fileType+i) = *(header+4+i);
	}

	for(i=0; avi_headers[i][0]; i++){
		if(!memcmp(header, avi_headers[i], 8))
			break;
	}

	if(!avi_headers[i][0])
		return -1;

	if(header[7] == 0x19)
		printf("avi file has been generated with a totally broken muxer\n");

	return 0;
}

