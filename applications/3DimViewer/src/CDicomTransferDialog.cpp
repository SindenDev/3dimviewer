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

#include "CDicomTransferDialog.h"
#include "ui_CDicomTransferDialog.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QDateTime>
#include <QFileIconProvider>
#include <QFileDialog>
#include <QDesktopServices>
#include <QNetworkInterface>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QHostInfo>
#if QT_VERSION >= 0x050000
	#include <QStandardPaths>
#endif
#include <QWaitCondition>
#include <QMutex>

#include <data/CDensityData.h>
#include <data/CModelManager.h>
#include <C3DimApplication.h>
#include <cpreferencesdialog.h>

///////////////////////////////////////////////////////////////////////////////
// Thread safe progress function for the threaded dicom receiver

#ifdef __APPLE__
 bool threadSafeProgressFn(int val, int total, void* customData)
#else
 bool __stdcall threadSafeProgressFn(int val, int total, void* customData)
#endif
{
    if (NULL!=customData)
    {
        QMetaObject::invokeMethod((CDicomTransferDialog*)customData, "updateProgressBar", Q_ARG(int, val), Q_ARG(int, total));
        return !((CDicomTransferDialog*)customData)->cancelThread(); // not thread safe access but we're accessing just a bool
    }
    return true;
}

/////////////////////////////////////////////
// Threaded DICOM receiver implementation

CThreadedDICOMReceiver::CThreadedDICOMReceiver(CDicomTransferDialog *pDialog, int port)
{
    m_pDialog = pDialog;
    m_nPort = port;
	opt_secureConnection = OFFalse; // default: no secure connection
	opt_endOfStudyTimeout = -1;		// default: no end of study timeout
	opt_bitPreserving = OFFalse;	// write data exactly as read
	opt_networkTransferSyntax = EXS_Unknown; // preferred network transfer syntaxes - default is explicit VR local byte order 
	opt_acceptAllXfers = OFFalse;
	opt_rejectWithoutImplementationUID = OFFalse;
	opt_writeTransferSyntax = EXS_Unknown;
	opt_sequenceType = EET_ExplicitLength;
	opt_paddingType = EPD_withoutPadding;
	opt_groupLength = EGL_recalcGL;
	opt_useMetaheader = OFTrue;
	opt_correctUIDPadding = OFFalse;
	opt_fileNameExtension = ".dcm";
	opt_outputDirectory = ".";
}

CThreadedDICOMReceiver::~CThreadedDICOMReceiver()
{
}

void CThreadedDICOMReceiver::setDestPath(QString path)
{
	m_destPath = path;
#ifdef _WIN32
	std::wstring uniName = (const wchar_t*)m_destPath.utf16();
	std::string str = C3DimApplication::wcs2ACP(uniName);
#else
	std::string str = m_destPath.toStdString();
#endif
	opt_outputDirectory = str.c_str();
	assert(!str.empty());
	
}

QString CThreadedDICOMReceiver::of2Qt(OFString str)
{
	std::string s = str.c_str();
#ifdef _WIN32
    std::wstring wName = C3DimApplication::ACP2wcs(s);
    return QString::fromUtf16( (const ushort*)wName.c_str() );
#else
	return QString::fromStdString(s);	
#endif
}

#define OFLOG_ANY(x,y) { std::stringstream x; x<< y; std::string str = x.str(); emit statusMessage(QString::fromStdString(str)); }
#define OFLOG_ERROR(x,y) OFLOG_ANY(x,y)
#define OFLOG_INFO(x,y) OFLOG_ANY(x,y)
#define OFLOG_DEBUG(x,y) OFLOG_ANY(x,y)
#define OFLOG_FATAL(x,y) OFLOG_ANY(x,y)
#define OFLOG_WARN(x,y) OFLOG_ANY(x,y)

// This function deals with the execution of end-of-study-events.
void CThreadedDICOMReceiver::executeEndOfStudyEvents()    
{
  
}

DUL_PRESENTATIONCONTEXT * CThreadedDICOMReceiver::findPresentationContextID(LST_HEAD * head, T_ASC_PresentationContextID presentationContextID)
{
	if (NULL == head)
		return NULL;

	LST_HEAD **l = &head;
	if (NULL == *l)
		return NULL;
  
	DUL_PRESENTATIONCONTEXT * pc = OFstatic_cast(DUL_PRESENTATIONCONTEXT *, LST_Head(l));
	LST_Position(l, OFstatic_cast(LST_NODE *, pc));
	while (pc) 
	{
		if (pc->presentationContextID == presentationContextID)
			break;
		pc = OFstatic_cast(DUL_PRESENTATIONCONTEXT *, LST_Next(l));
	}
	return pc;
}


/** accept all presentation contexts for unknown SOP classes,
 *  i.e. UIDs appearing in the list of abstract syntaxes
 *  where no corresponding name is defined in the UID dictionary.
 *  @param params pointer to association parameters structure
 *  @param transferSyntax transfer syntax to accept
 *  @param acceptedRole SCU/SCP role to accept
 */
