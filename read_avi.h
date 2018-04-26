#pragma warning(disable:4996)	// version���� ���� warning ����
// For BMP file
typedef struct tagBITMAPFILEHEADER {
	unsigned short bfType;		// BM �� ����
	unsigned long bfSize;		// ��ü���� ũ��(byte����)
	unsigned short bfReserved1;		// ����� ����
	unsigned short bfReserved2;		// ����� ����
	unsigned long bfoffBits;		// �������� ��ġ������ �Ÿ� 
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	unsigned long biSize;			// ����ü�� ũ��
	long biWidth;					// ������ ��(�ȼ�����)
	long biHeight;					// ������ ����
	unsigned short biPlanes;		// ��Ʈ �÷��� ��
	unsigned short biBitCount;		// �ȼ��� ��Ʈ��
	unsigned long biCompression;	// ���� ����
	unsigned long biSizeImage;		// ������ ũ��(byte ����)
	long biXPelsPerMeter;			// ���� �ػ�
	long biYpelsPerMeter;			// ���� �ػ�
	unsigned long biClrUsed;		// ���� ��� �����
	unsigned long biClrImportant;	// �߿��� �����ε���
} BITMAPINFOHEADER;


typedef struct tagRGBQUAD {
	unsigned char rgbBlue;			// B����
	unsigned char rgbGreen;			// G����
	unsigned char rgbRead;			// R����
	unsigned char rgbReserved1;		// ����� ����
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
