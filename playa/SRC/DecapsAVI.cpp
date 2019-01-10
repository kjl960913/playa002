/**************************************************************************************
 *                                                                                    *
 * This application contains code from OpenDivX and is released as a "Larger Work"    *
 * under that license. Consistant with that license, this application is released     *
 * under the GNU General Public License.                                              *
 *                                                                                    *
 * The OpenDivX license can be found at: http://www.projectmayo.com/opendivx/docs.php *
 * The GPL can be found at: http://www.gnu.org/copyleft/gpl.html                      *
 *                                                                                    *
 * Copyright (c) 2001 - Project Mayo                                                  *
 *                                                                                    *
 * Authors: Damien Chavarria                                                          *
 *          DivX Advanced Research Center <darc at projectmayo.com>                   *
 *                                                                                    *
 **************************************************************************************/

#include "DecapsAVI.h"

/*
 * Some Useful functions
 *
 */

/*
 * Some useful functions
 */

/*
 * copy n into dst as a 4 byte, 
 * little endian number.
 * should also work on 
 * big endian machines 
 *
 */

static void long2str(char *dst, int n)
{
	dst[0] = (n      ) & 0xff;
	dst[1] = (n >>  8) & 0xff;
	dst[2] = (n >> 16) & 0xff;
	dst[3] = (n >> 24) & 0xff;
}

/* Convert a string of 4 
 * or 2 bytes to a number,
 * also working on big 
 * endian machines 
 *
 */

unsigned long str2ulong(char *str)
{
    unsigned long result;

	result = ( ((unsigned long) str[0] & 0xFF) | ( ((unsigned long)str[1] &0xFF) << 8) |
			  (((unsigned long) str[2] & 0xFF) << 16) | ( ((unsigned long) str[3] & 0xFF) << 24) );

	return result;
}

static unsigned long str2ushort(char *str)
{
	return ( str[0] | (str[1]<<8) );
}

/*
 * AVI Decaps Class
 * ----------------
 * ----------------
 *
 */

/*
 * Get Type:
 * ---------
 *
 * - Get the type of the Media Item
 *
 */

media_type_t MediaDecapsAVI::GetType()
{
	return MEDIA_TYPE_DECAPS;
}

/*
 * Get Name:
 * ---------
 *
 * - Get the name of the Media Item
 *
 */

char *MediaDecapsAVI::GetName()
{
	return "AVI Demuxer";
}

/*
 * Connect:
 * --------
 *
 * - Accepts Input Media
 *
 */

MP_RESULT MediaDecapsAVI::Connect(MediaItem *item)
{
	if(item->GetType() == MEDIA_TYPE_INPUT) {

		this->input = (MediaItemInput *) item;
	}

	this->video_pos  = 0;
	this->audio_posc = 0;
	this->audio_posb = 0;

	this->idx         = NULL;
	this->video_index = NULL;
	this->audio_index = NULL;


	if(this->IsAVI()) {
		
		if(this->FillHeader(1)) {
		
			return MP_RESULT_OK;
		}
		else {

			this->input->Seek(0, INPUT_SEEK_SET);

			this->IsAVI();
			
			if(this->FillHeader(0)) {
		
				return MP_RESULT_OK;
			}
		}
	}
	
	return MP_RESULT_ERROR;
}

/*
 * ReleaseConnections()
 * --------------------
 *
 * - Cleanup
 *
 */

MP_RESULT MediaDecapsAVI::ReleaseConnections()
{
	this->input = NULL;

	if(this->idx != NULL)
		free(this->idx);

	if(this->video_index != NULL)
		free(this->video_index);
	
	if(this->audio_index != NULL)
		free(this->audio_index);

	this->idx         = NULL;
	this->video_index = NULL;
	this->audio_index = NULL;

	this->audio_strn = 0;
	this->video_strn = 0;

	memset(&this->waveformatex, 0, sizeof(MPEGLAYER3WAVEFORMAT));
	memset(&this->bitmapinfoheader, 0, sizeof(BITMAPINFOHEADER));
	
	return MP_RESULT_OK;
}

DWORD         MediaDecapsAVI::GetCaps()
{
	return 0;
}

MP_RESULT     MediaDecapsAVI::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}


/*
 * Constructor
 *
 */

