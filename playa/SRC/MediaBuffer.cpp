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

#include "MediaBuffer.h"
#include <stdio.h>

/*
 * MediaBuffer Class 
 * -----------------
 *
 */

/*
 * constructor && destructor
 *
 */

MediaBuffer::MediaBuffer()
{
	this->data = NULL;
	this->size = 0;
}

MediaBuffer::~MediaBuffer()
{

}

/*
 * Info functions
 */

void *MediaBuffer::GetData()
{
	return this->data;
}

unsigned int MediaBuffer::GetSize()
{
	return this->size;
}

/*
 * Alloc and Realloc
 * Functions
 */

MP_RESULT MediaBuffer::Alloc(int size)
{
    unsigned int size_local;

	if(this->data)
		return MP_RESULT_ERROR;

	this->size = size;

	if(this->data != NULL) {
	
		free(this->data);
	}

	this->data = (void *) new char[size];

	if(this->data != NULL) {

		memset(this->GetData(), 0, this->GetSize());
	}
	else {

		return MP_RESULT_ERROR;
	}

	return MP_RESULT_OK;
}

MP_RESULT MediaBuffer::ReAlloc(int size)
{
	if(!this->data)
		return MP_RESULT_ERROR;

	this->data = realloc(this->data, size);
	this->size = size;

	return MP_RESULT_OK;
}

/*
 * Free functions
 */

MP_RESULT MediaBuffer::Free()
{
	if(this->data)
		free(this->data);

	this->data = NULL;

	return MP_RESULT_OK;
}
