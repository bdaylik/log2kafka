/**
 * @file Mapper.hh
 * @brief An utility class that map text entries to AVRO datum generic
 * instances for serialization.
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

#ifndef _LOG2KAFKA_MAPPER_HH_
#define _LOG2KAFKA_MAPPER_HH_

#include <sstream>

#include <avro/Types.hh>
#include <avro/Generic.hh>

#include <boost/xpressive/xpressive.hpp>

#include "config.hh"

/**
 * An utility class that map text entries to AVRO datum generic instances for
 * serialization.
 */
class Mapper: public avro::ValidSchema {
public:

    Mapper();
    virtual ~Mapper();

    /*-- getters/setters --*/

    /**
     * Set the regular expression pattern to use for fields mapping.
     */
    void pattern(std::string pattern);

    /**
     * Return the regular expresion pattern mapper used for fields mapping.
     */
    const std::string& pattern() const;

    /*-- methods --*/

    /**
     * Map an entry in a generic AVRO datum instance using the pattern and
     * schema definition of the mapper.
     */
    void map(avro::GenericDatum& datum, const std::string& entry);

private:

    /*-- static fields --*/

#ifdef _LOG2KAFKA_USE_LOG4CXX_
    /**
     * Class logger.
     */
    static log4cxx::LoggerPtr logger;
#endif

    /*-- fields --*/

    /**
     * Regular expresion pattern to use to map entries to the AVRO schema
     * definition.
     */
    std::string _pattern;

    /**
     * Compiled regular expression pattern.
     */
    boost::xpressive::sregex _regex;
};

#endif /* _LOG2KAFKA_MAPPER_HH_ */