MediaDecapsAVI::MediaDecapsAVI()
{
	this->video_pos  = 0;
	this->audio_posc = 0;
	this->audio_posb = 0;

	this->idx         = NULL;
	this->video_index = NULL;
	this->audio_index = NULL;

	this->hIOMutex = CreateMutex (NULL, FALSE, NULL);

	this->input = NULL;
}

MediaDecapsAVI::~MediaDecapsAVI()
{
	CloseHandle(this->hIOMutex);
}

int MediaDecapsAVI::IsAVI()
{
	MediaBuffer *mb = new MediaBuffer();
	mb->Alloc(12);

	if(this->input != NULL) {
		
		if( this->input->Read(mb, 12) != 12 ) {
			
			delete mb;
			return 0;
		}
		

		if( strncmp((char *) mb->GetData(), "RIFF", 4) !=0 || 
			strncmp(((char *) mb->GetData()) + 8, "AVI ", 4) !=0 ) {
			return 0;
		}
		
		delete mb;
		return 1;
	}
	
	return 0;
}

int MediaDecapsAVI::SampleSize()
{
	int s;
	
	s = ((this->a_bits + 7) / 8) * this->a_chans;
	
	if(s == 0)
		s = 1;
	
	return s;
}

int MediaDecapsAVI::FillHeader(int getIndex)
{
	unsigned long  n, rate; 
	long           scale, idx_type;
	char          *hdrl_data;
	long           i, hdrl_len = 0;
	long           nvi, nai, ioff;
	long           tot;
	int            lasttag = 0;
	int            vids_strh_seen = 0;
	int            vids_strf_seen = 0;
	int            auds_strh_seen = 0;
	int            auds_strf_seen = 0;
	int            num_stream = 0;
	MediaBuffer   *mb = new MediaBuffer();
	
	if(this->input == NULL)
		return MP_RESULT_ERROR;

	mb->Alloc(256);

    /* go through the AVI file and 
     * extract the header list,
     * the start position of the 'movi' 
     * list and an optionally
     * present idx1 tag 
     */
	
	hdrl_data = 0;
	
	while(1)
    {
		if( this->input->Read(mb, 8) != 8 ) 
			break; 
	
			/*
			 * we assume it's EOF 
			 *
		     */
		
		n = str2ulong(((char *) mb->GetData()) + 4);
		n = PAD_EVEN(n);
		
		if(strncmp((char *) mb->GetData(), "LIST", 4) == 0)
		{
			
			if( this->input->Read(mb, 4) != 4 ) {

				mb->Free();
				return 0;
			}
			
			n -= 4;
			
			if(strncmp((char *) mb->GetData(), "hdrl", 4) == 0)
			{
			

				MediaBuffer *mbh;
				hdrl_len = n;

				char buffer[256];
				sprintf(buffer, "size = %d", n);

				mbh = new MediaBuffer();
				mbh->Alloc(n);

				hdrl_data = (char *) new char[n];
				
				if(hdrl_data == NULL) { 
					
					mb->Free();
					return 0;
				}
				
				if( this->input->Read(mbh, n) != n ) {
					
					mb->Free();
					return 0;
				}

				memcpy(hdrl_data, mbh->GetData(), n);
				mbh->Free();
			}
			else if(strncmp((char *) mb->GetData(), "movi", 4) == 0)
			{
				this->movi_start = this->input->Seek(0, INPUT_SEEK_CUR);
				
				this->input->Seek(n, INPUT_SEEK_CUR);
			}
			else
				this->input->Seek(n, INPUT_SEEK_CUR);
		}
		else if(strncmp((char *) mb->GetData(),"idx1",4) == 0)
		{

		/*
		 * n must be a multiple of 16, 
		 * but the reading does not
		 * break if this is not the case 
		 *
	     */
			
			this->n_idx = this->max_idx = n/16;
			this->idx = (char((*)[16]) ) new char[n];
			
			if(this->idx == NULL) { 
				
				mb->Free();
				return 0;
			}
			
			unsigned long read;
			MediaBuffer  *mbi = new MediaBuffer();

			mbi->Alloc(n);

			if( (read = this->input->Read(mbi, n)) != n ) {
				
				break;
			}

			memcpy(this->idx, mbi->GetData(), n);
			mbi->Free();
		}
		else {
		  
			this->input->Seek(n, INPUT_SEEK_CUR);
		}
	}
	
	if(!hdrl_data) {
		
		mb->Free();
		return 0;
	}
	
	if(!this->movi_start) {
		
		mb->Free();
		return 0;
	}
	

	/* 
     * interpret the header list 
     *
     */
	
	for(i=0; i < hdrl_len; )
	{
	   /* 
	    * list tags are completly ignored 
   	    *
		*/
		
		if(strncmp(hdrl_data + i, "LIST", 4)==0) 
		{ 
			i+= 12; 
			continue; 
		}
		
		n = str2ulong(hdrl_data+i+4);
		n = PAD_EVEN(n);
		
		/* 
		* interpret the tag and its args 
		*
		*/
		
		if(strncmp(hdrl_data + i, "strh", 4)==0)
		{
			i += 8;
			if(strncmp(hdrl_data + i, "vids", 4) == 0 && !vids_strh_seen)
			{
				memcpy(this->compressor,hdrl_data+i+4,4);
				
				this->compressor[4] = 0;
				
				scale = str2ulong(hdrl_data+i+20);
				rate  = str2ulong(hdrl_data+i+24);
				
				if(scale!=0) 
					this->fps = (double)rate/(double)scale;
				
				this->video_frames = str2ulong(hdrl_data+i+32);
				this->video_strn = num_stream;
				
				vids_strh_seen = 1;
				lasttag = 1; 
			}
			else if (strncmp (hdrl_data + i, "auds", 4) == 0 && ! auds_strh_seen)
			{
				this->audio_bytes = str2ulong(hdrl_data + i + 32)*this->SampleSize();
				this->audio_strn = num_stream;
				
				auds_strh_seen = 1;
				lasttag = 2; 
			}
			else
				lasttag = 0;
			num_stream++;
		}
		else if(strncmp(hdrl_data + i, "strf", 4)==0)
		{
			i += 8;
			if(lasttag == 1)
			{
				
			/*
 			 * keep a copy of 
			 * the bitmapinfoheader
			 * 
			 */
				
			memcpy(&this->bitmapinfoheader, 
					hdrl_data + i,
					sizeof(BITMAPINFOHEADER));  
				
				this->width  = str2ulong(hdrl_data+i+4);
				this->height = str2ulong(hdrl_data+i+8);
				vids_strf_seen = 1;
			}
			else if(lasttag == 2)
			{
				/*
				 * keep a copy of the WAVEFORMATEX
				 */

				memcpy(&this->waveformatex,
					   hdrl_data + i,
					   sizeof(MPEGLAYER3WAVEFORMAT));

				this->a_fmt   = str2ushort(hdrl_data + i  );
				this->a_chans = str2ushort(hdrl_data + i + 2);
				this->a_rate  = str2ulong (hdrl_data + i + 4);
				this->a_bits  = str2ushort(hdrl_data + i + 14) == 0 ? 16 : str2ushort(hdrl_data + i + 14);
				
				auds_strf_seen = 1;
			}
			lasttag = 0;
		}
		else
		{
			i += 8;
			lasttag = 0;
		}
		
		i += n;
	}
	
	free(hdrl_data);
	
	if(!vids_strh_seen || !vids_strf_seen || this->video_frames==0) { 
		
		mb->Free();
		return 0;
	}
	
	this->video_tag[0] = this->video_strn/10 + '0';
	this->video_tag[1] = this->video_strn%10 + '0';
	this->video_tag[2] = 'd';
	this->video_tag[3] = 'b';
	
	/* 
    * audio tag is set to "99wb" 
    * if no audio present 
    *
    */
	
	if(!this->a_chans) 
		this->audio_strn = 99;
	
	this->audio_tag[0] = this->audio_strn/10 + '0';
	this->audio_tag[1] = this->audio_strn%10 + '0';
	this->audio_tag[2] = 'w';
	this->audio_tag[3] = 'b';
	
	this->input->Seek(this->movi_start, INPUT_SEEK_SET);
	
	/* 
	 * get index if wanted 
	 *
	 */
	
	if(!getIndex) 
		return 1;
	
	/* 
	 * if the file has an idx1, 
	 * check if this is relative
	 * to the start of the file or 
	 * to the start of the movi list 
	 *
     */
	
	idx_type = 0;
	
	if(this->idx)
	{
		unsigned long pos, len;
		
		/*
		 * search the first videoframe 
		 * in the idx1 and look where
         * it is in the file 
		 *
		 */
		
		for(i = 0; i < this->n_idx; i++) {
			if( strncmp(this->idx[i], this->video_tag, 3) == 0 ) 
				break;
		}
		
		if(i >= this->n_idx) 
			return 0;
		
		pos = str2ulong(this->idx[i] + 8);
		len = str2ulong(this->idx[i] + 12);
		
		this->input->Seek(pos, INPUT_SEEK_SET);
		
		if(this->input->Read(mb, 8)!=8) 
			return 0;
		
		if( strncmp((char *) mb->GetData(), this->idx[i], 4) == 0 && str2ulong((char *) mb->GetData() + 4) == len )
		{
		  /* 
		   * index from start of file
		   *
		   */
			
			idx_type = 1; 
		}
		else
		{
			this->input->Seek(pos + this->movi_start - 4,INPUT_SEEK_SET);
			
			if(this->input->Read(mb, 8) != 8) 
				return 0;
			
			if( strncmp((char *) mb->GetData(), this->idx[i], 4) == 0 && str2ulong((char *) mb->GetData() + 4) == len )
			{
				/* 
				 * index from start of movi list 
				 *
				 */
				
				idx_type = 2; 
			}
		}

		/* 
		 * idx_type remains 0 if neither 
		 * of the two tests above succeeds 
		 *
		 */
	}
	
	if(idx_type == 0)
	{

		/* 
	 	 * we must search through 
		 * the file to get the index 
		 *
		 */
		
		this->input->Seek(this->movi_start, INPUT_SEEK_SET);
		
		this->max_idx = 4096;
		this->n_idx   = 0;

		this->idx = (char((*)[16]) ) new char[this->max_idx];
		
		while(1)
		{
			
			if( this->input->Read(mb, 8) != 8 ) 
				break;
			
			n = str2ulong((char *) mb->GetData() + 4);
			
			/* 
			 * the movi list may contain sub-lists, 
			 * ignore them 
			 * 
			 */
			
			if(strncmp((char *) mb->GetData(), "LIST", 4) == 0)
			{
				this->input->Seek(4, INPUT_SEEK_CUR);
				continue;
			}
			
			/* 
			 * check if we got a tag ##db, 
			 * ##dc or ##wb 
			 * 
			 */
			
			if( ((((char *) mb->GetData())[2]=='d' || ((char *) mb->GetData())[2]=='D') &&
				 (((char *) mb->GetData())[3]=='b' || ((char *) mb->GetData())[3]=='B' || 
				  ((char *) mb->GetData())[3]=='c' || ((char *) mb->GetData())[3]=='C') ) || 
				((((char *) mb->GetData())[2]=='w' || ((char *) mb->GetData())[2]=='W') &&
				 (((char *) mb->GetData())[3]=='b' || ((char *) mb->GetData())[3]=='B') ) )
			{
				this->AddIndexEntry(((char *) mb->GetData()),
									0, 
									this->input->Seek(0, INPUT_SEEK_CUR) - 8, 
									n);
			}
			
			this->input->Seek(PAD_EVEN(n), INPUT_SEEK_CUR);
		}

		idx_type = 1;
	}
	
	/* 
     * now generate the video index 
     * and audio index arrays 
     * 
     */
	
	nvi = 0;
	nai = 0;
	
	for(i=0; i < this->n_idx; i++)
	{
		if(strncmp(this->idx[i], this->video_tag, 3) == 0) 
			nvi++;
		
		if(strncmp(this->idx[i], this->audio_tag, 4) == 0) 
			nai++;
	}
	
	this->video_frames = nvi;
	this->audio_chunks = nai;
	
	if(this->video_frames == 0) {
	
		mb->Free();
		return 0;
	}

	this->video_index = (video_index_entry *) new char[(nvi*sizeof(video_index_entry))];
	
	if(this->video_index == 0) { 
	
		mb->Free();
		return 0;
	}

	if(this->audio_chunks)
	{
		this->audio_index = (audio_index_entry *) new char[(nai*sizeof(audio_index_entry))];
		
		if(this->audio_index == 0) {

			mb->Free();
			return 0;
		}
	}
	
	nvi = 0;
	nai = 0;
	tot = 0;
	
	ioff = idx_type == 1 ? 8 : this->movi_start+4;
	
	for(i=0; i < this->n_idx; i++)
	{
		if(strncmp(this->idx[i], this->video_tag, 3) == 0)
		{
			this->video_index[nvi].flags = str2ulong(this->idx[i]+ 4);
			this->video_index[nvi].pos   = str2ulong(this->idx[i]+ 8)+ioff;
			this->video_index[nvi].len   = str2ulong(this->idx[i]+12);
			
			nvi++;
		}
		if(strncmp(this->idx[i], this->audio_tag, 4) == 0)
		{
			this->audio_index[nai].pos = str2ulong(this->idx[i]+ 8)+ioff;
			this->audio_index[nai].len = str2ulong(this->idx[i]+12);
			this->audio_index[nai].tot = tot;
			
			tot += this->audio_index[nai].len;
			
			nai++;
		}
	}
	
	this->audio_bytes = tot;
	
	/* 
     * reposition the file 
     *
     */
	
	this->input->Seek(this->movi_start, INPUT_SEEK_SET);
	
	this->video_pos = 0;
	mb->Free();
	
	return 1;
}

