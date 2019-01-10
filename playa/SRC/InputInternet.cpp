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

#include "InputInternet.h"

/*
 * Input internet media
 * --------------------
 *
 * - Reads HTTP/FTP files
 *
 */

/*
 * HTTP filling thread
 */

DWORD WINAPI httpThreadFunc(LPVOID lpData)
{
	MediaInputInternet *input = (MediaInputInternet *) lpData;
	DWORD read;
	char  buffer[8192];

	while(input->downloaded < input->size) {

		read = 0;
		InternetReadFile(input->http, (void *) buffer, 8192, &read);

		fwrite(buffer, 1, read, input->dnld);
		input->downloaded += read;

		WaitForSingleObject(input->bufferingMutex, INFINITE);

		if(input->file)
			fclose(input->file);
		
		input->file = fopen("c:\\temp.avi", "rb");

		fseek(input->file, input->lastReadPos, SEEK_SET);

		ReleaseMutex(input->bufferingMutex);
	}

	return 0;
}

/*
 * Constructor
 * -----------
 * 
 * 
 *
 */

MediaInputInternet::MediaInputInternet()
{
	this->file = NULL;

	this->hInternet  = NULL;		
	this->http       = NULL;
	this->buffering  = 0;
	this->downloaded = 0;

	this->size       = BUFFERING_SIZE;

	this->bufferingMutex = CreateMutex(NULL, FALSE, NULL);
}

/*
 * Destructor
 * ----------
 * 
 * 
 *
 */


MediaInputInternet::~MediaInputInternet()
{
	CloseHandle(this->bufferingMutex);
}

/*
 * Get Type:
 * ---------
 *
 * - Get the type of the Media Item
 *
 */

media_type_t MediaInputInternet::GetType()
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

char *MediaInputInternet::GetName()
{
	return "HTTP/FTP Input";
}

/*
 * Connect:
 * --------
 *
 * - Accepts nothing
 *
 */

MP_RESULT MediaInputInternet::Connect(MediaItem *item)
{
	return MP_RESULT_ERROR;
}

MP_RESULT MediaInputInternet::ReleaseConnections()
{
	return MP_RESULT_ERROR;
}

DWORD         MediaInputInternet::GetCaps()
{
	return MEDIA_CAPS_BUFFERIZE;
}

MP_RESULT     MediaInputInternet::Configure(HINSTANCE hInstance, HWND hwnd)
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

MP_RESULT MediaInputInternet::Open(char *url, media_input_mode_t mode) 
{
	this->file = NULL;

	this->hInternet   = NULL;		
	this->http        = NULL;
	this->buffering   = 0;
	this->downloaded  = 0;
	this->lastReadPos = 0;

	if(url != NULL) {

		/*
		 * First, open an internet
		 * connection if available
		 *
		 */

		this->hInternet = InternetOpen("The Playa - OpenDivX Viewer",
									   INTERNET_OPEN_TYPE_DIRECT,
									   NULL, NULL, 0);

		if(this->hInternet == NULL) {
				
			MP_ERROR("Could not open an Internet connection");
			return MP_RESULT_ERROR;
		}

		/*
		 * Now try to open the URL
		 *
		 */

		this->http = InternetOpenUrl(this->hInternet, url, NULL, -1, 0, 0);
			
		if(this->http == NULL) {
			
			InternetCloseHandle(this->hInternet);

			MP_ERROR("The location could not be opened");
			return MP_RESULT_ERROR;
		}
		
		/*
		 * Get the file size
		 *
		 */

		DWORD buffer;
		DWORD dwLength = 4;

		if(!HttpQueryInfo(this->http, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &buffer, &dwLength, NULL)) {

			InternetCloseHandle(this->http);
			InternetCloseHandle(this->hInternet);
			
			MP_ERROR("Could not get the media size");
			
			return MP_RESULT_ERROR;
		}
		else {

			this->size = buffer;			
		}

		/*
		 * Create the temp file
		 *
		 */

		this->dnld = fopen("c:\\temp.avi", "w+b");

		if(this->dnld == NULL) {

			InternetCloseHandle(this->http);
			InternetCloseHandle(this->hInternet);

			MP_ERROR("Could not create temp file, check your settings.");
			return MP_RESULT_ERROR;
		}
		
		/*
		 * And launch the filling thread
		 *
		 */

		this->httpThread = CreateThread(NULL, 0, httpThreadFunc, (LPVOID) this, 0, &this->httpId);
		
		/*
		 * And open the file for reading
		 *
		 */

		if(this->file == NULL) {

			WaitForSingleObject(this->bufferingMutex, INFINITE);

			if( (this->file = fopen("c:\\temp.avi", "rb")) == NULL) {
	
				TerminateThread(this->httpThread, 0);

				fclose(this->dnld);

				InternetCloseHandle(this->http);
				InternetCloseHandle(this->hInternet);
			
				MP_ERROR("Could not open temp file!");
				return MP_RESULT_ERROR;
			}

			ReleaseMutex(this->bufferingMutex);
		}
	}

	return MP_RESULT_OK;
}

