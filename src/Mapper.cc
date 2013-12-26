/**
 * @file Mapper.cc
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

#include "Mapper.hh"

using namespace std;
using namespace boost::xpressive;

#ifdef _LOG2KAFKA_USE_LOG4CXX_
using namespace log4cxx;

/*-- Static fields --*/

LoggerPtr Mapper::logger(Logger::getLogger("Mapper"));
#endif

/*-- constructors/destructor --*/

Mapper::Mapper() {
}

Mapper::~Mapper() {
}

/*-- getters/setters --*/

void Mapper::pattern(std::string pattern) {
    this->_pattern = pattern;
}

const string& Mapper::pattern() const {
    return this->_pattern;
}

/*-- methods --*/

void Mapper::map(avro::GenericDatum& datum, const string& entry) {

    if (_regex.regex_id() == 0) {
        _regex = sregex::compile(_pattern);
    }

    smatch what;

    if (regex_match(entry, what, _regex)) {

        LOG_DEBUG("Valid entry detected: " << what[0].str());

        if (datum.type() == avro::AVRO_RECORD) {
            std::istringstream ss;

            avro::GenericRecord& record = datum.value<avro::GenericRecord>();
            LOG_DEBUG("Field count: " << record.fieldCount());

            for (size_t i = 0; i < record.fieldCount(); ++i) {
                avro::GenericDatum& field = record.fieldAt(i);
                ss.str(what[i + 1].str());

                switch (field.type()) {
                case avro::Type::AVRO_BOOL:
                    bool boolValue;
                    ss >> boolValue;
                    record.setFieldAt(i, avro::GenericDatum(boolValue));
                    break;

                case avro::Type::AVRO_INT:
                    int32_t intValue;
                    ss >> intValue;
                    record.setFieldAt(i, avro::GenericDatum(intValue));
                    break;

                case avro::Type::AVRO_LONG:
                    int64_t longValue;
                    ss >> longValue;
                    record.setFieldAt(i, avro::GenericDatum(longValue));
                    break;

                case avro::Type::AVRO_FLOAT:
                    float floatValue;
                    ss >> floatValue;
                    record.setFieldAt(i, avro::GenericDatum(floatValue));
                    break;

                case avro::Type::AVRO_DOUBLE:
                    double doubleValue;
                    ss >> doubleValue;
                    record.setFieldAt(i, avro::GenericDatum(doubleValue));
                    break;

                case avro::Type::AVRO_STRING:
                    default:
                    record.setFieldAt(i, avro::GenericDatum(ss.str()));
                }

                LOG_DEBUG("Field " << i << " = " << ss.str());

                ss.clear();
            }
        }
    }
    else {
        throw MapperMatchException();
    }
}
