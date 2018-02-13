///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
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

#include <data/CDicomDCTk.h>
#include <data/DicomTags.h>
#include <data/DicomTagUtils.h>

//VPL
#include <VPL/ImageIO/DicomSlice.h>
#include <VPL/System/FileBrowser.h>

// DCMTK
#ifdef __APPLE__
  #undef UNICODE
#endif
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcxfer.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcostrmb.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/cmdlnarg.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmjpeg/djencode.h>
#include <dcmtk/dcmjpeg/djrplol.h>
#include <dcmtk/dcmjpeg/dipijpeg.h>
#include <dcmtk/ofstd/ofconapp.h>
#include <dcmtk/dcmjpeg/djdecode.h>

// STL
#include <fstream>

// SHA-1 hash function
#include <math/sha1.h>



namespace data 
{

//=================================================================================================
CDicomDCTk::CDicomDCTk(const std::string & file)
:CDicom()
{
    m_pHandle = new DcmFileFormat;

    if( !file.empty() )
    {
        this->loadFile( file );
    }
}

//=================================================================================================
data::CDicomDCTk::CDicomDCTk()
:CDicom()
{
    m_pHandle = new DcmFileFormat;
}

//=================================================================================================
data::CDicomDCTk::~CDicomDCTk()
{
    if( m_pHandle != nullptr)
    {
        delete m_pHandle;
    }
}

//=================================================================================================


//=================================================================================================
bool data::CDicomDCTk::loadFile(const std::string & file)
{
    OFCondition status = m_pHandle->loadFile( file.c_str() );
    if( status.good() )
    {
        m_sFileName = file;
        return (m_bOk = true);
    }
    else
    {
        // Cannot read the file
        return false;
    }
}

//=================================================================================================
long data::CDicomDCTk::saveToBuffer(char * buffer, long length)
{
    long size = 0;
    
    if ( m_bOk )
    {
        DcmOutputBufferStream stream( static_cast< void* >( buffer ), static_cast< unsigned >( length ) );

        m_pHandle->transferInit();

        switch ( m_Compression )
        {
            case LOSSLESS:
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361 || PACKAGE_VERSION_NUMBER == 362)
                if ( m_pHandle->write( stream, EXS_JPEGProcess14SV1, EET_UndefinedLength, NULL ) == EC_Normal )
#else
                if ( m_pHandle->write( stream, EXS_JPEGProcess14SV1TransferSyntax, EET_UndefinedLength, NULL ) == EC_Normal )
#endif
                {
                    size = long(stream.tell());
                }
                break;

//            case RAW:
            default:
                if ( m_pHandle->write( stream, EXS_Unknown, EET_UndefinedLength, NULL ) == EC_Normal )
                {
                    size = long(stream.tell());
                }
                break;
        }

        m_pHandle->transferEnd();
    }

    return size;
}

//=================================================================================================
bool data::CDicomDCTk::setPatientName(const std::string & name)
{
    DcmDataset *dataset = m_pHandle->getDataset();
    if( !dataset )
    {
        return false;
    }

    OFCondition	status = dataset->putAndInsertString( DCM_PatientName, name.c_str() );
    return status.good();
}

//==================================================================================================
std::string data::CDicomDCTk::hash(const char * input)
{
    int l = strlen( input );

    unsigned char * buf = new unsigned char [ l + 1 ];
    for ( int i  = 0; i < l; i++ ) buf[i] = static_cast< unsigned char >( input[i] );
    buf[l] = '\0';

    unsigned char hash[20];

    //sha1( buf, l, hash );

    SHA1_CTX ctx;

    SHA1Init(&ctx);
    for( int i=0; i<1000; i++ )	SHA1Update(&ctx, buf, l );
    SHA1Final(hash, &ctx);

    unsigned char * hashbuf = new unsigned char [ 41 ];

    const unsigned char chrs[17] = "0123456789abcdef";

    for ( int i = 0; i < 20; i++ )
    {
        int hi = hash[i] / 16;
        int lo = hash[i] % 16;
        hashbuf[2*i] = chrs[hi];
        hashbuf[2*i+1] = chrs[lo];
    }

    hashbuf[40] = '\0';

    std::string r( reinterpret_cast< char* >( hashbuf ) );
    delete [] hashbuf;

    return r;
}

void makeHexaString( unsigned char * input, unsigned char * output, int length )
{
    const unsigned char chrs[17] = "0123456789abcdef";

    for ( int i = 0; i < length; i++ )
    {
        int hi = input[i] / 16;
        int lo = input[i] % 16;
        output[2*i] = chrs[hi];
        output[2*i+1] = chrs[lo];
    }
}

//=================================================================================================
std::string data::CDicomDCTk::getStudyId()
{
    if ( m_bOk )
    {
        DcmDataset * dataset = m_pHandle->getDataset();

        OFString buffer;
        OFCondition status = dataset->findAndGetOFString( TAG_STUDY_UID, buffer );
        if ( status.good() )
        {
            return std::string( buffer.c_str() );
        }
        else return std::string( "" );
    }
    else return std::string( "" );
}

//=================================================================================================
std::string data::CDicomDCTk::getSerieId()
{
    if ( m_bOk )
    {
        DcmDataset * dataset = m_pHandle->getDataset();

        OFString buffer;
        OFCondition status = dataset->findAndGetOFString( TAG_SERIES_UID, buffer );
        if ( status.good() )
        {
            return std::string( buffer.c_str() );
        }
        else return std::string( "" );
    }
    else return std::string( "" );
}

//=================================================================================================
int data::CDicomDCTk::getBitsAllocated()
{
    Uint16 bits = 0;
    if ( m_bOk )
    {
        DcmDataset * dataset = m_pHandle->getDataset();        
        OFCondition status = dataset->findAndGetUint16(TAG_BITS_ALLOCATED,bits);
        if (!status.good() )
            bits = 0;        
    }
    return bits;
}



//=================================================================================================
int data::CDicomDCTk::getSliceId()
{
    if( !m_bOk )
    {
        return -1;
    }

    DcmDataset * dataset = m_pHandle->getDataset();

    // This version doesn't work for Planmeca's multi-frame dicom files
/*    OFString buffer;
    OFCondition	status = dataset->findAndGetOFString( TAG_SLICE_NUMBER, buffer );
    if ( status.good() ) 
    {
        int num;
        sscanf( buffer.c_str(), "%d", &num );
        return num;
    }
    else
    {
        return -1;
    }*/

    DcmStack stack;
    OFCondition	status = dataset->findAndGetElements( TAG_IMAGE_POSITION, stack );
    if( status.good() && !stack.empty() )
    {
        // get the image position 	
        DcmElement * elem = dynamic_cast<DcmElement *>( stack.elem(0) );
        if( !elem )
        {
            return -1;
        }

        // retrieve the position
        Float64 f1, f2, f3;
        status = elem->getFloat64(f1, 0);
        status = elem->getFloat64(f2, 1);
        status = elem->getFloat64(f3, 2);
        if( status.bad() )
        {
            return -1;
        }

        return convPos2Id( double(f1), double(f2), double(f3) );
    }
    else
    {
        return -1;
    }
}