OFCondition CThreadedDICOMReceiver::acceptUnknownContextsWithTransferSyntax(
    T_ASC_Parameters * params,
    const char* transferSyntax,
    T_ASC_SC_ROLE acceptedRole)
{
    OFCondition cond = EC_Normal;
    int n = ASC_countPresentationContexts(params);
    for (int i = 0; i < n; i++)
    {
        T_ASC_PresentationContext pc;
        cond = ASC_getPresentationContext(params, i, &pc);
        if (cond.bad()) 
            return cond;
        OFBool abstractOK = OFFalse;
        OFBool accepted = OFFalse;
        if (dcmFindNameOfUID(pc.abstractSyntax) == NULL)
        {
            abstractOK = OFTrue;
            // check the transfer syntax
            for (int k = 0; (k < OFstatic_cast(int, pc.transferSyntaxCount)) && !accepted; k++)
            {
                if (strcmp(pc.proposedTransferSyntaxes[k], transferSyntax) == 0)
                    accepted = OFTrue;
            }
        }
        if (accepted)
        {
            cond = ASC_acceptPresentationContext(params, pc.presentationContextID, transferSyntax, acceptedRole);
            if (cond.bad()) 
                return cond;
        }
        else 
        {
            T_ASC_P_ResultReason reason;
            // do not refuse if already accepted
            DUL_PRESENTATIONCONTEXT * dpc = findPresentationContextID(params->DULparams.acceptedPresentationContext, pc.presentationContextID);
            if ((dpc == NULL) || ((dpc != NULL) && (dpc->result != ASC_P_ACCEPTANCE)))
            {
                if (abstractOK)
                    reason = ASC_P_TRANSFERSYNTAXESNOTSUPPORTED;
                else
                    reason = ASC_P_ABSTRACTSYNTAXNOTSUPPORTED;
                // If previously this presentation context was refused because of bad transfer syntax let it stay that way.
                if ((dpc != NULL) && (dpc->result == ASC_P_TRANSFERSYNTAXESNOTSUPPORTED))
                    reason = ASC_P_TRANSFERSYNTAXESNOTSUPPORTED;
                cond = ASC_refusePresentationContext(params, pc.presentationContextID, reason);
                if (cond.bad()) 
                    return cond;
            }
        }
    }
    return EC_Normal;
}


/** accept all presenstation contexts for unknown SOP classes,
 *  i.e. UIDs appearing in the list of abstract syntaxes
 *  where no corresponding name is defined in the UID dictionary.
 *  This method is passed a list of "preferred" transfer syntaxes.
 *  @param params pointer to association parameters structure
 *  @param transferSyntax transfer syntax to accept
 *  @param acceptedRole SCU/SCP role to accept
 */
OFCondition CThreadedDICOMReceiver::acceptUnknownContextsWithPreferredTransferSyntaxes(
    T_ASC_Parameters * params,
    const char* transferSyntaxes[], int transferSyntaxCount,
    T_ASC_SC_ROLE acceptedRole)
{
	OFCondition cond = EC_Normal;
	// Accept in the order "least wanted" to "most wanted" transfer syntax.  
	// Accepting a transfer syntax will override previously accepted transfer syntaxes.
	for (int i = transferSyntaxCount - 1; i >= 0; i--)
	{
		cond = acceptUnknownContextsWithTransferSyntax(params, transferSyntaxes[i], acceptedRole);
		if (cond.bad()) return cond;
	}
	return cond;
} 

OFCondition CThreadedDICOMReceiver::echoSCP( T_ASC_Association * assoc, T_DIMSE_Message * msg, T_ASC_PresentationContextID presID)
{
    OFString temp_str;
    // assign the actual information of the C-Echo-RQ command to a local variable
    T_DIMSE_C_EchoRQ *req = &msg->msg.CEchoRQ;
    /*if (storescpLogger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
    {
    OFLOG_INFO(storescpLogger, "Received Echo Request");
    OFLOG_DEBUG(storescpLogger, DIMSE_dumpMessage(temp_str, *req, DIMSE_INCOMING, NULL, presID));
    } else */
    {
        OFLOG_INFO(storescpLogger, "Received Echo Request (MsgID " << req->MessageID << ")");
    }
    // the echo succeeded
    OFCondition cond = DIMSE_sendEchoResponse(assoc, presID, req, STATUS_Success, NULL);
    if (cond.bad())
    {
        OFLOG_ERROR(storescpLogger, "Echo SCP Failed: " << DimseCondition::dump(temp_str, cond));
    }
    return cond;
}


struct StoreCallbackData
{
	char* imageFileName;
	DcmFileFormat* dcmff;
	T_ASC_Association* assoc;
	CThreadedDICOMReceiver* pRecv;
};

