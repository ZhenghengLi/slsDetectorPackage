/*
 * qTabDataOutput.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
/**********************************************************************
 * TO DO
 * 1. Rate correction auto: for eiger depends on settings, tdeadtime{vv,vv,vv} in postprocessing.h
 * ********************************************************************/

#include "qTabDataOutput.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Include Headers
#include <QFileDialog>
// C++ Include Headers
#include <iostream>
#include <string>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------



qTabDataOutput::qTabDataOutput(QWidget *parent,multiSlsDetector*& detector):
						QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDataOutput::~qTabDataOutput(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetupWidgetWindow(){
	// Detector Type
	detType=myDet->getDetectorsType();

	//rate correction  - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER))
		chkRate->setEnabled(true);

	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD))
		chkAngular->setEnabled(true);

	/** error message **/
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	black = QPalette();
	black.setColor(QPalette::Active,QPalette::WindowText,Qt::black);

	red1 = new QPalette();
	red1->setColor(QPalette::Text,Qt::red);
	black1 = new QPalette();
	black1->setColor(QPalette::Text,Qt::black);

	flatFieldTip = dispFlatField->toolTip();
	errFlatFieldTip = QString("<nobr>Flat field corrections.</nobr><br>"
			"<nobr> #flatfield# filename</nobr><br><br>")+
			QString("<nobr><font color=\"red\">"
					"Enter a valid file to enable Flat Field.</font></nobr>");
	outDirTip = boxOutDir->toolTip();



	Initialization();

	disconnect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));
	PopulateDetectors();
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));

	//flat field correction from server
#ifdef VERBOSE
	cout  << "Getting flat field" << endl;
#endif
	UpdateFlatFieldFromServer();


	//rate correction - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER)){
#ifdef VERBOSE
		cout  << "Getting rate correction" << endl;
#endif
		UpdateRateCorrectionFromServer();
	}


	//update angular conversion from server
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD)){
#ifdef VERBOSE
		cout  << "Getting angular conversion" << endl;
#endif
		int ang;
		if(myDet->getAngularConversion(ang))
			chkAngular->setChecked(true);
		emit AngularConversionSignal(chkAngular->isChecked());
	}


	//discard bad channels from server
#ifdef VERBOSE
	cout  << "Getting bad channel correction" << endl;
	cout << "func " << myDet->getBadChannelCorrection() << endl;
#endif



	disconnect(chkDiscardBad,		SIGNAL(toggled(bool)));
	if(myDet->getBadChannelCorrection())
		chkDiscardBad->setChecked(true);
	else
		chkDiscardBad->setChecked(false);
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));

#ifdef VERBOSE
	cout  << "done" << endl;
#endif

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Initialization(){
	//output dir
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));
	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));


	connect(btnOutputBrowse,	SIGNAL(clicked()), 			this, 	SLOT(browseOutputDir()));
	//flat field correction
	connect(chkFlatField,		SIGNAL(toggled(bool)), 		this, 	SLOT(SetFlatField()));
	connect(btnFlatField,		SIGNAL(clicked()), 			this, 	SLOT(BrowseFlatFieldPath()));
	//rate correction
	connect(chkRate,			SIGNAL(toggled(bool)), 			this,	SLOT(SetRateCorrection()));
	connect(radioAuto,			SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioDeadTime,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(spinDeadTime,		SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));
	//angular correction
	connect(chkAngular,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetAngularCorrection()));
	//discard bad channels
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));

}
//-------------------------------------------------------------------------------------------------------------------------------------------------

