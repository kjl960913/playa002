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

#include "Input.h"

/*
 * Input Class
 */

/*
 * Constructor 
 *
 */

MediaInput::MediaInput()
{
	this->inputInternet = new MediaInputInternet();
	this->inputFile     = new MediaInputFile();

	this->input = NULL;
}

/*
 * Destructor
 *
 */

MediaInput::~MediaInput()
{
	delete this->inputFile;
	delete this->inputInternet;
}

/*
 * Media Item functions
 */

media_type_t MediaInput::GetType()
{
	return MEDIA_TYPE_INPUT;
}

char        *MediaInput::GetName()
{	
	if(this->input)
		return this->input->GetName();

	return "Input Wrapper";
}

MP_RESULT MediaInput::Connect(MediaItem *item)
{
	return MP_RESULT_ERROR;
}

MP_RESULT MediaInput::ReleaseConnections()
{
	return MP_RESULT_ERROR;
}

DWORD         MediaInput::GetCaps()
{
	if(this->input)
		return this->input->GetCaps();
	
	return 0;
}

MP_RESULT     MediaInput::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Input Item functions
 */

/*
 * Open:
 * -----
 *
 * - choose the correct input item
 *
 */

MP_RESULT MediaInput::Open(char *url, media_input_mode_t mode)
{
	if(this->input != NULL)
		return MP_RESULT_ERROR;

	/*
	 * look to see what type
	 * of url this is...
	 *
	 */

	if(url != NULL) {

		if(strstr(url, "http://") != NULL || strstr(url, "ftp://") != NULL ||
		   strstr(url, "HTTP://") != NULL || strstr(url, "FTP://") != NULL) {

			if(this->inputInternet->Open(url, mode) != MP_RESULT_OK)
				return MP_RESULT_ERROR;

			this->input = this->inputInternet;

			return MP_RESULT_OK;
		}
		else {
			
			if(this->inputFile->Open(url, mode) != MP_RESULT_OK)
				return MP_RESULT_ERROR;

			this->input = this->inputFile;

			return MP_RESULT_OK;
		}
	}

	return MP_RESULT_ERROR;
}

long MediaInput::GetSize()
{
	if(this->input != NULL)
		return this->input->GetSize();

	return MP_RESULT_ERROR;
}

long MediaInput::GetBufferSize()
{
	if(this->input != NULL)
		return this->input->GetBufferSize();

	return MP_RESULT_ERROR;
}

long MediaInput::GetBufferPosition()
{
	if(this->input != NULL)
		return this->input->GetBufferPosition();

	return MP_RESULT_ERROR;
}

long MediaInput::GetBufferingSize()
{
	if(this->input != NULL)
		return this->input->GetBufferingSize();

	return MP_RESULT_ERROR;
}

unsigned int MediaInput::Read(MediaBuffer *mb, unsigned int size)
{
	if(this->input != NULL)
		return this->input->Read(mb, size);

	return MP_RESULT_ERROR;
}

unsigned int MediaInput::Seek(int size, media_input_seek_t method)
{
	if(this->input != NULL)
		return this->input->Seek(size, method);

	return MP_RESULT_ERROR;
}

unsigned int  MediaInput::GetLine(MediaBuffer *mb)
{
	if(this->input != NULL)
		return this->input->GetLine(mb);

	return 0;
}

BOOL MediaInput::EndOfFile()
{
	if(this->input != NULL)
		return this->input->EndOfFile();

	return FALSE;
}

MP_RESULT MediaInput::Close()
{
	if(this->input != NULL)
		this->input->Close();

	this->input = NULL;

	return MP_RESULT_ERROR;
}

