// in file lossless.h
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef unsigned char byte;

class readlss{

private:
	char	ColorFormat[4];
	char	fps;
	short	m_width,m_height;
	long	header_size;
	long	num_frame;
	long	*offset;
	long	*length;
	
	short	block_x,block_y;
	long	frame_size;
	long	curr_frame_index;

	FILE	*flss;

	// memory
	byte	*buffer;
	byte	*curr_frame,*diff_frame;
	byte	*file_buffer;
	
public:

	long	lss_info(char *fname);
	long	initialize_lss_read(char *fname);
	long	finalize_lss_read();
	long	read_oneframe_uyvy_lss(byte *output_frame, long frame_index);
	void	decoding_diff_frame(byte *diff_frame);
	void	decoding_intra_frame(byte *output_frame);
	long	WhatIsNumFrame(){ return num_frame; }
	long	WhatIsWidth(){ return (m_width>>1); }
	long	WhatIsHeight(){ return m_height; }


};
