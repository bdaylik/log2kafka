/**
 * @file Util.hh
 * @brief Utility functions.
 * @author Reinaldo Silva
 * @version 1.0
 * @date 2013
 * @copyright Copyright 2013 Produban. All rights reserved.
 * @copyright Licensed under the Apache License, Version 2.0
 * @copyright http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * Copyright 2013 Produban
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifndef _LOG2KAFKA_UTIL_HH_
#define _LOG2KAFKA_UTIL_HH_

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "config.hh"

/**
 * Global utility functions class.
 */
class Util {
public:

    /**
     * Boost clone function for older libraries.
     * TODO: conditional compilation, verify boost version number
     */
    static boost::filesystem::path getTempDirectoryPath();

private:

    /*-- static fields --*/

#ifdef _LOG2KAFKA_USE_LOG4CXX_
    /**
     * Class logger.
     */
    static log4cxx::LoggerPtr logger;
#endif
};

#endif /* _LOG2KAFKA_UTIL_HH_ */