int MediaDecapsAVI::AddIndexEntry(char *tag, 
								  long flags, 
								  long pos, 
								  long len)
{
	void *ptr;
	
	if(this->n_idx*16 >= this->max_idx)
	{
		ptr = realloc((void *)this->idx,
			(this->max_idx + 4096) * 16);
		
		if(ptr == 0) {
			return -1;
		}
		
		this->max_idx += 4096;
		this->idx = (char((*)[16]) ) ptr;
	}
	
	/* 
    * add the index entry 
    *
    */
	
	memcpy(this->idx[this->n_idx], tag, 4);
	
	long2str(this->idx[this->n_idx]+ 4, flags);
	
	long2str(this->idx[this->n_idx]+ 8, pos);
	
	long2str(this->idx[this->n_idx]+12, len);
	
	/* 
     * update counter
     *
     */
	
	this->n_idx++;
	
	return 0;
}

int  MediaDecapsAVI::ReFillIndex()
{
	unsigned long  n; 
	long           idx_type;
	long           i;
	long           nvi, nai, ioff;
	long           tot;
	MediaBuffer   *mb = new MediaBuffer();
	
	if(this->input == NULL)
		return MP_RESULT_ERROR;

	mb->Alloc(256);

	/* 
 	 * we must search through 
	 * the file to get the index 
	 *
	 */

	this->input->Seek(this->movi_start, INPUT_SEEK_SET);
		
	this->max_idx = 4096;
	this->n_idx   = 0;

	this->idx = (char((*)[16]) ) realloc(this->idx, this->max_idx*16);
		
	while(1)
	{
			
		if( this->input->Read(mb, 8) != 8 ) 
			break;
			
		n = str2ulong((char *) mb->GetData() + 4);
			
		/* 
		 * the movi list may contain sub-lists, 
		 * ignore them 
		 * 
		 */
			
		if(strncmp((char *) mb->GetData(), "LIST", 4) == 0)
		{
			this->input->Seek(4, INPUT_SEEK_CUR);
			continue;
		}
			
		/* 
		 * check if we got a tag ##db, 
		 * ##dc or ##wb 
		 * 
		 */
			
		if( ((((char *) mb->GetData())[2]=='d' || ((char *) mb->GetData())[2]=='D') &&
			 (((char *) mb->GetData())[3]=='b' || ((char *) mb->GetData())[3]=='B' || 
			  ((char *) mb->GetData())[3]=='c' || ((char *) mb->GetData())[3]=='C') ) || 
			((((char *) mb->GetData())[2]=='w' || ((char *) mb->GetData())[2]=='W') &&
			 (((char *) mb->GetData())[3]=='b' || ((char *) mb->GetData())[3]=='B') ) )
		{
			this->AddIndexEntry(((char *) mb->GetData()),
								0, 
								this->input->Seek(0, INPUT_SEEK_CUR) - 8, 
								n);
		}
			
		this->input->Seek(PAD_EVEN(n), INPUT_SEEK_CUR);
	}

	idx_type = 1;
	
	/* 
     * now generate the video index 
     * and audio index arrays 
     * 
     */
	
	nvi = 0;
	nai = 0;
	
	for(i=0; i < this->n_idx; i++)
	{
		if(strncmp(this->idx[i], this->video_tag, 3) == 0) 
			nvi++;
		
		if(strncmp(this->idx[i], this->audio_tag, 4) == 0) 
			nai++;
	}
	
	this->video_frames = nvi;
	this->audio_chunks = nai;
	
	if(this->video_frames == 0) 
		return 0;
	
	this->video_index = (video_index_entry *) realloc(this->video_index, nvi*sizeof(video_index_entry));
	
	if(this->video_index == 0) 
		return 0;
	
	if(this->audio_chunks)
	{
		this->audio_index = (audio_index_entry *) realloc(this->audio_index, nai*sizeof(audio_index_entry));
		
		if(this->audio_index==0) {
			mb->Free();
			return 0;
		}
	}
	
	nvi = 0;
	nai = 0;
	tot = 0;
	
	ioff = idx_type == 1 ? 8 : this->movi_start+4;
	
	for(i=0; i < this->n_idx; i++)
	{
		if(strncmp(this->idx[i], this->video_tag, 3) == 0)
		{
			this->video_index[nvi].flags = str2ulong(this->idx[i]+ 4);
			this->video_index[nvi].pos   = str2ulong(this->idx[i]+ 8)+ioff;
			this->video_index[nvi].len   = str2ulong(this->idx[i]+12);
			
			nvi++;
		}
		if(strncmp(this->idx[i], this->audio_tag, 4) == 0)
		{
			this->audio_index[nai].pos = str2ulong(this->idx[i]+ 8)+ioff;
			this->audio_index[nai].len = str2ulong(this->idx[i]+12);
			this->audio_index[nai].tot = tot;
			
			tot += this->audio_index[nai].len;
			
			nai++;
		}
	}
	
	this->audio_bytes = tot;
	mb->Free();

	return 1;
}



