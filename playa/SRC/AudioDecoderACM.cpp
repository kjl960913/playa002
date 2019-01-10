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

#include "AudioDecoderACM.h"

/*
 * ACM wrapper
 *
 */

MediaAudioDecoderACM::MediaAudioDecoderACM()
{
	this->out_buffer  = NULL;
	this->in_buffer   = NULL;
	this->oFormat     = NULL;

	this->ring        = new MediaRingBuffer();
}

MediaAudioDecoderACM::~MediaAudioDecoderACM()
{
	delete this->ring;
}

/*
 * MediaItem Funcs
 */

media_type_t  MediaAudioDecoderACM::GetType()
{
	return MEDIA_TYPE_AUDIO_DECODER;
}

char		 *MediaAudioDecoderACM::GetName()
{
	if(this->hacm != NULL) {

		char            *name;
		HACMDRIVERID     dId;
		ACMDRIVERDETAILS details;

		name = (char *) new char[128];

		if(acmDriverID((HACMOBJ) this->hacm, &dId, 0) > 0) {
			
			return NULL;
		}

		memset(&details, 0, sizeof(ACMDRIVERDETAILS));
		details.cbStruct = sizeof(ACMDRIVERDETAILS);

		if(acmDriverDetails(dId, &details, 0) == MMSYSERR_INVALHANDLE) {

			return NULL;
		}

		strcpy(name, details.szLongName);
		return name;
	}

	return "ACM Wrapper";
}
	
MP_RESULT     MediaAudioDecoderACM::Connect(MediaItem *item)
{
	MMRESULT      mmres;
	WAVEFORMATEX *inFormat;
	DWORD         dwOutputFormatSize;

	if(item && item->GetType() == MEDIA_TYPE_DECAPS) {

		this->decaps = (MediaItemDecaps *) item;

		inFormat = this->decaps->GetAudioFormat(0);

		/*
		 * Mp3 stuff
		 */

		if(inFormat->wFormatTag == 85 && inFormat->cbSize != 12) {

			/*
			 * Try to guess the parameters
			 */

			MPEGLAYER3WAVEFORMAT *format;

			format = (MPEGLAYER3WAVEFORMAT *) inFormat;

			format->wfx.cbSize      = 12;

			format->wID             = 1;
			format->fdwFlags        = 2;
			format->nBlockSize      = 313;
			format->nFramesPerBlock = 1;
			format->nCodecDelay     = 1393;
		}

		/*
	 	 * Get the output size
		 */

		if (acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, (LPVOID)&dwOutputFormatSize)) {
			
			return MP_RESULT_ERROR;
		}

		this->oFormat = (WAVEFORMATEX *) new WAVEFORMATEX;
		this->oFormat->wFormatTag = WAVE_FORMAT_PCM;

		if (acmFormatSuggest(NULL, inFormat, this->oFormat, dwOutputFormatSize, ACM_FORMATSUGGESTF_WFORMATTAG)) {

		}

		if (oFormat->wBitsPerSample!=8 && oFormat->wBitsPerSample!=16)
			oFormat->wBitsPerSample=16;
	
		if (oFormat->nChannels!=1 && oFormat->nChannels!=2)
			oFormat->nChannels = 2;

		oFormat->nBlockAlign		= (oFormat->wBitsPerSample/8) * oFormat->nChannels;
		oFormat->nAvgBytesPerSec	= oFormat->nBlockAlign * oFormat->nSamplesPerSec;
		oFormat->cbSize				= 0;

		mmres = acmStreamOpen(&hacm, NULL, inFormat, 
							  this->oFormat, NULL, NULL, 0, ACM_STREAMOPENF_NONREALTIME);

		if(mmres != 0) {

			/*
			 * No Audio
			 */
			
			return MP_RESULT_ERROR;
		}

		/*
		 * Here we have a valid stream
		 *
		 */

		this->inputSize  = max(2048 - (2048 % inFormat->nBlockAlign),  8*inFormat->nBlockAlign);
		this->outputSize = 0;

		if (acmStreamSize(this->hacm, this->inputSize, &this->outputSize, ACM_STREAMSIZEF_SOURCE))
			return MP_RESULT_ERROR;

		this->ring->Init();

		this->in_buffer  = (char *) new char[this->inputSize];
		this->out_buffer = (char *) new char[this->outputSize];

		this->acmInLeft = 0;
		this->acmLeft   = 0;

		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaAudioDecoderACM::ReleaseConnections()
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

	acmStreamClose(this->hacm, 0);

	return MP_RESULT_OK;
}