//=================================================================================================
void data::CDicomDCTk::getSliceIds(tDicomNumList& Numbers)
{
    if( !m_bOk )
    {
        return;
    }

    DcmDataset * dataset = m_pHandle->getDataset();

    // This version doesn't work for Planmeca's multi-frame dicom files
/*    DcmStack Stack;
    OFCondition	status = dataset->findAndGetElements( TAG_SLICE_NUMBER, Stack );
    if( status.good() )
    {
        for( unsigned long i = 0; i < Stack.card(); ++i )
        {
            // Get element from the stack
            DcmByteString * strtag = dynamic_cast<DcmByteString *>(Stack.elem(i));
            if( !strtag )
            {
                continue;
            }

            // Retrieve the slice number
            OFString buffer;
            status = strtag->getOFString(buffer, 0);
            if( status.good() )
            {
                int num;
                sscanf( buffer.c_str(), "%d", &num );
                Numbers.push_back(num);
            }
        }
    }*/

    DcmStack Stack;
    OFCondition	status = dataset->findAndGetElements( TAG_IMAGE_POSITION, Stack );
    if( status.good() )
    {
        for( unsigned long i = 0; i < Stack.card(); ++i )
        {
            // get the image position 	
            DcmElement * elem = dynamic_cast<DcmElement *>( Stack.elem(i) );
            if( !elem )
            {
                continue;
            }

            // retrieve the position
            Float64 f1, f2, f3;
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            status = elem->getFloat64(f3, 2);
            if( status.bad() )
            {
                continue;
            }

            Numbers.push_back( convPos2Id( double(f1), double(f2), double(f3) ) );
        }
    }
    if (Numbers.empty())
    {               
        double spacing = 0;
        OFCondition	status = dataset->findAndGetElements( TAG_SPACING_BETWEEN_SLICES, Stack );
        if (!status.bad() && Stack.card()>0)
        {            
            DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
            if (elem)
            {
                Float64 f1 = 0;
                status = elem->getFloat64(f1, 0);
                if (!status.bad() && f1!=0)
                    spacing = f1;
            }
        }        

		if (0==spacing)
		{
			OFCondition	status = dataset->findAndGetElements( TAG_THICKNESS, Stack );
			if (!status.bad() && Stack.card()>0)
			{            
				DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
				if (elem)
				{
					Float64 f1 = 0;
					status = elem->getFloat64(f1, 0);
					if (!status.bad() && f1!=0)
						spacing = f1;
				}
			}
		}
        //
        long int numOfFrames=0;
        dataset->findAndGetLongInt(DCM_NumberOfFrames,numOfFrames);
        //
        if (spacing>0 && numOfFrames>0)
        {
            Float64 f1, f2, f3;
            f1 = f2 = f3 = 0;
            for( unsigned long i = 0; i < numOfFrames; ++i )
            {
                f3 += spacing;
                Numbers.push_back( convPos2Id( double(f1), double(f2), double(f3) ) );
            }
        }
    }
}

//=================================================================================================
double data::CDicomDCTk::getPixelSpacing()
{
    if (m_bOk)
    {
        DcmDataset * dataset = m_pHandle->getDataset();

        OFString buffer;
        OFCondition status = dataset->findAndGetOFString(TAG_PIXEL_SIZE, buffer);
        if (status.good())
        {
            return std::atof(buffer.c_str());
        }
        else return 0.0;
    }
    else return 0.0;
}

//=================================================================================================
bool data::CDicomDCTk::anonymize(const std::string & name)
{
    DcmDataset *dataset = m_pHandle->getDataset();
    if( !dataset )
    {
        return false;
    }

    DcmStack  stack;
    OFCondition	status;

    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0008, 0x0080 ), DcmVR( EVR_LO ) ), "anonymized", true );
    if ( !status.good() ) return false;

    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0008, 0x0081 ), DcmVR( EVR_ST ) ), "anonymized", true );
    if ( !status.good() ) return false;

    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0008, 0x0090 ), DcmVR( EVR_ST ) ), "anonymized", true );
    if ( !status.good() ) return false;

    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0008, 0x1050 ), DcmVR( EVR_ST ) ), "anonymized", true );
    if ( !status.good() ) return false;

    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0008, 0x1070 ), DcmVR( EVR_ST ) ), "anonymized", true );
    if ( !status.good() ) return false;

    OFString buffer;
    std::string	strhash;

    status = dataset->findAndGetOFString( DcmTag( DcmTagKey( 0x0020, 0x000d ), DcmVR( EVR_UI ) ), buffer );
    if ( !status.good() ) return false;
    strhash = this->hash( buffer.c_str() );
    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0020, 0x000d ), DcmVR( EVR_UI ) ), strhash.c_str(), true );
    if ( !status.good() ) return false;

    status = dataset->findAndGetOFString( DcmTag( DcmTagKey( 0x0020, 0x000e ), DcmVR( EVR_UI ) ), buffer );
    if ( !status.good() ) return false;
    strhash = this->hash( buffer.c_str() );
    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0020, 0x000e ), DcmVR( EVR_UI ) ), strhash.c_str(), true );
    if ( !status.good() ) return false;

    status = dataset->putAndInsertString( DcmTag( DcmTagKey( 0x0010, 0x0030 ), DcmVR( EVR_DA ) ), "00000000", true );
    if ( !status.good() ) return false;

    return setPatientName( name );
}

//=================================================================================================
std::string	data::CDicomDCTk::getPatientName()
{
    if ( m_bOk )
    {
        DcmDataset *dataset = m_pHandle->getDataset();
        OFCondition	status;
        OFString buffer;

        status = dataset->findAndGetOFString( DCM_PatientName, buffer );
        if ( status.good() )
        {
            return std::string( buffer.c_str() );
        }
        else return std::string( "" );
    }
    else return std::string( "" );
}

//=================================================================================================
int	data::CDicomDCTk::getNumberOfFrames()
{
    if ( m_bOk )
    {
        DcmDataset *dataset = m_pHandle->getDataset();
        OFCondition	status;
        DcmTag tag = TAG_NUMBER_OF_FRAMES;
        long int numOfFrames = 0;
        dataset->findAndGetLongInt(DCM_NumberOfFrames, numOfFrames);
        return numOfFrames;        
    }
    return 0;
}

//=================================================================================================
bool data::CDicomDCTk::compressLosslessJPEG()
{
    DJEncoderRegistration::registerCodecs(); // register JPEG codecs

    DcmDataset *dataset = m_pHandle->getDataset();
    DcmItem *metaInfo = m_pHandle->getMetaInfo();
    if( !dataset || !metaInfo )
    {
        DJEncoderRegistration::cleanup(); // deregister JPEG codecs
        return false;
    }

    DJ_RPLossless params; // codec parameters, we use the defaults		

    // this causes the lossless JPEG version of the dataset to be created
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361 || PACKAGE_VERSION_NUMBER == 362)
    dataset->chooseRepresentation(EXS_JPEGProcess14SV1, &params);
#else
    dataset->chooseRepresentation(EXS_JPEGProcess14SV1TransferSyntax, &params);
#endif

    // check if everything went well
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361 || PACKAGE_VERSION_NUMBER == 362)
    if (dataset->canWriteXfer(EXS_JPEGProcess14SV1))
#else
    if (dataset->canWriteXfer(EXS_JPEGProcess14SV1TransferSyntax))
