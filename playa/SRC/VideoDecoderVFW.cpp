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

#include "VideoDecoderVFW.h"

/*
 * Video For Windows 
 * video decoder plugin
 *
 */


MediaVideoDecoderVFW::MediaVideoDecoderVFW()
{
	this->hic    = NULL;
	this->decaps = NULL;

	this->invertFlag = 1;

	this->inputBuffer = new MediaBuffer();
}

MediaVideoDecoderVFW::~MediaVideoDecoderVFW()
{
	delete this->inputBuffer;
}

/*
 * Media Item functions
 */

media_type_t  MediaVideoDecoderVFW::GetType()
{
	return MEDIA_TYPE_VIDEO_DECODER;
}

char *MediaVideoDecoderVFW::GetName()
{
	if(this->hic) {

		ICINFO icInfo;
		char *name;
		int i;
			
		ICGetInfo(this->hic, &icInfo, sizeof(ICINFO));

		name = (char *) new char[128];

		for(i=0; i < 128; i++) { 
			name[i] = icInfo.szDescription[i] & 255;
		}
	
		return name;
	}
	
	return "Video For Windows Decoder";
}

MP_RESULT     MediaVideoDecoderVFW::Connect(MediaItem *item)
{
	HRESULT h;

	if(item != NULL && item->GetType() == MEDIA_TYPE_DECAPS) {

		this->decaps = (MediaItemDecaps *) item;

		/*
		 * Look if an appropriate
		 * codec can be found...
		 *
		 */

		if(this->decaps->GetVideoHeader(0) == NULL)
			return MP_RESULT_ERROR;

		memcpy(&this->in_bih.bmiHeader, this->decaps->GetVideoHeader(0), sizeof(BITMAPINFOHEADER));

		this->hic = ICOpen(mmioFOURCC('V', 'I', 'D', 'C'), 
							          this->in_bih.bmiHeader.biCompression,
							          ICMODE_FASTDECOMPRESS);

		if(this->hic != 0) {

			ZeroMemory(&this->out_bih, sizeof(BITMAPINFO));

			this->out_bih.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
			this->out_bih.bmiHeader.biBitCount = 24;

			h = ICDecompressGetFormat( this->hic, &this->in_bih, &this->out_bih ); 

			if(h == ICERR_OK) {

				h = ICDecompressQuery(this->hic, &this->in_bih, &this->out_bih);
			    
				this->out_bih.bmiHeader.biSizeImage = out_bih.bmiHeader.biWidth*out_bih.bmiHeader.biHeight*out_bih.bmiHeader.biBitCount/8;

				h = ICDecompressBegin(this->hic, &this->in_bih, &this->out_bih);  
			
				this->invertFlag = 1;

				if(this->in_bih.bmiHeader.biCompression == mmioFOURCC('D', 'I', 'V', '3') ||	
					this->in_bih.bmiHeader.biCompression == mmioFOURCC('D', 'I', 'V', '4')) {

					this->videoMode  = VIDEO_MODE_YUY2;
				}
				else {

					this->videoMode  = VIDEO_MODE_RGB24;
				}

				if(h != ICERR_OK) {
					
					this->hic    = 0;
					this->decaps = NULL;

					return MP_RESULT_ERROR;
				}

				this->inputBuffer->Alloc(VFW_INPUT_SIZE);

				return MP_RESULT_OK;
			}
		}
		
	}

	this->decaps = NULL;

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaVideoDecoderVFW::ReleaseConnections()
{
	if(this->hic) {

		ICDecompressEnd(this->hic);
		ICClose(this->hic);

		this->inputBuffer->Free();
	}

	return MP_RESULT_OK;
}

DWORD         MediaVideoDecoderVFW::GetCaps()
{
	return 0;
}

MP_RESULT     MediaVideoDecoderVFW::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_OK;
}

/*
 * Video Decoder Functions
 *
 */

unsigned int       MediaVideoDecoderVFW::GetFrameSize()
{
	if(this->hic) {

		return this->out_bih.bmiHeader.biSizeImage;
	}

	return 0;
}

media_video_mode_t MediaVideoDecoderVFW::GetVideoMode()
{
	return this->videoMode;
}

MP_RESULT          MediaVideoDecoderVFW::SetQuality(DWORD quality)
{
	return MP_RESULT_OK;
}

DWORD          MediaVideoDecoderVFW::GetQuality()
{
	return 0;
}

BOOL			   MediaVideoDecoderVFW::GetInvertFlag()
{
	return this->invertFlag;
}

