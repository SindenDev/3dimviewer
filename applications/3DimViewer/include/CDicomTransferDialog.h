///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//
// Based on storescp.cc from dcmtk
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CDICOMTRANSFERDIALOG_H
#define CDICOMTRANSFERDIALOG_H

#include <QDialog>
#include <QThread>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/dcasccfg.h>      /* for class DcmAssociationConfiguration */
#include <dcmtk/dcmdata/dcxfer.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmdata/dcmetinf.h>

/////////////////////////////////////////////
// Threaded DICOM receiver

class CDicomTransferDialog;

class CThreadedDICOMReceiver : public QObject
{
    Q_OBJECT
public:
    QString m_destPath;
    CThreadedDICOMReceiver(CDicomTransferDialog *pDialog, int port);
	~CThreadedDICOMReceiver();	
	static QString of2Qt(OFString str);
public slots:
    void doWork(void* pMain);
	void setDestPath(QString path);
signals:
	void statusMessage(QString message);
    void done(int);

protected:
    CDicomTransferDialog *m_pDialog;
    int                   m_nPort;

	OFString			callingPresentationAddress;			// remote hostname or IP address will be stored here
	OFString			lastCallingPresentationAddress;
	OFBool				opt_secureConnection;				// default: no secure connection
	long				opt_endOfStudyTimeout;				// default: no end of study timeout
	OFBool				opt_bitPreserving;					// write data exactly as read
	E_TransferSyntax	opt_networkTransferSyntax;			// preferred network transfer syntaxes - default is explicit VR local byte order 
	OFBool				opt_acceptAllXfers;					// accept all syntaxes
	OFBool				opt_rejectWithoutImplementationUID;
	E_TransferSyntax	opt_writeTransferSyntax;
	E_EncodingType		opt_sequenceType;
	E_PaddingEncoding	opt_paddingType;
	E_GrpLenEncoding	opt_groupLength;
	OFBool				opt_useMetaheader;
	OFBool				opt_correctUIDPadding;
	OFString			opt_outputDirectory;				// output directory equals "."
	OFString			opt_fileNameExtension;

	OFList<OFString>   outputFileNameArray;

	void			executeEndOfStudyEvents();
	DUL_PRESENTATIONCONTEXT *findPresentationContextID(LST_HEAD * head, T_ASC_PresentationContextID presentationContextID);
	OFCondition		acceptUnknownContextsWithTransferSyntax(T_ASC_Parameters * params,  const char* transferSyntax,  T_ASC_SC_ROLE acceptedRole);
	OFCondition		acceptUnknownContextsWithPreferredTransferSyntaxes(T_ASC_Parameters * params,  const char* transferSyntaxes[], int transferSyntaxCount,  T_ASC_SC_ROLE acceptedRole = ASC_SC_ROLE_DEFAULT);
	OFCondition		echoSCP( T_ASC_Association * assoc, T_DIMSE_Message * msg, T_ASC_PresentationContextID presID);
	OFCondition		storeSCP( T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID);
	OFCondition		processCommands(T_ASC_Association * assoc);
	OFCondition		acceptAssociation(T_ASC_Network *net, DcmAssociationConfiguration& asccfg);
    OFCondition     performAssociationCleanup(T_ASC_Association *assoc, OFCondition cond);
	friend void storeSCPCallback(
					void *callbackData,
					T_DIMSE_StoreProgress *progress,
					T_DIMSE_C_StoreRQ *req,
					char * /*imageFileName*/, DcmDataset **imageDataSet,
					T_DIMSE_C_StoreRSP *rsp,
					DcmDataset **statusDetail);
};

/////////////////////////////////////////////

namespace Ui {
class CDicomTransferDialog;
}

class CDicomTransferDialog : public QDialog
{
    Q_OBJECT    
public:
    explicit CDicomTransferDialog(QWidget *parent = NULL);
    ~CDicomTransferDialog();
    // reformats dicom formated date and time to locale based format
    static QString formatDicomDateAndTime(const QString& wsDate, const QString& wsTime);	
	// gets ip address
	static quint32 getIPAddress();
	void AddRow(const QString& text, const QColor& color = QColor(QColor::Invalid));
	//! Folder that should be opened when the dialog is closed
	QString m_openDicomFolder;
private:
    Ui::CDicomTransferDialog *ui;

    // Work thread related stuff
    QThread                     m_workThread;
    bool                        m_bCancelThread;
    CThreadedDICOMReceiver*     m_pDataReceiver;

protected slots:
	void on_pushButtonBrowsePath_clicked();
	void on_pushButtonLoadDicomData_clicked();
	void onStatusMessage(QString text) { AddRow(text); }
public slots:
    bool    cancelThread() const { return m_bCancelThread; }
    void    updateProgressBar(int val, int total);
};

#endif // CDICOMTRANSFERDIALOG_H
