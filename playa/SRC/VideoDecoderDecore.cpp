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

#include "VideoDecoderDecore.h"
#include <windows.h>
#include <commctrl.h>

/*
 * Settings DlgProc
 *
 */

static BOOL APIENTRY DecoreSettingsDlgProc( HWND hDlg, UINT message, UINT wParam, LONG lParam) {

	MediaVideoDecoderDecore *dec = (MediaVideoDecoderDecore *) GetWindowLong(hDlg, DWL_USER);

	switch(message) {
		case WM_INITDIALOG:
			
			SetWindowLong(hDlg, DWL_USER, lParam);
			dec = (MediaVideoDecoderDecore *)lParam;

			SendMessage(GetDlgItem(hDlg, IDC_DECORE_SLIDER), TBM_SETRANGE, (WPARAM) (BOOL) FALSE, (LPARAM) MAKELONG(0, 6)); 
			SendMessage(GetDlgItem(hDlg, IDC_DECORE_SLIDER), TBM_SETPOS, TRUE, dec->postprocessing/10);


			return TRUE;

		case WM_SYSCOMMAND:
			if (wParam == SC_CLOSE)
			{
				EndDialog (hDlg, TRUE);
				return (TRUE);
			}
			break;

		case WM_COMMAND:

	        switch (wParam)
			{
				DEC_SET dec_set;

				case ID_DECORE_OK:
				
					dec->postprocessing = 10*SendMessage(GetDlgItem(hDlg, IDC_DECORE_SLIDER), TBM_GETPOS, 0, 0);
					
					dec_set.postproc_level = dec->postprocessing;
					if(dec->decoreFunction)
						dec->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_SETPP, &dec_set, NULL);

					EndDialog (hDlg, TRUE);
					break;

				case ID_DECORE_APPLY:

					dec->postprocessing = 10*SendMessage(GetDlgItem(hDlg, IDC_DECORE_SLIDER), TBM_GETPOS, 0, 0);

					dec_set.postproc_level = dec->postprocessing;
					if(dec->decoreFunction)
						dec->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_SETPP, &dec_set, NULL);
				
					break;
				
				case ID_DECORE_CLOSE	:
				
					EndDialog (hDlg, TRUE);
					break;
			}
			break;

		case WM_DESTROY:
			return TRUE;
	}

	return FALSE;
}


/*
 * openDivX video 
 * decoder plugin
 *
 */

MediaVideoDecoderDecore::MediaVideoDecoderDecore()
{
	this->decaps      = NULL;
	this->inputBuffer = new MediaBuffer();

	this->invertFlag     = 1;
	this->postprocessing = 60;

	this->decoreFunction = NULL;
}

MediaVideoDecoderDecore::~MediaVideoDecoderDecore()
{
	delete this->inputBuffer;
}

/*
 * Media Item functions
 */

media_type_t  MediaVideoDecoderDecore::GetType()
{
	return MEDIA_TYPE_VIDEO_DECODER;
}

char         *MediaVideoDecoderDecore::GetName()
{
	return "OpenDivX Video Decoder";
}

