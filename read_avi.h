#pragma warning(disable:4996)	// version으로 인한 warning 해제
// For BMP file
typedef struct tagBITMAPFILEHEADER {
	unsigned short bfType;		// BM 값 저장
	unsigned long bfSize;		// 전체파일 크기(byte단위)
	unsigned short bfReserved1;		// 예약된 변수
	unsigned short bfReserved2;		// 예약된 변수
	unsigned long bfoffBits;		// 영상데이터 위치까지의 거리 
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	unsigned long biSize;			// 구조체의 크기
	long biWidth;					// 영상의 폭(픽셀단위)
	long biHeight;					// 영상의 높이
	unsigned short biPlanes;		// 비트 플레인 수
	unsigned short biBitCount;		// 픽셀당 비트수
	unsigned long biCompression;	// 압축 유무
	unsigned long biSizeImage;		// 영상의 크기(byte 단위)
	long biXPelsPerMeter;			// 가로 해상도
	long biYpelsPerMeter;			// 세로 해상도
	unsigned long biClrUsed;		// 실제 사용 색상수
	unsigned long biClrImportant;	// 중요한 색상인덱스
} BITMAPINFOHEADER;


typedef struct tagRGBQUAD {
	unsigned char rgbBlue;			// B성분
	unsigned char rgbGreen;			// G성분
	unsigned char rgbRead;			// R성분
	unsigned char rgbReserved1;		// 예약된 변수
} RGBQUAD;


typedef struct tagBITMAPINFO { 
  BITMAPINFOHEADER bmiHeader; 
 // RGBQUAD          bmiColors[1]; 
} BITMAPINFO; 


typedef unsigned long DWORD;
typedef DWORD FOURCC;
typedef unsigned char BYTE;
typedef unsigned short WORD;


typedef struct _avimainheader {
    char fcc[4];
    DWORD  cb;
    DWORD  dwMicroSecPerFrame;
    DWORD  dwMaxBytesPerSec;
    DWORD  dwPaddingGranularity;
    DWORD  dwFlags;
    DWORD  dwTotalFrames;
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwReserved[4];
} AVIMAINHEADER;

typedef struct _avistreamheader {
     char fcc[4];
     DWORD  cb;
     char fccType[4];
     char fccHandler[4];
     DWORD  dwFlags;
     WORD   wPriority;
     WORD   wLanguage;
     DWORD  dwInitialFrames;
     DWORD  dwScale;
     DWORD  dwRate;
     DWORD  dwStart;
     DWORD  dwLength;
     DWORD  dwSuggestedBufferSize;
     DWORD  dwQuality;
     DWORD  dwSampleSize;
     struct {
         short int left;
         short int top;
         short int right;
         short int bottom;
     }  rcFrame;
} AVISTREAMHEADER;


#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))
#define FCC_P(pch4) ((pch4[0]) | (pch4[1] << 8) | (pch4[2] << 16) | (pch4[3] << 24))


long initialize_avi_read(char *infilename, long *dimx, long *dimy, long *NumFrame);
long read_oneframe_uyvy_avi(char *infilename, BYTE *oneframe);
long finalize_avi_read(char *infilename);
