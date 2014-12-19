///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include <data/CDicom.h>
#include <data/DicomTags.h>

// DCMTK
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

// STL
#include <fstream>

// SHA-1 hash function
#include <math/sha1.h>


//=================================================================================================
data::CDicom::CDicom( const std::string & file  )
    : m_bOk( false )
    , m_lSize( -1 )
    , m_sFileName( "" )
    , m_Compression( RAW )
{
    m_pHandle = new DcmFileFormat;

    if( !file.empty() )
    {
        this->loadFile( file );
    }
}

//=================================================================================================
data::CDicom::CDicom()
    : m_bOk( false )
    , m_lSize( -1 )
    , m_sFileName( "" )
    , m_Compression( RAW )
{
    m_pHandle = new DcmFileFormat;
}

//=================================================================================================
data::CDicom::~CDicom()
{
    if( m_pHandle )
    {
        delete m_pHandle;
    }
}

//=================================================================================================
long data::CDicom::getFileSize( const std::string & file )
{
    std::ifstream inFile( file.c_str(), std::ios::in|std::ios::binary );

    if ( inFile.bad() )
    {
        return -1;
    }
    else
    {
        std::ifstream::pos_type beg, end;
        inFile.seekg(0, std::ios::beg);
        beg = inFile.tellg();
        inFile.seekg(0, std::ios::end);
        end = inFile.tellg();

        inFile.close();

        return long(end - beg);
    }
}

//=================================================================================================
const std::string& data::CDicom::getFileName()
{
	return m_sFileName;
}

//=================================================================================================
bool data::CDicom::loadFile( const std::string & file )
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
long data::CDicom::saveToBuffer( char * buffer, long length )
{
    long size = 0;
    
    if ( m_bOk )
    {
        DcmOutputBufferStream stream( static_cast< void* >( buffer ), static_cast< unsigned >( length ) );

        m_pHandle->transferInit();

        switch ( m_Compression )
        {
            case LOSSLESS:
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361)
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
bool data::CDicom::setPatientName( const std::string & name )
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
std::string data::CDicom::hash( const char * input )
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
std::string data::CDicom::getStudyId()
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
std::string data::CDicom::getSerieId()
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
int data::CDicom::getBitsAllocated()
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

//==============================================================================================
int data::CDicom::convPos2Id( double a, double b, double c )
{
    static int MaxId = 0x3FFFFFFF;
    
    double dValue = a * a + b * b + c * c;
	assert(dValue>=0);

    int sign = 1, res = 0;
    if (a<0) sign=-sign;
    if (b<0) sign=-sign;
    if (c<0) sign=-sign;
    if (int(dValue * 1000.0)<MaxId)
        res = int(dValue * 1000.0);
    else
        res = int(dValue) | 0x40000000;
    res*=sign;

    //char ss[256]={};
    //sprintf(ss,"%.3f %.3f %.3f   %.3f %d\n",a,b,c,dValue,res);
    //OutputDebugStringA(ss);
    return res;
}

//=================================================================================================
int data::CDicom::getSliceId()
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
void data::CDicom::getSliceIds(tDicomNumList& Numbers)
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
    else
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
bool data::CDicom::anonymize( const std::string & name )
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
std::string	data::CDicom::getPatientName()
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
bool data::CDicom::compressLosslessJPEG()
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
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361)
    dataset->chooseRepresentation(EXS_JPEGProcess14SV1, &params);
#else
    dataset->chooseRepresentation(EXS_JPEGProcess14SV1TransferSyntax, &params);
#endif

    // check if everything went well
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361)
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
