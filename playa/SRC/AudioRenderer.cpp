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

#include "AudioRenderer.h"

/*
 * Audio Renderer Class
 *
 * - Only one available for the moment
 *
 */


#define PERIOD 30


MediaAudioRenderer::MediaAudioRenderer()
{
	this->lpDirectSound = NULL;
	this->lpBuffer      = NULL;
}

MediaAudioRenderer::~MediaAudioRenderer()
{
}

/*
 * Media Item functions
 */

media_type_t  MediaAudioRenderer::GetType()
{
	return MEDIA_TYPE_AUDIO_RENDERER;
}

char         *MediaAudioRenderer::GetName()
{
	return "DirectSound Audio Renderer";
}

/*
 * We accept nothing
 *
 */
	
MP_RESULT     MediaAudioRenderer::Connect(MediaItem *item)
{
	return MP_RESULT_ERROR;
}

MP_RESULT     MediaAudioRenderer::ReleaseConnections()
{
	return MP_RESULT_ERROR;
}

DWORD         MediaAudioRenderer::GetCaps()
{
	return 0;
}

MP_RESULT     MediaAudioRenderer::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Open
 */

MP_RESULT     MediaAudioRenderer::Open(HWND hwnd, WAVEFORMATEX *inFormat)
{
	if(hwnd && inFormat) {

		this->ourFormat = inFormat;

		HRESULT hr;
			
		if(DirectSoundCreate(NULL, &this->lpDirectSound, NULL) == DS_OK) {

			/*
			 * Set cooperative mode
			 */

			hr = lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_NORMAL);

			if(hr != DS_OK) {

				this->lpDirectSound->Release();
				this->lpDirectSound = NULL;

				return MP_RESULT_ERROR;
			}

			/*
			 * Create the buffer
			 */

			DSBUFFERDESC  dsbdesc;
			WAVEFORMATEX  wf;
			
			memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
			dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    
			ZeroMemory(&wf, sizeof(WAVEFORMATEX));
			memcpy(&wf, this->ourFormat, sizeof(WAVEFORMATEX));

			dsbdesc.lpwfxFormat = (LPWAVEFORMATEX) &wf;
    
			dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;			
			dsbdesc.dwBufferBytes = 2 * this->ourFormat->nAvgBytesPerSec;

			hr = lpDirectSound->CreateSoundBuffer(&dsbdesc, &lpBuffer, NULL);

			if(hr != DS_OK) {

				this->lpDirectSound->Release();
				this->lpDirectSound = NULL;

				return MP_RESULT_ERROR;
			}
		
			/*
			 * here we're ok!
			 */

			this->dwBufferSize = 2 * this->ourFormat->nAvgBytesPerSec;
			this->lInTimer = FALSE;
		
			return MP_RESULT_OK;
		}
	}

	return MP_RESULT_ERROR;
}


/*
 * Audio Renderer Functions
 */

MP_RESULT     MediaAudioRenderer::SetCallback(void *lpData, MediaAudioCallback callback)
{
	this->lpData   = lpData;
	this->callback = callback;

	return MP_RESULT_OK;
}

MP_RESULT     MediaAudioRenderer::SetVolume(unsigned int volume)
{
	if(this->lpBuffer) {

		double vol;

		vol = - ( 70*pow(10, (100.0 - (double) volume)*2.0/100.0) );

		if(this->lpBuffer != NULL)
			this->lpBuffer->SetVolume((int) vol);
	}

	return MP_RESULT_ERROR;
}

unsigned long MediaAudioRenderer::GetAudioTime()
{
	return (unsigned long) (( ((double) this->dwPlayed * (1000/(this->ourFormat->wBitsPerSample*this->ourFormat->nChannels/8)))) / (this->ourFormat->nSamplesPerSec));
}

/*********************************************************
 *                                                       *
 *                   AUDIO THREAD FUNCTION               *
 *                                                       *
 *********************************************************/