/*
void qTabDataOutput::setOutputDir(){
	disconnect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(setOutputDir()));

	QString path = dispOutputDir->text();

	string oldPath = myDet->getFilePath();
	bool error=false;

	//gets rid of the end '/'s
	while(path.endsWith('/')) path.chop(1);
	dispOutputDir->setText(path);

	if(myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG){
		for(int i=0;i<myDet->getNumberOfDetectors();i++){
			slsDetector *det = 	myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet);
			oldPath = det->getFilePath();
			det->setFilePath(string(path.toAscii().constData()));
			qDefs::checkErrorMessage(det);
			if(det->getFilePath()!=(string(path.toAscii().constData()))){
				error=true;
				qDefs::checkErrorMessage(det);
				break;
			}
		}
		if(error){
			//set it back for the ones which got set
			for(int i=0;i<myDet->getNumberOfDetectors();i++){
				slsDetector *det = 	myDet->getSlsDetector(i);
				qDefs::checkErrorMessage(myDet);
				det->setFilePath(oldPath);
				qDefs::checkErrorMessage(det);
			}
		}//set it in multi as well if it worked so that they reflect the same
		else
			myDet->setFilePath(string(path.toAscii().constData()));
	}
	else	{
		for(int i=0;i<myDet->getNumberOfDetectors();i++){
			slsDetector *det = 	myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet);
			det->setFilePath(string(path.toAscii().constData()));
			qDefs::checkErrorMessage(det);
			if(det->getFilePath()!=(string(path.toAscii().constData()))){
				error=true;
				qDefs::checkErrorMessage(det);
				break;
			}
		}
	}

	if(!error){
	//if(QFile::exists(path)){
		lblOutputDir->setText("Output Directory: ");
		lblOutputDir->setPalette(chkRate->palette());
		lblOutputDir->setToolTip(outDirTip);
		dispOutputDir->setToolTip(outDirTip);
		btnOutputBrowse->setToolTip(outDirTip);

		myDet->setFilePath(string(path.toAscii().constData()));
	#ifdef VERBOSE
		cout << "Output Directory changed to :"<<myDet->getFilePath() << endl;
	#endif
	}
	else{
		lblOutputDir->setText("Output Directory:*");
		lblOutputDir->setPalette(red);
		QString errTip = outDirTip +
				QString("<nobr><font color=\"red\">"
								"Enter a valid path to change <b>Output Directory</b>.</font></nobr>");
		lblOutputDir->setToolTip(errTip);
		dispOutputDir->setToolTip(errTip);
		btnOutputBrowse->setToolTip(errTip);
	}

	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(setOutputDir()));

	qDefs::checkErrorMessage(myDet);
}

*/
//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::browseOutputDir()
{
	QString directory = QFileDialog::getExistingDirectory(this,tr("Choose Output Directory "),dispOutputDir->text());
	if (!directory.isEmpty())
		dispOutputDir->setText(directory);
	SetOutputDir();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetFlatField(){
#ifdef VERYVERBOSE
	cout << "Entering Set Flat Field Correction Function" << endl;
#endif
	// so that it doesnt call it twice
	disconnect(dispFlatField,		SIGNAL(editingFinished()),	this, 			SLOT(SetFlatField()));

	//enable/disable
	dispFlatField->setEnabled(chkFlatField->isChecked());
	btnFlatField->setEnabled(chkFlatField->isChecked());

	if(chkFlatField->isChecked()){
		if(dispFlatField->text().isEmpty()){
			chkFlatField->setToolTip(errFlatFieldTip);
			dispFlatField->setToolTip(errFlatFieldTip);
			chkFlatField->setPalette(red);
			chkFlatField->setText("Flat Field File:*");
#ifdef VERBOSE
		cout << "Flat Field File is not set." << endl;
#endif
		}else{
			QString fName = dispFlatField->text();
			QString file = fName.section('/',-1);
			QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);

			chkFlatField->setToolTip(flatFieldTip);
			dispFlatField->setToolTip(flatFieldTip);
			chkFlatField->setPalette(chkRate->palette());
			chkFlatField->setText("Flat Field File:");
			//set ff dir
			myDet->setFlatFieldCorrectionDir(dir.toAscii().constData());
			//set ff file and catch error if -1
			if(myDet->setFlatFieldCorrectionFile(file.toAscii().constData())<0){
				string sDir = dir.toAscii().constData(),sFile = file.toAscii().constData();
				if(sDir.length()<1) {sDir = string(QDir::current().absolutePath().toAscii().constData()); /*"/home/";*/}
				qDefs::Message(qDefs::WARNING,"Invalid Flat Field file: "+sDir+"/"+sFile+
						".\nUnsetting Flat Field.","Data Output");

				//Unsetting flat field
				myDet->setFlatFieldCorrectionFile("");
				dispFlatField->setText("");
				chkFlatField->setToolTip(errFlatFieldTip);
				dispFlatField->setToolTip(errFlatFieldTip);
				chkFlatField->setPalette(red);
				chkFlatField->setText("Flat Field File:*");
#ifdef VERBOSE
		cout << "Invalid Flat Field File - "<< sDir << sFile << ". Unsetting Flat Field." << endl;
#endif
			}
			else{
#ifdef VERBOSE
		cout << "Setting flat field file to "<< dispFlatField->text().toAscii().constData() << endl;
#endif
			}
		}
	}else{
		chkFlatField->setToolTip(flatFieldTip);
		dispFlatField->setToolTip(flatFieldTip);
		chkFlatField->setPalette(chkRate->palette());
		chkFlatField->setText("Flat Field File:");
		//Unsetting flat field
		myDet->setFlatFieldCorrectionFile("");
		dispFlatField->setText("");
#ifdef VERBOSE
		cout << "Unsetting flat field correction file" << endl;
#endif
	}

	connect(dispFlatField,SIGNAL(editingFinished()),this,SLOT(SetFlatField()));

	qDefs::checkErrorMessage(myDet);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDataOutput::UpdateFlatFieldFromServer(){
	disconnect(dispFlatField,	SIGNAL(editingFinished()),	this, 	SLOT(SetFlatField()));

	dispFlatField->setText(QString(myDet->getFlatFieldCorrectionDir().c_str())+"/"+QString(myDet->getFlatFieldCorrectionFile().c_str()));
#ifdef VERBOSE
	cout << "Getting flat field correction file" << dispFlatField->text().toAscii().constData() << endl;
#endif
	//calls setflatfield to ensure the file still exists or disable it
	if(!QString(myDet->getFlatFieldCorrectionFile().c_str()).compare("none")){
		dispFlatField->setText("");
		chkFlatField->setChecked(false);
#ifdef VERBOSE
		cout << "Flat Field is not set." << endl;
#endif
	}
	else
		chkFlatField->setChecked(true);

	chkFlatField->setToolTip(flatFieldTip);
	dispFlatField->setToolTip(flatFieldTip);
	chkFlatField->setPalette(chkRate->palette());
	chkFlatField->setText("Flat Field File:");

	connect(dispFlatField,		SIGNAL(editingFinished()),	this,	SLOT(SetFlatField()));

	qDefs::checkErrorMessage(myDet);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDataOutput::BrowseFlatFieldPath(){
	QString fName = dispFlatField->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir =  QString(myDet->getFlatFieldCorrectionDir().c_str());/*"/home/";*/
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Flat Field Correction File"),dir,
			tr("Flat Field Correction Files(*.dat)"),0,QFileDialog::ShowDirsOnly);
	if (!fName.isEmpty()){
		dispFlatField->setText(fName);
		SetFlatField();
	}

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetRateCorrection(){
	disconnect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));

#ifdef VERBOSE
	cout << "Entering Set Rate Correction function" << endl;
#endif

	if(chkRate->isChecked()){
		radioAuto->setEnabled(true);
		radioDeadTime->setEnabled(true);
		//set auto as default if nothing selected
		if(!radioAuto->isChecked()&&!radioDeadTime->isChecked())
			radioAuto->setChecked(true);
		//auto
		if(radioAuto->isChecked()){
			spinDeadTime->setEnabled(false);
			myDet->setRateCorrection(-1);
#ifdef VERBOSE
			cout << "Setting rate corrections with default dead time"  << endl;
#endif
		}//custom dead time
		else{
			spinDeadTime->setEnabled(true);
			myDet->setRateCorrection((double)spinDeadTime->value());
#ifdef VERBOSE
			cout << "Setting rate corrections with dead time "<< spinDeadTime->value() << endl;
#endif
		}
	}//unsetting
	else{
		radioAuto->setEnabled(false);
		radioDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);
		//Unsetting rate correction

		myDet->setRateCorrection(0);
#ifdef VERBOSE
		cout << "Unsetting rate correction" << endl;
#endif
	}
	connect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateRateCorrectionFromServer(){
	disconnect(chkRate,			SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));

	double rate;
	rate = (double)myDet->getRateCorrectionTau();
	qDefs::checkErrorMessage(myDet);