void storeSCPCallback(
    void *callbackData,
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ *req,
    char * /*imageFileName*/, DcmDataset **imageDataSet,
    T_DIMSE_C_StoreRSP *rsp,
    DcmDataset **statusDetail)
    /*
     * This function.is used to indicate progress when storescp receives instance data over the
     * network. On the final call to this function (identified by progress->state == DIMSE_StoreEnd)
     * this function will store the data set which was received over the network to a file.
     * Earlier calls to this function will simply cause some information to be dumped to stdout.
     *
     * Parameters:
     *   callbackData  - [in] data for this callback function
     *   progress      - [in] The state of progress. (identifies if this is the initial or final call
     *                   to this function, or a call in between these two calls.
     *   req           - [in] The original store request message.
     *   imageFileName - [in] The path to and name of the file the information shall be written to.
     *   imageDataSet  - [in] The data set which shall be stored in the image file
     *   rsp           - [inout] the C-STORE-RSP message (will be sent after the call to this function)
     *   statusDetail  - [inout] This variable can be used to capture detailed information with regard to
     *                   the status information which is captured in the status element (0000,0900). Note
     *                   that this function does specify any such information, the pointer will be set to NULL.
     */
{
    // remember callback data
    StoreCallbackData *cbdata = OFstatic_cast(StoreCallbackData *, callbackData);

    // determine if the association shall be aborted
    OFBool  opt_abortDuringStore = cbdata->pRecv->m_pDialog->cancelThread() ? OFTrue : OFFalse; // thread unsafe access to bool
    OFBool  opt_abortAfterStore = OFFalse;
    if ((opt_abortDuringStore && progress->state != DIMSE_StoreBegin) ||
        (opt_abortAfterStore && progress->state == DIMSE_StoreEnd))
    {
        emit cbdata->pRecv->statusMessage("ABORT initiated (due to command line options)");
        ASC_abortAssociation((OFstatic_cast(StoreCallbackData*, callbackData))->assoc);
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
        return;
    }

    // if opt_sleepAfter is set, the user requires that the application shall
    // sleep a certain amount of seconds after having received one PDU.
    OFCmdUnsignedInt   opt_sleepDuring = 0;
    if (opt_sleepDuring > 0)
        OFStandard::sleep(OFstatic_cast(unsigned int, opt_sleepDuring));

    // indicate progress state
    switch (progress->state)
    {
    case DIMSE_StoreBegin:
        threadSafeProgressFn(0, 100, cbdata->pRecv->m_pDialog);
        break;
    case DIMSE_StoreEnd:
        threadSafeProgressFn(0, 100, cbdata->pRecv->m_pDialog);
        break;
    default:
        {   // totalBytes is just an approximation, therefore we apply a hack with a multiplier
            int mult = 1;
            while (progress->progressBytes > progress->totalBytes*mult && mult<=10)
                mult++;
            threadSafeProgressFn(progress->progressBytes, progress->totalBytes*mult, cbdata->pRecv->m_pDialog);
        }
        break;
    }

    // if this is the final call of this function, save the data which was received to a file
    if (progress->state == DIMSE_StoreEnd)
    {
        OFString tmpStr;

        // do not send status detail information
        *statusDetail = NULL;

        // Concerning the following line: an appropriate status code is already set in the resp structure,
        // it need not be success. For example, if the caller has already detected an out of resources problem
        // then the status will reflect this.  The callback function is still called to allow cleanup.
        //rsp->DimseStatus = STATUS_Success;

        // we want to write the received information to a file only if this information
        // is present and the option opt_bitPreserving is not set.
        if ((imageDataSet != NULL) && (*imageDataSet != NULL) && !cbdata->pRecv->opt_bitPreserving)
        {
            OFString fileName = cbdata->imageFileName;

            // update global variables outputFileNameArray
            // (might be used in executeOnReception() and renameOnEndOfStudy)
            cbdata->pRecv->outputFileNameArray.push_back(OFStandard::getFilenameFromPath(tmpStr, fileName));

            // determine the transfer syntax which shall be used to write the information to the file
            E_TransferSyntax xfer = cbdata->pRecv->opt_writeTransferSyntax;
            if (xfer == EXS_Unknown) 
                xfer = (*imageDataSet)->getOriginalXfer();

            // store file either with meta header or as pure dataset
            emit cbdata->pRecv->statusMessage("Storing DICOM file " + CThreadedDICOMReceiver::of2Qt(fileName));
            if (OFStandard::fileExists(fileName))
            {
                emit cbdata->pRecv->statusMessage("! DICOM file already exists, overwriting " + CThreadedDICOMReceiver::of2Qt(fileName));
            }
            OFCmdUnsignedInt   opt_filepad = 0;
            OFCmdUnsignedInt   opt_itempad = 0;
            OFCondition cond = cbdata->dcmff->saveFile(fileName.c_str(), xfer, cbdata->pRecv->opt_sequenceType, cbdata->pRecv->opt_groupLength,
                cbdata->pRecv->opt_paddingType, OFstatic_cast(Uint32, opt_filepad), OFstatic_cast(Uint32, opt_itempad),
                (cbdata->pRecv->opt_useMetaheader) ? EWM_fileformat : EWM_dataset);
            if (cond.bad())
            {
                emit cbdata->pRecv->statusMessage("!! cannot write DICOM file " + CThreadedDICOMReceiver::of2Qt(fileName) + CThreadedDICOMReceiver::of2Qt(OFString(cond.text())));
                rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
            }

            // check the image to make sure it is consistent, i.e. that its sopClass and sopInstance correspond
            // to those mentioned in the request. If not, set the status in the response message variable.
            if ((rsp->DimseStatus == STATUS_Success))
            {
                DIC_UI sopClass;
                DIC_UI sopInstance;
                // which SOP class and SOP instance ?
                if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, cbdata->pRecv->opt_correctUIDPadding))
                {
                    emit cbdata->pRecv->statusMessage("!! bad DICOM file: " + CThreadedDICOMReceiver::of2Qt(fileName));
                    rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
                }
                else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
                {
                    rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
                }
                else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
                {
                    rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
                }
            }
        }

        // in case opt_bitPreserving is set, do some other things
        if (cbdata->pRecv->opt_bitPreserving)
        {
            // we need to set outputFileNameArray and outputFileNameArrayCnt to be
            // able to perform the placeholder substitution in executeOnReception()
            cbdata->pRecv->outputFileNameArray.push_back(OFStandard::getFilenameFromPath(tmpStr, cbdata->imageFileName));
        }
    }
}



