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

#include "Serializer.hh"

using namespace std;
using namespace boost::filesystem;
using namespace boost::xpressive;
using namespace log4cxx;

LoggerPtr Serializer::logger(Logger::getLogger("Serializer"));
const string Serializer::schemaMarker = "//--AVRO--";

Serializer::Serializer() {
}

Serializer::Serializer(string path) :
        schemasBasePath(path) {

    LOG4CXX_DEBUG(logger, "Schema established to = " << path);
}

Serializer::~Serializer() {
}

string Serializer::serialize(const string& entry) {

    string result;

    path schemaPath(schemasBasePath);
    string fullSchemaPath = system_complete(schemaPath).string();

    LOG4CXX_DEBUG(logger, "Full schema path: " << fullSchemaPath);

    ifstream schemaFile(fullSchemaPath);

    if (!schemaFile.is_open()) {
        string errorMessage("Unable to open file: " + fullSchemaPath);
        LOG4CXX_ERROR(logger, errorMessage);
        throw iostream::failure(errorMessage);
    }

    avro::ValidSchema schema;

    Mapper mapper = loadMapper(schemaFile, schema);
    schemaFile.close();

    avro::GenericDatum datum(schema);

    if (mapper.pattern() == "") { // No mapping pattern loaded, change to raw mode
        LOG4CXX_WARN(logger, "No mapping pattern loaded, changing to raw mode");
        result = entry;
    }
    else {
        mapper.map(datum, entry);

        auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
        avro::EncoderPtr e = avro::validatingEncoder(schema, avro::binaryEncoder());
        e->init(*out);

        avro::encode(*e, datum);
        e->flush();

        auto_ptr<avro::InputStream> inraw = avro::memoryInputStream(*out);
        avro::StreamReader reader(*inraw);

        stringstream ss;
        while (reader.hasMore()) {
            ss << reader.read();
        }

        result = ss.str();
    }

    return result;
}

Mapper Serializer::loadMapper(istream &is, avro::ValidSchema &schema) {
    Mapper mapper;

    if (!is.good()) {
        LOG4CXX_WARN(logger, "Invalid schema file. Changing to raw serialization.");
        return mapper;
    }

    try {
        /* Extract header, no json related data */

        sregex rex = sregex::compile("\\s*pattern\\s*:\\s*(.*)\\s*");
        smatch what;

        string header;
        std::size_t found;

        for (;;) {
            getline(is, header);
            if (is.eof()) break;

            found = header.find(schemaMarker, 0);

            if (found != string::npos) {
                break;
            }
            else {
                // Extract the regular expression pattern for mapping (if present)

                if (regex_match(header, what, rex)) {
                    mapper.pattern(what[1]);
                    LOG4CXX_DEBUG(logger, "Mapper pattern to use: " << mapper.pattern());
                }
            }
        }

        avro::compileJsonSchema(is, schema);

        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
            debugSchemaNode(schema);
        }
    }
    catch (const avro::Exception &e) {
        LOG4CXX_WARN(logger, "Unexpected AVRO error. Changing to raw mode.\nDetail: " << e.what());
    }

    return mapper;
}

void Serializer::debugSchemaNode(const avro::ValidSchema &schema) const {
    const avro::NodePtr &root = schema.root();

    LOG4CXX_DEBUG(logger, "SCHEMA:"
            << "\nfullname: " << root->name()
            << "\nnames: " << root->names()
            << "\nleaves: " << root->leaves()
            << "\nschema: " << *root);
}
