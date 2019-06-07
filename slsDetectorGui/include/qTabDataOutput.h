#pragma once

#include "ui_form_tab_dataoutput.h"

class multiSlsDetector;

class qTabDataOutput:public QWidget, private Ui::TabDataOutputObject{
	Q_OBJECT

public:
	qTabDataOutput(QWidget *parent,multiSlsDetector* detector);
	~qTabDataOutput();
	void Refresh();

	private slots:
	void GetOutputDir();
	void BrowseOutputDir();
	void SetOutputDir();
	void SetFileFormat(int format);
	void SetOverwriteEnable(bool enable);
	void SetTenGigaEnable(bool enable);
	void EnableRateCorrection();
	void SetRateCorrection();
	void SetSpeed(int speed);
	void SetFlags();

	private:
	void SetupWidgetWindow();
	void Initialization();
	void PopulateDetectors();
	void EnableBrowse();
	void GetFileEnable();
	void GetFileName();

	void GetFileFormat();
	void GetFileOverwrite();
	void GetTenGigaEnable();
	void GetRateCorrection();
	void GetSpeed();
	void GetFlags();

	multiSlsDetector *myDet;
	// Button group for radiobuttons for rate
	QButtonGroup *btnGroupRate;
	// enum for the Eiger clock divider
	enum {
		FULLSPEED,
		HALFSPEED,
		QUARTERSPEED,
		NUMBEROFSPEEDS
	};
	// enum for the Eiger readout flags1
	enum { 
		CONTINUOUS, 
		STOREINRAM 
	};
	// enum for the Eiger readout flags2
	enum { 
		PARALLEL, 
		NONPARALLEL
	};
};

