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

#include "Subtitles.h"

/*
 * The subtitles
 * reader wrapper Class...
 *
 */

MediaSubtitler::MediaSubtitler()
{
	this->microDVDSubtitler = new MediaSubtitlerMicroDVD();
	this->subtitler         = NULL;
}

MediaSubtitler::~MediaSubtitler()
{
	delete this->microDVDSubtitler;
}

/*
 * Media Item functions
 */

media_type_t  MediaSubtitler::GetType()
{
	return MEDIA_TYPE_SUBTITLER;
}

char         *MediaSubtitler::GetName()
{
	if(this->subtitler) {

		return this->subtitler->GetName();
	}

	return "Subtitler Wrapper";
}

MP_RESULT     MediaSubtitler::Connect(MediaItem *item)
{	
	if(this->subtitler)
		return this->subtitler->Connect(item);

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaSubtitler::ReleaseConnections()
{
	if(this->subtitler)
		return this->subtitler->ReleaseConnections();

	return MP_RESULT_ERROR;
}

DWORD         MediaSubtitler::GetCaps()
{
	return 0;
}

MP_RESULT     MediaSubtitler::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Subtitler
 */

MP_RESULT     MediaSubtitler::Open(char *lpFilename)
{
	if(this->microDVDSubtitler->Open(lpFilename) == MP_RESULT_OK) {

		this->subtitler = (MediaItemSubtitler *) this->microDVDSubtitler;
		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

subtitles_t  *MediaSubtitler::GetSubtitles(DWORD frameNumber)
{
	if(this->subtitler)
		return this->subtitler->GetSubtitles(frameNumber);

	return NULL;
}

MP_RESULT     MediaSubtitler::Close()
{
	if(this->subtitler)
		return this->subtitler->Close();

	return MP_RESULT_ERROR;
}