MP_RESULT          MediaVideoDecoderVFW::SetVideoMode(media_video_mode_t mode)
{
	if(this->hic) {
		
		switch(mode) {

		case VIDEO_MODE_YUY2:

			memcpy(&this->out_bih, &this->in_bih, sizeof(BITMAPINFO));

			this->out_bih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

			this->out_bih.bmiHeader.biCompression = mmioFOURCC('Y', 'U', 'Y', '2');
			this->out_bih.bmiHeader.biBitCount    = 16;
				
			ICDecompressEnd(this->hic);

			/*
			 * We must ignore the retrun value
			 * as the DivX codec will return -2 
			 * (invalid format) although it
			 * accepts it !!
			 * 
			 */

			ICDecompressQuery(this->hic, &this->in_bih, &this->out_bih);

			this->out_bih.bmiHeader.biSizeImage = out_bih.bmiHeader.biWidth*out_bih.bmiHeader.biHeight*out_bih.bmiHeader.biBitCount/8;

			this->videoMode  = VIDEO_MODE_YUY2;
			this->invertFlag = 1;

			this->out_bih.bmiHeader.biCompression = 0;
			ICDecompressBegin(this->hic, &this->in_bih, &this->out_bih);
			this->out_bih.bmiHeader.biCompression = mmioFOURCC('Y', 'U', 'Y', '2');

			return MP_RESULT_OK;

			break;


		case VIDEO_MODE_RGB16:

			memcpy(&this->out_bih, &this->in_bih, sizeof(BITMAPINFO));

			this->out_bih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) + 12;

			this->out_bih.bmiHeader.biCompression = BI_BITFIELDS;
			this->out_bih.bmiHeader.biBitCount    = 16;

			((DWORD *) this->out_bih.bmiColors)[0] = (DWORD) 0xF800;
			((DWORD *) this->out_bih.bmiColors)[1] = (DWORD) 0x07E0;
			((DWORD *) this->out_bih.bmiColors)[2] = (DWORD) 0x001F;
				
			ICDecompressEnd(this->hic);

			if(ICDecompressQuery(this->hic, &this->in_bih, &this->out_bih) == ICERR_OK) {

				this->out_bih.bmiHeader.biSizeImage = out_bih.bmiHeader.biWidth*out_bih.bmiHeader.biHeight*out_bih.bmiHeader.biBitCount/8;

				this->invertFlag = 1;
				this->videoMode  = VIDEO_MODE_RGB16;

				ICDecompressBegin(this->hic, &this->in_bih, &this->out_bih);
				return MP_RESULT_OK;
			}
			break;

		case VIDEO_MODE_RGB24:
			
			memcpy(&this->out_bih, &this->in_bih, sizeof(BITMAPINFO));
			
			this->out_bih.bmiHeader.biCompression = BI_RGB;
			this->out_bih.bmiHeader.biBitCount    = 24;


			ICDecompressEnd(this->hic);
			
			if(ICDecompressQuery(this->hic, &this->in_bih, &this->out_bih) == ICERR_OK) {

				this->out_bih.bmiHeader.biSizeImage = out_bih.bmiHeader.biWidth*out_bih.bmiHeader.biHeight*out_bih.bmiHeader.biBitCount/8;

				this->videoMode = VIDEO_MODE_RGB24;
				this->invertFlag = 1;

				ICDecompressBegin(this->hic, &this->in_bih, &this->out_bih);
				return MP_RESULT_OK;
			}
			break;

		case VIDEO_MODE_RGB32:

			memcpy(&this->out_bih, &this->in_bih, sizeof(BITMAPINFO));

			this->out_bih.bmiHeader.biCompression = BI_RGB;
			this->out_bih.bmiHeader.biBitCount    = 32;

			ICDecompressEnd(this->hic);

			if(ICDecompressQuery(this->hic, &this->in_bih, &this->out_bih) == ICERR_OK) {

				this->out_bih.bmiHeader.biSizeImage = out_bih.bmiHeader.biWidth*out_bih.bmiHeader.biHeight*out_bih.bmiHeader.biBitCount/8;

				this->videoMode = VIDEO_MODE_RGB32;
				this->invertFlag = 1;

				ICDecompressBegin(this->hic, &this->in_bih, &this->out_bih);
				return MP_RESULT_OK;
			}
			break;

		default:
			return MP_RESULT_ERROR;
		}
	}		

	return MP_RESULT_ERROR;
}

MP_RESULT          MediaVideoDecoderVFW::Decompress(MediaBuffer *mb_out, unsigned int stride)
{
	unsigned int size;
	HRESULT h;

	if(this->decaps && mb_out && this->hic) {

		size = this->decaps->GetNextVideoFrameSize(0);
	
		if(size > this->inputBuffer->GetSize())
			this->inputBuffer->ReAlloc(size);

		if(this->decaps->ReadVideoFrame(0, this->inputBuffer) != MP_RESULT_OK) {

			return MP_RESULT_ERROR;
		}

		this->in_bih.bmiHeader.biSizeImage = size;

		if(size == 0) {
		
			return MP_RESULT_OK;
		}
		else {

			h = ICDecompress(this->hic, ICDECOMPRESS_NOTKEYFRAME,
							 &this->in_bih.bmiHeader, this->inputBuffer->GetData(),
							 &this->out_bih.bmiHeader, mb_out->GetData());

			return MP_RESULT_OK;
		}

	}	
	
	return MP_RESULT_ERROR;
}

MP_RESULT          MediaVideoDecoderVFW::Drop(MediaBuffer *mb_out, unsigned int stride)
{
	unsigned int size;
	HRESULT h;

	if(this->decaps && mb_out && this->hic) {

		size = this->decaps->GetNextVideoFrameSize(0);
	
		if(size > this->inputBuffer->GetSize())
			this->inputBuffer->ReAlloc(size);

		this->decaps->ReadVideoFrame(0, this->inputBuffer);

		this->in_bih.bmiHeader.biSizeImage = size;

		if(size == 0) {
		
			return MP_RESULT_ERROR;
		}
		else {

			h = ICDecompress(this->hic, ICDECOMPRESS_HURRYUP,
							 &this->in_bih.bmiHeader, this->inputBuffer->GetData(),
							 &this->out_bih.bmiHeader, mb_out->GetData());

			return MP_RESULT_OK;
		}

	}	
	
	return MP_RESULT_ERROR;
}
