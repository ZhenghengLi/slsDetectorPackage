/*
 * qTabDeveloper.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDEVELOPER_H_
#define QTABDEVELOPER_H_

/** Form Header */
#include "ui_form_tab_developer.h"
/** Project Class Headers */
class multiSlsDetector;

/**
 *@short sets up the Developer parameters
 */
class qTabDeveloper:public QWidget, private Ui::TabDeveloperObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabDeveloper(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabDeveloper();

	/** To refresh and update widgets
	 */
	void Refresh();

private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();


};



#endif /* QTABDEVELOPER_H_ */
