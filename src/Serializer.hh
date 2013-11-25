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

#ifndef _LOG2KAFKA_SERIALIZER_HH_
#define _LOG2KAFKA_SERIALIZER_HH_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <avro/Types.hh>
#include <avro/Node.hh>
#include <avro/Compiler.hh>
#include <avro/Encoder.hh>
#include <avro/Decoder.hh>
#include <avro/Specific.hh>
#include <avro/Generic.hh>

#include <boost/filesystem.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <log4cxx/logger.h>

#include "Constants.hh"
#include "Mapper.hh"

class Serializer {
public:
    Serializer();
    explicit Serializer(std::string path);
    virtual ~Serializer();

    std::string serialize(const std::string& entry);

private:
    static log4cxx::LoggerPtr logger;
    static const std::string schemaMarker;

    std::string schemasBasePath;

    Mapper loadMapper(std::istream &is, avro::ValidSchema &schema);
    void debugSchemaNode(const avro::ValidSchema &schema) const;
};

#endif /* _LOG2KAFKA_SERIALIZER_HH_ */