OFCondition CThreadedDICOMReceiver::storeSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  T_ASC_PresentationContextID presID)
    /*
     * This function processes a DIMSE C-STORE-RQ commmand that was received over the network connection.
     * Parameters:
     *   assoc  - [in] The association (network connection to another DICOM application).
     *   msg    - [in] The DIMSE C-STORE-RQ message that was received.
     *   presID - [in] The ID of the presentation context which was specified in the PDV which contained
     *                 the DIMSE command.
     */
{
    OFCondition cond = EC_Normal;
    char imageFileName[4096] = {};

    // assign the actual information of the C-STORE-RQ command to a local variable
    T_DIMSE_C_StoreRQ * req = &msg->msg.CStoreRQ;

    // create a real filename (consisting of path and filename)
    {
        // don't create new UID, use the study instance UID as found in object
        sprintf(imageFileName, "%s%c%s.%s%s", opt_outputDirectory.c_str(), PATH_SEPARATOR, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"),
            req->AffectedSOPInstanceUID, opt_fileNameExtension.c_str());
    }

    // dump some information if required
    OFString str;
    /*if (storescpLogger.isEnabledFor(OFLogger::DEBUG_LOG_LEVEL))
    {
    OFLOG_INFO(storescpLogger, "Received Store Request");
    OFLOG_DEBUG(storescpLogger, DIMSE_dumpMessage(str, *req, DIMSE_INCOMING, NULL, presID));
    } else */
    {
        OFLOG_INFO(storescpLogger, "Received Store Request (MsgID " << req->MessageID << ", "
            << dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "OT") << ")");
    }

    // intialize some variables
    StoreCallbackData callbackData = {};
    callbackData.assoc = assoc;
    callbackData.imageFileName = imageFileName;
    DcmFileFormat dcmff;
    callbackData.dcmff = &dcmff;
    callbackData.pRecv = this;

    // store SourceApplicationEntityTitle in metaheader
    if (assoc && assoc->params)
    {
        const char *aet = assoc->params->DULparams.callingAPTitle;
        if (aet) dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
    }

    // define an address where the information which will be received over the network will be stored
    DcmDataset *dset = dcmff.getDataset();

    // if opt_bitPreserving is set, the user requires that the data shall be
    // written exactly as it was received. Depending on this option, function
    // DIMSE_storeProvider must be called with certain parameters.
    const int opt_dimse_timeout = 0; // timeout for DIMSE messages (default: unlimited)
    const T_DIMSE_BlockingMode opt_blockMode = DIMSE_BLOCKING;
    if (opt_bitPreserving)
    {
        cond = DIMSE_storeProvider(assoc, presID, req, imageFileName, opt_useMetaheader, NULL,
            storeSCPCallback, &callbackData, opt_blockMode, opt_dimse_timeout);
    }
    else
    {
        cond = DIMSE_storeProvider(assoc, presID, req, NULL, opt_useMetaheader, &dset,
            storeSCPCallback, &callbackData, opt_blockMode, opt_dimse_timeout);
    }

    // if some error occured, dump corresponding information and remove the outfile if necessary
    if (cond.bad())
    {
        OFString temp_str;
        OFLOG_ERROR(storescpLogger, "Store SCP Failed: " << DimseCondition::dump(temp_str, cond));
        if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0)
            OFStandard::deleteFile(imageFileName);
    }

    // if everything was successful so far, go ahead and handle possible end-of-study events
    if (cond.good())
        executeEndOfStudyEvents();

    return cond;
}


OFCondition CThreadedDICOMReceiver::processCommands(T_ASC_Association * assoc)
    /*
     * This function receives DIMSE commmands over the network connection
     * and handles these commands correspondingly. Note that in case of
     * storescp only C-ECHO-RQ and C-STORE-RQ commands can be processed.
     *
     * Parameters:
     *   assoc - [in] The association (network connection to another DICOM application).
     */
{
  OFCondition cond = EC_Normal;
  T_DIMSE_Message msg;
  T_ASC_PresentationContextID presID = 0;
  DcmDataset *statusDetail = NULL;

  // start a loop to be able to receive more than one DIMSE command
  while( cond == EC_Normal || cond == DIMSE_NODATAAVAILABLE || cond == DIMSE_OUTOFRESOURCES )
  {
    // receive a DIMSE command over the network
    if( opt_endOfStudyTimeout == -1 )
      cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &presID, &msg, &statusDetail);
    else
      cond = DIMSE_receiveCommand(assoc, DIMSE_NONBLOCKING, OFstatic_cast(int, opt_endOfStudyTimeout), &presID, &msg, &statusDetail);

    // check what kind of error occurred. 
    if( cond == DIMSE_NODATAAVAILABLE )
    {
    }

    // if the command which was received has extra status
    // detail information, dump this information
    if (statusDetail != NULL)
    {
      OFLOG_DEBUG(storescpLogger, "Status Detail:" << OFendl << DcmObject::PrintHelper(*statusDetail));
      delete statusDetail;
    }

    // check if peer did release or abort, or if we have a valid message
    if (cond == EC_Normal)
    {
      // in case we received a valid message, process this command
      // note that storescp can only process a C-ECHO-RQ and a C-STORE-RQ
      switch (msg.CommandField)
      {
        case DIMSE_C_ECHO_RQ:
          // process C-ECHO-Request
          cond = echoSCP(assoc, &msg, presID);
          break;
        case DIMSE_C_STORE_RQ:
          // process C-STORE-Request
          cond = storeSCP(assoc, &msg, presID);
          break;
        default:
          OFString tempStr;
          // we cannot handle this kind of message
          cond = DIMSE_BADCOMMANDTYPE;
          OFLOG_ERROR(storescpLogger, "Expected C-ECHO or C-STORE request but received DIMSE command 0x"
               << STD_NAMESPACE hex << STD_NAMESPACE setfill('0') << STD_NAMESPACE setw(4)
               << OFstatic_cast(unsigned, msg.CommandField));
          OFLOG_DEBUG(storescpLogger, DIMSE_dumpMessage(tempStr, msg, DIMSE_INCOMING, NULL, presID));
          break;
      }
    }
  }
  return cond;
}

