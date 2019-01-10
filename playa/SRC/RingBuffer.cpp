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

#include "RingBuffer.h"

/*
 * Ring Buffer Class
 *
 */

MediaRingBuffer::MediaRingBuffer()
{
	this->ring = (char *) new char[RING_SIZE];

	this->read_pos  = 0;
	this->write_pos = 0;
}

MediaRingBuffer::~MediaRingBuffer()
{
	free(this->ring);
}

/*
 * Read Write Funcs
 */

void MediaRingBuffer::Init()
{
	this->read_pos  = 0;
	this->write_pos = 0;
}


void MediaRingBuffer::Read(char *outData, unsigned int size)
{
	if(write_pos <= read_pos) {
    
	    if(read_pos + size < RING_SIZE) {
			memcpy(outData, ring + read_pos, size);
			read_pos += size;
		}
		else {
			
			if(read_pos + size < RING_SIZE + write_pos) {
	
				unsigned int before, after;
	
				before = (RING_SIZE - 1) - read_pos;
				after = size - before;
	
				memcpy(outData, ring + read_pos, before);
				memcpy(outData + before, ring, after);
	
				read_pos = after;
			}
			else {
			}
		}
	}
	else {
		
		if(read_pos + size <= write_pos) {
			
			memcpy(outData, ring + read_pos, size);
			read_pos += size;
		}
		else {
		}
	} 
}

void MediaRingBuffer::Write(char *inData, unsigned int size)
{
	if(write_pos >= read_pos) {
    
		if(write_pos + size < RING_SIZE) {
			
			memcpy(ring + write_pos, inData, size);
			write_pos += size;
		}
		else {
		
			if(write_pos + size < RING_SIZE + read_pos) {
	
				unsigned int before, after;

				before = (RING_SIZE - 1) - write_pos;
				after = size - before;

				memcpy(ring + write_pos, inData, before);
				memcpy(ring, inData + before, after);

				write_pos = after;
			}
		}
	}
	else {
    
		if(write_pos + size <= read_pos) {
			
			memcpy(ring + write_pos, inData, size);
			write_pos += size;
		}
	}
}

int  MediaRingBuffer::IsFullFor(unsigned int size)
{
	if(write_pos == read_pos)
		return 0;

	if(write_pos > read_pos) {

		if(write_pos + size < read_pos + RING_SIZE)
			return 0;

		return 1;
	}
	else {

		if(write_pos + size < read_pos)
			return 0;

		return 1;
	}
}
