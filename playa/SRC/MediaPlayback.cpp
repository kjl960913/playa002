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

#include "MediaPlayback.h"
#include "DebugFile.h"

DebugFile *debug;

/*
 * Audio Callback
 *
 */

unsigned int PlaybackAudioCallback(void *lpData, void *buffer, unsigned int size);

/*
 * Playback Class 
 * --------------
 * 
 * - Offers a frontend to all the
 *   mediaclasses items and filters
 *
 */

MediaPlayback::MediaPlayback()
{
	this->input         = new MediaInput();
	this->decaps        = new MediaDecaps();
	this->videoDecoder  = new MediaVideoDecoder();
	this->videoBuffer   = new MediaVideoBuffer();
	this->videoRenderer = new MediaVideoRenderer();

	this->audioDecoder  = new MediaAudioDecoder();
	this->audioRenderer = new MediaAudioRenderer();

	this->subtitler     = new MediaSubtitler();

	this->hasVideo      = FALSE;
	this->hasAudio      = FALSE;
	this->hasSubtitles  = FALSE;

	this->fullscreen    = FALSE;
	this->fastForward   = FALSE;
	this->rewind        = FALSE;

	this->playing       = FALSE;  
	this->paused        = FALSE;

	this->buffering         = FALSE;
	this->bufferingProgress = 0;
	this->volume            = 100;
	
	this->playbackMutex = CreateMutex (NULL, FALSE, NULL);
}

MediaPlayback::~MediaPlayback()
{
	delete this->input;
	delete this->decaps;
	delete this->videoDecoder;
	delete this->videoRenderer;
	delete this->videoBuffer;
	delete this->subtitler;

	CloseHandle(this->playbackMutex);
}

