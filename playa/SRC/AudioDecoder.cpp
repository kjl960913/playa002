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

#include "AudioDecoder.h"

/*
 * Audio Decoder Wrapper
 *
 */

MediaAudioDecoder::MediaAudioDecoder()
{
	this->acmDecoder = new MediaAudioDecoderACM();
	this->mp3Decoder = new MediaAudioDecoderMP3();

	this->decoder    = NULL;
}

MediaAudioDecoder::~MediaAudioDecoder()
{
	delete this->acmDecoder;
	delete this->mp3Decoder;
}

/*
 * MediaItem Funcs
 */

media_type_t  MediaAudioDecoder::GetType()
{
	return MEDIA_TYPE_AUDIO_DECODER;
}

char         *MediaAudioDecoder::GetName()
{
	if(this->decoder)
		return this->decoder->GetName();

	return "Audio Decoder Wrapper";
}
	
MP_RESULT     MediaAudioDecoder::Connect(MediaItem *item)
{

	if(this->acmDecoder->Connect(item) == MP_RESULT_OK) {

		this->decoder = (MediaItemAudioDecoder *) this->acmDecoder;
		
		return MP_RESULT_OK;
	}

	if(this->mp3Decoder->Connect(item) == MP_RESULT_OK) {

		this->decoder = (MediaItemAudioDecoder *) this->mp3Decoder;
		
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaAudioDecoder::ReleaseConnections()
{
	if(this->decoder) {

		this->decoder->ReleaseConnections();
	}

	this->decoder = NULL;
	
	return MP_RESULT_OK;
}

DWORD         MediaAudioDecoder::GetCaps()
{
	return 0;
}

MP_RESULT     MediaAudioDecoder::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Audio Decoder
 */

WAVEFORMATEX *MediaAudioDecoder::GetAudioFormat()
{
	if(this->decoder)
		return this->decoder->GetAudioFormat();

	return NULL;
}

MP_RESULT     MediaAudioDecoder::EmptyAudioBuffer()
{
	if(this->decoder)
		return this->decoder->EmptyAudioBuffer();

	return MP_RESULT_ERROR;
}

unsigned int  MediaAudioDecoder::Decompress(void *buffer, unsigned int size)
{
	if(this->decoder)
		return this->decoder->Decompress(buffer, size);

	return 0;
}
