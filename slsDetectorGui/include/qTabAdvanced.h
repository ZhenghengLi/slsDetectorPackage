#pragma once

#include "qDefs.h"
#include "sls_detector_defs.h"

#include "ui_form_tab_advanced.h"

class multiSlsDetector;
class slsDetector;

class qDrawPlot;

#include <QStackedLayout>
#include <QSpacerItem>
/**
 *@short sets up the advanced parameters
 */
class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 */
	qTabAdvanced(QWidget *parent, multiSlsDetector* detector);

	/**
	 * Destructor
	 */
	~qTabAdvanced();

public slots:
	/**
	 * To refresh and update widgets
	 */
	void Refresh();

private:
	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();

	/**
	 * Add ROI Input
	 * @param num number of inputs to add
	 */
	void AddROIInput(int num);

	/**
	 * Update the setalltrimbits value from server
	 */
	void updateAllTrimbitsFromServer();


private slots:

	/**
	 * Sets control port
	 * @param port control port
	 */
	void SetControlPort(int port);

	/**
	 * Sets stop port
	 * @param port stop port
	 */
	void SetStopPort(int port);

	/**
	 * Sets receiver tcp port
	 * @param port receiver tcp port
	 */
	void SetRxrTCPPort(int port);

	/**
	 * Sets receiver udp port
	 * @param port receiver udp port
	 */
	void SetRxrUDPPort(int port);

	/**
	 * Sets client zmq receiver port
	 * @param port client zmq receiver port
	 */
	void SetCltZmqPort(int port);

	/**
	 * Sets receiver zmq transmitting port
	 * @param port receiver zmq transmitting port
	 */
	void SetRxrZmqPort(int port);

	/**
	 * Sets receiver online
	 * @param index 1 for online and 0 for offline
	 */
	void SetReceiverOnline(int index);

	/**
	 * Sets detector online
	 * @param index 1 for online and 0 for offline
	 */
	void SetOnline(int index);

	/**
	 * Sets network parameters like receiver udp ip,
	 * receiver udp mac, detector ip and detector mac
	 */
	void SetNetworkParameters();

	/**
	 * Sets client zmq ip to listen to
	 */
	void SetClientZMQIP();

	/**
	 * Sets receiver zmq ip to stream from
	 */
	void SetReceiverZMQIP();

	/**
	 * Sets the receiver. which also sets the receiver parameters
	 */
	void SetReceiver();

	/**
	 * Add ROI Input if the value changed in the last slot
	 */
	void AddROIInputSlot();

	/**
	 * Clears all the ROI inputs
	 */
	void clearROI();

	/**
	 * Gets ROIs from detector and updates it
	 */
	void updateROIList();

	/**
	 * Sets ROI in detector
	 */
	void setROI();

	/**
	 * Clears ROI in detector
	 */
	void clearROIinDetector();

	/**
	 * Select Readout
	 * @param index position index of readout
	 */
	void SetDetector(int index);

	/**
	 * Set all trimbits to a value
	 */
	void SetAllTrimbits();

	/**
	 * Set storage cells
	 * @param value value to set to
	 */
	void SetNumStoragecells(int value);

	/**
	 * Set sub frame exposure time
	 */
	void SetSubExposureTime();

	/**
	 * Set sub frame dead time
	 */
	void SetSubDeadTime();


private:

	/** The multi detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/** Tool Tip */
	QString		errOnlineTip;
	QString		detOnlineTip;
	QString 	rxrOnlineTip;
	QPalette	red;

	/** ROI */
	std::vector <QLabel*> 	lblFromX;
	std::vector <QSpinBox*> 	spinFromX;
	std::vector <QLabel*> 	lblFromY;
	std::vector <QSpinBox*> 	spinFromY;
	std::vector <QLabel*> 	lblToX;
	std::vector <QSpinBox*> 	spinToX;
	std::vector <QLabel*> 	lblToY;
	std::vector <QSpinBox*> 	spinToY;
	int numRois;

};


