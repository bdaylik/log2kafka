/**
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

#ifndef _LOG2KAFKA_MAPPER_HH_
#define _LOG2KAFKA_MAPPER_HH_

#include <sstream>

#include <avro/Types.hh>
#include <avro/Generic.hh>

#include <boost/xpressive/xpressive.hpp>

#include <log4cxx/logger.h>

#include "Constants.hh"
#include "Exceptions.hh"

class Mapper: public avro::ValidSchema {
public:
    Mapper();
    explicit Mapper(std::string pattern);
    virtual ~Mapper();

    void pattern(std::string pattern);
    const std::string& pattern() const;

    void map(avro::GenericDatum& datum, const std::string& entry);

private:
    static log4cxx::LoggerPtr logger;

    std::string _pattern;
    boost::xpressive::sregex _regex;
};

#endif /* _LOG2KAFKA_MAPPER_HH_ */