BOOL MediaDecapsAVI::isKeyframe(long frame)
{
  if(frame < 0 || frame > this->video_frames)
    frame = 0;

  if(!this->video_index)         
    { 
      /*
       * we still return 1 to avoid looping
       * on waiting for a keyframe.
       *
       */

      return 1; 
    }

  return this->video_index[frame].flags & AVIIF_KEYFRAME;
}

unsigned int  MediaDecapsAVI::GetNumberOfVideoStreams()
{
	/*
	 * TEMP!
	 */

	return 1;
}

unsigned int  MediaDecapsAVI::GetNumberOfAudioStreams()
{
	if(this->audio_strn == 99)
		return 0;
	
	return this->audio_strn;
}

unsigned int  MediaDecapsAVI::GetVideoWidth(unsigned int StreamId)
{
	return this->width;
}

unsigned int  MediaDecapsAVI::GetVideoHeight(unsigned int StreamId)
{
	return this->height;
}

double        MediaDecapsAVI::GetVideoFrameRate(unsigned int StreamId)
{
	if(this->fps == 0)
		this->fps = 25;

	if(this->fps == 23)
		this->fps = 25;

	return this->fps;
}

char		 *MediaDecapsAVI::GetVideoCompression(unsigned int StreamId)
{
	return this->compressor;
}