/*
 * GetSize:
 * --------
 *
 */

long MediaInputInternet::GetSize()
{
	if(this->file)
		return this->size;

	return MP_RESULT_ERROR;
}

long MediaInputInternet::GetBufferSize()
{
	if(this->dnld) {

		return this->downloaded;
	}

	return 0;
}

long MediaInputInternet::GetBufferPosition()
{
	if(this->dnld) {

		return this->lastReadPos;
	}

	return 0;
}

long MediaInputInternet::GetBufferingSize()
{
	return BUFFERING_SIZE;
}

/*
 * Read:
 * -----
 *
 * - Reads some 
 *   binary data
 *
 */

unsigned int MediaInputInternet::Read(MediaBuffer *mb, unsigned int size)
{
	if(!mb || !this->file) {
	
		return MP_RESULT_ERROR;
	}

	if(size > mb->GetSize())
		mb->ReAlloc(size);

	if(this->file != NULL) {

		unsigned int result;

		WaitForSingleObject(this->bufferingMutex, INFINITE);
		
		result = fread(mb->GetData(), 1, size, this->file);

		this->lastReadPos = ftell(this->file);

		ReleaseMutex(this->bufferingMutex);

		return result;
	}
	else {
		return 0;
	}
}

/*
 * Seek:
 * -----
 *
 * - Seek into the file
 *
 */

unsigned int MediaInputInternet::Seek(int size, media_input_seek_t method)
{
	unsigned int result;

	if(!this->file)
		return MP_RESULT_ERROR;
		
	WaitForSingleObject(this->bufferingMutex, INFINITE);

	switch(method) {

		case INPUT_SEEK_SET:

			result = fseek(this->file, size, SEEK_SET);

			this->lastReadPos = ftell(this->file);
			ReleaseMutex(this->bufferingMutex);

			return result;
			break;

			
		case INPUT_SEEK_CUR:

			if(size == 0) {
				
				result = ftell(this->file);	

				this->lastReadPos = result;
				ReleaseMutex(this->bufferingMutex);

				return result;
			}
			else {
				
				result = fseek(this->file, size, SEEK_CUR);
				
				this->lastReadPos = ftell(this->file);
				ReleaseMutex(this->bufferingMutex);

				return result;
			}
			break;

		case INPUT_SEEK_END:

			result = fseek(this->file, size, SEEK_END);

			this->lastReadPos = ftell(this->file);
			ReleaseMutex(this->bufferingMutex);

			return result;
			break;
	}

	ReleaseMutex(this->bufferingMutex);

	return MP_RESULT_ERROR;
}

/*
 * GetLine:
 * --------
 *
 * - Read an entire line.
 *
 */

unsigned int MediaInputInternet::GetLine(MediaBuffer *mb)
{
	if(!this->file)
		return MP_RESULT_ERROR;

	WaitForSingleObject(this->bufferingMutex, INFINITE);
	
	fgets((char *) mb->GetData(), mb->GetSize(), this->file);

	ReleaseMutex(this->bufferingMutex);

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

BOOL MediaInputInternet::EndOfFile()
{
	return FALSE;
}


/*
 * Close:
 * ------
 *
 * - Closes the file
 *
 */

MP_RESULT MediaInputInternet::Close()
{
	if(!this->file)
		return MP_RESULT_ERROR;

	WaitForSingleObject(this->bufferingMutex, INFINITE);

	TerminateThread(this->httpThread, 0);
	
	ReleaseMutex(this->bufferingMutex);

	fclose(this->dnld);

	InternetCloseHandle(this->http);
	InternetCloseHandle(this->hInternet);

	if(this->file)
		fclose(this->file);

	DeleteFile("c:\\temp.avi");

	return MP_RESULT_OK;
}