MP_RESULT     MediaPlayback::OpenMediaSource(char *lpFilename) 
{
	this->hasVideo      = FALSE;

	this->fastForward   = FALSE;
	this->rewind        = FALSE;

	this->playing       = FALSE;  
	this->paused        = FALSE;

	this->hasVideo      = FALSE;
	this->hasAudio      = FALSE;
	this->hasSubtitles  = FALSE;

	this->buffering     = FALSE;
	this->hasToBuffer   = FALSE;
	this->bufferingProgress = 0;

	if(lpFilename) {

		if(this->input->Open(lpFilename, INPUT_OPEN_BINARY) == MP_RESULT_OK) {

			if(this->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

				this->buffering         = TRUE;
				this->bufferingProgress = 0;
			}

			strcpy(this->filename, lpFilename);
			this->filename[strlen(lpFilename)] = '\0';

			return MP_RESULT_OK;
		}
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaPlayback::OpenMediaFromSource(HWND hwndPlayback)
{
	if(this->input && hwndPlayback) {

		if(this->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

			this->buffering = 0;
		}

		
		if(this->decaps->Connect(this->input) == MP_RESULT_OK) {

			/*
			 * Video Streams
			 */

			if(this->decaps->GetNumberOfVideoStreams() > 0) {
					
			if(this->videoDecoder->Connect(decaps) == MP_RESULT_OK) {

					/*
					 * We have a valid decoder so now we
					 * try to setup a video renderer and
					 * to get the same video mode...
					 *
					 */

					media_video_mode_t codecMode, rendererMode;

					codecMode = this->videoDecoder->GetVideoMode();

					if(this->fullscreen)  {

						if(this->videoRenderer->InitFullscreen(hwndPlayback, 
															   this->decaps->GetVideoWidth(0),
															   this->decaps->GetVideoHeight(0), 
															   codecMode) == MP_RESULT_OK) {
							
							rendererMode = this->videoRenderer->GetVideoMode();

							if(this->videoDecoder->SetVideoMode(rendererMode) == MP_RESULT_OK) {
	
								if(this->videoBuffer->Connect(this->videoDecoder) == MP_RESULT_OK) {
									
									this->hasVideo     = TRUE;
									this->hwndPlayback = hwndPlayback;
	
									this->end = FALSE;
	
									this->videoRect    = NULL;
								}
							}

						}
					}
					else {

						if(this->videoRenderer->Init(hwndPlayback, 
													 this->decaps->GetVideoWidth(0),
													 this->decaps->GetVideoHeight(0), 
													 codecMode) == MP_RESULT_OK) {
							
							rendererMode = this->videoRenderer->GetVideoMode();

							if(this->videoDecoder->SetVideoMode(rendererMode) == MP_RESULT_OK) {
	
								if(this->videoBuffer->Connect(this->videoDecoder) == MP_RESULT_OK) {
									
									this->hasVideo     = TRUE;
									this->hwndPlayback = hwndPlayback;
	
									this->end = FALSE;
	
									this->videoRect    = NULL;
								}
							}
						}
					}
				}
			}

			/*
			 * Audio Streams
			 */

			if(this->decaps->GetNumberOfAudioStreams() > 0) {

				if(this->audioDecoder->Connect(this->decaps) == MP_RESULT_OK) {

					/*
					 * We have a valid audio decompressor
					 * Now we try to setup the default
					 * audio renderer in the given mode
					 *
					 */

					if(this->audioRenderer->Open(hwndPlayback, this->audioDecoder->GetAudioFormat()) == MP_RESULT_OK) {

						/*
						 * Setup the callback
						 */

						this->audioRenderer->SetVolume(this->volume);
						this->audioRenderer->SetCallback((LPVOID) this, PlaybackAudioCallback);

						this->hasAudio = TRUE;
					}
				}
			}

			/*
			 * Subtitles Streams
			 */

			if(this->HasVideo() && !(this->input->GetCaps() & MEDIA_CAPS_BUFFERIZE)) {

				char *subFilename;

				subFilename = (char *) new char[strlen(this->filename)];

				strncpy(subFilename, this->filename, strlen(this->filename) - 4);
				subFilename[strlen(this->filename) - 4] = '\0';
				strcat(subFilename, ".sub");
	
				if(this->subtitler->Open(subFilename) == MP_RESULT_OK) {
	
					if(this->videoRenderer->Connect(this->subtitler) == MP_RESULT_OK) {

						this->hasSubtitles = TRUE;
					}
				}
			}
			

			if(this->hasVideo) {
					
				return MP_RESULT_OK;
			}
		}
	}

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaPlayback::OpenMedia(char *lpFilename, HWND hwndPlayback)
{
	if(lpFilename && hwndPlayback) {

		if(this->OpenMediaSource(lpFilename) == MP_RESULT_OK) {

			if(this->OpenMediaFromSource(hwndPlayback) == MP_RESULT_OK) {

				return MP_RESULT_OK;
			}
		}
	}

	this->Close();

	return MP_RESULT_ERROR;
}

BOOL          MediaPlayback::HasVideo()
{
	return this->hasVideo;
}

BOOL          MediaPlayback::HasAudio()
{
	return this->hasAudio;
}

char         *MediaPlayback::GetFilename()
{	
	if(this->HasAudio() || this->HasVideo()) {

		return this->filename;
	}

	return NULL;
}

unsigned int  MediaPlayback::GetVideoWidth()
{
	if(this->decaps)
		return this->decaps->GetVideoWidth(0);
	
	return 0;
}

unsigned int  MediaPlayback::GetVideoHeight()
{
	if(this->decaps)
		return this->decaps->GetVideoHeight(0);
	
	return 0;
}

unsigned long MediaPlayback::GetVideoTime()
{
	if(this->decaps) {

		return (unsigned long) ((double) this->videoFrames * 1000.0 / (double) this->decaps->GetVideoFrameRate(0));
	}

	return 0;
}

unsigned long MediaPlayback::GetAudioTime()
{
	return this->audioRenderer->GetAudioTime();
}

double        MediaPlayback::GetPlaybackProgress()
{
	if(this->HasAudio() || this->HasVideo()) {

		return (double) ((double) this->decaps->GetCurrentVideoFrame(0) * 100 / (double) this->decaps->GetTotalVideoFrames(0));
	}

	return 0;
}

unsigned int  MediaPlayback::GetActualTime()
{
	if(this->HasVideo()) {
	
		return this->decaps->GetCurrentVideoFrame(0) / this->decaps->GetVideoFrameRate(0);
	}

	return 0;
}

BOOL          MediaPlayback::IsBuffering()
{
	return this->buffering;
}

DWORD         MediaPlayback::GetBufferingProgress()
{
	return this->bufferingProgress;
}

MP_RESULT     MediaPlayback::UpdateBuffering()
{
	DWORD size;

	size = this->input->GetBufferSize();
						
	this->bufferingProgress = size * 100 / this->input->GetBufferingSize();

	return MP_RESULT_OK;
}

unsigned int  MediaPlayback::GetTotalTime()
{
	if(this->HasVideo()) {

		return (unsigned int) ( (double) this->decaps->GetTotalVideoFrames(0)/ (double) this->decaps->GetVideoFrameRate(0));
	}

	return 0;
}

double        MediaPlayback::GetCurrentFps()
{
	return 0;
}

BOOL          MediaPlayback::IsPaused()
{
	return (this->paused || this->buffering);
}

BOOL          MediaPlayback::IsPlaying()
{
	return this->playing;
}

BOOL          MediaPlayback::IsInFullscreen()
{
	return this->fullscreen;
}

BOOL          MediaPlayback::IsOverlay()
{
	if(this->videoRenderer) {

		return (strstr(this->videoRenderer->GetName(), "Overlay") != NULL);
	}

	return FALSE;
}


/**************************************************************
 *                                                            *
 *                 AUDIO CALLBACK FUNCTION                    *
 *                 -----------------------                    *
 *                                                            *
 **************************************************************/

unsigned int PlaybackAudioCallback(void *lpData, void *buffer, unsigned int size)
{
	int real_size;
	
	MediaPlayback *playback = (MediaPlayback *) lpData;

	if(playback && playback->playing && !playback->paused) {

		playback->audioBytes += size;

		if(playback->audioDecoder) {

			if((real_size = playback->audioDecoder->Decompress(buffer, size)) != size) {
			
				if(playback->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {
					 
					if(!(playback->input->GetBufferSize() >= playback->input->GetSize())) {

						playback->hasToBuffer = TRUE;
				
						return real_size;
					}
					else {

						if(!playback->end) {
					
							playback->end = TRUE;

							playback->Pause();
							playback->decaps->UpdateForSize();
							playback->Pause();

							return real_size;
						}
						else {
				
							memset(buffer, 0, size);
							return size;
						}
					}
			}
			else {

					memset(buffer, 0, size);
					return size;
				}
			}

			return size;
		}
	}
	else {

		memset(buffer, 0, size);
		return size;
	}

	return 0;
}

/**************************************************************
 *                                                            *
 *                 VIDEO THREADS FUNCTIONS                    *
 *                 -----------------------                    *
 *                                                            *
 **************************************************************/

/*
 * VIDEO ONLY THREAD 
 *
 */

DWORD WINAPI PlaybackVideoOnlyThreadFunc( LPVOID lpData)
{
	MediaPlayback *playback = (MediaPlayback *) lpData;
	int timeDiff;
	MediaBuffer *frame = NULL;
		
	if(playback != NULL) {

		playback->baseTime = GetTickCount();

		while(playback->playing) {

			/*
			 * Check if we're buffering
			 *
			 */


			if(playback->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

				if((playback->input->GetBufferPosition() > 0.95*playback->input->GetBufferSize()) && 
				  !(playback->input->GetBufferSize() >= playback->input->GetSize())) {

startBuffering:

					DWORD size  = playback->input->GetBufferSize();
					DWORD sizeI = playback->input->GetBufferSize();

					/*
					 * We wait for some more buffering
					 */

					playback->bufferingProgress = 0;
					playback->buffering = TRUE;

					while(size < sizeI + playback->input->GetBufferingSize() && !(playback->input->GetBufferSize() >= playback->input->GetSize())) {

						Sleep(10);
						size = playback->input->GetBufferSize();
						
						playback->bufferingProgress = (size - sizeI) * 100 / playback->input->GetBufferingSize();
					}

					playback->buffering = FALSE;

					playback->decaps->UpdateForSize();
					playback->baseTime = GetTickCount();
					playback->videoFrames = 0;

					RECT rect;

					GetClientRect(playback->hwndPlayback, &rect);
					InvalidateRect(playback->hwndPlayback, &rect, TRUE); 
	 				UpdateWindow(playback->hwndPlayback);
				}
			}

startPlaying:

			frame = NULL;

			if(!playback->paused) {

				if(playback->fastForward) {

					playback->decaps->SeekNextKeyFrame(0);
				}
				else {
					if(playback->rewind) {
	
						playback->decaps->SeekPreviousKeyFrame(0);
					}
				}
	
				/*
				 * synchro here
				 *
				 */

				if(!playback->fastForward && !playback->rewind) {

					timeDiff = playback->GetVideoTime() - (GetTickCount() - playback->baseTime);

					if(timeDiff > 10) {

						Sleep(timeDiff/2);
					}

					if(timeDiff < -150) {
					
						/*
						 * drop here
						 */	
					
						playback->videoBuffer->DropOneFrame();
						playback->videoFrames++;
					}

					if(timeDiff < -230) {
					
						/*
						 * drop here
						 */	
						
						playback->videoBuffer->DropOneFrame();
						playback->videoFrames++;
					}
				}
				else {

					Sleep(80);
				}


				frame = playback->videoBuffer->GetOneFrame();
				playback->videoFrames++;

				if(frame == NULL) {

					if(playback->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

						if(playback->input->GetBufferSize() < playback->input->GetSize()) {
						
							goto startBuffering;
						}
					}

					if(!playback->loop) {

						SendMessage(playback->hwndPlayback, WM_PLAYA_PLAYBACK_END, 0, 0);
						playback->Stop(TRUE);
					
					}
					else {

						playback->decaps->Rewind(0, 0);
						
						playback->videoBuffer->StopBuffering();
						playback->videoBuffer->StartBuffering(playback->decaps->GetVideoWidth(0));

						playback->videoFrames = 0;
						playback->baseTime    = GetTickCount();
						
						goto startPlaying;
					}

					break;
				}

				WaitForSingleObject(playback->playbackMutex, INFINITE);

				if(!playback->paused) {

				    if(playback->fullscreen) {
						
						playback->videoRenderer->DrawFullscreen(frame, playback->decaps->GetCurrentVideoFrame(0), 
																playback->videoDecoder->GetInvertFlag(), playback->desktopMode);
					}
					else {
						playback->videoRenderer->Draw(frame, playback->videoRect, playback->decaps->GetCurrentVideoFrame(0), 
													  playback->videoDecoder->GetInvertFlag());
					}
				}

				ReleaseMutex(playback->playbackMutex);				

			}
		}
	}
	else {

		MessageBox(playback->hwndPlayback, "Playing : NULL playback engine!", "", MB_OK);
	}

	return 0;
}

/*
 * Video Synched on Audio Thread
 *
 */

DWORD WINAPI PlaybackVideoAndAudioThreadFunc( LPVOID lpData)
{
	MediaPlayback *playback = (MediaPlayback *) lpData;
	long  timeDiff;
	MediaBuffer *frame = NULL;

	if(playback != NULL)
	{
		while(playback->playing) {

			if(playback->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

				if(((playback->input->GetBufferPosition() > 0.95*playback->input->GetBufferSize()) && 
				  !(playback->input->GetBufferSize() >= playback->input->GetSize())) || playback->hasToBuffer) {

startBufferingWAudio:

					if(!playback->audioRenderer->paused)
							playback->audioRenderer->Pause();

					if(playback->input->GetBufferSize() >= playback->input->GetSize()) {
	
						playback->decaps->UpdateForSize();

						if(playback->audioRenderer->paused)
							playback->audioRenderer->Pause();

						goto startPlayingWAudio;
					}

					DWORD size  = playback->input->GetBufferSize();
					DWORD sizeI = playback->input->GetBufferSize();

					/*
					 * We wait for some more buffering
					 */

					playback->bufferingProgress = 0;
					playback->buffering = TRUE;

					while(size < sizeI + playback->input->GetBufferingSize() && !(playback->input->GetBufferSize() >= playback->input->GetSize())) {

						Sleep(10);
						size = playback->input->GetBufferSize();
						
						playback->bufferingProgress = (size - sizeI) * 100 / playback->input->GetBufferingSize();
					}

					playback->buffering   = FALSE;
					playback->hasToBuffer = FALSE;

					playback->decaps->UpdateForSize();

					RECT rect;

					GetClientRect(playback->hwndPlayback, &rect);
					InvalidateRect(playback->hwndPlayback, &rect, TRUE); 
	 				UpdateWindow(playback->hwndPlayback);
		
					if(playback->audioRenderer->paused)
						playback->audioRenderer->Pause();
				}
			}

startPlayingWAudio:

			frame = NULL;

			if(!playback->paused) {

				if(playback->fastForward) {

					playback->decaps->SeekNextKeyFrame(0);
				}
				else {
					if(playback->rewind) {
	
						playback->decaps->SeekPreviousKeyFrame(0);
					}
				}

				/*
				 * synchro here
				 *
				 */

				if(!playback->fastForward && !playback->rewind) {

					timeDiff = playback->GetVideoTime() - playback->GetAudioTime();



					if(timeDiff == 0) {
					
						/*
						 * Nothing to do
						 */
					}
					else {

						if(timeDiff > 10) {
							Sleep(timeDiff/2);
						}

						if(timeDiff < -150) {
					
							/*
							 * drop here
							 */	
					
							playback->videoBuffer->DropOneFrame();
							playback->videoFrames++;
						}


						if(timeDiff < -230) {
						
							/*
							 * drop here
							 */	
							
							playback->videoBuffer->DropOneFrame();
							playback->videoFrames++;
						}
					}
				}
				else {

					Sleep(120);
				}

				frame = playback->videoBuffer->GetOneFrame();
				playback->videoFrames++;

				if(frame == NULL) {

					/*
					 * We're at the end of the file
					 * or at the end of the buffering...
					 */

					if(playback->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

						if(playback->input->GetBufferSize() < playback->input->GetSize()) {

							goto startBufferingWAudio;
						}
						else {

							if(!playback->audioRenderer->paused)
								playback->audioRenderer->Pause();

							playback->decaps->UpdateForSize();

							frame = playback->videoBuffer->GetOneFrame();

							if(playback->audioRenderer->paused)
								playback->audioRenderer->Pause();
						}
					}
				}

				if(frame == NULL) {

					if(!playback->loop) {
						
						SendMessage(playback->hwndPlayback, WM_PLAYA_PLAYBACK_END, 0, 0);
						playback->Stop(TRUE);
					}
					else {

						playback->decaps->Rewind(0, 0);
						
						if(!playback->fastForward && !playback->rewind) {
		
							if(playback->hasAudio)
								playback->audioRenderer->Stop();
	
							if(playback->hasAudio)
								playback->audioDecoder->EmptyAudioBuffer();
						}

						playback->audioBytes      = 0;
						playback->videoFrames     = 0;
						
						playback->videoBuffer->StopBuffering();
						playback->videoBuffer->StartBuffering(playback->decaps->GetVideoWidth(0));
					
						if(!playback->fastForward && !playback->rewind) {

							if(playback->hasAudio)
								playback->audioRenderer->Start();
						}
						
						goto startPlayingWAudio;
					}

					break;
				}

				WaitForSingleObject(playback->playbackMutex, INFINITE);

				if(!playback->paused) {
					
					if(playback->fullscreen) {
						
						playback->videoRenderer->DrawFullscreen(frame, playback->decaps->GetCurrentVideoFrame(0), playback->videoDecoder->GetInvertFlag(), playback->desktopMode);
					}
					else {
						
						playback->videoRenderer->Draw(frame, playback->videoRect, playback->decaps->GetCurrentVideoFrame(0), playback->videoDecoder->GetInvertFlag());
					}
				}

				ReleaseMutex(playback->playbackMutex);
			}
		}
	}

	return 0;
}

/*
 * GDI Function
 *
 */

MP_RESULT     MediaPlayback::FlipToGDI() 
{
	return MP_RESULT_OK;
}


/**************************************************************
 *                                                            *
 *                 VIDEO PLAYBACK FUNCTIONS                   *
 *                 ------------------------                   *
 *                                                            *
 **************************************************************/


MP_RESULT     MediaPlayback::Play()
{
	if(this->HasAudio() || this->HasVideo()) {

		if(this->fastForward) {
	
			this->fastForward = FALSE;

			this->baseTime = GetTickCount();

			this->videoFrames = 0;
			this->audioBytes  = 0;

			if(this->decaps) {
			
				this->decaps->ReSeekAudio(0);
			}

			if(this->hasAudio) {

				this->audioDecoder->EmptyAudioBuffer();
			
				this->audioRenderer->SetVolume(this->volume);

				this->audioRenderer->Stop();
				this->audioRenderer->Start();
			}
		}
		else {

			if(this->rewind) {
	
				this->rewind  = FALSE;

				this->baseTime = GetTickCount();

				this->videoFrames     = 0;
				this->audioBytes      = 0;

				if(this->decaps) {
				
					this->decaps->ReSeekAudio(0);
				}

				if(this->hasAudio) {

					this->audioDecoder->EmptyAudioBuffer();
			
					this->audioRenderer->SetVolume(this->volume);
					this->audioRenderer->Stop();
					this->audioRenderer->Start();
				}
			}
			else {

				if(this->paused) {

					this->Pause();
					return MP_RESULT_OK;
				}

				if(this->playing)
					return MP_RESULT_ERROR;

				this->playing = TRUE;
				this->paused  = FALSE;

				this->videoFrames = 0;
				this->audioBytes  = 0;

				this->decaps->Rewind(0, 0);

				if(this->HasAudio()) {
			
					this->audioDecoder->EmptyAudioBuffer();

					this->audioRenderer->SetVolume(this->volume);

					this->videoBuffer->StartBuffering(this->decaps->GetVideoWidth(0));

					if(this->audioRenderer->paused && (this->input->GetCaps() & MEDIA_CAPS_BUFFERIZE)) {

						this->audioRenderer->Pause();
						this->audioRenderer->Stop();
						this->audioRenderer->Start();
					}
					else {
						this->audioRenderer->Start();
					}

					this->videoThread = CreateThread( NULL, 0, PlaybackVideoAndAudioThreadFunc, (LPVOID) this, 0, &this->videoThreadId );
		
					return MP_RESULT_OK;
				}
				else {
				
					if(this->HasVideo()) {

						this->videoBuffer->StartBuffering(this->decaps->GetVideoWidth(0));
	
						this->videoThread = CreateThread( NULL, 0, PlaybackVideoOnlyThreadFunc, (LPVOID) this, 0, &this->videoThreadId );

						return MP_RESULT_OK;
					}
				}
			}
		}
	}

	return MP_RESULT_ERROR;	
}

MP_RESULT     MediaPlayback::Pause()
{
	if(this->playing) {

		if(this->paused) {

			if(this->hasAudio) {
				this->audioRenderer->Pause();
			}

			this->paused = FALSE;

			ReleaseMutex(this->playbackMutex);

			this->baseTime += (GetTickCount() - this->stopTime);

			ResumeThread(this->videoThread);
		}
		else {
		
			WaitForSingleObject(this->playbackMutex, INFINITE);

			if(!this->fastForward && !this->rewind) {
				
				if(this->hasAudio) {
					this->audioRenderer->Pause();
				}
			}
			else {
				
				this->videoFrames     = 0;

				if(this->hasAudio) {
					
					this->decaps->ReSeekAudio(0);
					this->audioDecoder->EmptyAudioBuffer();
				
					this->audioBytes     = 0;

					this->audioRenderer->Stop();
					this->audioRenderer->Start();
					this->audioRenderer->Pause();
				}
			}

			this->fastForward = FALSE;
			this->rewind      = FALSE;

			this->stopTime = GetTickCount();

			SuspendThread(this->videoThread);
			this->paused = TRUE;
		}
	}

	return MP_RESULT_ERROR;	
}

MP_RESULT     MediaPlayback::NextFrame()
{
	return MP_RESULT_ERROR;	
}

MP_RESULT     MediaPlayback::Stop(int redrawWindow)
{
	if(this->paused)
		this->Pause();

	if(this->playing) {

		WaitForSingleObject(this->playbackMutex, INFINITE);	
		
		if(this->hasAudio) {

			this->audioRenderer->Stop();
		}

		this->playing = FALSE;

		this->buffering = FALSE;
		this->bufferingProgress = 0;

		if(this->HasVideo()) {

			this->videoBuffer->StopBuffering();
		}

		ReleaseMutex(this->playbackMutex);
	}

	this->fastForward  = FALSE;
	this->rewind       = FALSE;

	this->playing      = FALSE;
	this->paused       = FALSE;

	this->audioBytes      = 0;
	this->videoFrames     = 0;

	this->decaps->Rewind(0, 0);

	if(this->hasAudio)
		this->audioDecoder->EmptyAudioBuffer();

	if(this->videoRenderer)
		this->videoRenderer->Stop();

	/*
	 * Do redraw here!
	 */

	RECT rect;

	if(redrawWindow) {
		
		GetClientRect(this->hwndPlayback, &rect);
		InvalidateRect(this->hwndPlayback, &rect, TRUE); 
	 	UpdateWindow(this->hwndPlayback);
	}

	TerminateThread(this->videoThread, 0);

	return MP_RESULT_ERROR;	
}

MP_RESULT     MediaPlayback::FastForward()
{
	if(this->paused) {

		this->fastForward = TRUE;
		this->paused      = FALSE;

		ReleaseMutex(this->playbackMutex);
		this->baseTime += (GetTickCount() - this->stopTime);

		ResumeThread(this->videoThread);

		return MP_RESULT_OK;
	}

	if(this->playing && !this->rewind && !this->fastForward) {

		if(this->hasAudio) {
			this->audioRenderer->Stop();
		}

		this->stopTime = GetTickCount();

		this->fastForward = TRUE;
	}

	return MP_RESULT_OK;	
}

MP_RESULT     MediaPlayback::Rewind()
{
	if(this->paused) {

		this->rewind = TRUE;
		this->paused = FALSE;

		ReleaseMutex(this->playbackMutex);
		this->baseTime += (GetTickCount() - this->stopTime);

		ResumeThread(this->videoThread);

		return MP_RESULT_OK;
	}

	if(this->playing && !this->fastForward && !this->rewind) {

		if(this->hasAudio) {
			this->audioRenderer->Stop();
		}

		this->stopTime = GetTickCount();

		this->rewind = TRUE;
	}


	return MP_RESULT_OK;	
}

MP_RESULT     MediaPlayback::MaintainImage()
{
	if(this->HasVideo()) {

		if(this->fullscreen) {

			this->videoRenderer->DrawFullscreen(this->videoBuffer->GetLastFrame(), 0, this->videoDecoder->GetInvertFlag(), this->desktopMode);
		}
		else {
			
			this->videoRenderer->Draw(this->videoBuffer->GetLastFrame(), this->videoRect, 0, this->videoDecoder->GetInvertFlag());
		}
	}

	return MP_RESULT_OK;
}

MP_RESULT     MediaPlayback::Seek(int percent)
{
	int has_to_play = 0;
	RECT rect;
	
	if(this->HasAudio() || this->HasVideo()) {
	
		if(this->playing) {

			Stop(FALSE);
			has_to_play = 1;
		}

		this->decaps->Seek(0, 0, percent);

		if(this->input->GetCaps() & MEDIA_CAPS_BUFFERIZE) {

			this->decaps->UpdateForSize();
		}

		if(this->hasAudio)
			this->audioDecoder->EmptyAudioBuffer();

		if(has_to_play) {
	
			this->playing = TRUE;
			this->paused  = FALSE;

			this->videoFrames     = 0;
			this->audioBytes      = 0;

			if(this->HasAudio()) {
			
				this->audioRenderer->Start();

				this->videoThread = CreateThread( NULL, 0, PlaybackVideoAndAudioThreadFunc, (LPVOID) this, 0, &this->videoThreadId );
			}
			else {
				
				this->videoThread = CreateThread( NULL, 0, PlaybackVideoOnlyThreadFunc, (LPVOID) this, 0, &this->videoThreadId );
			}
		}

		GetClientRect(this->hwndPlayback, &rect);
		InvalidateRect(this->hwndPlayback, &rect, TRUE); 
	 	UpdateWindow(this->hwndPlayback);

	}
	
	return MP_RESULT_ERROR;	
}

MP_RESULT     MediaPlayback::SetFullscreen(int active, HWND hwnd)
{
	if(active && !this->fullscreen) {

		int has_to_play = 0;

		if(this->playing && !this->paused) {
			
			this->Pause();
			has_to_play = 1;
		}
		
		this->fullscreen = TRUE;

		if(this->videoRenderer->InitFullscreen(hwnd, this->decaps->GetVideoWidth(0), this->decaps->GetVideoHeight(0)) != MP_RESULT_OK) {
		
			this->videoRenderer->Init(hwnd, this->decaps->GetVideoWidth(0), this->decaps->GetVideoHeight(0));
			this->fullscreen = FALSE;

			return MP_RESULT_ERROR;
		}
			
		if(has_to_play) {

			this->Pause();
		}
	}
	else {
	
	if(this->fullscreen) {
	
			int has_to_play = 0;

			if(this->playing && !this->paused) {
			
				this->Pause();
				has_to_play = 1;
			}

			this->videoRenderer->Init(hwnd, this->decaps->GetVideoWidth(0), this->decaps->GetVideoHeight(0));
			this->fullscreen = FALSE;
		
			if(has_to_play) {

				this->Pause();
			}
		}
	}
	
	return MP_RESULT_ERROR;	
}

MP_RESULT     MediaPlayback::SetVolume(int volume)
{
	this->volume = volume;

	if(this->hasAudio) {

		this->audioRenderer->SetVolume(this->volume);
	}

	return MP_RESULT_OK;	
}

MP_RESULT     MediaPlayback::SetLoop(int loop)
{
	this->loop = loop;

	return MP_RESULT_OK;	
}

MP_RESULT     MediaPlayback::SetDesktopMode(BOOL on)
{
	this->desktopMode = on;

	return MP_RESULT_OK;
}

MP_RESULT     MediaPlayback::SetVideoRect(RECT *rect)
{
	this->videoRect = rect;

	return MP_RESULT_OK;
}

MP_RESULT     MediaPlayback::Close()
{
	if(this->input)
		this->input->Close();

	if(this->HasVideo()) {

		/*
		 * Do video Cleanup
		 *
		 */

		this->Stop(FALSE);

		this->videoRenderer->Close();

		this->videoBuffer->StopBuffering();

		this->videoBuffer->ReleaseConnections();
		this->videoDecoder->ReleaseConnections();
		this->decaps->ReleaseConnections();

	}

	if(this->HasAudio()) {

		/*
		 * Do audio Cleanup
		 *
		 */

		this->audioRenderer->Close();
		this->audioDecoder->ReleaseConnections();
	}

	if(this->hasSubtitles) {

		this->subtitler->ReleaseConnections();
		this->subtitler->Close();
	}

	return MP_RESULT_OK;	
}