OFCondition CThreadedDICOMReceiver::performAssociationCleanup(T_ASC_Association *assoc, OFCondition cond)
{
    if (cond.code() == DULC_FORKEDCHILD)
        return cond;

    OFString temp_str;
    cond = ASC_dropSCPAssociation(assoc);
    if (cond.bad())
    {
        OFLOG_FATAL(storescpLogger, DimseCondition::dump(temp_str, cond));
        return cond;
    }
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad())
    {
        OFLOG_FATAL(storescpLogger, DimseCondition::dump(temp_str, cond));
        return cond;
    }
    return cond;
}

OFCondition CThreadedDICOMReceiver::acceptAssociation(T_ASC_Network *net, DcmAssociationConfiguration& asccfg)
{
    OFString temp_str;

    const char* knownAbstractSyntaxes[] = { UID_VerificationSOPClass };
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    // try to receive an association. Here we either want to use blocking or
    // non-blocking, depending on if the option --eostudy-timeout is set.
    T_ASC_Association *assoc = NULL;
    OFCondition cond;
    if (opt_endOfStudyTimeout == -1)
        cond = ASC_receiveAssociation(net, &assoc, ASC_DEFAULTMAXPDU, NULL, NULL, opt_secureConnection, DUL_NOBLOCK, 15);
    else
        cond = ASC_receiveAssociation(net, &assoc, ASC_DEFAULTMAXPDU, NULL, NULL, opt_secureConnection, DUL_NOBLOCK, OFstatic_cast(int, opt_endOfStudyTimeout));

    if (cond.code() == DULC_FORKEDCHILD)
    {
        // OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
        return cond;
    }

    if (!cond.bad())
    {
        OFLOG_INFO(storescpLogger, "Association Received");

        /* dump presentation contexts if required */
        if (0)
        {
            OFLOG_DEBUG(storescpLogger, "Parameters:" << OFendl << ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_RQ));
        }

        OFBool opt_refuseAssociation = m_pDialog->cancelThread() ? OFTrue : OFFalse;
        if (opt_refuseAssociation)
        {
            T_ASC_RejectParameters rej = {
                ASC_RESULT_REJECTEDPERMANENT,
                ASC_SOURCE_SERVICEUSER,
                ASC_REASON_SU_NOREASON
            };
            OFLOG_INFO(storescpLogger, "Refusing Association (user request)");
            cond = ASC_rejectAssociation(assoc, &rej);
            if (cond.bad())
            {
                OFLOG_ERROR(storescpLogger, "Association Reject Failed: " << DimseCondition::dump(temp_str, cond));
            }

            return performAssociationCleanup(assoc,cond);
        }

        switch (opt_networkTransferSyntax)
        {
        case EXS_LittleEndianImplicit:
            /* we only support Little Endian Implicit */
            transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 1;
            break;
        case EXS_LittleEndianExplicit:
            /* we prefer Little Endian Explicit */
            transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 3;
            break;
        case EXS_BigEndianExplicit:
            /* we prefer Big Endian Explicit */
            transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 3;
            break;
        case EXS_JPEGProcess14SV1:
            /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
            transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_JPEGProcess1:
            /* we prefer JPEGBaseline (default lossy for 8 bit images) */
            transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_JPEGProcess2_4:
            /* we prefer JPEGExtended (default lossy for 12 bit images) */
            transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_JPEG2000LosslessOnly:
            /* we prefer JPEG2000 Lossless */
            transferSyntaxes[0] = UID_JPEG2000LosslessOnlyTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_JPEG2000:
            /* we prefer JPEG2000 Lossy */
            transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_JPEGLSLossless:
            /* we prefer JPEG-LS Lossless */
            transferSyntaxes[0] = UID_JPEGLSLosslessTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_JPEGLSLossy:
            /* we prefer JPEG-LS Lossy */
            transferSyntaxes[0] = UID_JPEGLSLossyTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_MPEG2MainProfileAtMainLevel:
            /* we prefer MPEG2 MP@ML */
            transferSyntaxes[0] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_MPEG2MainProfileAtHighLevel:
            /* we prefer MPEG2 MP@HL */
            transferSyntaxes[0] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_MPEG4HighProfileLevel4_1:
            /* we prefer MPEG4 HP/L4.1 */
            transferSyntaxes[0] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_MPEG4BDcompatibleHighProfileLevel4_1:
            /* we prefer MPEG4 BD HP/L4.1 */
            transferSyntaxes[0] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
        case EXS_RLELossless:
            /* we prefer RLE Lossless */
            transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
#ifdef WITH_ZLIB
        case EXS_DeflatedLittleEndianExplicit:
            /* we prefer Deflated Explicit VR Little Endian */
            transferSyntaxes[0] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
            transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
            transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 4;
            break;
#endif
        default:
            if (opt_acceptAllXfers)
            {
                /* we accept all supported transfer syntaxes
                 * (similar to "AnyTransferSyntax" in "storescp.cfg")
                 */
                transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
                transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
                transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
                transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
                transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;
                transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
                transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
                transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
                transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
                transferSyntaxes[9] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
                transferSyntaxes[10] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
                transferSyntaxes[11] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
                transferSyntaxes[12] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
                if (gLocalByteOrder == EBO_LittleEndian)
                {
                    transferSyntaxes[13] = UID_LittleEndianExplicitTransferSyntax;
                    transferSyntaxes[14] = UID_BigEndianExplicitTransferSyntax;
                }
                else {
                    transferSyntaxes[13] = UID_BigEndianExplicitTransferSyntax;
                    transferSyntaxes[14] = UID_LittleEndianExplicitTransferSyntax;
                }
                transferSyntaxes[15] = UID_LittleEndianImplicitTransferSyntax;
                numTransferSyntaxes = 16;
            }
            else {
                /* We prefer explicit transfer syntaxes.
                 * If we are running on a Little Endian machine we prefer
                 * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
                 */
                if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
                {
                    transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
                    transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
                }
                else
                {
                    transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
                    transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
                }
                transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
                numTransferSyntaxes = 3;
            }
            break;
        }

        {
            /* accept the Verification SOP Class if presented */
            cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params, knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes), transferSyntaxes, numTransferSyntaxes);
            if (cond.bad())
            {
                OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
                return performAssociationCleanup(assoc, cond);
            }

            /* the array of Storage SOP Class UIDs comes from dcuid.h */
            cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params, dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs, transferSyntaxes, numTransferSyntaxes);
            if (cond.bad())
            {
                OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
                return performAssociationCleanup(assoc, cond);
            }

            OFBool opt_promiscuous = OFFalse;
            if (opt_promiscuous)
            {
                // accept everything not known not to be a storage SOP class 
                cond = acceptUnknownContextsWithPreferredTransferSyntaxes(
                    assoc->params, transferSyntaxes, numTransferSyntaxes);
                if (cond.bad())
                {
                    OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
                    return performAssociationCleanup(assoc, cond);
                }
            }
        }
        // set our app title 
        const char *       opt_respondingAETitle = "TraumaTech";
        ASC_setAPTitles(assoc->params, NULL, NULL, opt_respondingAETitle);

        // acknowledge or reject this association 
        char buf[BUFSIZ] = {};
        cond = ASC_getApplicationContextName(assoc->params, buf);
        if ((cond.bad()) || strcmp(buf, UID_StandardApplicationContext) != 0)
        {
            // reject: the application context name is not supported 
            T_ASC_RejectParameters rej =
            {
                ASC_RESULT_REJECTEDPERMANENT,
                ASC_SOURCE_SERVICEUSER,
                ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED
            };

            OFLOG_INFO(storescpLogger, "Association Rejected: Bad Application Context Name: " << buf);
            cond = ASC_rejectAssociation(assoc, &rej);
            if (cond.bad())
            {
                OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
            }
            return performAssociationCleanup(assoc, cond);

        }
        else if (opt_rejectWithoutImplementationUID && strlen(assoc->params->theirImplementationClassUID) == 0)
        {
            // reject: the no implementation Class UID provided 
            T_ASC_RejectParameters rej =
            {
                ASC_RESULT_REJECTEDPERMANENT,
                ASC_SOURCE_SERVICEUSER,
                ASC_REASON_SU_NOREASON
            };

            OFLOG_INFO(storescpLogger, "Association Rejected: No Implementation Class UID provided");
            cond = ASC_rejectAssociation(assoc, &rej);
            if (cond.bad())
            {
                OFLOG_DEBUG(storescpLogger, DimseCondition::dump(temp_str, cond));
            }
            return performAssociationCleanup(assoc, cond);
        }
        else
        {
            cond = ASC_acknowledgeAssociation(assoc);
            if (cond.bad())
            {
                OFLOG_ERROR(storescpLogger, DimseCondition::dump(temp_str, cond));
                return performAssociationCleanup(assoc, cond);
            }
            OFString remoteMachine = OFSTRING_GUARD(assoc->params->DULparams.callingPresentationAddress);
            OFLOG_INFO(storescpLogger, "Association Acknowledged (Max Send PDV: " << assoc->sendPDVLength << ") " << remoteMachine);
            if (ASC_countAcceptedPresentationContexts(assoc->params) == 0)
                OFLOG_INFO(storescpLogger, "    (but no valid presentation contexts)");
            /* dump the presentation contexts which have been accepted/refused */
            if (0)
            {
                OFLOG_DEBUG(storescpLogger, ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_AC));
            }
        }

