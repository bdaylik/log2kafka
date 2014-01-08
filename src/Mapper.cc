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

LoggerPtr Mapper::logger(Logger::getLogger("Mapper"));
#endif

/*-- constructors/destructor --*/

Mapper::Mapper() {
}

Mapper::~Mapper() {
}

/*-- getters/setters --*/

void Mapper::pattern(string pattern) {
    pattern_ = pattern;
}

const string& Mapper::pattern() const {
    return pattern_;
}

const string& Mapper::compactJson() {
    if (compactJson_.length() == 0 && root()->isValid()) {
        ostringstream oss;
        toJson(oss);

        /* clean pretty-print format */

        sregex newlinePlusLeadingSpace = _ln >> *_s;
        compactJson_ = regex_replace(oss.str(), newlinePlusLeadingSpace, "");
    }

    return compactJson_;
}

/*-- methods --*/

void Mapper::map(avro::GenericDatum& datum, const string& entry) {

    if (regex_.regex_id() == 0) {
        regex_ = sregex::compile(pattern_);
    }

    smatch what;

    if (regex_match(entry, what, regex_)) {

        LOG_DEBUG("Valid entry detected: " << what[0].str());

        if (datum.type() == avro::AVRO_RECORD) {
            istringstream ss;

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