#endif
    {
        // force the meta-header UIDs to be re-generated when storing the file 
        // since the UIDs in the data set may have changed 
        delete metaInfo->remove(DCM_MediaStorageSOPClassUID);
        delete metaInfo->remove(DCM_MediaStorageSOPInstanceUID);

        m_Compression = LOSSLESS;

        // store in lossless JPEG format		
        DJEncoderRegistration::cleanup(); // deregister JPEG codecs
        return true;
    }
    else
    {
        DJEncoderRegistration::cleanup(); // deregister JPEG codecs
        return false;	
    }	
}


const std::string trimSpaces(const std::string& str)
{
    static const std::string WHITESPACE = " \t";

    const size_t beginStr = str.find_first_not_of(WHITESPACE);
    if (beginStr == std::string::npos)
    {
        // no content
        return std::string("");
    }

    const size_t endStr = str.find_last_not_of(WHITESPACE);
    const size_t range = endStr - beginStr + 1;

    return str.substr(beginStr, range);
}


//! Reads all informative DICOM tags (patient name, etc.) from the dataset.
//! - Throws exception on failure.
void readTagsDCTk(DcmDataset * dataset, vpl::img::CDicomSlice & slice)
{
    if (!dataset)
    {
        throw CDicomLoadingFailure();
    }

    dataset->convertToUTF8();

    OFString buffer;
    DcmTag tag;
    OFCondition	status;

    tag = TAG_NUMBER_OF_FRAMES;
    long int numOfFrames = 0;
    dataset->findAndGetLongInt(DCM_NumberOfFrames, numOfFrames);

    // slice/image number
    tag = TAG_SLICE_NUMBER;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else
    {
        int val = 0;
        sscanf(buffer.c_str(), "%d", &val);
        slice.m_iSliceNumber = val;
    }

    // patients name 
    tag = TAG_PATIENTS_NAME;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sPatientName = std::string("");
    }
    else
    {
        slice.m_sPatientName = trimSpaces(std::string(buffer.c_str()));
    }

    // patient id 
    tag = TAG_PATIENTS_ID;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sPatientId = std::string("");
    }
    else
    {
        slice.m_sPatientId = trimSpaces(std::string(buffer.c_str()));
    }

    // patient birthday
    tag = TAG_PATIENTS_BIRTHDAY;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sPatientBirthday = std::string("");
    }
    else
    {
        slice.m_sPatientBirthday = trimSpaces(std::string(buffer.c_str()));
    }

    // patient sex
    tag = TAG_PATIENTS_SEX;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sPatientSex = std::string("");
    }
    else
    {
        slice.m_sPatientSex = trimSpaces(std::string(buffer.c_str()));
    }

    // patient description
    tag = TAG_PATIENTS_DESCRIPTION;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sPatientDescription = "";
    }
    else
    {
        slice.m_sPatientDescription = trimSpaces(std::string(buffer.c_str()));
    }

    // study uid
    tag = TAG_STUDY_UID;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sStudyUid = std::string("");
    }
    else
    {
        slice.m_sStudyUid = std::string(buffer.c_str());
    }

    // study id
    tag = TAG_STUDY_ID;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sStudyId = std::string("");
    }
    else
    {
        slice.m_sStudyId = std::string(buffer.c_str());
    }

    // study date
    tag = TAG_STUDY_DATE;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sStudyDate = std::string("");
    }
    else
    {
        slice.m_sStudyDate = trimSpaces(std::string(buffer.c_str()));
    }

    // study description
    tag = TAG_STUDY_DESCRIPTION;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sStudyDescription = std::string("");
    }
    else
    {
        slice.m_sStudyDescription = trimSpaces(std::string(buffer.c_str()));
    }

    // series uid
    tag = TAG_SERIES_UID;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        if (numOfFrames>7) // allow multiframe images with no series id
            slice.m_sSeriesUid = std::string("");
        else
            throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sSeriesUid = std::string("");
    }
    else
    {
        slice.m_sSeriesUid = std::string(buffer.c_str());
    }

    // series number
    Uint32	u;
    tag = TAG_SERIES_NUMBER;
    status = dataset->findAndGetUint32(tag, u);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_iSeriesNumber = 0;
    }
    else
    {
        slice.m_iSeriesNumber = u;
    }

    // modality
    tag = TAG_MODALITY;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sModality = "CT";
    }
    else
    {
        slice.m_sModality = trimSpaces(std::string(buffer.c_str()));
    }

    // image type
    tag = TAG_IMAGE_TYPE;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sImageType = "";
    }
    else
    {
        slice.m_sImageType = trimSpaces(std::string(buffer.c_str()));
    }

    // scan options
    tag = TAG_SCAN_OPTIONS;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sScanOptions = "";
    }
    else
    {
        slice.m_sScanOptions = trimSpaces(std::string(buffer.c_str()));
    }

    // series date
    tag = TAG_SERIES_DATE;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        // assign default value
        slice.m_sSeriesDate = std::string("");
    }
    else
    {
        slice.m_sSeriesDate = trimSpaces(std::string(buffer.c_str()));
    }

    // series time
    tag = TAG_SERIES_TIME;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sSeriesTime = "";
    }
    else
    {
        slice.m_sSeriesTime = trimSpaces(std::string(buffer.c_str()));
    }

    // series description
    tag = TAG_SERIES_DESCRIPTION;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sSeriesDescription = "";
    }
    else
    {
        slice.m_sSeriesDescription = trimSpaces(std::string(buffer.c_str()));
    }

    Float64	f1, f2;

    // window center 
    tag = TAG_WINDOW_CENTER;
    status = dataset->findAndGetFloat64(tag, f1);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_iWindowCenter = 1000;
    }
    else
    {
        slice.m_iWindowCenter = static_cast< int >(floor(f1));
    }

    // window width
    tag = TAG_WINDOW_WIDTH;
    status = dataset->findAndGetFloat64(tag, f1);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_iWindowWidth = 1000;
    }
    else
    {
        slice.m_iWindowWidth = static_cast< int >(floor(f1));
    }

    // pixel representation
    tag = TAG_PIXEL_REPRESENTATION;
    status = dataset->findAndGetUint32(tag, u);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_uPixelRepresentation = 0;
    }
    else
    {
        slice.m_uPixelRepresentation = u;
    }

    // This doesn't work for Planmeca's multi-frame dicom files
    // thickness
    /*    tag	= TAG_THICKNESS;
    status = dataset->findAndGetFloat64( tag, f1 );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
    throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
    // assign default value
    slice.setThickness( 1.0 );
    }
    else
    {
    slice.setThickness( f1 );
    }

    // dx and dy
    tag = TAG_PIXEL_SIZE;
    status = dataset->findAndGetFloat64( tag, f1, 0 );
    status = dataset->findAndGetFloat64( tag, f2, 1 );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
    throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
    slice.setPixel( 1.0, 1.0 );
    }
    else
    {
    slice.setPixel( f1, f2 );
    }*/

    DcmStack stack;

    // thickness
    tag = TAG_THICKNESS;
    status = dataset->findAndGetElements(tag, stack);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        double spacing = 0;
        DcmStack Stack;
        OFCondition	status = dataset->findAndGetElements(TAG_SPACING_BETWEEN_SLICES, Stack);
        if (!status.bad() && Stack.card()>0)
        {
            DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
            if (elem)
            {
                Float64 f1 = 0;
                status = elem->getFloat64(f1, 0);
                if (!status.bad() && f1 != 0)
                    spacing = f1;
            }
        }
        if (spacing>0)
            slice.setThickness(spacing);
        else
            slice.setThickness(1.0);
    }
    else
    {
        DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
        if (!elem && COMPULSORY_TAGS.contains(tag))
        {
            throw CDicomLoadingFailure();
        }
        else if (!elem)
        {
            slice.setThickness(1.0);
        }
        else
        {
            // retrieve the position
            status = elem->getFloat64(f1, 0);
            if (status.bad() && COMPULSORY_TAGS.contains(tag))
            {
                throw CDicomLoadingFailure();
            }
            else if (status.bad())
            {
                slice.setThickness(1.0);
            }
            else
            {
                slice.setThickness(f1);
            }
        }
    }

    // dx and dy
    tag = TAG_PIXEL_SIZE;
    status = dataset->findAndGetElements(tag, stack);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.setPixel(1.0, 1.0);