#ifdef BUGGY_IMPLEMENTATION_CLASS_UID_PREFIX
        /* active the dcmPeerRequiresExactUIDCopy workaround code
         * (see comments in dimse.h) for a implementation class UID
         * prefix known to exhibit the buggy behaviour.
         */
        if (0 == strncmp(assoc->params->theirImplementationClassUID,
            BUGGY_IMPLEMENTATION_CLASS_UID_PREFIX,
            strlen(BUGGY_IMPLEMENTATION_CLASS_UID_PREFIX)))
        {
            dcmEnableAutomaticInputDataCorrection.set(OFFalse);
            dcmPeerRequiresExactUIDCopy.set(OFTrue);
        }
#endif

        // store previous values for later use
        lastCallingPresentationAddress = callingPresentationAddress;
        // store calling presentation address (i.e. remote hostname)
        callingPresentationAddress = OFSTRING_GUARD(assoc->params->DULparams.callingPresentationAddress);

        /* now do the real work, i.e. receive DIMSE commmands over the network connection */
        /* which was established and handle these commands correspondingly. In case of */
        /* storescp only C-ECHO-RQ and C-STORE-RQ commands can be processed. */
        cond = processCommands(assoc);

        if (cond == DUL_PEERREQUESTEDRELEASE)
        {
            OFLOG_INFO(storescpLogger, "Association Release");
            cond = ASC_acknowledgeRelease(assoc);
        }
        else if (cond == DUL_PEERABORTEDASSOCIATION)
        {
            OFLOG_INFO(storescpLogger, "Association Aborted");
        }
        else
        {
            OFLOG_ERROR(storescpLogger, "DIMSE failure (aborting association): " << DimseCondition::dump(temp_str, cond));
            /* some kind of error so abort the association */
            cond = ASC_abortAssociation(assoc);
        }
    }
    else // if some kind of error occured, take care of it
    {
        // check what kind of error occurred. If no association was
        // received, check if certain other conditions are met
        if (cond == DUL_NOASSOCIATIONREQUEST)
        {

        }
        // If something else was wrong we might have to dump an error message.
        else
        {
            OFLOG_ERROR(storescpLogger, "Receiving Association failed: " << DimseCondition::dump(temp_str, cond));
        }

        // no matter what kind of error occurred, we need to do a cleanup
    }

    return performAssociationCleanup(assoc, cond);
}



