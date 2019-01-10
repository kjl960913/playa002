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

#include "Decaps.h"

/*
 * Decaps:
 * -------
 *
 * - Decaps Wrapper
 *
 */


MediaDecaps::MediaDecaps()
{
	this->decaps    = NULL;
	this->decapsAVI = new MediaDecapsAVI();
}

MediaDecaps::~MediaDecaps()
{

}
	
/*
 * Media Item functions
 */

media_type_t  MediaDecaps::GetType()
{
	return MEDIA_TYPE_DECAPS;
}

char         *MediaDecaps::GetName()
{
	if(this->decaps)
		return this->decaps->GetName();
	
	return "AVI Wrapper";
}
	
MP_RESULT     MediaDecaps::Connect(MediaItem *item)
{

	/*
	 * Try the AVI system first
	 *
	 */

	if(this->decapsAVI->Connect(item) == MP_RESULT_OK) {

		this->decaps = this->decapsAVI;
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::ReleaseConnections()
{
	if(this->decaps != NULL) {

		this->decaps->ReleaseConnections();
		this->decaps = NULL;
	}

	return MP_RESULT_OK;
}

DWORD         MediaDecaps::GetCaps()
{
	return 0;
}

MP_RESULT     MediaDecaps::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Decaps functions
 */

unsigned int  MediaDecaps::GetNumberOfVideoStreams()
{
	if(this->decaps) {

		return this->decaps->GetNumberOfVideoStreams();
	}

	return MP_RESULT_ERROR;
}

unsigned int  MediaDecaps::GetNumberOfAudioStreams()
{
	if(this->decaps) {

		return this->decaps->GetNumberOfAudioStreams();
	}

	return MP_RESULT_ERROR;
}

unsigned int  MediaDecaps::GetVideoWidth(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetVideoWidth(StreamId);
	}

	return MP_RESULT_ERROR;
}

unsigned int  MediaDecaps::GetVideoHeight(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetVideoHeight(StreamId);
	}

	return MP_RESULT_ERROR;
}

double        MediaDecaps::GetVideoFrameRate(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetVideoFrameRate(StreamId);
	}

	return MP_RESULT_ERROR;
}

char		 *MediaDecaps::GetVideoCompression(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetVideoCompression(StreamId);
	}

	return "NULL";
}

BITMAPINFOHEADER *MediaDecaps::GetVideoHeader(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetVideoHeader(StreamId);
	}

	return NULL;
}

unsigned long MediaDecaps::GetCurrentVideoFrame(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetCurrentVideoFrame(StreamId);
	}

	return 0;
}

unsigned long MediaDecaps::GetTotalVideoFrames(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetTotalVideoFrames(StreamId);
	}

	return 0;
}

unsigned long MediaDecaps::GetTotalVideoTime(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetTotalVideoTime(StreamId);
	}

	return 0;
}
	
unsigned int  MediaDecaps::GetAudioBits(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetAudioBits(StreamId);
	}

	return 0;
}

unsigned int  MediaDecaps::GetAudioChannels(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetAudioChannels(StreamId);
	}

	return 0;
}

unsigned int  MediaDecaps::GetAudioFrequency(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetAudioFrequency(StreamId);
	}

	return 0;
}

WAVEFORMATEX *MediaDecaps::GetAudioFormat(unsigned int StreamId) 
{
	if(this->decaps) {

		return this->decaps->GetAudioFormat(StreamId);
	}

	return NULL;
}


unsigned int  MediaDecaps::GetNextVideoFrameSize(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->GetNextVideoFrameSize(StreamId);
	}

	return 0;
}

unsigned int  MediaDecaps::ReadVideoFrame(unsigned int StreamId, MediaBuffer *mb)
{
	if(this->decaps) {

		return this->decaps->ReadVideoFrame(StreamId, mb);
	}

	return MP_RESULT_ERROR;
}

unsigned int  MediaDecaps::ReadAudioData(unsigned int StreamId, char *buffer, unsigned int bytes)
{
	if(this->decaps) {

		return this->decaps->ReadAudioData(StreamId, buffer, bytes);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaDecaps::UpdateForSize()
{
	if(this->decaps)
		return this->decaps->UpdateForSize();

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::SeekAudio(unsigned int StreamId, long bytes)
{
	if(this->decaps) {

		return this->decaps->SeekAudio(StreamId, bytes);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::SeekVideo(unsigned int StreamId, long frame)
{
	if(this->decaps) {

		return this->decaps->SeekVideo(StreamId, frame);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::ReSeekAudio(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->ReSeekAudio(StreamId);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::Seek(unsigned int videoStreamId, unsigned int audioStreamId, int percent)
{
	if(this->decaps) {

		return this->decaps->Seek(videoStreamId, audioStreamId, percent);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::Rewind(unsigned int videoStreamId, unsigned int audioStreamId)
{
	if(this->decaps) {

		return this->decaps->Rewind(videoStreamId, audioStreamId);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::SeekNextKeyFrame(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->SeekNextKeyFrame(StreamId);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaDecaps::SeekPreviousKeyFrame(unsigned int StreamId)
{
	if(this->decaps) {

		return this->decaps->SeekPreviousKeyFrame(StreamId);
	}

	return MP_RESULT_ERROR;
}