#define	TAG_IMAGER_PIXEL_SPACING				DcmTag( DcmTagKey( 0x0018, 0x1164 ), DcmVR( EVR_FD ))
        tag = TAG_IMAGER_PIXEL_SPACING;
        status = dataset->findAndGetElements(tag, stack);
        if (!status.bad())
        {
            DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
            if (elem)
            {
                // retrieve the position
                status = elem->getFloat64(f1, 0);
                status = elem->getFloat64(f2, 1);
                if (!status.bad())
                    slice.setPixel(f1, f2);
            }
        }
    }
    else
    {
        DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
        if (!elem && COMPULSORY_TAGS.contains(tag))
        {
            throw CDicomLoadingFailure();
        }
        else if (!elem)
        {
            slice.setPixel(1.0, 1.0);
        }
        else
        {
            // retrieve the position
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            if (status.bad() && COMPULSORY_TAGS.contains(tag))
            {
                throw CDicomLoadingFailure();
            }
            else if (status.bad())
            {
                slice.setPixel(1.0, 1.0);
            }
            else
            {
                slice.setPixel(f1, f2);
            }
        }
    }

    // slope
    /*    tag	= TAG_SLOPE;
    status = dataset->findAndGetFloat64( tag, f1 );
    if( status.good() )
    {
    slice.m_dSlope = f1;
    }*/

    // intercept
    /*    tag	= TAG_INTERCEPT;
    status = dataset->findAndGetFloat64(tag, f1 );
    if( status.good() )
    {
    slice.m_dIntercept = f1;
    }*/

    // manufacturer
    tag = TAG_MANUFACTURER;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sManufacturer = "";
    }
    else
    {
        slice.m_sManufacturer = trimSpaces(std::string(buffer.c_str()));
    }

    // manufacturer's model name
    tag = TAG_MODEL_NAME;
    status = dataset->findAndGetOFString(tag, buffer);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.bad())
    {
        slice.m_sModelName = "";
    }
    else
    {
        slice.m_sModelName = trimSpaces(std::string(buffer.c_str()));
    }
}

//=============================================================================

//! Reads tags representing orientation of ccordinate system from the dataset.
//! - Throws exception on failure.
void readOrientTagDCTk(DcmDataset * dataset,
    vpl::img::CVector3D & XAxis,
    vpl::img::CVector3D & YAxis,
    vpl::img::CVector3D & ZAxis
    )
{
    if (!dataset)
    {
        throw CDicomLoadingFailure();
    }

    // Orientation tag found flag
    bool bFound = false;

    DcmTag tag;
    OFCondition	status;
    DcmStack orientlist;

    // image orientation
    tag = TAG_IMAGE_ORIENTATION;
    status = dataset->findAndGetElements(tag, orientlist);
    if (status.bad() && COMPULSORY_TAGS.contains(tag))
    {
        throw CDicomLoadingFailure();
    }
    else if (status.good())
    {
        DcmElement * elem = dynamic_cast<DcmElement *>(orientlist.elem(0));
        if (!elem && COMPULSORY_TAGS.contains(tag))
        {
            throw CDicomLoadingFailure();
        }
        else if (elem)
        {
            // retrieve the position
            Float64 f1, f2, f3, f4, f5, f6;
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            status = elem->getFloat64(f3, 2);
            status = elem->getFloat64(f4, 3);
            status = elem->getFloat64(f5, 4);
            status = elem->getFloat64(f6, 5);
            if (status.bad() && COMPULSORY_TAGS.contains(tag))
            {
                throw CDicomLoadingFailure();
            }
            else if (status.good())
            {
                XAxis.setXYZ(f1, f2, f3);
                YAxis.setXYZ(f4, f5, f6);
                bFound = true;
            }
        }
    }

    if (XAxis.getLength() < 0.00001 || YAxis.getLength() < 0.00001)
    {
        bFound = false;
    }

    // Use default values
    if (!bFound)
    {
        XAxis.setXYZ(1, 0, 0);
        YAxis.setXYZ(0, 1, 0);
    }

    // Calculate the z-axis
    ZAxis.vectorProduct(XAxis, YAxis);

    // Final normalization
    XAxis.normalize();
    YAxis.normalize();
    ZAxis.normalize();
}

//=============================================================================

