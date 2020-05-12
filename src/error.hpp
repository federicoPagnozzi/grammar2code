//
//  error.hpp
//  Grammar2Code
//
//  Created by Franco Mascia on 02/04/13.
//  Copyright (c) 2013 Franco Mascia. All rights reserved.
//
//  This file is distributed under the BSD 2-Clause License. See LICENSE.TXT
//  for details.
//

#ifndef __Grammar2Code__Error__
#define __Grammar2Code__Error__

#include <string>

class Error {
public:
    static void set_exec_name(const std::string& name);
    static void fatal(std::string message);
    static void warning(std::string message);

private:
    static std::string executable_name;
};

#endif /* defined(__Grammar2Code__Error__) */
