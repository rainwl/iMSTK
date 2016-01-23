// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CORE_ERRORLOG_H
#define CORE_ERRORLOG_H

// STL includes
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>

// iMSTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/Timer.h"

#define PRINT_ERROR_LOCATION std::cout << "Error! In file: " << __FILE__ << "; at line: " << __LINE__ << std::endl;

/// \brief This is class is for error logging of the whole iMSTK system.
/// All errors should be reported to the instance of this class.
/// Functions are thread-safe unless indicated.
class ErrorLog : public CoreClass
{

private:
    std::vector<std::string> errors; ///< error messages
    std::vector<int> timeStamps; ///< time stamps for errors
    std::mutex logLock; ///< mutex to sync access to logs
    core::Timer time; ///< Timer for timestamps
    bool consoleOutput; ///< Flag to print errors to stdout

public:
    bool isOutputtoConsoleEnabled;
    ErrorLog();

    /// \brief Add the error in the repository.It is thread safe. It can be called by multiple threads.
    ///
    /// \detail If the error message is longer than IMSTK_MAX_ERRORLOG or
    /// empty, function will return with error
    /// \param p_text A string containing the error message
    /// \return Returns true if the error was successfully logged, and false on error
    bool addError(const std::string& p_text);

    /// \brief Clean up all the errors in the repository.
    ///
    void cleanAllErrors();

    /// \brief Print the last error. It is not thread safe.
    ///
    void printLastErrUnsafe();

    /// \brief Print the last error.
    ///
    void printLastErr();

    /// \brief Copy all errors logged to console
    ///
    /// \param flag Set true to enable, false to disable copy to console
    void setConsoleOutput(bool flag);
};

#endif