bool CDicomDCTk::loadDicom(const vpl::sys::tString &dir, const std::string &filename, vpl::img::CDicomSlice &slice, sExtendedTags& tags, bool bLoadImageData)
{
    vpl::sys::CFileBrowserU browser;
    vpl::sys::tString oldDir = browser.getDirectory();
    browser.setDirectory(dir);

    // DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile(filename.c_str());
        if (!status.good())
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if (!dataset)
        {
            throw CDicomLoadingFailure();
        }

        OFString buffer;
        DcmTag tag;
        DcmStack poslist, orientlist;
        Float64	f1, f2, f3;
        unsigned long slicepos = 0;
        double spacing = 0;
        f1 = f2 = f3 = 0;

        // This code doesn't work for Planmeca's multi-frame files
        /*        tag = TAG_IMAGE_POSITION;
        status = dataset->findAndGetFloat64( tag, f1, 0 );
        status = dataset->findAndGetFloat64( tag, f2, 1 );
        status = dataset->findAndGetFloat64( tag, f3, 2 );
        if( status.bad() )
        {
        throw CDicomLoadingFailure();
        }
        else
        {
        slice.m_ImagePosition.setXYZ( f1, f2, f3 );
        }*/


        tag = TAG_NUMBER_OF_FRAMES;
        long int numOfFrames = 0;
        dataset->findAndGetLongInt(DCM_NumberOfFrames, numOfFrames);

        // try to locate all slice positions
        tag = TAG_IMAGE_POSITION;
        status = dataset->findAndGetElements(tag, poslist);
        if (status.bad())
        {
            DcmStack Stack;
            OFCondition	status = dataset->findAndGetElements(TAG_SPACING_BETWEEN_SLICES, Stack);
            if (!status.bad() && Stack.card()>0)
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                if (elem)
                {
                    // retrieve the position
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1 != 0)
                        spacing = f1;
                }
            }
            if (0 == spacing)
            {
                OFCondition	status = dataset->findAndGetElements(TAG_THICKNESS, Stack);
                if (!status.bad() && Stack.card()>0)
                {
                    DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                    if (elem)
                    {
                        Float64 f1 = 0;
                        status = elem->getFloat64(f1, 0);
                        if (!status.bad() && f1 != 0)
                            spacing = f1;
                    }
                }
            }
        }
        if ((status.bad() || poslist.empty()) && 0 == spacing)
        {
            throw CDicomLoadingFailure();
        }

        if (spacing>0)
        {
            slice.m_ImagePosition.setXYZ(f1, f2, f3);
        }
        else
        {
            // get the image position 	    
            slicepos = poslist.card() / 2;
            DcmElement * elem = dynamic_cast<DcmElement *>(poslist.elem(slicepos));
            if (!elem)
            {
                throw CDicomLoadingFailure();
            }

            // retrieve the position
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            status = elem->getFloat64(f3, 2);
            if (status.bad())
            {
                if (numOfFrames>7) // multiframe - we don't really care
                {
                    f1 = f2 = f3 = 0;
                }
                else
                    throw CDicomLoadingFailure();
            }
            else
            {
                slice.m_ImagePosition.setXYZ(f1, f2, f3);
            }
        }

        // get the image orientation
        vpl::img::CVector3D normal_image;
        readOrientTagDCTk(dataset, slice.m_ImageOrientationX, slice.m_ImageOrientationY, normal_image);

        // slice position calculation
        vpl::img::CPoint3D zero_point(0, 0, 0);
        vpl::img::CVector3D position_vector(zero_point, slice.m_ImagePosition);
        slice.setPosition(normal_image.dotProduct(normal_image, position_vector));

        // prepare for potential decompression
        OFBool opt_verbose = OFFalse;
        E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
        E_UIDCreation opt_uidcreation = EUC_default;
        E_PlanarConfiguration opt_planarconfig = EPC_default;
        DJDecoderRegistration::registerCodecs(opt_decompCSconversion, opt_uidcreation, opt_planarconfig, opt_verbose);

        E_TransferSyntax es = dataset->getOriginalXfer();
        OFCondition error = dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
        if (error.bad())
        {
            throw CDicomLoadingFailure();
        }

        // format conversion
        vpl::img::tDensityPixel pMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
        vpl::img::tDensityPixel pMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
        //        double dMin = pMin;
        //        double dMax = pMax;
        //        double dMinRelative = dMin;
        //        double dMaxRelative = dMax;
        //        image->getMinMaxValues( dMinRelative, dMaxRelative, 0 );

        if (bLoadImageData)
        {
            // Create a new image
            // - Load a single frame from the dataset!
            image = new DicomImage(dataset, es, 0UL, slicepos, 1UL);
            if (!image.get() || image->getStatus() != EIS_Normal)
            {
                throw CDicomLoadingFailure();
            }

            // create pixel data
            vpl::tSize xSize = static_cast< vpl::tSize >(image->getWidth());
            vpl::tSize ySize = static_cast< vpl::tSize >(image->getHeight());
            //            unsigned short depth = static_cast< unsigned short >( image->getDepth() );

            // resize the slice
            slice.vpl::img::CDImage::resize(xSize, ySize, slice.getMargin());

            image->deleteDisplayLUT(0);
            image->hideAllOverlays();
            image->removeAllOverlays();
            image->deleteOverlayData();
            image->setNoDisplayFunction();
            image->setNoVoiTransformation();
            image->getOverlayCount();

            // take pixel data pointer 
            const DiPixel * theData = image->getInterData();

            // take pixel data representation
            EP_Representation theData_representation = theData->getRepresentation();

            // test pixel data representation
            if (theData_representation != EPR_Uint16 && theData_representation != EPR_Sint16 &&
                theData_representation != EPR_Uint8 && theData_representation != EPR_Sint8 &&
                theData_representation != EPR_Uint32 && theData_representation != EPR_Sint32)
            {
                throw CDicomLoadingFailure();
            }

            // take slice image data pointer from pixel data
            const vpl::img::tDensityPixel * thePixels = reinterpret_cast< const vpl::img::tDensityPixel* >(theData->getData());
            if (!thePixels)
            {
                throw CDicomLoadingFailure();
            }
            const vpl::img::tPixel8 * thePixels8u = reinterpret_cast< const vpl::img::tPixel8* >(theData->getData());
            const vpl::sys::tInt8   * thePixels8 = reinterpret_cast< const vpl::sys::tInt8* >(theData->getData());
            const vpl::sys::tInt32  * thePixels32 = reinterpret_cast< const vpl::sys::tInt32* >(theData->getData());

#pragma omp parallel for
            for (vpl::tSize y = 0; y < ySize; y++)
            {
                for (vpl::tSize x = 0; x < xSize; x++)
                {
                    vpl::img::tDensityPixel Value = 0;
                    switch (theData_representation)
                    {
                    case EPR_Uint8:
                        Value = (8000 / 255.0 * thePixels8u[y * xSize + x]) - 1000;
                        break;
                    case EPR_Sint8:
                        Value = (8000 / 255.0 * (thePixels8[y * xSize + x] + 128)) - 1000;
                        break;
                    case EPR_Uint32:
                    case EPR_Sint32:
                        Value = thePixels32[y * xSize + x];
                        break;
                    default:
                        Value = thePixels[y * xSize + x];
                        break;
                    }
                    Value = std::min(Value, pMax);
                    Value = std::max(Value, pMin);
                    slice(x, y) = Value;
                }
            }
        }

        // Read all required DICOM tags
        readTagsDCTk(dataset, slice);
    }

    // Any failure?
    catch (CDicomLoadingFailure &e)
    {
        browser.setDirectory(oldDir);
        DJDecoderRegistration::cleanup();
        return false;
    }

    browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

