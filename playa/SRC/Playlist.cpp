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
 * Authors: Damien Chavarria <adrc at projectmayo.com>                                *
 *                                                                                    *
 **************************************************************************************/


#include "Playlist.h"


/*
 * The Playlist Class
 *
 *
 */

Playlist::Playlist()
{
	this->current   = -1;
	this->itemCount = 0;

	this->playlist = NULL;
}

Playlist::~Playlist()
{
	this->Reset();	
}

void             Playlist::Reset()
{
	playlist_node_t *node, *temp;

	this->current   = -1;
	this->itemCount = 0;

	if(this->playlist != NULL) {
	
		node = playlist;

		while(node->next != NULL) {

			temp = node;
			node = node->next;
			free(temp);
		}

		free(node);

		this->playlist = NULL;
	}
}

void             Playlist::AddItem(char *filename)
{
	playlist_node_t *node;
	playlist_node_t *iterator;

	if(filename) {

		node = (playlist_node_t *) new playlist_node_t;

		node->item.filename = (char *) new char[strlen(filename)];
		strcpy(node->item.filename, filename);

		node->next = NULL;

		if(this->itemCount == 0) {

			this->playlist  = node;
			this->itemCount = 1;
			this->current   = 0;
		}
		else {

			DWORD i;

			iterator = this->playlist;

			for(i=0; i<this->itemCount-1; i++) {

				if(iterator->next != NULL)
					iterator = iterator->next;
			}

			iterator->next = node;
			this->itemCount++;
		}
	}
}

void             Playlist::RemoveItemAt(DWORD i)
{
	DWORD           count;
	playlist_node_t *node;

	if(this->itemCount == 0)
		return;

	if(this->itemCount == 1) {

		this->Reset();
	}
	else {

		if( i>=1 && i<this->itemCount) {

			if(i == 0) {
		
				node = this->playlist;
				this->playlist = this->playlist->next;
				free(node);

				this->itemCount--;
			}
			else {

				node = this->playlist;
	
				for(count = 0; count < i-1; count++) {
				
					if(node->next != NULL)
						node = node->next;
				}

				if(node->next->next == NULL) {
					
					free(node->next);
					node->next = NULL;
				
					this->itemCount--;
				}
				else {

					playlist_node_t *temp;

					temp = node->next;
					node->next = temp->next;

					free(temp);

					this->itemCount--;
				}
			}
		}
	}
}

void             Playlist::NextItem()
{
	if(this->current < this->itemCount - 1)
		this->current++;
}

void             Playlist::PreviousItem()
{
	if(this->current > 0)
		this->current--;
}

DWORD            Playlist::GetItemCount()
{
	return this->itemCount;
}

DWORD            Playlist::GetCurrentPosition()
{
	return this->current;
}

playlist_item_t *Playlist::GetCurrentItem()
{
	if(current == -1 || this->itemCount <= 0) {
		
		return NULL;
	}
	else {

		playlist_node_t *node;
		DWORD i;

		node = playlist;

		for(i=0; i < this->current; i++) {

			node = node->next;
		}

		return &node->item;
	}
}

playlist_item_t *Playlist::GetItemAt(DWORD pos)
{
	if(current == -1 || this->itemCount <= 0) {
		
		return NULL;
	}
	else {

		playlist_node_t *node;
		DWORD i;

		node = playlist;

		for(i=0; i < pos; i++) {

			node = node->next;
		}

		return &node->item;
	}

}

void             Playlist::SetCurrentPosition(DWORD pos)
{
	if(pos >= 0 && pos < this->itemCount) {

		this->current = pos;
	}
}
