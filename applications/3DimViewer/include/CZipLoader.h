///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2018 3Dim Laboratory s.r.o.
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

#ifndef _CZIPLOADER_H
#define _CZIPLOADER_H

#include <zlib.h>
#include <zip/unzip.h>
#include <QString>
#include <QTemporaryFile>
#include <QDir>
#include <QDirIterator>
#include <mainwindow.h>
#include <C3DimApplication.h>

// simple helper class for dicom data loading from zip archive
class CZipLoader
{
protected:
    QString m_archive;
    QString m_tempDir;
public:
    //! Constructor
    CZipLoader() {}
    //! Destructor removes temporary directory with extracted zip contents
    ~CZipLoader() 
    {
        if (!m_tempDir.isEmpty())
            if (!removeDir(m_tempDir, true))
            {
             //   MainWindow::getInstance()->m_cleanupList.push_back(m_tempDir);
            }

    }
    // checks filename for zip extension
    bool setZipArchive(const QString& fileName, bool bCheckExtension = true)
    {
        if (!bCheckExtension && !fileName.isEmpty())
        {
            m_archive = fileName;
            return true;
        }
        QFileInfo pathInfo( fileName );
        QString ext = pathInfo.suffix();
        if (0==ext.compare("zip",Qt::CaseInsensitive)) // return zip files
        {
            m_archive = fileName;
            return true;    
        }
        return false;
    }
    //! decompress zip archive to a temporary directory which will be deleted with this object
    QString decompress()
    {
        // get temp dir name
        {
            QTemporaryFile file;
            file.open();
            m_tempDir = file.fileName();
            if (m_tempDir.isEmpty())
            {
                m_tempDir = QDir::tempPath();
                m_tempDir+="/3dvziptmp";
            }
            m_tempDir+="/";
        }
       
        // make sure that the directory exists
        {
            QDir dir;
            bool ok = dir.mkpath(m_tempDir);
            Q_ASSERT(ok);        
        }

        bool bDecompressedSomething = false;

        std::string str = m_archive.toStdString();
        unzFile hZIP = unzOpen(str.c_str());
#ifdef _WIN32
        if (!hZIP) // try acp
        {
            std::wstring uniName = (const wchar_t*)m_archive.utf16();
            str = C3DimApplication::wcs2ACP(uniName);
            hZIP = unzOpen(str.c_str());
        }
#endif
        if (hZIP)
        {
#define CUSTOM_MAX_PATH 4096
            char pFileName[CUSTOM_MAX_PATH]={};
            bool bContinue = (UNZ_OK == unzGoToFirstFile(hZIP));
            while (bContinue) 
            {
                unz_file_info fi = {};
                memset(pFileName,0,CUSTOM_MAX_PATH*sizeof(char));
                if (unzGetCurrentFileInfo(hZIP, &fi, pFileName, CUSTOM_MAX_PATH, NULL, 0, NULL, 0) == UNZ_OK) 
                {
#undef CUSTOM_MAX_PATH
                    std::string s = pFileName;
                    QString file = m_tempDir;
                    if (s.compare("/") != 0 && s.compare("\\") != 0)
                        file += QString::fromStdString(s);
                    if (file.endsWith('/') || file.endsWith('\\'))
                    {
                        QDir dir;
                        bool ok = dir.mkpath(file);
                        Q_ASSERT(ok);
                    }
                    else
                    {
                        // sometimes it was trying to create a file in non-existing directory,
                        // so make sure the directory is created (just create it)
                        QFileInfo fInfo(file);
                        QString dirPath = fInfo.dir().absolutePath();
                        QDir dir;
                        bool ok = dir.mkpath(dirPath);
                        Q_ASSERT(ok);

                        if (fi.uncompressed_size>0)
                        {
                            char* pDecompressed=new char[fi.uncompressed_size];
                            if (NULL!=pDecompressed)
                            {
                                int nRead = 0;
                                if (unzOpenCurrentFile(hZIP) == UNZ_OK) 
                                {
                                    nRead = unzReadCurrentFile(hZIP, pDecompressed, fi.uncompressed_size);
                                    if (UNZ_CRCERROR == unzCloseCurrentFile(hZIP))
                                        nRead = 0;
                                }
                                if (nRead>0)
                                {
                                    QFile f(file);
                                    { // make sure that the full path exists
                                        QFileInfo inff(file);                                        
                                        dir = QDir(inff.dir());
                                        dir.mkpath(dir.absolutePath());
                                    }
                                    if( f.open(QIODevice::WriteOnly) )
                                    {
                                        f.write(pDecompressed,fi.uncompressed_size);
                                        f.close();
                                        bDecompressedSomething = true;
                                    }
                                }
                                delete [] pDecompressed;
                            }
                        }
                    }                    
                }
                bContinue = (UNZ_OK == unzGoToNextFile(hZIP));
            }
            unzClose(hZIP);
        }
        if (bDecompressedSomething)
            return m_tempDir;
        else
        {
            removeDir(m_tempDir, true);
            m_tempDir.clear();
            return m_archive; // return original zip name
        }
    }
    //! removeDir erases specified directory and all subdirectories
    static bool removeDir(const QString& dirName, bool bContinueOnFail) 
    {
        if (dirName.isEmpty())
            return false;
        bool result = true;
        QDir dir(dirName); 
        if (dir.exists()) 
        {
            QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
            Q_FOREACH(QFileInfo info, list) 
            {
                if (info.isDir()) 
                    result = removeDir(info.absoluteFilePath(), bContinueOnFail);
                else 
                    result = QFile::remove(info.absoluteFilePath());
                if (!result && !bContinueOnFail)
                    return result;
            }
            result = dir.rmdir(dirName);
        }
        return result;
    }
};


#endif _CZIPLOADER_H