//=============================================================================
int CDicomDCTk::loadDicom(const vpl::sys::tString &dir, const std::string &filename, tDicomSlices &slices, sExtendedTags& tags, bool bLoadImageData, bool bIgnoreBitsStoredTag)
{
    vpl::sys::CFileBrowserU browser;
    vpl::sys::tString oldDir = browser.getDirectory();
    browser.setDirectory(dir);

    // DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile(filename.c_str());
        if (!status.good())
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if (!dataset)
        {
            throw CDicomLoadingFailure();
        }

        if (bIgnoreBitsStoredTag)
        {
            // we have met dicom data that have BitsStored tag set but use all allocated bits
            Uint16  bitsAllocated = 0,
                bitsStored = 0,
                highestBit = 0;
            dataset->findAndGetUint16(DCM_BitsAllocated, bitsAllocated);
            if (bitsAllocated>0)
            {
                dataset->findAndGetUint16(DCM_BitsStored, bitsStored);
                bitsStored = bitsAllocated;
                dataset->putAndInsertUint16(DCM_BitsStored, bitsStored);
                dataset->findAndGetUint16(DCM_HighBit, highestBit);
                highestBit = bitsStored - 1;
                dataset->putAndInsertUint16(DCM_HighBit, highestBit);
            }
        }

        OFString buffer;
        DcmTag tag;
        DcmStack poslist;
        double spacing = 0;
        bool bUsedThicknessForSpacing = false;

        // try to locate all slice position
        tag = TAG_IMAGE_POSITION;
        status = dataset->findAndGetElements(tag, poslist);
        if (status.bad())
        {
            DcmStack Stack;
            OFCondition	status = dataset->findAndGetElements(TAG_SPACING_BETWEEN_SLICES, Stack);
            if (!status.bad() && Stack.card()>0)
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                if (elem)
                {
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1 != 0)
                        spacing = f1;
                }
            }
            if (0 == spacing)
            {
                OFCondition	status = dataset->findAndGetElements(TAG_THICKNESS, Stack);
                if (!status.bad() && Stack.card()>0)
                {
                    DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                    if (elem)
                    {
                        Float64 f1 = 0;
                        status = elem->getFloat64(f1, 0);
                        if (!status.bad() && f1 != 0)
                        {
                            spacing = f1;
                            bUsedThicknessForSpacing = true;
                        }
                    }
                }
            }
        }
        if ((status.bad() || poslist.empty()) && 0 == spacing)
        {
            throw CDicomLoadingFailure();
        }

        double sliceThickness = 0;
        {   // get slice thickness
            DcmStack stack;
            DcmTag  tag = TAG_THICKNESS;
            status = dataset->findAndGetElements(tag, stack);
            if (!status.bad() && stack.card()>0)
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
                if (elem)
                {
                    // retrieve the position
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1 != 0)
                        sliceThickness = f1;
                }
            }
        }
        if (0 == spacing && 0 == sliceThickness)
            sliceThickness = 1;

        // get the image orientation
        vpl::img::CVector3D ImageOrientationX, ImageOrientationY, normal_image;
        readOrientTagDCTk(dataset, ImageOrientationX, ImageOrientationY, normal_image);

        // Parse all required tags and prepare a slice prototype
        vpl::img::CDicomSlice slicetags;
        readTagsDCTk(dataset, slicetags);
        slicetags.m_ImageOrientationY = ImageOrientationY;
        slicetags.m_ImageOrientationX = ImageOrientationX;
        slicetags.m_ImagePosition.setXYZ(0, 0, 0);
        slicetags.setPosition(0.0);

        // Prepare for potential decompression
        OFBool opt_verbose = OFFalse;
        E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
        E_UIDCreation opt_uidcreation = EUC_default;
        E_PlanarConfiguration opt_planarconfig = EPC_default;
        DJDecoderRegistration::registerCodecs(opt_decompCSconversion, opt_uidcreation, opt_planarconfig, opt_verbose);

        E_TransferSyntax es = dataset->getOriginalXfer();
        OFCondition error = dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
        if (error.bad())
        {
            throw CDicomLoadingFailure();
        }

        // Format conversion
        vpl::img::tDensityPixel pMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
        vpl::img::tDensityPixel pMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
        //        double dMin = pMin;
        //        double dMax = pMax;
        //        double dMinRelative = dMin;
        //        double dMaxRelative = dMax;

        if (bLoadImageData)
        {
            // - Load all frames from the dataset!
            image = new DicomImage(dataset, es, 0UL, 0UL, 0UL);
            if (!image.get() || image->getStatus() != EIS_Normal || (0 == poslist.card() && 0 == spacing) || 0 == image->getFrameCount() /*image->getFrameCount() != poslist.card()*/)
            {
                throw CDicomLoadingFailure();
            }

            // Setup the DCMTk image to read raw image data
            image->deleteDisplayLUT(0);
            image->hideAllOverlays();
            image->removeAllOverlays();
            image->deleteOverlayData();
            image->setNoDisplayFunction();
            image->setNoVoiTransformation();
            image->getOverlayCount();

            // Format conversion
            //            image->getMinMaxValues( dMinRelative, dMaxRelative, 0 );
        }

        // Load all frames
        //for( unsigned long i = 0; i < poslist.card(); ++i ) // we have a file which has multiple frames but only one position record
        for (signed long i = 0; i < image->getFrameCount(); ++i)
        {
            // Create a new slice
            vpl::img::CDicomSlicePtr pSlice = new vpl::img::CDicomSlice(slicetags);

            // Get image position 	
            //DcmElement * elem = dynamic_cast<DcmElement *>(poslist.elem(i));

            // Planmeca dicom files were upside down - it is because posList entries are in stack like structure,
            // so we have to take them in reverse order!
            if (0 == poslist.card()) // there are also some iCATSystem files without image position tag, but have slicethickness and spacing defined
            {
                Float64 f1, f2, f3;
                f1 = f2 = f3 = 0;
                if (bUsedThicknessForSpacing) // OFFIS_DCMTK_360 Wagner^Gerhard
                    f3 = i * std::max(sliceThickness, spacing); // just a guess
                else
                    f3 = -i * std::max(sliceThickness, spacing); // just a guess
                pSlice->m_ImagePosition.setXYZ(f1, f2, f3);
                vpl::img::CPoint3D zero_point(0, 0, 0);
                vpl::img::CVector3D position_vector(zero_point, pSlice->m_ImagePosition);
                pSlice->setPosition(normal_image.dotProduct(normal_image, position_vector));
            }
            else
            {
                double correctionF3 = 0;
                DcmElement * elem = NULL;
                if (i >= poslist.card())
                {
                    elem = dynamic_cast<DcmElement *>(poslist.elem(0));
                    correctionF3 = -i * sliceThickness; // just a guess
                }
                else
                    elem = dynamic_cast<DcmElement *>(poslist.elem(poslist.card() - i - 1));

                if (elem)
                {
                    // Retrieve the position
                    Float64 f1, f2, f3;
                    status = elem->getFloat64(f1, 0);
                    status = elem->getFloat64(f2, 1);
                    status = elem->getFloat64(f3, 2);
                    f3 += correctionF3;
                    if (status.good())
                    {
                        pSlice->m_ImagePosition.setXYZ(f1, f2, f3);
                    }

                    // slice position vector calculation. length and normalization
                    vpl::img::CPoint3D zero_point(0, 0, 0);
                    vpl::img::CVector3D position_vector(zero_point, pSlice->m_ImagePosition);
                    pSlice->setPosition(normal_image.dotProduct(normal_image, position_vector));
                }
            }

            if (bLoadImageData)
            {
                // Get the image size
                vpl::tSize xSize = static_cast< vpl::tSize >(image->getWidth());
                vpl::tSize ySize = static_cast< vpl::tSize >(image->getHeight());
                //                unsigned short depth = static_cast< unsigned short >( image->getDepth() );

                // Resize the slice
                pSlice->vpl::img::CDImage::resize(xSize, ySize, pSlice->getMargin());

                // take pixel data pointer 
                const DiPixel * theData = image->getInterData();
                // take pixel data representation
                EP_Representation theData_representation = theData->getRepresentation();

                // test pixel data representation
                if (theData_representation != EPR_Uint16 && theData_representation != EPR_Sint16 &&
                    theData_representation != EPR_Uint8 && theData_representation != EPR_Sint8 &&
                    theData_representation != EPR_Uint32 && theData_representation != EPR_Sint32)
                {
                    //image->getOutputData(16);
                    //deleteOutputData();
                    throw CDicomLoadingFailure();
                }

                // take slice image data pointer from pixel data
                const vpl::img::tDensityPixel * thePixels = reinterpret_cast< const vpl::img::tDensityPixel* >(theData->getData());
                if (!thePixels)
                {
                    throw CDicomLoadingFailure();
                }

                const vpl::img::tPixel8 * thePixels8u = reinterpret_cast< const vpl::img::tPixel8* >(theData->getData());
                const vpl::sys::tInt8   * thePixels8 = reinterpret_cast< const vpl::sys::tInt8* >(theData->getData());
                const vpl::sys::tInt32  * thePixels32 = reinterpret_cast< const vpl::sys::tInt32* >(theData->getData());

                // move to the corresponding slice
                thePixels += i * xSize * ySize;
                thePixels8 += i * xSize * ySize;
                thePixels8u += i * xSize * ySize;
                thePixels32 += i * xSize * ySize;

#pragma omp parallel for
                for (vpl::tSize y = 0; y < ySize; y++)
                {
                    for (vpl::tSize x = 0; x < xSize; x++)
                    {
                        vpl::img::tDensityPixel Value = 0;
                        switch (theData_representation)
                        {
                        case EPR_Uint8:
                            Value = vpl::img::tDensityPixel((8000 * int(thePixels8u[y * xSize + x]) / 255) - 1000);
                            break;
                        case EPR_Sint8:
                            Value = vpl::img::tDensityPixel((8000 * (int(thePixels8[y * xSize + x]) + 128) / 255) - 1000);
                            break;
                        case EPR_Uint32:
                        case EPR_Sint32:
                            Value = vpl::img::tDensityPixel(thePixels32[y * xSize + x]);
                            break;
                        default:
                            Value = vpl::img::tDensityPixel(thePixels[y * xSize + x]);
                        }
                        Value = std::min(Value, pMax);
                        Value = std::max(Value, pMin);
                        pSlice->set(x, y, Value);
                    }
                }
            }

            // Store the new slice
            slices.push_back(pSlice);
        }
    }

    // Any failure?
    catch (CDicomLoadingFailure &)
    {
        browser.setDirectory(oldDir);
        DJDecoderRegistration::cleanup();
        return false;
    }

    browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

