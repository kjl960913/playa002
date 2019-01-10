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

#include "InputFile.h"

/*
 * Input file media
 * ----------------
 *
 * - Reads local files
 *
 */

/*
 * Constructor
 * -----------
 * 
 * 
 *
 */

MediaInputFile::MediaInputFile()
{
	this->file = NULL;
}

/*
 * Destructor
 * ----------
 * 
 * 
 *
 */


MediaInputFile::~MediaInputFile()
{

}

/*
 * Get Type:
 * ---------
 *
 * - Get the type of the Media Item
 *
 */

media_type_t MediaInputFile::GetType()
{
	return MEDIA_TYPE_INPUT;
}

/*
 * Get Name:
 * ---------
 *
 * - Get the name of the Media Item
 *
 */

char *MediaInputFile::GetName()
{
	return "FILE Input";
}

/*
 * Connect:
 * --------
 *
 * - Accepts nothing
 *
 */

MP_RESULT MediaInputFile::Connect(MediaItem *item)
{
	return MP_RESULT_ERROR;
}

MP_RESULT MediaInputFile::ReleaseConnections()
{
	return MP_RESULT_ERROR;
}

DWORD         MediaInputFile::GetCaps()
{
	return 0;
}

MP_RESULT     MediaInputFile::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Open:
 * -----
 *
 * - Opens the file
 *
 */

MP_RESULT MediaInputFile::Open(char *url, media_input_mode_t mode) 
{
	this->file = NULL;

	if(url != NULL) {

		switch(mode) {

			case INPUT_OPEN_BINARY:

				this->file = fopen(url, "rb");
				break;

			case INPUT_OPEN_ASCII:

				this->file = fopen(url, "rt");
				break;

			default:
			
				this->file = fopen(url, "rb");
				break;
		}
	}

	if(this->file == NULL) {

		return MP_RESULT_ERROR;
	}

	/*
	 * Get the file size
	 */

	this->Seek(0, INPUT_SEEK_END);
	this->size = this->Seek(0, INPUT_SEEK_CUR);
	this->Seek(0, INPUT_SEEK_SET);

	return MP_RESULT_OK;
}

/*
 * GetSize()
 * ---------
 *
 */

long MediaInputFile::GetSize()
{
	return MP_RESULT_ERROR;
}

long MediaInputFile::GetBufferSize()
{
	return MP_RESULT_ERROR;
}

long MediaInputFile::GetBufferPosition()
{
	return MP_RESULT_ERROR;
}

long MediaInputFile::GetBufferingSize()
{
	return MP_RESULT_ERROR;
}

/*
 * Read:
 * -----
 *
 * - Reads some 
 *   binary data
 *
 */

unsigned int MediaInputFile::Read(MediaBuffer *mb, unsigned int size)
{
	if(!mb || !this->file)
		return MP_RESULT_ERROR;

	if(size > mb->GetSize())
		mb->ReAlloc(size);

	return fread(mb->GetData(), 1, size, this->file);
}

/*
 * Seek:
 * -----
 *
 * - Seek into the file
 *
 */

unsigned int MediaInputFile::Seek(int size, media_input_seek_t method)
{
	if(!this->file)
		return MP_RESULT_ERROR;

	switch(method) {

		case INPUT_SEEK_SET:

			return fseek(this->file, size, SEEK_SET);
			break;

			
		case INPUT_SEEK_CUR:

			if(size == 0) {
				return ftell(this->file);	
			}
			else {
				return fseek(this->file, size, SEEK_CUR);
			}
			break;

		case INPUT_SEEK_END:

			return fseek(this->file, size, SEEK_END);
			break;
	}

	return MP_RESULT_ERROR;
}

/*
 * GetLine:
 * --------
 *
 * - Read an entire line.
 *
 */

unsigned int MediaInputFile::GetLine(MediaBuffer *mb)
{
	if(!this->file)
		return MP_RESULT_ERROR;

	fgets((char *) mb->GetData(), mb->GetSize(), this->file);

	return MP_RESULT_OK;
}

/*
 * EndOfFile:
 * ----------
 *
 * - Returns TRUE if we are at the end
 *   of the file...
 *
 */

BOOL MediaInputFile::EndOfFile()
{
	if(!this->file)
		return TRUE;

	return feof(this->file);
}


/*
 * Close:
 * ------
 *
 * - Closes the file
 *
 */

MP_RESULT MediaInputFile::Close()
{
	if(!this->file)
		return MP_RESULT_ERROR;

	fclose(this->file);

	return MP_RESULT_OK;
}

