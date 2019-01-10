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

#include "AudioDecoderMP3.h"

/*
 * ACM wrapper
 *
 */

MediaAudioDecoderMP3::MediaAudioDecoderMP3()
{
	this->oFormat     = NULL;
	this->out_buffer  = NULL;
	this->in_buffer   = NULL;
}


MediaAudioDecoderMP3::~MediaAudioDecoderMP3()
{

}

/*
 * MP3 decoding wrapper
 *
 */

int MediaAudioDecoderMP3::DecompressMp3(char *outmemory, int outmemsize, int *done)
{

	if(this->last_result == MP3_OK) {
		this->last_result = decodeMP3(&this->mp, NULL, 0, outmemory, outmemsize, done);
    
    if(this->last_result == MP3_NEED_MORE) {
     
      if( this->decaps->ReadAudioData(0, this->in_buffer, 16384) == 16384) {
		
		  this->last_result = decodeMP3(&this->mp, (char *) this->in_buffer, 16384, outmemory, outmemsize, done);
		  return 1;
      }
      else {
	
		  return 0;
      }
    }
    else {
      return 1;
    }

  }
  else {
    
    if( this->decaps->ReadAudioData(0, this->in_buffer, 16384) == 16384) {
      this->last_result = decodeMP3(&this->mp, (char *) this->in_buffer, 16384, outmemory, outmemsize, done);
      return 1;
    }
    else {
      
      return 0;
    }
  }
}

/*
 * MediaItem Funcs
 */

media_type_t  MediaAudioDecoderMP3::GetType()
{
	return MEDIA_TYPE_AUDIO_DECODER;
}

char         *MediaAudioDecoderMP3::GetName()
{
	/*
	 * UPDATE!
	 */

	return "MPEG-1 Layer III Audio Decoder";
}
	
MP_RESULT     MediaAudioDecoderMP3::Connect(MediaItem *item)
{
	WAVEFORMATEX *inFormat;

	if(item && item->GetType() == MEDIA_TYPE_DECAPS) {

		this->decaps = (MediaItemDecaps *) item;

		inFormat = this->decaps->GetAudioFormat(0);

		if(inFormat->wFormatTag == 0x55 || inFormat->wFormatTag == 0x50) {

			/*
			 * The Audio Stream is MPEG-1
			 */


			/* 
			 * Init the decoder
			 */

			InitMP3(&this->mp);
	  
		    this->last_result = MP3_NEED_MORE;
	        ring_init();

			this->in_buffer  = (char *) new char[16384];
			this->out_buffer = (char *) new char[65536];

			if(this->DecompressMp3(this->out_buffer, 16384, &this->real_size) == MP3_ERR) {

				ExitMP3(&this->mp);
			    return MP_RESULT_ERROR;
			}
	
			/*
			 * Buffering
			 */

			while(!ring_full(this->real_size))
				{
					if(this->DecompressMp3(this->out_buffer, 16384, &this->real_size) == 1)
				        ring_write(this->out_buffer, this->real_size);
				}

			/*
			 * and sets up the output
			 * format for the renderer
			 */

			this->oFormat = (WAVEFORMATEX *) new WAVEFORMATEX;
			
			memcpy(this->oFormat, inFormat, sizeof(WAVEFORMATEX));
			this->oFormat->wFormatTag = WAVE_FORMAT_PCM;

			if (oFormat->wBitsPerSample != 8 && oFormat->wBitsPerSample != 16)
				oFormat->wBitsPerSample = 16;

			if (oFormat->nChannels!=1 && oFormat->nChannels!=2)
				oFormat->nChannels = 2;

			oFormat->nBlockAlign		= (oFormat->wBitsPerSample/8) * oFormat->nChannels;
			oFormat->nAvgBytesPerSec	=  oFormat->nBlockAlign * oFormat->nSamplesPerSec;
			oFormat->cbSize		        =  0;
			
			return MP_RESULT_OK;
		}
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaAudioDecoderMP3::ReleaseConnections()
{
	/*
	 * Cleanup
	 */

	this->decaps = NULL;

	free(this->in_buffer);
	this->in_buffer = NULL;
	
	free(this->out_buffer);
	this->out_buffer = NULL;

	free(this->oFormat);
	this->oFormat = NULL;

	ExitMP3(&this->mp);

	return MP_RESULT_OK;
}

DWORD         MediaAudioDecoderMP3::GetCaps()
{
	return 0;
}

MP_RESULT     MediaAudioDecoderMP3::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Audio Decoder
 */

WAVEFORMATEX *MediaAudioDecoderMP3::GetAudioFormat()
{
	return this->oFormat;
}

MP_RESULT     MediaAudioDecoderMP3::EmptyAudioBuffer()
{
	ring_init();

	this->last_result = MP3_NEED_MORE;
	ExitMP3(&this->mp);
	InitMP3(&this->mp);

	return MP_RESULT_OK;
}

unsigned int MediaAudioDecoderMP3::Decompress(void *buffer, unsigned int size)
{
	DWORD i;

	if(this->oFormat && buffer) {
		
		if(size == 0)
			return 0;

		if(size < 32768) {

			/*
			 * we only read once
			 */

			while(!ring_full(this->real_size)) {
    
				if(this->DecompressMp3(this->out_buffer, 16384, &this->real_size) == 1) {
					ring_write(this->out_buffer, this->real_size);
				}
				else {

					return 0;
				}
			}
  
			ring_read((char *) buffer, size);

			return size;
		}
		else {

			int blocks = size / 32768;

			for(i = 0; i < blocks; i++) {
			
				while(!ring_full(this->real_size)) {
    
					if(this->DecompressMp3(this->out_buffer, 16384, &this->real_size) == 1) {
						ring_write(this->out_buffer, this->real_size);
					}
					else {

						return 0;
					}
				}
  
				ring_read(((char *) buffer) + i*32768, 32768);
			}

			/*
			 * the last bit
			 */
			
			int left = size - (blocks * 32768);

			if(left > 0)
				Decompress((void *) (((char *) buffer) + size - left), left);

			return size;
		}
	}

	return 0;
}