BITMAPINFOHEADER *MediaDecapsAVI::GetVideoHeader(unsigned int StreamId)
{
	return &this->bitmapinfoheader;
}

unsigned long MediaDecapsAVI::GetCurrentVideoFrame(unsigned int StreamId)
{
	return this->video_pos;
}


unsigned long MediaDecapsAVI::GetTotalVideoFrames(unsigned int StreamId)
{
	return this->video_frames;
}

unsigned long MediaDecapsAVI::GetTotalVideoTime(unsigned int StreamId)
{
	if(this->fps == 0) {
		
		return 0;
	}
	else {
		
		return (unsigned long) (this->video_frames * 1000 / (double) this->fps);
	}
}
	
unsigned int  MediaDecapsAVI::GetAudioBits(unsigned int StreamId)
{
	return this->a_bits;
}

unsigned int  MediaDecapsAVI::GetAudioChannels(unsigned int StreamId)
{
	return this->a_chans;
}

unsigned int  MediaDecapsAVI::GetAudioFrequency(unsigned int StreamId)
{
	return this->a_rate;
}

WAVEFORMATEX *MediaDecapsAVI::GetAudioFormat(unsigned int StreamId)
{
	return (WAVEFORMATEX *) &this->waveformatex;
}


unsigned int  MediaDecapsAVI::GetNextVideoFrameSize(unsigned int StreamId)
{
	if(!this->video_index) { 
		return 0; 
	}

	return(this->video_index[this->video_pos].len);
}

