/***************************************************************************
                          touchlecture.cpp  -  description
                             -------------------
    begin                : Sun Dec 10 2000
    copyright            : (C) 2000 by Haavard Froeiland
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

#include "touchlecture.h"
#include "touchlecture.moc"


TouchLecture::TouchLecture()
{
	fileName="";
	level=0;
	pos=1;
};

QString TouchLecture::getName()
{
	return fileName;
}

void TouchLecture::loadLectureFile(QString f)
{
	QFile file(f);
	if(file.exists())
	{
		if (file.open(IO_ReadOnly) )      // file opened successfully
		{
			fileName=f;
			unsigned int maxLevel=0;
			levelVector.clear();

			QTextStream t( &file );
			QString s;
			levelVector.insert(levelVector.end(),new vector<QString>);
			while ( !t.eof() )
			{             // until end of file...
				s = t.readLine();              // line of text excluding '\n'
				if(s.at(0)!='#') // # means that it's a comment
				{
					if(s.isEmpty())
					{
						// we detected a line-space so this might be a new level
						if(!(levelVector[maxLevel]->empty()))
						{
							levelVector.insert(levelVector.end(),new vector<QString>);
							maxLevel++;
						}
					}
					else
					{
						// adding the line
						levelVector[maxLevel]->insert(levelVector[maxLevel]->end(),s);
					}
				}
			}
			file.close();
			if(levelVector[maxLevel]->empty())
			{
				levelVector.erase(&levelVector[maxLevel]);
				maxLevel--;
			}

			setLevel(level);
			pos=1;
		}
		else
		{
			//i18n needed here. What do I need to include?
			QMessageBox::information( parent, "Error reading training file",
											"There was an error when trying to open: "+f );
		}
	}
	else
	{
		//i18n needed here. What do I need to include?
		QMessageBox::information( parent,"Missing training file",
										"The trainingfile specifide dosn't exists: "+f);
	}
}


QString TouchLecture::getNextLine()
{
  if (levelVector.empty())
	{
    return "No file loaded";
	}
  if(level<(levelVector.size()))
  {
     vector<QString> test=*levelVector[level];

     if (pos>=(test.size()-1))
	   {
		   pos=1;
	   }
     return test[pos++];
  }
  return "Error in getNextLine()";
};


void TouchLecture::levelUp()
{
	if(level<(levelVector.size()-1))
	{
		level++;

		//is good to start at pos 0 every time you change level?
		pos=1;
		setLevel(level);
	}
};

void TouchLecture::levelDown()
{
	if(level>0)
	{
		level--;
		//is good to start at pos 0 every time you change level?
		pos=1;
		setLevel(level);
		}
	};

void TouchLecture::setLevel(unsigned int l)
{
	if(l<(levelVector.size()))
	{
		level=l;
		emit levelChanged(level);
		{
			vector<QString> test=*levelVector[level];

			emit levelMessage(QString(test[0]));

		}
	}
	else
	{
		level=levelVector.size()-1;
		setLevel(level);
	}
};

