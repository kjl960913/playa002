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

#include "VideoDecoder.h"

/*
 * video decoder 
 * plugin wrapper
 *
 */


MediaVideoDecoder::MediaVideoDecoder()
{
	this->decoder       = NULL;

	this->decoreDecoder = new MediaVideoDecoderDecore();
	this->vfwDecoder    = new MediaVideoDecoderVFW();
}

MediaVideoDecoder::~MediaVideoDecoder()
{
	delete this->decoreDecoder;
	delete this->vfwDecoder;
}

/*
 * Media Item functions
 */

media_type_t  MediaVideoDecoder::GetType()
{
	return MEDIA_TYPE_VIDEO_DECODER;
}

char         *MediaVideoDecoder::GetName()
{
	if(this->decoder)
		return this->decoder->GetName();

	return "Video Decoder Wrapper";
}

MP_RESULT     MediaVideoDecoder::Connect(MediaItem *item)
{

	/*
	 * The OpenDivX decoder
	 */

	if(this->decoreDecoder->Connect(item) == MP_RESULT_OK) {

		this->decoder = this->decoreDecoder;
		return MP_RESULT_OK;
	}

	/*
	 * The VFW decoder
	 */

	if(this->vfwDecoder->Connect(item) == MP_RESULT_OK) {

		this->decoder = this->vfwDecoder;
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaVideoDecoder::ReleaseConnections()
{
	/*
	 * Do Cleanup here!
	 */

	if(this->decoder) {

		this->decoder->ReleaseConnections();
		this->decoder = NULL;
	}

	return MP_RESULT_OK;
}

DWORD         MediaVideoDecoder::GetCaps()
{
	if(this->decoder)
		return this->decoder->GetCaps();
	
	return 0;
}

MP_RESULT     MediaVideoDecoder::Configure(HINSTANCE hInstance, HWND hwnd)
{
	if(this->decoder)
		return this->decoder->Configure(hInstance, hwnd);

	return MP_RESULT_ERROR;
}

/*
 * Video Decoder Functions
 *
 */

unsigned int MediaVideoDecoder::GetFrameSize() 
{
	if(this->decoder)
		return this->decoder->GetFrameSize();

	return 0;
}

media_video_mode_t MediaVideoDecoder::GetVideoMode() 
{
	if(this->decoder)
		return this->decoder->GetVideoMode();

	return VIDEO_MODE_NONE;
}

BOOL MediaVideoDecoder::GetInvertFlag()
{
	if(this->decoder)
		return this->decoder->GetInvertFlag();

	return FALSE;
}


MP_RESULT          MediaVideoDecoder::SetVideoMode(media_video_mode_t mode)
{
	if(this->decoder)
		return this->decoder->SetVideoMode(mode);
	
	return MP_RESULT_ERROR;
}

MP_RESULT          MediaVideoDecoder::SetQuality(DWORD quality)
{
	if(this->decoder)
		return this->decoder->SetQuality(quality);
	
	return MP_RESULT_ERROR;
}

DWORD          MediaVideoDecoder::GetQuality()
{
	if(this->decoder)
		return this->decoder->GetQuality();
	
	return 0;
}

MP_RESULT          MediaVideoDecoder::Decompress(MediaBuffer *mb_out, unsigned int stride)
{
	if(this->decoder)
		return this->decoder->Decompress(mb_out, stride);

	return MP_RESULT_ERROR;
}

MP_RESULT          MediaVideoDecoder::Drop(MediaBuffer *mb_out, unsigned int stride)
{
	if(this->decoder)
		return this->decoder->Drop(mb_out, stride);

	return MP_RESULT_ERROR;
}