#ifdef VERBOSE
	cout << "Getting rate correction from server:" << rate << " : ";
#endif
	if(rate==0){
#ifdef VERBOSE
	cout << "None" << endl;
#endif
		radioAuto->setEnabled(false);
		radioDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);

		chkRate->setChecked(false);
	}else if(rate<0){
#ifdef VERBOSE
	cout << "Auto" << endl;
#endif
		radioAuto->setEnabled(true);
		radioDeadTime->setEnabled(true);
		spinDeadTime->setEnabled(false);

		chkRate->setChecked(true);
		radioAuto->setChecked(true);
	}else{
#ifdef VERBOSE
	cout << "Custom" << endl;
#endif
		radioAuto->setEnabled(true);
		radioDeadTime->setEnabled(true);
		spinDeadTime->setEnabled(true);

		chkRate->setChecked(true);
		radioDeadTime->setChecked(true);
		spinDeadTime->setValue((double)rate);
	}

	connect(chkRate,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetAngularCorrection(){
#ifdef VERYVERBOSE
	cout << "Entering Set Angular Correction function" << endl;
#endif
	if(chkAngular->isChecked()){
		if(myDet->setAngularConversionFile("default")){
#ifdef VERBOSE
		cout << "Setting angular conversion to default"  << endl;
#endif
		}else{
			qDefs::Message(qDefs::WARNING,"Angular Conversion could not be set. Please set the default file name using the command line, if you haven't already.","Data Output");
			chkAngular->setChecked(false);
		}
	}else{
		myDet->setAngularConversionFile("");
#ifdef VERBOSE
		cout << "Unsetting angular correction" << endl;
#endif
	}

	emit AngularConversionSignal(chkAngular->isChecked());

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::DiscardBadChannels(){
  //#ifdef VERYVERBOSE
	cout << "Entering Discard bad channels function" << endl;
	//#endif
	if(chkDiscardBad->isChecked()){
#ifdef VERBOSE
		cout << "Setting bad channel correction to default"  << endl;
#endif
		myDet->setBadChannelCorrection("default");
	}else{
#ifdef VERBOSE
		cout << "Unsetting bad channel correction" << endl;
#endif
		myDet->setBadChannelCorrection("");
	}

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating DataOutput Tab" << endl;
#endif


	// output dir
#ifdef VERBOSE
	cout  << "Getting output directory" << endl;
#endif
	disconnect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));
	PopulateDetectors();
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));


	//flat field correction from server
