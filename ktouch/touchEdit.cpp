/***************************************************************************
                          touchEdit.cpp  -  description
                             -------------------
    begin                : Tue Nov 27 2001
    copyright            : (C) 2001 by Haavard Froeiland
    email                : haavard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "touchEdit.h"
#include "touchEdit.moc"
#include <kdebug.h>

#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlcdnumber.h>

TouchEdit::TouchEdit( QWidget* parent, TouchLecture* l)
         : TouchEditLayout(parent)
{
	lecture=l;


 	fillLevelList();
  levelList->setCurrentItem(0);
	
}



TouchEdit::~TouchEdit()
{

}



void TouchEdit::fetchLevel(int level)
{
	QString res;
	vector<QString> levelData=*(lecture->levelVector)[level];

	levelNumber->display(level+1);

 	description->setText(levelData[0]);

			for(unsigned int j=1;j<levelData.size();j++)
			{
       	if(j>1) res.append("\n");
				res.append(levelData[j]);
			}
		
	levelText->setText(res);
}

/** Moves level one place up */
void TouchEdit::moveUp()
{
	int current=levelList->currentItem();
	if(current==-1)
  	kdDebug() << "No item selected " << current << endl;
	else
  {
		if(current>0)
	  {
			kdDebug() << "moveing up " << current << endl;
 		 	swapLevel(current,current-1);
 	  }
  	else
  		kdDebug() << "Can't performe move, this item is on the top!" << endl;
  }
}

/** Moves level one place down */
void TouchEdit::moveDown()
{
	int current=levelList->currentItem();
 	if(current==-1)
  	kdDebug() << "No item selected " << current << endl;
	else
  {

		if(current<(lecture->levelVector.size()-1))
    {
		  kdDebug() << "moveing down " << current << endl;
  	  swapLevel(current,current+1);
  	}
  	else
  		kdDebug() << "Can't performe move, this item is on the bottom" << endl;
  }

}
/** No descriptions */
void TouchEdit::swapLevel(int a,int b)
{
  // moving data around
	vector<QString> *levelTmp=lecture->levelVector[b];
	lecture->levelVector[b]=lecture->levelVector[a];
	lecture->levelVector[a]=levelTmp;

	// fixing up view
  levelList->clear();
	fillLevelList();
  levelList->setCurrentItem(b);

}
/** Filles up the level list */
void TouchEdit::fillLevelList()
{
	for(unsigned int i=0;i<lecture->levelVector.size();i++)
	{
		
			vector<QString> levelData=*(lecture->levelVector)[i];
			levelList->insertItem(levelData[0]);
	}

}