unsigned int MediaDecapsAVI::ReadVideoFrame(unsigned int StreamId, MediaBuffer *mb)
{
	unsigned int m;
	unsigned int n;

	/*
	 * Security checks
     *
     */


	if(!this->video_index || !mb) {
		
		return MP_RESULT_ERROR; 
    }

	if(this->video_pos < 0 || this->video_pos >= this->video_frames - 1) {

		return MP_RESULT_ERROR;
    }

    /*
     * Request the mutex 
     * for reading the file;
     */

	WaitForSingleObject(this->hIOMutex, INFINITE);

	n = this->video_index[this->video_pos].len;

	this->input->Seek(this->video_index[this->video_pos].pos, INPUT_SEEK_SET);

	if(mb->GetSize() < n) {
		
		mb->ReAlloc(n);
	}

	if ((m = this->input->Read(mb, n)) != n) {

		this->video_pos++;

	    ReleaseMutex(this->hIOMutex);

	char msg[256];
	sprintf(msg, "index %u and %u at %u", this->video_index[this->video_pos].len, this->video_index[this->video_pos].pos, this->video_pos);
	MessageBox(NULL, msg, "", MB_OK);	

	sprintf(msg, "read %d on %u at %d on %d", m, n, this->video_pos, this->video_frames);
	MessageBox(NULL, msg, "", MB_OK);	
		
		return n;
    }

    this->video_pos++;

    /*
     * Release the Mutex.
     */

    ReleaseMutex(this->hIOMutex);

    return MP_RESULT_OK;	
}