MP_RESULT     MediaVideoDecoderDecore::Connect(MediaItem *item)
{

	if(item != NULL && item->GetType() == MEDIA_TYPE_DECAPS) {

		/*
		 * Add code here!
		 */

		this->decaps = (MediaItemDecaps *) item;

		if(strstr(this->decaps->GetVideoCompression(0), "DIVX") != NULL || 
		   strstr(this->decaps->GetVideoCompression(0), "divx") != NULL ||
		   strstr(this->decaps->GetVideoCompression(0), "DVX1") != NULL ||
		   strstr(this->decaps->GetVideoCompression(0), "dvx1") != NULL ) {
	
			this->hDivXDll       = NULL;
			this->decoreFunction = NULL;

			this->hDivXDll = LoadLibrary("DivX.dll");

			if(this->hDivXDll != NULL) {

				this->decoreFunction = (decoreFunc) GetProcAddress(this->hDivXDll, "decore");	

				if(this->decoreFunction == NULL) {

					MP_ERROR("Can't find the \"decore\" entry point in DivX.dll. Please Install the latest DivX4Windows Package");
					
					this->decaps= NULL;

					return MP_RESULT_ERROR;
					}
			}
			else {

				MP_ERROR("Please instal the DivX For Windows package first.");

				this->decaps= NULL;
				
				return MP_RESULT_ERROR;
			}

			this->decParam.x_dim         = this->decaps->GetVideoWidth(0);
			this->decParam.y_dim         = this->decaps->GetVideoHeight(0);
			this->decParam.output_format = DEC_RGB565;

			this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_MEMORY_REQS, (LPVOID)&this->decParam, (LPVOID)&this->decMem);

			this->decParam.buffers.mp4_edged_ref_buffers = malloc(this->decMem.mp4_edged_ref_buffers_size);
			this->decParam.buffers.mp4_edged_for_buffers = malloc(this->decMem.mp4_edged_for_buffers_size);
			this->decParam.buffers.mp4_display_buffers   = malloc(this->decMem.mp4_display_buffers_size);
			this->decParam.buffers.mp4_state             = malloc(this->decMem.mp4_state_size);
			this->decParam.buffers.mp4_tables            = malloc(this->decMem.mp4_tables_size);
			this->decParam.buffers.mp4_stream            = malloc(this->decMem.mp4_stream_size);

			memset(this->decParam.buffers.mp4_state,  0, this->decMem.mp4_state_size);
			memset(this->decParam.buffers.mp4_tables, 0, this->decMem.mp4_tables_size);
			memset(this->decParam.buffers.mp4_stream, 0, this->decMem.mp4_stream_size);	

			this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL);

			this->inputBuffer->Alloc(DECORE_INPUT_SIZE);

			this->videoMode = VIDEO_MODE_UYVY;

			DEC_SET dec_set;
			dec_set.postproc_level = this->postprocessing;

			this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_SETPP, &dec_set, NULL);

			return MP_RESULT_OK;
		}
	}

	this->decaps = NULL;

	return MP_RESULT_ERROR;
}

MP_RESULT     MediaVideoDecoderDecore::ReleaseConnections()
{
	/*
	 * Do Cleanup here!
	 */

	this->decaps = NULL;
	this->inputBuffer->Free();

	if(this->decoreFunction)
		this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_RELEASE, NULL, NULL);

	if(this->hDivXDll) {
	
		FreeLibrary(this->hDivXDll);

		this->hDivXDll       = NULL;
		this->decoreFunction = NULL;
	}

	return MP_RESULT_OK;
}

DWORD         MediaVideoDecoderDecore::GetCaps()
{
	return MEDIA_CAPS_CAN_CONFIGURE;
}

MP_RESULT     MediaVideoDecoderDecore::Configure(HINSTANCE hInstance, HWND hwnd)
{
	HWND hDlg;

    hDlg = CreateDialogParam(hInstance, (LPCSTR)MAKEINTRESOURCE(IDD_DECORE_SETTINGS), hwnd, (DLGPROC) DecoreSettingsDlgProc, (LPARAM)(LPVOID)this);
    
    ShowWindow(hDlg, SW_SHOW);

	return MP_RESULT_OK;
}

/*
 * Video Decoder Functions
 *
 */

unsigned int MediaVideoDecoderDecore::GetFrameSize() 
{
	switch(this->videoMode) {

	case VIDEO_MODE_RGB8:
	case VIDEO_MODE_YUV9:

		return 0;
		break;

	case VIDEO_MODE_RGB16:

		return this->decParam.x_dim*this->decParam.y_dim*2;
		break;

	case VIDEO_MODE_RGB24:

		return this->decParam.x_dim*this->decParam.y_dim*3;
		break;

	case VIDEO_MODE_RGB32:

		return this->decParam.x_dim*this->decParam.y_dim*4;
		break;

	case VIDEO_MODE_YUV12:

		return this->decParam.x_dim*this->decParam.y_dim*12/8;
		break;

	case VIDEO_MODE_YUY2:

		return this->decParam.x_dim*this->decParam.y_dim*2;
		break;

	case VIDEO_MODE_UYVY:

		return this->decParam.x_dim*this->decParam.y_dim*2;
		break;
	}

	return 0;
}


media_video_mode_t MediaVideoDecoderDecore::GetVideoMode() 
{
	return this->videoMode;
}

