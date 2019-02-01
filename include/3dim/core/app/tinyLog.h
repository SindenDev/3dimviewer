///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2018 TESCAN 3DIM, s.r.o.
// All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ciso646>

namespace app {

    class Log {

    public:

        /*
        Opens file for appending.
        */
        Log(std::string filename) {
            m_name = filename;

            m_logFile.open(m_name, std::ios::app);

        }

        /*
        Just closes file if it's open.
        */
        ~Log() {

            if (m_logFile.is_open())
                m_logFile.close();

        }

        /*
        If file is closed it is opened and message is appended.
        */
        void append(std::string msg, bool print = false) {

            if (not m_logFile.is_open())
                m_logFile.open(m_name, std::ios::app);


            if (m_logFile.is_open())
                m_logFile << msg;

            if (print)
                std::cout << msg;

        }

        /*
        Closes file. for when you won't be needing the file for a while.
        */
        void done() {

            m_logFile.close();
        }

        std::ofstream m_logFile;
        std::string m_name;
    };
}