unsigned int MediaDecapsAVI::ReadAudioData(unsigned int StreamId, char *buffer, unsigned int bytes)
{
	unsigned int nr, pos, left = 0, todo, posc, posb, asked_bytes;
	MediaBuffer *mbi = new MediaBuffer();

	if(!this->audio_index || !buffer) {
		
		return MP_RESULT_ERROR; 
    }

	asked_bytes = bytes;

	posc = this->audio_posc;
	posb = this->audio_posb;

   nr = 0; 
   mbi->Alloc(bytes);

   /*
    * Request the read Mutex 
    */

   WaitForSingleObject(this->hIOMutex, INFINITE);

   /*
    * We loop until we parsed enough
	* chunks for the amount we want
    *
    */

	while(bytes > 0) {

		left = this->audio_index[this->audio_posc].len - this->audio_posb;

		if(left == 0) {

			if(this->audio_posc >= this->audio_chunks - 1) {
				
				ReleaseMutex(this->hIOMutex);

				this->audio_posb = posb;
				this->audio_posc = posc;

				return 0;
			}

			this->audio_posc++;
			this->audio_posb = 0;
			continue;
		}

		if(bytes < left)
			todo = bytes;
		else
			todo = left;

		pos = this->audio_index[this->audio_posc].pos + this->audio_posb;
      
		this->input->Seek(pos, INPUT_SEEK_SET);

	    if (this->input->Read(mbi, todo) != todo) {

			ReleaseMutex(this->hIOMutex);

			/*
			 * Do as if we just read nothing
			 */

			this->audio_posb = posb;
			this->audio_posc = posc;
			
			return 0;
		}

		memcpy(buffer + nr, mbi->GetData(), todo);

		bytes -= todo;
		nr    += todo;

		this->audio_posb += todo;
	}

	mbi->Free();
	delete mbi;

    /*
     * And release the Mutex.
     */

    ReleaseMutex(this->hIOMutex);

    return asked_bytes;
}