BOOL MediaVideoDecoderDecore::GetInvertFlag() 
{
	return this->invertFlag;
}

MP_RESULT          MediaVideoDecoderDecore::SetQuality(DWORD quality)
{
	this->postprocessing = quality;

	if(this->decoreFunction) {
	
		DEC_SET dec_set;
		dec_set.postproc_level = this->postprocessing;

		this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_SETPP, &dec_set, NULL);
	}

	return MP_RESULT_OK;
}

DWORD          MediaVideoDecoderDecore::GetQuality()
{
	return this->postprocessing;
}

MP_RESULT          MediaVideoDecoderDecore::SetVideoMode(media_video_mode_t mode)
{
	if(this->decoreFunction == NULL) {

		return MP_RESULT_ERROR;
	}

	this->videoMode = mode;

	switch(mode) {

	case VIDEO_MODE_RGB8:
	case VIDEO_MODE_YUV9:

		return MP_RESULT_ERROR;
		break;

	case VIDEO_MODE_RGB16:

		this->decParam.output_format = DEC_RGB565;

		this->invertFlag = 1;

		if(this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL) == DEC_OK)
			return MP_RESULT_OK;
		
		break;

	case VIDEO_MODE_RGB24:

		this->decParam.output_format = DEC_RGB24;

		this->invertFlag = 1;

		if(this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL) == DEC_OK)
			return MP_RESULT_OK;;
		
		break;

	case VIDEO_MODE_RGB32:

		this->decParam.output_format = DEC_RGB32;

		this->invertFlag = 1;

		if(this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL) == DEC_OK)
			return MP_RESULT_OK;;
		
		break;

	case VIDEO_MODE_YUV12:

		this->decParam.output_format = DEC_420;

		this->invertFlag = 0;

		if(this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL) == DEC_OK)
			return MP_RESULT_OK;;
		
		break;

	case VIDEO_MODE_YUY2:

		this->decParam.output_format = DEC_YUV2;

		this->invertFlag = 0;

		if(this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL) == DEC_OK)
			return MP_RESULT_OK;;

		break;

	case VIDEO_MODE_UYVY:

		this->decParam.output_format = DEC_UYVY;

		this->invertFlag = 0;

		if(this->decoreFunction(DECORE_PLAYA_ID, DEC_OPT_INIT, (LPVOID)&this->decParam, NULL) == DEC_OK)
			return MP_RESULT_OK;;

		break;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT          MediaVideoDecoderDecore::Decompress(MediaBuffer *mb_out, unsigned int stride)
{
	unsigned int size;

	if(this->decaps && mb_out && this->decoreFunction) {

		size = this->decaps->GetNextVideoFrameSize(0);
	
		if(size > this->inputBuffer->GetSize())
			this->inputBuffer->ReAlloc(size);


		if(this->decaps->ReadVideoFrame(0, this->inputBuffer) != MP_RESULT_OK) {

			return MP_RESULT_ERROR;
		}

		this->decFrame.length      = size;
		this->decFrame.bitstream   = this->inputBuffer->GetData();
		this->decFrame.bmp         = mb_out->GetData();
		this->decFrame.stride      = stride;
		this->decFrame.render_flag = 1;
 

		if(size != 0) {

			this->decoreFunction(DECORE_PLAYA_ID, 0, &this->decFrame, NULL);
		}

		return MP_RESULT_OK;
	}

	return MP_RESULT_ERROR;
}

MP_RESULT          MediaVideoDecoderDecore::Drop(MediaBuffer *mb_out, unsigned int stride)
{
	unsigned int size;

	if(this->decaps && mb_out && this->decoreFunction) {

		size = this->decaps->GetNextVideoFrameSize(0);
	
		if(size > this->inputBuffer->GetSize())
			this->inputBuffer->ReAlloc(size);

		this->decaps->ReadVideoFrame(0, this->inputBuffer);

		this->decFrame.length      = size;
		this->decFrame.bitstream   = this->inputBuffer->GetData();
		this->decFrame.bmp         = mb_out->GetData();
		this->decFrame.render_flag = 0;
  
		this->decoreFunction(DECORE_PLAYA_ID, 0, &this->decFrame, NULL);
	}

	return MP_RESULT_ERROR;
}