#ifdef VERBOSE
	cout  << "Getting flat field" << endl;
#endif
	UpdateFlatFieldFromServer();


	//rate correction - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER)){
#ifdef VERBOSE
		cout  << "Getting rate correction" << endl;
#endif
		UpdateRateCorrectionFromServer();
	}


	//update angular conversion from server
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD)){
#ifdef VERBOSE
		cout  << "Getting angular conversion" << endl;
#endif
		int ang;
		if(myDet->getAngularConversion(ang))
			chkAngular->setChecked(true);
		emit AngularConversionSignal(chkAngular->isChecked());
	}


	//discard bad channels from server
#ifdef VERBOSE
	cout  << "Getting bad channel correction" << endl;
	//	cout << "ff " << myDet->getBadChannelCorrection() << endl;
#endif
	

	disconnect(chkDiscardBad,		SIGNAL(toggled(bool)));
	if(myDet->getBadChannelCorrection())
		chkDiscardBad->setChecked(true);
	else
		chkDiscardBad->setChecked(false);
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));

	if(myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG){
		btnOutputBrowse->setEnabled(false);
		btnOutputBrowse->setToolTip("<font color=\"red\">This button is disabled as receiver PC is different from "
				"client PC and hence different directory structures.</font><br><br>" + dispOutputDir->toolTip());
	}else{
		btnOutputBrowse->setEnabled(true);
		btnOutputBrowse->setToolTip(dispOutputDir->toolTip());
	}

#ifdef VERBOSE
	cout  << "**Updated DataOutput Tab" << endl << endl;