void CALLBACK AudioTimeFunc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	MediaAudioRenderer *audioRenderer = (MediaAudioRenderer *) dwUser;
	
	if (InterlockedExchange(&audioRenderer->lInTimer, TRUE)) return;

	if(audioRenderer->callback) {
		
			void *buffer1, *buffer2;
			DWORD bytes1 = 0, bytes2 = 0;
			DWORD dwPlay, dwWrite, dwSize;

			audioRenderer->lpBuffer->GetCurrentPosition(&dwPlay, &dwWrite);

			if(audioRenderer->dwLastPlayPos == dwPlay) {
			
				InterlockedExchange(&audioRenderer->lInTimer, FALSE);
				return;
			}

			if(audioRenderer->dwLastPlayPos <= dwPlay) {

				dwSize = dwPlay - audioRenderer->dwLastPlayPos;

				audioRenderer->lpBuffer->Lock(audioRenderer->dwLastPlayPos,
											  dwSize,
											  &buffer1, &bytes1, 
											  &buffer2, &bytes2, 0);

				if(buffer1) {

					bytes1 = audioRenderer->callback(audioRenderer->lpData, buffer1, bytes1);
				}

				audioRenderer->lpBuffer->Unlock(buffer1, bytes1, buffer2, bytes2);

				audioRenderer->dwLastPlayPos += bytes1;
				audioRenderer->dwPlayed += bytes1;
			}
			else {

				DWORD bytes1_ori;

				dwSize = (audioRenderer->dwBufferSize - audioRenderer->dwLastPlayPos) + dwPlay;

				audioRenderer->lpBuffer->Lock(audioRenderer->dwLastPlayPos,
											  dwSize,
											  &buffer1, &bytes1, 
											  &buffer2, &bytes2, 0);

				bytes1_ori = bytes1;

				if(bytes1) {

					bytes1 = audioRenderer->callback(audioRenderer->lpData, buffer1, bytes1);
				}

				if(bytes2) {

					bytes2 = audioRenderer->callback(audioRenderer->lpData, buffer2, bytes2);
				}

				audioRenderer->lpBuffer->Unlock(buffer1, bytes1, buffer2, bytes2);

				audioRenderer->dwPlayed += bytes1 + bytes2;

				if(bytes1_ori == bytes1)
					audioRenderer->dwLastPlayPos = bytes2;
				else 
					audioRenderer->dwLastPlayPos += bytes1;
			}
			
			InterlockedExchange(&audioRenderer->lInTimer, FALSE);
			return;
	}
}

/*
 * Bufferize
 */

MP_RESULT     MediaAudioRenderer::Bufferize()
{
	if(this->lpBuffer) {

		void *buffer1, *buffer2;
		DWORD bytes1, bytes2, length;

		/*
		 * First bufferise some data
		 */

		this->lpBuffer->SetCurrentPosition(0);

		this->lpBuffer->Lock(0, this->dwBufferSize,
							    &buffer1, &bytes1, &buffer2, &bytes2, 0);


		length = this->callback(this->lpData, buffer1, this->dwBufferSize);

		this->lpBuffer->Unlock(buffer1, bytes1, buffer2, bytes2);

		this->dwNextWriteOffset = this->dwProgress = this->dwPlayed = this->dwLastPlayPos = 0;
	}

	return MP_RESULT_OK;
}

MP_RESULT     MediaAudioRenderer::Start()
{
	if(this->lpBuffer) {

		void *buffer1, *buffer2;
		DWORD bytes1, bytes2, length;

		/*
		 * First bufferise some data
		 */

		this->lpBuffer->SetCurrentPosition(0);

		this->lpBuffer->Lock(0, this->dwBufferSize,
							    &buffer1, &bytes1, &buffer2, &bytes2, 0);

		/*
		 * We do several calls to detect better
		 * an end of buffer when streaming for 
		 * example...
		 */

		length = this->callback(this->lpData, buffer1, this->dwBufferSize);

		this->lpBuffer->Unlock(buffer1, length, buffer2, bytes2);

		this->dwProgress = this->dwPlayed = this->dwLastPlayPos = 0;

		/*
		 * Then start playing!
		 */

		this->paused = 0;
		
		if(this->lpBuffer->Play(0, 0, DSBPLAY_LOOPING) != DS_OK) {

			return MP_RESULT_ERROR;
		}
	
		if( timeBeginPeriod( PERIOD ) != 0 ) {
			
			return MP_RESULT_ERROR;
		}
        else {
			
			this->uTimerID = timeSetEvent( PERIOD, 5, AudioTimeFunc, (DWORD)(LPVOID)this, TIME_PERIODIC );
		}

		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaAudioRenderer::Pause()
{
	if(this->paused) {

		this->paused = 0;
		this->lpBuffer->Play(0, 0, DSBPLAY_LOOPING);
		
		this->uTimerID = timeSetEvent( PERIOD, 5, AudioTimeFunc, (DWORD)(LPVOID)this, TIME_PERIODIC );
		
	}
	else {

		if(this->uTimerID) {

			timeKillEvent(this->uTimerID); 
 		}

		this->paused = 1;
		this->lpBuffer->Stop();
	}

	return MP_RESULT_OK;
}

MP_RESULT     MediaAudioRenderer::Stop()
{
	if(this->uTimerID) {

		timeKillEvent(this->uTimerID); 
 	}

	if(this->lpBuffer) {
		this->lpBuffer->Stop();
		timeEndPeriod(PERIOD); 
	}
	
	return MP_RESULT_OK;
}

MP_RESULT     MediaAudioRenderer::Close()
{
	this->Stop();

	if(this->lpDirectSound) {

		if(this->lpBuffer)
			this->lpBuffer->Release();

		this->lpBuffer = NULL;

		this->lpDirectSound->Release();

		this->lpDirectSound = NULL;
	}

	return MP_RESULT_OK;
}