// http://qt-project.org/wiki/Threads_Events_QObjects
void CThreadedDICOMReceiver::doWork(void* pMain)
{
#ifdef _OPENMP
    // because there is a bug in microsoft openmp implementation giving an own set of omp threads to every thread and not cleaning them up
    // we use this call to hack this behaviour
    omp_set_dynamic(omp_get_max_threads()); 
#endif
    
	CDicomTransferDialog* pDialog = (CDicomTransferDialog*)pMain;
	/*
	for(int i=0;i<100;i++)
	{
		if (NULL!=pDialog && pDialog->cancelThread())
			break;
		threadSafeProgressFn(i,100,pMain);
		QWaitCondition waitCondition;
		QMutex mutex;
 
		waitCondition.wait(&mutex, 100);
	}*/

	// initialize network, i.e. create an instance of T_ASC_Network*
	int opt_acse_timeout = 30;
	T_ASC_Network *net = NULL;
	OFCmdUnsignedInt   opt_port = m_nPort;
	OFCondition cond = ASC_initializeNetwork(NET_ACCEPTOR, OFstatic_cast(int, opt_port), opt_acse_timeout, &net);
	if (cond.bad())
	{
		if (NULL!=cond.text())
		{
			std::string str = cond.text();
			emit statusMessage(tr("Cannot create network %1").arg(QString::fromStdString(str)));
		}
		else
			emit statusMessage(tr("Cannot create network %1").arg(cond.code()));
	}
	else
	{
		emit statusMessage(tr("Network initialized, port %1, ip %2").arg(opt_port).arg(QHostAddress(CDicomTransferDialog::getIPAddress()).toString()));

		DcmAssociationConfiguration asccfg;
		while (cond.good())
		{
			if (NULL!=pDialog && pDialog->cancelThread())
				break;
			// receive an association and acknowledge or reject it. If the association was acknowledged, 
			// offer corresponding services and invoke one or more if required. 
			cond = acceptAssociation(net, asccfg);
		}
		// drop the network, i.e. free memory of T_ASC_Network* structure initialized by ASC_initializeNetwork
		cond = ASC_dropNetwork(&net);
		if (cond.bad())
		{
			if (NULL!=cond.text())
			{
				std::string str = cond.text();
				emit statusMessage(tr("Error %1").arg(QString::fromStdString(str)));
			}
			else
				emit statusMessage(tr("Error %1").arg(QString::number(cond.code())));
		}
		else
			emit statusMessage("Network deinitialized");
	}
    //this->deleteLater(); // will be deleted by CDicomTransferDialog
}


///////////////////////////////////////////////////////////////////////////////

// static method
QString CDicomTransferDialog::formatDicomDateAndTime(const QString& wsDate, const QString& wsTime)
{
    // convert date and time from dicom format
    QDate date = QDate::fromString(wsDate,"yyyy.MM.dd");
    if (!date.isValid())
         date = QDate::fromString(wsDate,"yyyyMMdd");
	if (!date.isValid() && !wsDate.isEmpty())
		return wsDate + " " + wsTime;
    QTime time = QTime::fromString(wsTime,"hhmmss.zzz");
    if (!time.isValid())
        time = QTime::fromString(wsTime,"hhmmss");
    if (!time.isValid())
        time = QTime::fromString(wsTime,"hh:mm:ss.zzz");
    if (!time.isValid())
        time = QTime::fromString(wsTime,"hh:mm:ss");
    if (!wsTime.isEmpty() && time.isValid())
	{		
		if (date.isValid())
		{
			QDateTime dt;
			dt.setDate(date);
			dt.setTime(time);
			return dt.toString(Qt::DefaultLocaleLongDate);
		}
		else
			return time.toString(Qt::DefaultLocaleLongDate);
	}
	else
	{
		if (date.isValid())
			return date.toString(Qt::DefaultLocaleLongDate);
	}
	return "";
}