//=============================================================================
bool CDicomDCTk::getDicomFileInfo(const vpl::sys::tString &dir, const std::string &filename, int& nFrames)
{
    vpl::sys::CFileBrowserU browser;
    vpl::sys::tString oldDir = browser.getDirectory();
    browser.setDirectory(dir);

    // DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    nFrames = 0;

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile(filename.c_str());
        if (!status.good())
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if (!dataset)
        {
            throw CDicomLoadingFailure();
        }

        OFString buffer;
        DcmTag tag;
        DcmStack stack;

        tag = TAG_NUMBER_OF_FRAMES;
        long int numOfFrames = 0;
        dataset->findAndGetLongInt(DCM_NumberOfFrames, numOfFrames);
        nFrames = numOfFrames;
    }

    // Any failure?
    catch (CDicomLoadingFailure &)
    {
        browser.setDirectory(oldDir);
        DJDecoderRegistration::cleanup();
        return false;
    }

    browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

//=============================================================================

bool CDicomDCTk::loadDicomDCTk2D(const vpl::sys::tString &dir, const std::string &filename, vpl::img::CDicomSlice &slice, sExtendedTags& tags, bool bLoadImageData, int nDesiredBits)
{
    vpl::sys::CFileBrowserU browser;
    vpl::sys::tString oldDir = browser.getDirectory();
    browser.setDirectory(dir);

    // DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile(filename.c_str());
        if (!status.good())
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if (!dataset)
        {
            throw CDicomLoadingFailure();
        }

        OFString buffer;
        DcmTag tag;
        DcmStack poslist, orientlist;
        Float64	f1, f2, f3;
        unsigned long slicepos = 0;
        double spacing = 0;
        f1 = f2 = f3 = 0;

        // This code doesn't work for Planmeca's multi-frame files
        /*        tag = TAG_IMAGE_POSITION;
        status = dataset->findAndGetFloat64( tag, f1, 0 );
        status = dataset->findAndGetFloat64( tag, f2, 1 );
        status = dataset->findAndGetFloat64( tag, f3, 2 );
        if( status.bad() )
        {
        throw CDicomLoadingFailure();
        }
        else
        {
        slice.m_ImagePosition.setXYZ( f1, f2, f3 );
        }*/

        // try to locate all slice positions
        tag = TAG_IMAGE_POSITION;
        status = dataset->findAndGetElements(tag, poslist);
        if (status.bad())
        {
            DcmStack Stack;
            OFCondition	status = dataset->findAndGetElements(TAG_SPACING_BETWEEN_SLICES, Stack);
            if (!status.bad() && Stack.card()>0)
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                if (elem)
                {
                    // retrieve the position
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1 != 0)
                        spacing = f1;
                }
            }
        }
        if (spacing>0 || 0 == poslist.card())
        {
            slice.m_ImagePosition.setXYZ(f1, f2, f3);
        }
        else
        {
            // get the image position 	    
            slicepos = poslist.card() / 2;
            DcmElement * elem = dynamic_cast<DcmElement *>(poslist.elem(slicepos));
            if (!elem)
            {
                throw CDicomLoadingFailure();
            }

            // retrieve the position
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            status = elem->getFloat64(f3, 2);
            if (status.bad())
            {
                throw CDicomLoadingFailure();
            }
            else
            {
                slice.m_ImagePosition.setXYZ(f1, f2, f3);
            }
        }

        int pixelMin = 0, pixelMax = 0, bitsUsed = 0;
        {	// extract more tags...
#define	TAG_DISTANCE_SOURCE_TO_DETECTOR				DcmTag( DcmTagKey( 0x0018, 0x1110 ), DcmVR( EVR_DS ))
#define	TAG_DISTANCE_SOURCE_TO_PATIENT				DcmTag( DcmTagKey( 0x0018, 0x1111 ), DcmVR( EVR_DS ))
#define	TAG_ESTIMATED_RADIOGRAPHIC_MAGNIFICATION	DcmTag( DcmTagKey( 0x0018, 0x1114 ), DcmVR( EVR_DS ))
#define	TAG_GRID_FOCAL_DISTANCE						DcmTag( DcmTagKey( 0x0018, 0x704c ), DcmVR( EVR_DS ))
#define	TAG_PATIENTS_AGE			                DcmTag( DcmTagKey( 0x0010, 0x1010 ), DcmVR( EVR_AS ))
#define	TAG_PATIENTS_SIZE			                DcmTag( DcmTagKey( 0x0010, 0x1020 ), DcmVR( EVR_DS ))
#define	TAG_PATIENTS_WEIGHT			                DcmTag( DcmTagKey( 0x0010, 0x1030 ), DcmVR( EVR_DS ))

            DcmStack dist;
            OFCondition	status;
            DcmTag tag = TAG_DISTANCE_SOURCE_TO_DETECTOR;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                double fDist = 0;
                status = elem->getFloat64(fDist, 0);
                tags.fDistanceSourceToDetector = fDist;
            }

            tag = TAG_DISTANCE_SOURCE_TO_PATIENT;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                double fDist = 0;
                status = elem->getFloat64(fDist, 0);
                tags.fDistanceSourceToPatient = fDist;
            }

            tag = TAG_ESTIMATED_RADIOGRAPHIC_MAGNIFICATION;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                double fDist = 0;
                status = elem->getFloat64(fDist, 0);
                tags.fEstimatedRadiographicMagnificationFactor = fDist;
            }

            tag = TAG_GRID_FOCAL_DISTANCE;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                double fDist = 0;
                status = elem->getFloat64(fDist, 0);
                tags.fGridFocalDistance = fDist;
            }

            // patient age
            tag = TAG_PATIENTS_AGE;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                Uint32 iDist = 0;
                status = elem->getUint32(iDist, 0);
                tags.iPatientAge = iDist;
            }

            // patient height
            tag = TAG_PATIENTS_SIZE;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                double fDist = 0;
                status = elem->getFloat64(fDist, 0);
                tags.fPatientHeight = fDist;
            }

            // patient weight
            tag = TAG_PATIENTS_WEIGHT;
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                double fDist = 0;
                status = elem->getFloat64(fDist, 0);
                tags.fPatientWeight = fDist;
            }

            tag = DcmTag(DcmTagKey(0x0028, 0x0106), DcmVR(EVR_US));
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                Uint16 val = 0;
                status = elem->getUint16(val, 0);
                pixelMin = val;
            }
            tag = DcmTag(DcmTagKey(0x0028, 0x0107), DcmVR(EVR_US));
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                Uint16 val = 0;
                status = elem->getUint16(val, 0);
                pixelMax = val;
            }
            tag = DcmTag(DcmTagKey(0x0028, 0x0101), DcmVR(EVR_US));
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                Uint16 val = 0;
                status = elem->getUint16(val, 0);
                bitsUsed = val;
            }
            // high bit
            tag = DcmTag(DcmTagKey(0x0028, 0x0102), DcmVR(EVR_US));
            status = dataset->findAndGetElements(tag, dist);
            if (status.bad() || dist.empty())
            {
            }
            else
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(dist.elem(0));
                Uint16 val = 0;
                status = elem->getUint16(val, 0);
                if (val>0 && val<bitsUsed)
                    bitsUsed = val + 1;
            }
        }


        // get the image orientation
        vpl::img::CVector3D normal_image;
        readOrientTagDCTk(dataset, slice.m_ImageOrientationX, slice.m_ImageOrientationY, normal_image);

        // slice position calculation
        vpl::img::CPoint3D zero_point(0, 0, 0);
        vpl::img::CVector3D position_vector(zero_point, slice.m_ImagePosition);
        slice.setPosition(normal_image.dotProduct(normal_image, position_vector));

        // prepare for potential decompression
        OFBool opt_verbose = OFFalse;
        E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
        E_UIDCreation opt_uidcreation = EUC_default;
        E_PlanarConfiguration opt_planarconfig = EPC_default;
        DJDecoderRegistration::registerCodecs(opt_decompCSconversion, opt_uidcreation, opt_planarconfig, opt_verbose);

        E_TransferSyntax es = dataset->getOriginalXfer();
        OFCondition error = dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
        if (error.bad())
        {
            throw CDicomLoadingFailure();
        }

        // format conversion
        // vpl::img::tDensityPixel pMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
        // vpl::img::tDensityPixel pMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();

        if (bLoadImageData)
        {
            // Create a new image
            // - Load a single frame from the dataset!
            image = new DicomImage(dataset, es, 0UL, slicepos, 1UL);
            if (!image.get() || image->getStatus() != EIS_Normal)
            {
                throw CDicomLoadingFailure();
            }

            // create pixel data
            vpl::tSize xSize = static_cast< vpl::tSize >(image->getWidth());
            vpl::tSize ySize = static_cast< vpl::tSize >(image->getHeight());
            //            unsigned short depth = static_cast< unsigned short >( image->getDepth() );

            // resize the slice
            slice.vpl::img::CDImage::resize(xSize, ySize, slice.getMargin());

            image->deleteDisplayLUT(0);
            image->hideAllOverlays();
            image->removeAllOverlays();
            image->deleteOverlayData();
            image->setNoDisplayFunction();
            image->setNoVoiTransformation();
            image->getOverlayCount();



            // take pixel data pointer 
            const DiPixel * theData = image->getInterData();

            // take pixel data representation
            EP_Representation theData_representation = theData->getRepresentation();

            // test pixel data representation
            if (theData_representation != EPR_Uint16 && theData_representation != EPR_Sint16 &&
                theData_representation != EPR_Uint8 && theData_representation != EPR_Sint8 &&
                theData_representation != EPR_Uint32 && theData_representation != EPR_Sint32)
            {
                throw CDicomLoadingFailure();
            }

            // http://dicomiseasy.blogspot.cz/2012/08/chapter-12-pixel-data.html
            if (pixelMin == pixelMax)
            {
                if (theData_representation == EPR_Uint16)
                {
                    pixelMin = 0;
                    if (bitsUsed != 0)
                        pixelMax = (1 << bitsUsed) - 1;
                    else
                        pixelMax = 65535;
                }
                if (theData_representation == EPR_Sint16)
                {
                    pixelMin = -32768;
                    if (bitsUsed != 0)
                        pixelMax = (1 << bitsUsed) - 1;
                    else
                        pixelMax = 32767;
                }
                if (theData_representation == EPR_Uint8)
                {
                    pixelMin = 0;
                    if (bitsUsed != 0)
                        pixelMax = (1 << bitsUsed) - 1;
                    else
                        pixelMax = 255;
                }
                if (theData_representation == EPR_Sint8)
                {
                    pixelMin = -128;
                    if (bitsUsed != 0)
                        pixelMax = (1 << bitsUsed) - 1;
                    else
                        pixelMax = 127;
                }
                if (theData_representation == EPR_Uint32)
                {
                    pixelMin = 0;
                    if (bitsUsed != 0)
                        pixelMax = (1 << bitsUsed) - 1;
                    else
                        pixelMax = 0x7FFFFFFF;
                }
            }

            // take slice image data pointer from pixel data
            const vpl::img::tDensityPixel * thePixels = reinterpret_cast< const vpl::img::tDensityPixel* >(theData->getData());
            if (!thePixels)
            {
                throw CDicomLoadingFailure();
            }

            const vpl::img::tPixel16 * thePixels16u = reinterpret_cast< const vpl::img::tPixel16* >(theData->getData());
            const vpl::img::tPixel8 * thePixels8u = reinterpret_cast< const vpl::img::tPixel8* >(theData->getData());
            const vpl::sys::tInt8   * thePixels8 = reinterpret_cast< const vpl::sys::tInt8* >(theData->getData());
            const vpl::sys::tInt32  * thePixels32 = reinterpret_cast< const vpl::sys::tInt32* >(theData->getData());
            const vpl::img::tPixel32* thePixels32u = reinterpret_cast< const vpl::img::tPixel32* >(theData->getData());

            int dstMax = 32767;
            if (nDesiredBits>0)
                dstMax = (1 << nDesiredBits) - 1;
#pragma omp parallel for
            for (vpl::tSize y = 0; y < ySize; y++)
            {
                for (vpl::tSize x = 0; x < xSize; x++)
                {
                    int Value = 0;
                    switch (theData_representation)
                    {
                    case EPR_Uint8:
                        Value = thePixels8u[y * xSize + x];
                        break;
                    case EPR_Sint8:
                        Value = thePixels8[y * xSize + x];
                        break;
                    case EPR_Sint32:
                        Value = thePixels32[y * xSize + x];
                        break;
                    case EPR_Uint32:
                        Value = thePixels32[y * xSize + x];
                        break;
                    case EPR_Uint16:
                        Value = thePixels16u[y * xSize + x];
                        break;
                    default:
                        Value = thePixels[y * xSize + x];
                    }

                    if (pixelMax != pixelMin)
                        Value = (dstMax * (Value - pixelMin)) / (pixelMax - pixelMin);
                    Value = std::min(Value, dstMax);
                    Value = std::max(Value, 0);
                    slice(x, y) = (vpl::img::tDensityPixel)Value;
                }
            }
        }

        // Read all required DICOM tags
        readTagsDCTk(dataset, slice);
    }

    // Any failure?
    catch (CDicomLoadingFailure &)
    {
        browser.setDirectory(oldDir);
        DJDecoderRegistration::cleanup();
        return false;
    }

    browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

 }//namespace data