#endif

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::PopulateDetectors(){
#ifdef VERBOSE
	cout << "Populating detectors" << endl;
#endif
	comboDetector->clear();
	if(myDet->setReceiverOnline() == slsDetectorDefs::OFFLINE_FLAG){
		boxOutDir->setTitle("Output Directory");
		comboDetector->addItem("All");
	}

	//if receiver, add detectors
	else{
		boxOutDir->setTitle("Receiver Output Directory");
		for(int i=0;i<myDet->getNumberOfDetectors();i++)
			comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
	}
	GetOutputDir();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::GetOutputDir(){
#ifdef VERBOSE
	cout << "Getting output directory" << endl;
#endif

	if(!comboDetector->itemText(0).compare("All")){
		dispReadOutputDir->setText(QString(myDet->getFilePath().c_str()));
	}else{
		slsDetector *det = 	myDet->getSlsDetector(comboDetector->currentIndex());
		qDefs::checkErrorMessage(myDet);
		dispReadOutputDir->setText(QString(det->getFilePath().c_str()));
	}


	VerifyOutputDirectory();

	//clear field to write
	disconnect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));
	dispOutputDir->setText("");
	dispOutputDir->setPalette(*black1);
	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qTabDataOutput::VerifyOutputDirectory(){
#ifdef VERBOSE
	cout << "Verifying output directory" << endl;
#endif
	bool error = false;
	QString path = "";
	QString errTip = outDirTip;
	QString detName = "";

	path = dispReadOutputDir->text();

	//no receiver
	if(!comboDetector->itemText(0).compare("All")){
		if(myDet->setFilePath(string(path.toAscii().constData())).empty()){
			qDefs::Message(qDefs::WARNING,string("Enter a valid output directory"),"Data Output");
			error = true;
			dispReadOutputDir->setText(QString(myDet->getFilePath().c_str()));
			boxOutDir->setTitle("Receiver Output Directory*");
		}else if(!qDefs::checkErrorMessage(myDet).empty()){
			error = true;
			dispReadOutputDir->setText(QString(myDet->getFilePath().c_str()));
			dispReadOutputDir->setPalette(*red1);
			boxOutDir->setPalette(red);
			boxOutDir->setTitle("Output Directory*");
			errTip = outDirTip +
					QString("<nobr><font color=\"red\">"
							"Enter a valid path to change <b>Output Directory</b>.</font></nobr>");
			boxOutDir->setToolTip(errTip);
		}
	}

	//receiver
	else{
		for(int i=0;i<myDet->getNumberOfDetectors();i++){
			detName = comboDetector->itemText(i);
			slsDetector *det = 	myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet);
			if(det->setFilePath(det->getFilePath()).empty()){
				qDefs::Message(qDefs::WARNING,string("Enter a valid output directory for ") +
						string(detName.toAscii().constData()) + string(" readout"),"Data Output");
				error = true;
				dispReadOutputDir->setText(QString(det->getFilePath().c_str()));
				boxOutDir->setTitle("Receiver Output Directory*");
			}else if(!qDefs::checkErrorMessage(det).empty()){
				error = true;
				dispReadOutputDir->setText(QString(det->getFilePath().c_str()));
				boxOutDir->setTitle("Receiver Output Directory*");
			}
		}
	}

	//if error, display in red
	if(error){
#ifdef VERBOSE
	cout << "The output path doesnt exist anymore" << endl;
#endif
		dispReadOutputDir->setPalette(*red1);
		boxOutDir->setPalette(red);
		errTip = errTip +
				QString("<br><nobr><font color=\"red\">"
						"Enter a valid path for ") + detName +
						QString( " readout to change <b>Output Directory</b>.</font></nobr>");
		boxOutDir->setToolTip(errTip);

		return slsDetectorDefs::FAIL;
	}

	//no error
	else{
#ifdef VERBOSE
	cout << "The output path has been verified" << endl;
#endif
		dispReadOutputDir->setPalette(*black1);
		boxOutDir->setPalette(black);
		if(!comboDetector->itemText(0).compare("All"))
			boxOutDir->setTitle("Output Directory");
		else
			boxOutDir->setTitle("Receiver Output Directory");
		boxOutDir->setToolTip(outDirTip);
	}

	return slsDetectorDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetOutputDir(){

#ifdef VERBOSE
	cout << "Setting output directory" << endl;
#endif

	bool error = false;
	QString path = dispOutputDir->text();

	if(path.isEmpty())
		return;

	disconnect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));

	//gets rid of the end '/'s
	while(path.endsWith('/')) path.chop(1);
	dispOutputDir->setText(path);

	if(!comboDetector->itemText(0).compare("All")){
		myDet->setFilePath(string(dispOutputDir->text().toAscii().constData()));
		if(!qDefs::checkErrorMessage(myDet).empty()){
#ifdef VERBOSE
	cout << "The output path could not be set" << endl;
#endif
			myDet->setFilePath(string(dispReadOutputDir->text().toAscii().constData()));
			error = true;
			dispOutputDir->setPalette(*red1);
		}
	}

	else{
			slsDetector *det = 	myDet->getSlsDetector(comboDetector->currentIndex());
			qDefs::checkErrorMessage(myDet);
			det->setFilePath(string(dispOutputDir->text().toAscii().constData()));
			if(!qDefs::checkErrorMessage(det).empty()){
#ifdef VERBOSE
	cout << "The output path could not be set" << endl;
#endif
				det->setFilePath(string(dispReadOutputDir->text().toAscii().constData()));
				error = true;
				dispOutputDir->setPalette(*red1);
			}

			cout<<"filepath:"<<det->getFilePath()<<endl;
	}


	if(!error){
#ifdef VERBOSE
	cout << "The output path has been modified" << endl;
#endif
		dispOutputDir->setPalette(*black1);
		dispReadOutputDir->setText(dispOutputDir->text());
		dispOutputDir->setText("");
		VerifyOutputDirectory();
	}

	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