MP_RESULT     MediaDecapsAVI::UpdateForSize()
{
	this->ReFillIndex();
	
	return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::SeekAudio(unsigned int StreamId, long bytes)
{
   long n0, n1, n;

   if(!this->audio_index) { 
       return MP_RESULT_ERROR; 
   }

   if(bytes < 0) 
	   bytes = 0;

   n0 = 0;
   n1 = this->audio_chunks;

   while(n0 < n1 - 1)
   {
      n = (n0 + n1) / 2;
      if(this->audio_index[n].tot > bytes)
         n1 = n;
      else
         n0 = n;
   }

   this->audio_posc = n0;

   if(this->audio_index[n0].len > 1000) {
	 this->audio_posb = bytes - this->audio_index[n0].tot;
   }
   else {
     this->audio_posb = 0;
   }

   return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::SeekVideo(unsigned int StreamId, long frame)
{
	if(!this->video_index) {
		
		return MP_RESULT_ERROR; 
    }

	if (frame < 0 ) {
		
		frame = 0;
	}
	
   this->video_pos = frame;

   return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::ReSeekAudio(unsigned int StreamId)
{
	double ratio;
	long audio_bytes;

    if(this->GetNumberOfAudioStreams() > 0) {

	  WaitForSingleObject(this->hIOMutex, INFINITE);

      ratio = (double) ((double) this->video_pos / (double) this->video_frames);
      
      /*
       * and set audio 
       * position
       *
       */

      audio_bytes  = (long) (ratio * this->audio_bytes);
      audio_bytes  -= (audio_bytes % this->waveformatex.nBlockSize);

      this->SeekAudio(StreamId, audio_bytes);

      ReleaseMutex(this->hIOMutex);
	}

    return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::Seek(unsigned int videoStreamId, unsigned int audioStreamId, int percent)
{
	long frame;
	double ratio;
	long audio_bytes;
  
	WaitForSingleObject(this->hIOMutex, INFINITE);

	/*
     * compute the desired 
     * frame number
     *
     */

    frame = (long) (percent * this->video_frames / 100);

    /*
     * and go to the next 
     * keyframe.
     *
     */
  
    while(!this->isKeyframe(frame) && frame < this->video_frames) {
      frame++;
    }

	/*
	 * If we reached the end
	 * we go back to the beginning...
	 */

	if(frame >= this->video_frames)
		frame = 0;

    /*
     * now set video 
     * position.
     *
     */
    
	this->SeekVideo(videoStreamId, frame);

    /*
     * calculate what ratio 
     * it corresponds to
     *
     */
    if(this->GetNumberOfAudioStreams() > 0) {
      
      ratio = (double) ((double) frame / (double) this->video_frames);
      
      /*
       * and set audio 
       * position
       *
       */

      audio_bytes  = (long) (ratio * this->audio_bytes);
      audio_bytes  -= (audio_bytes % this->waveformatex.nBlockSize);

      this->SeekAudio(audioStreamId, audio_bytes);

      ReleaseMutex(this->hIOMutex);

      return MP_RESULT_OK;
	}

    ReleaseMutex(this->hIOMutex);

	return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::Rewind(unsigned int videoStreamId, unsigned int audioStreamId)
{
	this->video_pos  = 0;
	this->audio_posc = 0;
	this->audio_posb = 0;

	return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::SeekNextKeyFrame(unsigned int StreamId)
{
	WaitForSingleObject(this->hIOMutex, INFINITE);

	/*
	 * Allways increment by one
	 */

	this->video_pos++;

	while(!isKeyframe(this->video_pos) && this->video_pos < this->video_frames)
		this->video_pos++;

    ReleaseMutex(this->hIOMutex);

	return MP_RESULT_OK;
}

MP_RESULT     MediaDecapsAVI::SeekPreviousKeyFrame(unsigned int StreamId)
{
	WaitForSingleObject(this->hIOMutex, INFINITE);
	
	/*
	 * Allways decrement by two
	 * since we read the last frame
	 */

	if(this->video_pos > 1) {

		this->video_pos--;
		this->video_pos--;
	}

	while(!isKeyframe(this->video_pos) && this->video_pos > 0)
		this->video_pos--;

    ReleaseMutex(this->hIOMutex);

	return MP_RESULT_OK;
}

