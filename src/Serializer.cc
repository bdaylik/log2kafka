/**
 * @file Serializer.cc
 * @brief Class responsible for text entries serialization.
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

#include "Serializer.hh"

using namespace std;
using namespace boost::filesystem;
using namespace boost::xpressive;

#ifdef _LOG2KAFKA_USE_LOG4CXX_
using namespace log4cxx;

log4cxx::LoggerPtr Serializer::logger(Logger::getLogger("Serializer"));
#endif

const string Serializer::schemaMarker = "//--AVRO--";

const static Magic magic = { { 'O', 'b', 'j', '\x01' } };
const static string AVRO_SCHEMA_KEY("avro.schema");
const static string AVRO_CODEC_KEY("avro.codec");
const static string AVRO_NULL_CODEC("null");

/*-- constructors/destructor --*/

Serializer::Serializer() {
}

Serializer::~Serializer() {
}

Serializer::Serializer(std::string configFilePath) :
    configFilePath_(boost::trim_copy(configFilePath)) {

    LOG_DEBUG("Schema established to = " << configFilePath);
    configure();
}

/*-- getters/setters --*/

void Serializer::configFilePath(std::string configFilePath) {
    boost::trim(configFilePath);
    this->configFilePath_ = configFilePath;
}

const string& Serializer::configFilePath() const {
    return this->configFilePath_;
}

/*-- methods --*/

void Serializer::configure() {

    if (configFilePath_.length() == 0) {
        LOG_WARN("No schema configuration file defined");
        return;
    }

    if (configFilePath_[0] != '/') {
        configFilePath_.insert(0, Constants::DEFAULT_CONFIG_PATH);
    }

    path schemaPath(configFilePath_);
    string fullSchemaPath = system_complete(schemaPath).string();

    LOG_DEBUG("Full schema path: " << fullSchemaPath);

    ifstream schemaFile(fullSchemaPath);

    if (schemaFile.is_open()) {
        loadMapper(schemaFile);
        schemaFile.close();
    }
    else {
        LOG_WARN("Unable to open file: " << fullSchemaPath);
    }
}

void Serializer::serialize(const string& entry, auto_ptr<avro::OutputStream>& data) {

    avro::GenericDatum datum(mapper_);

    if (mapper_.pattern() != "") {

        mapper_.map(datum, entry);
        sync_ = makeSync();

        avro::EncoderPtr baseEncoder = avro::binaryEncoder();
        baseEncoder->init(*data);

        writeHeader(baseEncoder);
        writeDataBlock(baseEncoder, datum, data->byteCount());

        LOG_DEBUG("Data buffer size: " << data->byteCount());

        if (Constants::IS_TRACE_ENABLED) {

            /* Persist to file */

            const char* tempFileName = buildTempFileName();

            LOG_TRACE("Generating persistent file: " << tempFileName);

            auto_ptr<avro::InputStream> inraw = avro::memoryInputStream(*data);
            auto_ptr<avro::OutputStream> fileStream = avro::fileOutputStream(tempFileName);

            copy(*inraw, *fileStream);

            fileStream->flush();
            fileStream.release();
        }
    }
    else {
        throw InvalidMapperException();
    }
}

void Serializer::loadMapper(istream &is) {

    if (!is.good()) {
        LOG_WARN("Invalid schema file. Changing to raw serialization");
        return;
    }

    try {
        /* Extract header, no json related data */

        sregex rex = sregex::compile("\\s*pattern\\s*:\\s*(.*)\\s*");
        smatch what;

        string header;
        size_t found;

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
                    mapper_.pattern(what[1]);
                    LOG_DEBUG("Mapper pattern to use: " << mapper_.pattern());
                }
            }
        }

        setMetadata(AVRO_CODEC_KEY, AVRO_NULL_CODEC);
        avro::compileJsonSchema(is, mapper_);
        setMetadata(AVRO_SCHEMA_KEY, mapper_.compactJson());

        if (Constants::IS_DEBUG_ENABLED) debugSchemaNode(mapper_);
    }
    catch (const avro::Exception &e) {
        LOG_WARN("Unexpected AVRO error. Changing to raw mode.\nDetail: " << e.what());
    }
}

const char* Serializer::buildTempFileName() {
    hash<string> hash_fn;
    time_t now = time(NULL);

    ostringstream fileName;
    fileName << "log2kafka_avro_" << hash_fn(ctime(&now)) << ".txt";

#ifdef BOOST_FILESYSTEM_V3
    path tempFile(temp_directory_path() / fileName.str());
#else
    path tempFile(Util::getTempDirectoryPath() / fileName.str());
#endif

    return tempFile.string().data();
}

void Serializer::debugSchemaNode(const avro::ValidSchema &schema) const {
    const avro::NodePtr &root = schema.root();

    LOG_DEBUG("SCHEMA:"
        << "\nfullname: " << root->name()
        << "\nnames: " << root->names()
        << "\nleaves: " << root->leaves()
        << "\nschema: " << *root);
}

void Serializer::setMetadata(const string& key, const string& value) {
    LOG_DEBUG("Setting metadata key: " << key);

    vector<uint8_t> v(value.size());
    copy(value.begin(), value.end(), v.begin());
    metadata_[key] = v;

    LOG_TRACE("Metadata key value set to: " << value);
}

void Serializer::writeHeader(avro::EncoderPtr& e) {
    LOG_DEBUG("Write header");

    avro::encode(*e, magic);
    avro::encode(*e, metadata_);
    avro::encode(*e, sync_);

    e->flush();
}

void Serializer::writeDataBlock(avro::EncoderPtr& e, const avro::GenericDatum& datum,
    int64_t byteCount) {

    LOG_DEBUG("Write data block");

    // A long indicating the count of objects in this block
    int64_t objectCount = 1;
    avro::encode(*e, objectCount);

    // A long indicating the size in bytes of the serialized objects in the
    // current block, after any codec is applied
    avro::encode(*e, byteCount);

    // The serialized objects. If a codec is specified, this is compressed by
    // that codec.
    avro::encode(*e, datum);

    // The file's 16-byte sync marker
    avro::encode(*e, sync_);

    e->flush();
}

boost::mt19937 random_generator(static_cast<uint32_t>(time(0)));

DataBlockSync Serializer::makeSync() {
    DataBlockSync sync;

    for (size_t i = 0; i < sync.size(); ++i) {
        sync[i] = random_generator();
    }

    LOG_DEBUG("Calculated data block sync marker = " << sync.data());

    return sync;
}
