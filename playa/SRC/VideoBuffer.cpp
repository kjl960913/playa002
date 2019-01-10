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

#include "VideoBuffer.h"

/* 
 * Video Buffer Class
 *
 * - only one implementation
 *
 */

MediaVideoBuffer::MediaVideoBuffer()
{
	DWORD i;

	for(i=0; i < 1; i++) {
	
		this->buffer[i] = new MediaBuffer();
	}

	this->bufferedFrames = 0;


	this->enabled = FALSE;

	this->decoder        = NULL;
	this->bufferingMutex = CreateMutex(NULL, FALSE, NULL);
}

MediaVideoBuffer::~MediaVideoBuffer()
{
	DWORD i;

	for(i=0; i < 1; i++) {
	
		delete this->buffer[i];
	}

	CloseHandle(this->bufferingMutex);
}

/*
 * Media Item functions
 */

media_type_t MediaVideoBuffer::GetType()
{
	return MEDIA_TYPE_VIDEO_BUFFER;
}

char *MediaVideoBuffer::GetName()
{
	return "Default Video Buffer";
}

MP_RESULT MediaVideoBuffer::Connect(MediaItem *item)
{
	DWORD i;

	/*
	 * Accept any Video Decoder
	 *
	 */

	if(item != NULL && item->GetType() == MEDIA_TYPE_VIDEO_DECODER) {

		this->decoder = (MediaItemVideoDecoder *) item;

		if(this->decoder->GetFrameSize() != 0) {
	
			for(i=0; i < 1; i++) {

				if(this->buffer[i] == NULL) {

					this->buffer[i] = new MediaBuffer();
				}
				
				this->buffer[i]->Alloc(this->decoder->GetFrameSize());
			}
		
			this->bufferedFrames = 0;
			this->endReachedAt   = VIDEO_BUFFER_END_NOT_REACHED;

			this->bufferingThread = NULL;

			return MP_RESULT_OK;
		}
	}

	this->decoder = NULL;

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoBuffer::ReleaseConnections()
{
	DWORD i;

	if(this->decoder) {

		for(i=0; i < 1; i++) {

			this->buffer[i]->Free();
		}

		this->decoder = NULL;
	}

	return MP_RESULT_OK;
}

DWORD         MediaVideoBuffer::GetCaps()
{
	return 0;
}

MP_RESULT     MediaVideoBuffer::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/**************************************************************
 *                                                            *
 *                  VIDEO BUFFERING THREAD                    *
 *                  ----------------------                    *
 *                                                            *
 **************************************************************/

DWORD WINAPI BufferingThreadFunc( LPVOID lpData)
{
	MediaVideoBuffer *buffer = (MediaVideoBuffer *) lpData;

	while(TRUE) {

		if(buffer->bufferedFrames >= VIDEO_BUFFER_SIZE) {
	
			buffer->suspended = TRUE;
			SuspendThread(buffer->bufferingThread);
		}
		else {

			WaitForSingleObject(buffer->bufferingMutex, INFINITE);

			if(buffer->decoder->Decompress(buffer->buffer[buffer->bufferedFrames], 0) != MP_RESULT_OK) {
			
				buffer->endReachedAt = buffer->bufferedFrames;
		
				ReleaseMutex(buffer->bufferingMutex);
				
				return 0;
			}

			buffer->bufferedFrames++;

			ReleaseMutex(buffer->bufferingMutex);
		}

	}

	return 0;
}

/**************************************************************/

/*
 * Video Buffer functions
 */

MP_RESULT MediaVideoBuffer::StartBuffering(unsigned int stride)
{
	DWORD i;

	this->stride = stride;

	if(this->enabled) {

		if(this->bufferingThread != NULL)
			return MP_RESULT_ERROR;

		this->bufferedFrames = 0;
		this->suspended      = 0;

		CloseHandle(this->bufferingMutex);
		this->bufferingMutex = CreateMutex(NULL, FALSE, NULL);

		/*
		 * Wait for the buffer to fill at half
		 */

		for(i=0; i<VIDEO_BUFFER_SIZE; i++) {

			if(this->decoder->Decompress(this->buffer[this->bufferedFrames], 0) != MP_RESULT_OK) {
			
				this->endReachedAt = this->bufferedFrames;
				return MP_RESULT_ERROR;
			}

			this->bufferedFrames++;
		}

		/*
		 * And launch the thread
		 */

		this->bufferingThread = CreateThread(NULL, 0, BufferingThreadFunc, (LPVOID) this, 0, &this->bufferingThreadId);
		SetThreadPriority(this->bufferingThread, THREAD_PRIORITY_ABOVE_NORMAL);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoBuffer::StopBuffering()
{
	if(this->enabled) {
		
		WaitForSingleObject(this->bufferingMutex, INFINITE);

		TerminateThread(this->bufferingThread, 0);
		this->bufferingThread = NULL;

		ReleaseMutex(this->bufferingMutex);
	
		this->bufferedFrames = 0;
		this->endReachedAt   = VIDEO_BUFFER_END_NOT_REACHED;
	}

	return MP_RESULT_OK;
}

MediaBuffer *MediaVideoBuffer::GetOneFrame()
{
	DWORD i;
	MediaBuffer *temp;

	if(this->enabled) {

		if(this->endReachedAt == 0)
			return NULL;
	
		WaitForSingleObject(this->bufferingMutex, INFINITE);

		if(this->bufferedFrames > 1) {

			if(this->endReachedAt < VIDEO_BUFFER_END_NOT_REACHED) {

				this->endReachedAt--;
			}

			/*
			 * Check if we're at the end
			 */

			if(this->endReachedAt == 0) {
		
				ReleaseMutex(this->bufferingMutex);
				return NULL;
			}

			temp = this->buffer[0];
	
			for(i=0; i < VIDEO_BUFFER_SIZE - 1; i++) {
		
				this->buffer[i] = this->buffer[i+1];
			}
		
			this->buffer[VIDEO_BUFFER_SIZE - 1] = temp;
	
			this->bufferedFrames--;

			ReleaseMutex(this->bufferingMutex);

			if(this->suspended && this->endReachedAt == VIDEO_BUFFER_END_NOT_REACHED) {

				this->suspended = FALSE;
				ResumeThread(this->bufferingThread);
			}

			return this->buffer[0];
		}
		else {

			/*
			 * We need to decompress it ourself
			 */
		
			if(this->decoder->Decompress(this->buffer[this->bufferedFrames], 0) != MP_RESULT_OK) {
			
				this->endReachedAt = 0;
		
				this->suspended = TRUE;
				SuspendThread(this->bufferingThread);
	
				ReleaseMutex(this->bufferingMutex);
				return NULL;
			}

			if(this->bufferedFrames == 1)
				this->bufferedFrames--;

			temp = this->buffer[0];

				for(i=0; i < VIDEO_BUFFER_SIZE - 1; i++) {
	
				this->buffer[i] = this->buffer[i+1];
			}
		
			this->buffer[VIDEO_BUFFER_SIZE - 1] = temp;
	
			ReleaseMutex(this->bufferingMutex);	

			if(this->suspended && this->endReachedAt == VIDEO_BUFFER_END_NOT_REACHED) {
		
				this->suspended = FALSE;
				ResumeThread(this->bufferingThread);
			}

			return temp;
		}
	}
	else {

		if(this->decoder->Decompress(this->buffer[0], this->stride) != MP_RESULT_OK) {
	
			return NULL;
		}

		return this->buffer[0];
	}
}

MediaBuffer  *MediaVideoBuffer::GetLastFrame()
{
	if(this->decoder) {

		return this->buffer[0];
	}

	return NULL;
}


MP_RESULT MediaVideoBuffer::DropOneFrame()
{
	if(this->decoder) {

		WaitForSingleObject(this->bufferingMutex, INFINITE);

		this->decoder->Drop(this->buffer[0], this->stride);
		
		ReleaseMutex(this->bufferingMutex);
	}

	return MP_RESULT_OK;
}