DWORD         MediaAudioDecoderACM::GetCaps()
{
	return 0;
}

MP_RESULT     MediaAudioDecoderACM::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Audio Decoder
 */

WAVEFORMATEX *MediaAudioDecoderACM::GetAudioFormat()
{
	return this->oFormat;
}

MP_RESULT     MediaAudioDecoderACM::EmptyAudioBuffer()
{
	if(this->ring)
		this->ring->Init();

	this->acmLeft   = 0;
	this->acmInLeft = 0;

	return MP_RESULT_OK;
}

unsigned int MediaAudioDecoderACM::Decompress(void *buffer, unsigned int size)
{
	DWORD    i, done;
	MMRESULT res;

	done = 0;

	if(this->hacm) {

		if(size < 16384) {

			if(this->acmLeft > 0 && !this->ring->IsFullFor(this->acmLeft)) {
			
				/*
				 * Copy what we have left
				 */
	
				this->ring->Write((char *) this->out_buffer, this->acmLeft);
				this->acmLeft = 0;
			}

			if(this->acmLeft > 0)
				goto done;
				
			if(!this->acmInLeft) {

				if(this->decaps->ReadAudioData(0, this->in_buffer, this->inputSize) != this->inputSize) {

					return done;
				}
			}

			memset(&this->acmHeader, 0, sizeof(ACMSTREAMHEADER));
	
			this->acmHeader.cbStruct    = sizeof(ACMSTREAMHEADER);
				
			if(this->acmInLeft > 0) {
				
				memcpy(this->in_buffer, ((char *) this->in_buffer) + this->inputSize - this->acmInLeft, this->acmInLeft);

				if(this->decaps->ReadAudioData(0, ((char *) this->in_buffer) + this->acmInLeft, this->inputSize - this->acmInLeft) != (this->inputSize - this->acmInLeft) ) {

					return done;
				}

				this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
				this->acmHeader.cbSrcLength = this->inputSize;
		
				this->acmInLeft = 0;
			}
			else {
					this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
					this->acmHeader.cbSrcLength = this->inputSize;
			}
			
			this->acmHeader.pbDst       = (unsigned char *) this->out_buffer;
			this->acmHeader.cbDstLength = this->outputSize;	

			res = acmStreamPrepareHeader(this->hacm, &this->acmHeader, 0);	
	
			if(res > 0) {

				return done;
			}

			res = acmStreamConvert(this->hacm, &this->acmHeader, 0);
	
			if(res > 0) {
	
				acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
				return done;
			}

			while(!this->ring->IsFullFor(this->acmHeader.cbDstLengthUsed)) {
	
				/*
				 * We store it
				 */

				this->ring->Write((char *) this->acmHeader.pbDst, this->acmHeader.cbDstLengthUsed);

				/*
				 * unprepare and start again
				 */

				acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
						
				if(!this->acmInLeft) {

					if(this->decaps->ReadAudioData(0, this->in_buffer, this->inputSize) != this->inputSize) {
					
						return done;
					}
				}

				memset(&this->acmHeader, 0, sizeof(ACMSTREAMHEADER));

				this->acmHeader.cbStruct    = sizeof(ACMSTREAMHEADER);
				
				if(this->acmInLeft > 0) {
		
					memcpy(this->in_buffer, ((char *) this->in_buffer) + this->inputSize - this->acmInLeft, this->acmInLeft);

					if(this->decaps->ReadAudioData(0, ((char *) this->in_buffer) + this->acmInLeft, this->inputSize - this->acmInLeft) != (this->inputSize - this->acmInLeft) ) {

						return done;
					}

					this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
					this->acmHeader.cbSrcLength = this->inputSize;
		
					this->acmInLeft = 0;
				}
				else {
					this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
					this->acmHeader.cbSrcLength = this->inputSize;
				}

				this->acmHeader.pbDst       = (unsigned char *) this->out_buffer;
				this->acmHeader.cbDstLength = this->outputSize;
	
				res = acmStreamPrepareHeader(this->hacm, &this->acmHeader, 0);
	
				if(res > 0) {
						
					return done;
				}	

				res = acmStreamConvert(this->hacm, &this->acmHeader, 0);
	
				if(res > 0) {	
	
					acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
					return done;
				}
				
				this->acmInLeft = this->inputSize - this->acmHeader.cbSrcLengthUsed;
			}

			this->acmLeft = this->acmHeader.cbDstLengthUsed;
	
			acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
done:
				
			this->ring->Read((char *) buffer, size);
			done += size;

			return done;
		}
		else {
	
			unsigned int blocks = size / 16384;
	
			for(i = 0; i < blocks; i++) {

			if(this->acmLeft > 0 && !this->ring->IsFullFor(this->acmLeft)) {
			
				/*
				 * Copy what we have left
				 */
		
				this->ring->Write((char *) this->out_buffer, this->acmLeft);
				this->acmLeft = 0;
			}

			if(this->acmLeft > 0)
				goto done2;
					
			if(!this->acmInLeft) {

				if(this->decaps->ReadAudioData(0, this->in_buffer, this->inputSize) != this->inputSize) {

					return done;
				}
			}

				memset(&this->acmHeader, 0, sizeof(ACMSTREAMHEADER));
		
				this->acmHeader.cbStruct    = sizeof(ACMSTREAMHEADER);
					
				if(this->acmInLeft > 0) {
				
					memcpy(this->in_buffer, ((char *) this->in_buffer) + this->inputSize - this->acmInLeft, this->acmInLeft);

					if(this->decaps->ReadAudioData(0, ((char *) this->in_buffer) + this->acmInLeft, this->inputSize - this->acmInLeft) != (this->inputSize - this->acmInLeft) ) {

						return done;
					}

					this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
					this->acmHeader.cbSrcLength = this->inputSize;
		
					this->acmInLeft = 0;
				}
				else {
					this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
					this->acmHeader.cbSrcLength = this->inputSize;
				}
			
				this->acmHeader.pbDst       = (unsigned char *) this->out_buffer;
				this->acmHeader.cbDstLength = this->outputSize;	

				res = acmStreamPrepareHeader(this->hacm, &this->acmHeader, 0);	
	
				if(res > 0) {
				
					return done;
				}
	
				res = acmStreamConvert(this->hacm, &this->acmHeader, 0);
	
				if(res > 0) {
	
					acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
					return done;
				}

				while(!this->ring->IsFullFor(this->acmHeader.cbDstLengthUsed)) {
	
					/*
					 * We store it
					 */

					this->ring->Write((char *) this->acmHeader.pbDst, this->acmHeader.cbDstLengthUsed);

					/*
					 * unprepare and start again
					 */
	
					acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
								
					if(!this->acmInLeft) {

						if(this->decaps->ReadAudioData(0, this->in_buffer, this->inputSize) != this->inputSize) {
					
							return done;
						}
					}

					memset(&this->acmHeader, 0, sizeof(ACMSTREAMHEADER));

					this->acmHeader.cbStruct    = sizeof(ACMSTREAMHEADER);
				
					if(this->acmInLeft > 0) {
					
						memcpy(this->in_buffer, ((char *) this->in_buffer) + this->inputSize - this->acmInLeft, this->acmInLeft);

						if(this->decaps->ReadAudioData(0, ((char *) this->in_buffer) + this->acmInLeft, this->inputSize - this->acmInLeft) != (this->inputSize - this->acmInLeft) ) {

							return done;
						}

						this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
						this->acmHeader.cbSrcLength = this->inputSize;
		
						this->acmInLeft = 0;
					}
					else {
					
						this->acmHeader.pbSrc       = (unsigned char *) this->in_buffer;
						this->acmHeader.cbSrcLength = this->inputSize;
					}	

					this->acmHeader.pbDst       = (unsigned char *) this->out_buffer;
					this->acmHeader.cbDstLength = this->outputSize;
	
					res = acmStreamPrepareHeader(this->hacm, &this->acmHeader, 0);
	
					if(res > 0) {
					
						return done;
					}		

					res = acmStreamConvert(this->hacm, &this->acmHeader, 0);
		
					if(res > 0) {	
		
						acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
						return done;
					}
				
					this->acmInLeft = this->inputSize - this->acmHeader.cbSrcLengthUsed;
				}

				this->acmLeft = this->acmHeader.cbDstLengthUsed;
	
				acmStreamUnprepareHeader(this->hacm, &this->acmHeader, 0);
done2:
					
				this->ring->Read(((char *) buffer) + i*16384, 16384);
				done += 16384;
			}

			/*
			 * The last bit
			 */
	 			
			int left = size - (blocks * 16384);

			if(left > 0) {
				this->Decompress((void *) ((char *) buffer + size - left), left);
			
				done += left;
			}

			return done;
		}
	}

	return 0;
}