quint32 CDicomTransferDialog::getIPAddress()
{	
	QNetworkConfigurationManager mgr;
    QNetworkConfiguration nconfig = mgr.defaultConfiguration();
    QNetworkSession session ( nconfig );
    QNetworkInterface ninter = session.interface();
    // the next statement gives you a funny name on windows
    qDebug() << ninter.name() << endl;
    // this gives ip addresses in different sequence, but is is a static method anyhow
    qDebug() << ninter.allAddresses() << endl;
	
	quint32 result = 0;
	QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
	QList<QHostAddress> hostNameLookupAddressList = hostInfo.addresses();
	QList<QHostAddress> interfaceAddressList = QNetworkInterface::allAddresses();
	QString hostIpStr;
    foreach(QHostAddress addr, hostNameLookupAddressList){
        if(addr.protocol() == QAbstractSocket::IPv4Protocol && interfaceAddressList.contains(addr)){			
			qDebug() << addr.toString();
			result = addr.toIPv4Address();
			return result;
        }
    }
 
	 
    // this provides two ip addresses (1 ipv4 and 1 ipv6) at least on my machine
    QList<QNetworkAddressEntry> laddr = ninter.addressEntries();
	foreach(QNetworkAddressEntry addr, laddr)
    {
		QHostAddress ha = addr.ip();
		if (QAbstractSocket::IPv4Protocol == ha.protocol())			
		{
			//qDebug() << ha << endl;
			result = ha.toIPv4Address();
			break;
		}
/*		if (QAbstractSocket::IPv6Protocol == ha.protocol())			
		{
			qDebug() << ha << endl;
			Q_IPV6ADDR adr = ha.toIPv6Address();
			//break;
		}*/
    }	
	return result;
}



CDicomTransferDialog::CDicomTransferDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
    ui(new Ui::CDicomTransferDialog)
{
    m_bCancelThread = false;
    m_pDataReceiver = NULL;

    ui->setupUi(this);

	CDicomTransferDialog::getIPAddress();

	QFileIconProvider iconProvider;
	ui->pushButtonBrowsePath->setIcon(iconProvider.icon(QFileIconProvider::Folder));

	QSettings settings;
#if QT_VERSION < 0x050000
	QString previousDir=settings.value("DICOMdir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
	QString previousDir=settings.value("DICOMdir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif
	ui->lineEditDestinationPath->setText(previousDir);

    settings.beginGroup("DicomTransferDialog");
    resize(settings.value("size",minimumSize()).toSize());
    settings.endGroup();

	// start thread
    int port = settings.value("DicomPort", DEFAULT_DICOM_PORT).toInt();
	m_pDataReceiver = new CThreadedDICOMReceiver(this, port);
	m_pDataReceiver->setDestPath(ui->lineEditDestinationPath->text());
	m_pDataReceiver->moveToThread(&m_workThread);
	m_workThread.setObjectName("Dicom receiver");
	m_workThread.start(/*QThread::LowPriority*/);
	QObject::connect(m_pDataReceiver,SIGNAL(statusMessage(QString)),this,SLOT(onStatusMessage(QString)));
	QMetaObject::invokeMethod(m_pDataReceiver, "doWork", Qt::QueuedConnection, Q_ARG(void*, this));
}

CDicomTransferDialog::~CDicomTransferDialog()
{
    QSettings settings;
    settings.beginGroup("DicomTransferDialog");
    settings.setValue("size",size());
    settings.endGroup();

	m_bCancelThread = true;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();
	m_workThread.quit();
	m_workThread.wait();
    if (NULL != m_pDataReceiver)
        delete m_pDataReceiver;
    m_pDataReceiver = NULL;
	QApplication::restoreOverrideCursor();
    delete ui;
}

void CDicomTransferDialog::AddRow(const QString& text, const QColor& color)
{
	int Id=ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(Id);
	QTableWidgetItem *i0 = new QTableWidgetItem(text);
	i0->setFlags(i0->flags() & ~Qt::ItemIsEditable);
	if (color.isValid())
		i0->setBackgroundColor(color);
	ui->tableWidget->setItem(Id,0,i0);
}

void CDicomTransferDialog::on_pushButtonBrowsePath_clicked()
{
	QSettings settings;
	QString previousDir=ui->lineEditDestinationPath->text();

	// get DICOM data directory
	QString fileName = QFileDialog::getExistingDirectory(this,tr("Set DICOM data destination path"),previousDir);
	if (fileName.isEmpty())
		return;

	settings.setValue("DICOMdir",fileName);
	ui->lineEditDestinationPath->setText(fileName);

	// TODO: set path in m_pDataReceiver correctly
	if (NULL!=m_pDataReceiver)
	//	QMetaObject::invokeMethod(m_pDataReceiver, "setDestPath", Qt::QueuedConnection, Q_ARG(QString, fileName));
		m_pDataReceiver->setDestPath(fileName); // !!! unsafe !!!
}

void CDicomTransferDialog::updateProgressBar(int val, int total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(val);    
}

void CDicomTransferDialog::on_pushButtonLoadDicomData_clicked()
{
	m_openDicomFolder = ui->lineEditDestinationPath->text();
	accept();
}

