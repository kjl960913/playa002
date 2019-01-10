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

#include "VideoRenderer.h"

/*
 * VideoRenderer Wrapper Class
 */

MediaVideoRenderer::MediaVideoRenderer()
{
	this->overlayRenderer = new MediaVideoRendererOverlay();
	this->rgbRenderer     = new MediaVideoRendererRGB();

	this->renderer        = NULL;
}

MediaVideoRenderer::~MediaVideoRenderer()
{
	delete this->overlayRenderer;
	delete this->rgbRenderer;
}

/*
 * Media Item functions
 */

media_type_t  MediaVideoRenderer::GetType()
{
	return MEDIA_TYPE_VIDEO_RENDERER;
}

char         *MediaVideoRenderer::GetName()
{
	if(this->renderer)
		return this->renderer->GetName();

	return "Video Renderer Wrapper";
}

MP_RESULT     MediaVideoRenderer::Connect(MediaItem *item)
{
	if(this->renderer)
		return this->renderer->Connect(item);

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaVideoRenderer::ReleaseConnections()
{
	/*
	 * Accepts nothing
	 */

	return MP_RESULT_OK;
}

DWORD         MediaVideoRenderer::GetCaps()
{
	return 0;
}

MP_RESULT     MediaVideoRenderer::Configure(HINSTANCE hInstance, HWND hwnd)
{
	return MP_RESULT_ERROR;
}

/*
 * Video Renderer Functions
 */

MP_RESULT MediaVideoRenderer::Init(HWND hwnd, 
								   unsigned int width, 
								   unsigned int height, 
								   media_video_mode_t preferedMode)
{
	this->renderer = NULL;

	switch(preferedMode) {

	case VIDEO_MODE_RGB16:
	case VIDEO_MODE_RGB24:
	case VIDEO_MODE_RGB32:

		if(this->rgbRenderer->Init(hwnd, width, height) == MP_RESULT_OK) {
				
			this->renderer = (MediaVideoRenderer *) this->rgbRenderer;
			return MP_RESULT_OK;
		}
		else {

			return MP_RESULT_ERROR;
		}
		break;

	case VIDEO_MODE_YUV12:
	case VIDEO_MODE_UYVY:
	case VIDEO_MODE_YUY2:

		if(this->overlayRenderer->Init(hwnd, width, height) == MP_RESULT_OK) {

			this->renderer = (MediaVideoRenderer *) this->overlayRenderer;
			return MP_RESULT_OK;
		}
		else {

			/*
			 * Try the standard renderer here!
			 */

			if(this->rgbRenderer->Init(hwnd, width, height) == MP_RESULT_OK) {
	
				this->renderer = (MediaVideoRenderer *) this->rgbRenderer;
				return MP_RESULT_OK;
			}
			else {

				return MP_RESULT_ERROR;
			}
		}
		break;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRenderer::InitFullscreen(HWND hwnd, 
											 unsigned int width, 
											 unsigned int height,
											 media_video_mode_t preferedMode)
{
	this->renderer = NULL;

	switch(preferedMode) {

	case VIDEO_MODE_RGB16:
	case VIDEO_MODE_RGB24:
	case VIDEO_MODE_RGB32:

		if(this->rgbRenderer->InitFullscreen(hwnd, width, height) == MP_RESULT_OK) {
				
			this->renderer = (MediaVideoRenderer *) this->rgbRenderer;
			return MP_RESULT_OK;
		}
		else {

			return MP_RESULT_ERROR;
		}
		break;

	case VIDEO_MODE_YUV12:
	case VIDEO_MODE_UYVY:
	case VIDEO_MODE_YUY2:

		if(this->overlayRenderer->InitFullscreen(hwnd, width, height) == MP_RESULT_OK) {

			this->renderer = (MediaVideoRenderer *) this->overlayRenderer;
			return MP_RESULT_OK;
		}
		else {

			/*
			 * Try the standard renderer here!
			 */

			if(this->rgbRenderer->InitFullscreen(hwnd, width, height) == MP_RESULT_OK) {
	
				this->renderer = (MediaVideoRenderer *) this->rgbRenderer;
				return MP_RESULT_OK;
			}
			else {

				return MP_RESULT_ERROR;
			}
		}
		break;
	}

	return MP_RESULT_ERROR;
}

/*
 * Not USED
 */

MP_RESULT MediaVideoRenderer::Init(HWND hwnd, unsigned int width, unsigned int height)
{
	if(this->renderer) {

		this->renderer->Close();
		return this->renderer->Init(hwnd, width, height);
	}

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRenderer::InitFullscreen(HWND hwnd, unsigned int width, unsigned int height)
{
	if(this->renderer) {

		this->renderer->Close();
		return this->renderer->InitFullscreen(hwnd, width, height);
	}

	return MP_RESULT_ERROR;
}

/*
 * Used
 */

MP_RESULT MediaVideoRenderer::Stop()
{
	if(this->renderer)
		return this->renderer->Stop();

	return MP_RESULT_ERROR;
}

RECT *MediaVideoRenderer::GetFullscreenRects()
{
	if(this->renderer)
		return this->renderer->GetFullscreenRects();

	return NULL;
}

media_video_mode_t MediaVideoRenderer::GetVideoMode()
{
	if(this->renderer)
		return this->renderer->GetVideoMode();

	return VIDEO_MODE_NONE;
}

MP_RESULT MediaVideoRenderer::Draw(MediaBuffer *buffer, RECT *rect, int frameNumber, int invertFlag)
{
	if(this->renderer)
		return this->renderer->Draw(buffer, rect, frameNumber, invertFlag);

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRenderer::DrawFullscreen(MediaBuffer *buffer, int frameNumber, int invertFlag, int desktop)
{
	if(this->renderer)
		return this->renderer->DrawFullscreen(buffer, frameNumber, invertFlag, desktop);

	return MP_RESULT_ERROR;
}

MP_RESULT MediaVideoRenderer::Close()
{
	if(this->renderer) {
		
		this->renderer->Close();
		this->renderer = NULL;
	}

	return MP_RESULT_ERROR;
}
