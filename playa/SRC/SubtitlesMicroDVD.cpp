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

#include "SubtitlesMicroDVD.h"

/*
 * The MicroDVD subtitles
 * reader Class...
 *
 */

MediaSubtitlerMicroDVD::MediaSubtitlerMicroDVD()
{
	this->inputFile   = new MediaInput();
	this->inputBuffer = new MediaBuffer();
}

MediaSubtitlerMicroDVD::~MediaSubtitlerMicroDVD()
{
	delete this->inputFile;
	delete this->inputBuffer;
}

media_type_t  MediaSubtitlerMicroDVD::GetType()
{
	return MEDIA_TYPE_SUBTITLER;
}

char         *MediaSubtitlerMicroDVD::GetName()
{
	return "MicroDVD Subtitle Reader";
}

MP_RESULT     MediaSubtitlerMicroDVD::Connect(MediaItem *item)
{
	/*
	 * We accept only decaps in case we
	 * need a framerate info...
	 */

	return MP_RESULT_OK;
}

MP_RESULT     MediaSubtitlerMicroDVD::ReleaseConnections()
{
	return MP_RESULT_OK;
}

DWORD         MediaSubtitlerMicroDVD::GetCaps()
{
	return 0;
}

MP_RESULT     MediaSubtitlerMicroDVD::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Try to parse a subtitle line
 */

MP_RESULT MediaSubtitlerMicroDVD::ParseSubtitle()
{
	char text[512];
	DWORD i, j, count;

	if( sscanf((char *) this->inputBuffer->GetData(), "{%d}{%d}", &this->firstFrame, &this->lastFrame) != 2)
		return MP_RESULT_ERROR;

	/*
	 * We got the frame number, now
	 * get the text...
	 */

	strcpy(text, strrchr((char *) this->inputBuffer->GetData(), '}') + 1);
	
	/*
	 * Now get the different lines
	 */

	i     = 0;
	count = 0;

	for(j=0; j < strlen(text); j++) {

		if(text[j] == '|') {

			this->subtitles.subtitlesText[i][count] = '\0';
			i++;
			count = 0;
		}
		else {

			if(text[j] == '\n') {

				this->subtitles.subtitlesText[i][count] = '\0';
				break;
			}
			else {

				this->subtitles.subtitlesText[i][count] = text[j];
				count++;
			}
		}
	}

	this->subtitles.nbSubtitles = i + 1;

	return MP_RESULT_OK;
}

/*
 * Seek to a specified frame!
 */

MP_RESULT MediaSubtitlerMicroDVD::SeekToFrame(DWORD frameNumber)
{
	if(frameNumber > this->lastFrame) {

		/*
		 * Seek forward
		 */

		do {

			this->inputFile->GetLine(this->inputBuffer);

			this->previousLastFrame = this->lastFrame;

			if(sscanf((char *) this->inputBuffer->GetData(), "{%d}{%d}", &this->firstFrame, &this->lastFrame) != 2) {

				return MP_RESULT_ERROR;
			}
		
		}
		while(this->firstFrame < frameNumber);

		this->ParseSubtitle();
	}
	else {

		/*
		 * Search from start
		 */

		this->inputFile->Seek(0, INPUT_SEEK_SET);

		this->firstFrame = 0;
		this->lastFrame  = 0;

		this->previousLastFrame = 0;

		SeekToFrame(frameNumber);
	}

	return MP_RESULT_OK;
}


/*
 * Subtitler
 */

MP_RESULT    MediaSubtitlerMicroDVD::Open(char *lpFilename)
{
	if(lpFilename) {

		this->firstFrame         = 0;
		this->lastFrame          = 0;
		this->firstSubtitleFrame = 0;
		this->previousLastFrame  = 0;

		if(this->inputFile->Open(lpFilename, INPUT_OPEN_ASCII) == MP_RESULT_OK) {

			/*
			 * Test if this is
			 * a MicroDVD file...
			 */
			
			this->inputBuffer->Alloc(SUB_INPUT_BUFFER_SIZE);

			this->subtitles.subtitlesText[0] = (char *) new char[256];
			this->subtitles.subtitlesText[1] = (char *) new char[256];
			this->subtitles.subtitlesText[2] = (char *) new char[256];
			this->subtitles.subtitlesText[3] = (char *) new char[256];
			this->subtitles.nbSubtitles = 0;

			this->inputFile->GetLine(this->inputBuffer);

			if(this->ParseSubtitle() == MP_RESULT_OK) {

				this->firstSubtitleFrame = this->firstFrame;
				this->previousLastFrame  = 0;

				/*
				 * We're ready to go!
				 */

				return MP_RESULT_OK;
			}

		}
	}

	return MP_RESULT_ERROR;
}

subtitles_t *MediaSubtitlerMicroDVD::GetSubtitles(DWORD frameNumber)
{
	if(frameNumber < this->firstSubtitleFrame) {
	
		return NULL;
	}

	if(frameNumber < this->firstFrame && frameNumber > this->previousLastFrame) {

		return NULL;
	}

	if(frameNumber >= this->firstFrame && frameNumber < this->lastFrame) {

		/*
		 * Ok! We got it!
		 */

		return &this->subtitles;
	}
	else {
	
		/*
		 * Seek to the right frame!
		 */

		this->SeekToFrame(frameNumber);
		
		return NULL;
	}

	return NULL;
}

MP_RESULT    MediaSubtitlerMicroDVD::Close()
{
	if(this->inputFile) {
	
		this->inputFile->Close();
	}

	return MP_RESULT_ERROR;
}


