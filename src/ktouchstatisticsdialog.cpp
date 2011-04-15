/***************************************************************************
 *   ktouchstatisticsdialog.cpp                                            *
 *   --------------------------                                            *
 *   Copyright (C) 2000 by Håvard Frøiland, 2004 by Andreas Nicolai        *
 *   ghorwin@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ktouchstatisticsdialog.h"
#include "ktouchstatisticsdialog.moc"

#include <utility>
#include <iterator>
#include <algorithm>

#include <kplotobject.h>
#include <kplotaxis.h>

#include <QtAlgorithms>
#include <QDialog>

#include <kpushbutton.h>
#include <kcombobox.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <ktabwidget.h>

#include "ktouch.h"

KTouchStatisticsDialog::KTouchStatisticsDialog(QWidget* parent)
	: QDialog(parent)
{
    setupUi(this);
    sessionsRadio->setChecked(true);
    WPMRadio->setChecked(true);
    eventRadio->setChecked(true);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()) );
    connect(lectureCombo, SIGNAL(activated(int)), this, SLOT(lectureActivated(int)) );
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearHistory()) );

    // connect the radio buttons with the chart update function
    connect(CPMRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );
    connect(WPMRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );
    connect(correctRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );
    connect(skillRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );

    connect(levelsRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );
    connect(sessionsRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );

    connect(eventRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );
    connect(timeRadio, SIGNAL(toggled(bool)), this, SLOT(updateChartTab()) );


    levelsRadio->setEnabled(false);
}
// ----------------------------------------------------------------------------

void KTouchStatisticsDialog::run(const KUrl& currentLecture, const KTouchStatisticsData& stats,
	const KTouchLevelStats& currLevelStats,
	const KTouchSessionStats& currSessionStats)
{
	lectureLabel1->setText(currentLecture.url());
	lectureLabel2->setText(currentLecture.url());
	// fill lecture combo with data
	// loop over all lecturestatistics
	lectureCombo->clear();
	QMap<KUrl, KTouchLectureStats>::const_iterator it = stats.m_lectureStats.constBegin();
	m_currentIndex = 0;
	while (it != stats.m_lectureStats.constEnd()) {
		QString t = it.value().m_lectureTitle;
		// if current lecture, remember index and adjust text
		if (it.key() == currentLecture) {
			m_currentIndex = lectureCombo->count();
			t = i18n("***current***  ") + t;
		}
		lectureCombo->addItem(t);
		++it;
	}
	if (lectureCombo->count()==0) {
		// this shouldn't happen if the dialog is run with proper data
		KMessageBox::information(this, i18n("No statistics data available yet."));
		return;
	}
	// remember stats
	m_allStats = stats;
	m_currLevelStats = currLevelStats;
	m_currSessionStats = currSessionStats;
	// modify current lecture entry
	lectureCombo->setCurrentIndex(m_currentIndex);
	lectureActivated(m_currentIndex);
	m_lectureIndex = m_currentIndex;
	
	// update the current tabs with current session/level data
	updateCurrentSessionTab();
	updateCurrentLevelTab();
	// set current session as current tab
	tabWidget->setCurrentWidget(currentTab);
	exec();
}
// ----------------------------------------------------------------------------

void KTouchStatisticsDialog::lectureActivated(int index) {
	if (m_allStats.m_lectureStats.count()==0) {
		// TODO : Reset all tabs to "empty" look
		m_lectureIndex = 0;
		return;
	}
	if (index >= static_cast<int>(m_allStats.m_lectureStats.count())) {
		kDebug() << "Lecture index out of range: " << index << " >= " << m_allStats.m_lectureStats.count();
		return;
	}
	m_lectureIndex = index;
	//kDebug() << "Lecture stats changed: " << it.data().m_lectureTitle;
	// update the tabs
	updateChartTab();
}
// ----------------------------------------------------------------------------

void KTouchStatisticsDialog::clearHistory() {
	if (KMessageBox::questionYesNo(this, i18n("Erase all statistics data for the current user?\rThe training session will restart at the current level.")) == KMessageBox::Yes) {
		KTouchPtr->clearStatistics(true); // clear statistics data in KTouch
		// clear and reset local copy
		m_allStats.clear();
		m_currLevelStats.clear();
		m_currSessionStats.clear();
		m_currSessionStats.m_charStats.clear();

		//reset lecture Combo
		QString s = lectureCombo->currentText();
		lectureCombo->clear();
		lectureCombo->addItem(s);
		m_currentIndex = 0;
		lectureCombo->setCurrentIndex(m_currentIndex);
		lectureActivated(m_currentIndex);
		updateChartTab();

		//reset and refresh statistic GUI
		correctnessBar->setValue(0);
		correctnessBarLevel->setValue(0);
		updateCurrentLevelTab();
		updateCurrentSessionTab();
	}
}
// ----------------------------------------------------------------------------

void KTouchStatisticsDialog::updateCurrentSessionTab() {
	// session/level/info
	QString levelnums;
    int last_level = -2;
    int levels_count = 0;
	if(!m_currSessionStats.m_levelNums.isEmpty())
	{
		QSet<unsigned int>::iterator last_it = m_currSessionStats.m_levelNums.end();
		  --last_it;
		  for (QSet<unsigned int>::ConstIterator it = m_currSessionStats.m_levelNums.constBegin();
			  it != m_currSessionStats.m_levelNums.constEnd(); ++it)
		  {
			  // do we have a level number that is not a subsequent level of the
			  // previous?
  
			  if ((static_cast<unsigned int>(last_level + 1) != *it) ||
		      (it == last_it))
			  {
				  if (it != m_currSessionStats.m_levelNums.begin()) {
					  if (levels_count > 1)	levelnums += "...";
					  else					levelnums += ',';
				  }
				  levels_count = 0;
				  levelnums += QString("%1").arg(*it+1);
				  
			  }
			  else {
				  ++levels_count;  // increase level count
			  }
			  last_level = *it;
		  }
	}
	levelLabel1->setText(levelnums);
    // general stats group
    elapsedTimeLCD->display(static_cast<int>(m_currSessionStats.m_elapsedTime));
    totalCharsLCD->display(static_cast<int>(m_currSessionStats.m_totalChars) );
    wrongCharsLCD->display(static_cast<int>(m_currSessionStats.m_totalChars-m_currSessionStats.m_correctChars) );
    wordsLCD->display(static_cast<int>(m_currSessionStats.m_words) );
    // typing rate group
	if (m_currSessionStats.m_elapsedTime == 0) {
		charSpeedLCD->display(0);
		wordSpeedLCD->display(0);
	}
	else {
		charSpeedLCD->display(static_cast<int>(m_currSessionStats.m_correctChars/m_currSessionStats.m_elapsedTime*60.0) );
		wordSpeedLCD->display(static_cast<int>(m_currSessionStats.m_words/m_currSessionStats.m_elapsedTime*60.0) );
	}
    // accuracy
    correctnessBar->setValue(static_cast<int>(
		(100.0*m_currSessionStats.m_correctChars)/m_currSessionStats.m_totalChars) );
	// create sorted list of missed characters
	QList<KTouchCharStats> charList;
	qCopy(m_currSessionStats.m_charStats.constBegin(), m_currSessionStats.m_charStats.constEnd(), std::back_inserter(charList));
	qSort(charList.begin(), charList.end(), higher_miss_hit_ratio);
	
	QList<KTouchCharStats>::const_iterator it2=charList.constBegin();
    unsigned int i=0;
    for (; i<8 && it2!=charList.constEnd(); ++i, ++it2) {
        if (it2->missHitRatio()==0)
            break;  // stop listing keys when their hit-miss-ration is zero
        switch (i) {
          case 0 :  charLabel1->setText( QString(it2->m_char) ); charProgress1->setEnabled(true);
                    charProgress1->setValue( it2->missHitRatio() ); break;
          case 1 :  charLabel2->setText( QString(it2->m_char) ); charProgress2->setEnabled(true);
                    charProgress2->setValue( it2->missHitRatio() ); break;
          case 2 :  charLabel3->setText( QString(it2->m_char) ); charProgress3->setEnabled(true);
                    charProgress3->setValue( it2->missHitRatio() ); break;
          case 3 :  charLabel4->setText( QString(it2->m_char) ); charProgress4->setEnabled(true);
                    charProgress4->setValue( it2->missHitRatio() ); break;
          case 4 :  charLabel5->setText( QString(it2->m_char) ); charProgress5->setEnabled(true);
                    charProgress5->setValue( it2->missHitRatio() ); break;
          case 5 :  charLabel6->setText( QString(it2->m_char) ); charProgress6->setEnabled(true);
                    charProgress6->setValue( it2->missHitRatio() ); break;
          case 6 :  charLabel7->setText( QString(it2->m_char) ); charProgress7->setEnabled(true);
                    charProgress7->setValue( it2->missHitRatio() ); break;
          case 7 :  charLabel8->setText( QString(it2->m_char) ); charProgress8->setEnabled(true);
                    charProgress8->setValue( it2->missHitRatio() ); break;
        }
    }
	// set remaining char labels and progress bars to zero
    for(; i<8; ++i) {
        switch (i) {
          case 0 :  charLabel1->setText(" "); charProgress1->setValue(0); charProgress1->setEnabled(false); break;
          case 1 :  charLabel2->setText(" "); charProgress2->setValue(0); charProgress2->setEnabled(false); break;
          case 2 :  charLabel3->setText(" "); charProgress3->setValue(0); charProgress3->setEnabled(false); break;
          case 3 :  charLabel4->setText(" "); charProgress4->setValue(0); charProgress4->setEnabled(false); break;
          case 4 :  charLabel5->setText(" "); charProgress5->setValue(0); charProgress5->setEnabled(false); break;
          case 5 :  charLabel6->setText(" "); charProgress6->setValue(0); charProgress6->setEnabled(false); break;
          case 6 :  charLabel7->setText(" "); charProgress7->setValue(0); charProgress7->setEnabled(false); break;
          case 7 :  charLabel8->setText(" "); charProgress8->setValue(0); charProgress8->setEnabled(false); break;
        }
    }
}
// ----------------------------------------------------------------------------

void KTouchStatisticsDialog::updateCurrentLevelTab() {
	// level info
	levelLabel2->setText( QString("%1").arg(m_currLevelStats.m_levelNum+1) );
    // general stats group
    elapsedTimeLCDLevel->display(static_cast<int>(m_currLevelStats.m_elapsedTime));
    totalCharsLCDLevel->display(static_cast<int>(m_currLevelStats.m_totalChars) );
    wrongCharsLCDLevel->display(static_cast<int>(m_currLevelStats.m_totalChars-m_currLevelStats.m_correctChars) );
    wordsLCDLevel->display(static_cast<int>(m_currLevelStats.m_words) );
    // typing rate group
	if (m_currLevelStats.m_elapsedTime == 0) {
		charSpeedLCDLevel->display(0);
		wordSpeedLCDLevel->display(0);
	}
	else {
		charSpeedLCDLevel->display(static_cast<int>(m_currLevelStats.m_correctChars/m_currLevelStats.m_elapsedTime*60.0) );
		wordSpeedLCDLevel->display(static_cast<int>(m_currLevelStats.m_words/m_currLevelStats.m_elapsedTime*60.0) );
	}
    // accuracy
    correctnessBarLevel->setValue(static_cast<int>(
		(100.0*m_currLevelStats.m_correctChars)/m_currLevelStats.m_totalChars) );
	// create sorted list of missed characters
	QList<KTouchCharStats> charList;
	qCopy(m_currLevelStats.m_charStats.constBegin(), m_currLevelStats.m_charStats.constEnd(), std::back_inserter(charList) );
	qSort(charList.begin(), charList.end(), higher_miss_hit_ratio);
	
	QList<KTouchCharStats>::const_iterator it=charList.constBegin();
    unsigned int i=0;
    for (; i<8 && it!=charList.constEnd(); ++i, ++it) {
        if (it->missHitRatio()==0)
            break;  // stop listing keys when their hit-miss-ration is zero
        switch (i) {
          case 0 :  charLabel1Level->setText( QString(it->m_char) ); charProgress1->setEnabled(true);
                    charProgress1Level->setValue( it->missHitRatio() ); break;
          case 1 :  charLabel2Level->setText( QString(it->m_char) ); charProgress2->setEnabled(true);
                    charProgress2Level->setValue( it->missHitRatio() ); break;
          case 2 :  charLabel3Level->setText( QString(it->m_char) ); charProgress3->setEnabled(true);
                    charProgress3Level->setValue( it->missHitRatio() ); break;
          case 3 :  charLabel4Level->setText( QString(it->m_char) ); charProgress4->setEnabled(true);
                    charProgress4Level->setValue( it->missHitRatio() ); break;
          case 4 :  charLabel5Level->setText( QString(it->m_char) ); charProgress5->setEnabled(true);
                    charProgress5Level->setValue( it->missHitRatio() ); break;
          case 5 :  charLabel6Level->setText( QString(it->m_char) ); charProgress6->setEnabled(true);
                    charProgress6Level->setValue( it->missHitRatio() ); break;
          case 6 :  charLabel7Level->setText( QString(it->m_char) ); charProgress7->setEnabled(true);
                    charProgress7Level->setValue( it->missHitRatio() ); break;
          case 7 :  charLabel8Level->setText( QString(it->m_char) ); charProgress8->setEnabled(true);
                    charProgress8Level->setValue( it->missHitRatio() ); break;
        }
    }
	// set remaining char labels and progress bars to zero
    for(; i<8; ++i) {
        switch (i) {
          case 0 :  charLabel1Level->setText(" "); charProgress1Level->setValue(0); charProgress1Level->setEnabled(false); break;
          case 1 :  charLabel2Level->setText(" "); charProgress2Level->setValue(0); charProgress2Level->setEnabled(false); break;
          case 2 :  charLabel3Level->setText(" "); charProgress3Level->setValue(0); charProgress3Level->setEnabled(false); break;
          case 3 :  charLabel4Level->setText(" "); charProgress4Level->setValue(0); charProgress4Level->setEnabled(false); break;
          case 4 :  charLabel5Level->setText(" "); charProgress5Level->setValue(0); charProgress5Level->setEnabled(false); break;
          case 5 :  charLabel6Level->setText(" "); charProgress6Level->setValue(0); charProgress6Level->setEnabled(false); break;
          case 6 :  charLabel7Level->setText(" "); charProgress7Level->setValue(0); charProgress7Level->setEnabled(false); break;
          case 7 :  charLabel8Level->setText(" "); charProgress8Level->setValue(0); charProgress8Level->setEnabled(false); break;
        }
    }
}
// ----------------------------------------------------------------------------

void KTouchStatisticsDialog::updateChartTab() {
//    kDebug() << "[KTouchStatisticsDialog::updateChartTab]";

	// remove all current chart objects
	chartWidget->removeAllPlotObjects();
	// if no lecture data is available, return
	if (m_allStats.m_lectureStats.count()== 0 || 
        m_lectureIndex >= static_cast<unsigned int>(m_allStats.m_lectureStats.count()))  return;
	// what kind of chart is required?
	if (levelsRadio->isChecked()) {
		// TODO : nothing yet
	}
	else {
		QMap<KUrl, KTouchLectureStats>::const_iterator it = m_allStats.m_lectureStats.constBegin();	
		unsigned int index = m_lectureIndex;
		while (index-- > 0) ++it;
		QList< std::pair<double, double> > data;
		QString caption = "Session data";

		if(WPMRadio->isChecked()){ // words per minute
			// loop over all session data entries in *it and store words per minute data
			for (QList<KTouchSessionStats>::const_iterator session_it = (*it).m_sessionStats.constBegin();
				session_it != (*it).m_sessionStats.constEnd(); ++session_it)
			{
				double t = (*session_it).m_elapsedTime;
				double wpm = (t == 0) ? 0 : (*session_it).m_words/t*60.0;
				double tp = (*session_it).m_timeRecorded.toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, wpm) );
			}
			// add current session if selected lecture matches
			if (m_currentIndex == m_lectureIndex && 
				m_currSessionStats.m_elapsedTime != 0) 
			{
				double t = m_currSessionStats.m_elapsedTime;
				double wpm = m_currSessionStats.m_words/t*60.0;
				double tp = QDateTime::currentDateTime().toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, wpm) );
			}
			chartWidget->axis( KPlotWidget::LeftAxis )->setLabel( i18n("Words per minute") );
		}
        else if(CPMRadio->isChecked()){ // chars per minute
			// loop over all session data entries in *it and store chars per minute data
			for (QList<KTouchSessionStats>::const_iterator session_it = (*it).m_sessionStats.constBegin();
				session_it != (*it).m_sessionStats.constEnd(); ++session_it)
			{
				double t = (*session_it).m_elapsedTime;
				double cpm = (t == 0) ? 0 : (*session_it).m_correctChars/t*60.0;
				double tp = (*session_it).m_timeRecorded.toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, cpm) );
			}
			// add current session
			if (m_currentIndex == m_lectureIndex && 
				m_currSessionStats.m_elapsedTime != 0) 
			{
				double t = m_currSessionStats.m_elapsedTime;
				double cpm = m_currSessionStats.m_correctChars/t*60.0;
				double tp = QDateTime::currentDateTime().toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, cpm) );
			}
			chartWidget->axis( KPlotWidget::LeftAxis )->setLabel( i18n("Characters per minute") );
        }
        else if (correctRadio->isChecked()){ // correctness
			// loop over all session data entries in *it and store correctness data
			for (QList<KTouchSessionStats>::const_iterator session_it = (*it).m_sessionStats.constBegin();
				session_it != (*it).m_sessionStats.constEnd(); ++session_it)
			{
				double tc = (*session_it).m_totalChars;
				double corr = (tc == 0) ? 0 : (*session_it).m_correctChars/tc;
				double tp = (*session_it).m_timeRecorded.toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, corr) );
			}
			// add current session
			if (m_currentIndex == m_lectureIndex && 
				m_currSessionStats.m_totalChars != 0) 
			{
				double tc = m_currSessionStats.m_totalChars;
				double corr = m_currSessionStats.m_correctChars/tc;
				double tp = QDateTime::currentDateTime().toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, corr) );
			}
			chartWidget->axis( KPlotWidget::LeftAxis )->setLabel( i18n("Correctness") );
        }
        else if (skillRadio->isChecked()){
			// loop over all session data entries in *it and store correctness data
			for (QList<KTouchSessionStats>::const_iterator session_it = (*it).m_sessionStats.constBegin();
				session_it != (*it).m_sessionStats.constEnd(); ++session_it)
			{
				double tc = (*session_it).m_totalChars;
				double corr = (tc == 0) ? 0 : (*session_it).m_correctChars/tc;
				double t = (*session_it).m_elapsedTime;
				double cpm = (t == 0) ? 0 : (*session_it).m_correctChars/t*60.0;
				double skill = corr*cpm;
				double tp = (*session_it).m_timeRecorded.toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, skill) );
			}
			// add current session
			if (m_currentIndex == m_lectureIndex && 
				m_currSessionStats.m_totalChars != 0 && 
				m_currSessionStats.m_elapsedTime != 0)
			{
				double tc = m_currSessionStats.m_totalChars;
				double corr = m_currSessionStats.m_correctChars/tc;
				double t = m_currSessionStats.m_elapsedTime;
				double cpm = m_currSessionStats.m_correctChars/t*60.0;
				double skill = corr*cpm;
				double tp = QDateTime::currentDateTime().toTime_t()/(3600.0*24);
				data.push_back(std::make_pair(tp, skill) );
			}
			chartWidget->axis( KPlotWidget::LeftAxis )->setLabel( i18n("Skill") );
        }
        else{
            return;
		}

	    // Create plot object for session statistics
	    KPlotObject * ob;
	    if (data.size() <= 1) return;
	    ob = new KPlotObject( Qt::red, KPlotObject::Lines, 2 );

	    // Add some random points to the plot object
	    double min_x = 1e20;
	    double max_x = -1;
	    double max_y = -1;
	    for (int i=0; i<data.size(); ++i) {
		    double x;
		    if (timeRadio->isChecked()) {
			    x = data[i].first - data[0].first;
			    chartWidget->axis( KPlotWidget::BottomAxis )->setLabel( i18n( "Time since first practice session in days" ));
		    }
		    else {	
			    x = i+1;
			    chartWidget->axis( KPlotWidget::BottomAxis )->setLabel( i18n( "Sessions" ));
		    }
		    ob->addPoint( x, data[i].second );
		    min_x = std::min(x, min_x);
		    max_x = std::max(x, max_x);
		    max_y = std::max(data[i].second, max_y);
	    }
	    if (max_y == 0)
            max_y = 1;
    
	    max_y *= 1.1;
	    chartWidget->setLimits( min_x, max_x, -0.1*max_y, max_y );
	    // Add plot object to chart
	    chartWidget->addPlotObject(ob);
    }
}
// ----------------------------------------------------------------------------


