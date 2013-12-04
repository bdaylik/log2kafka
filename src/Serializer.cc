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
using namespace log4cxx;

/* Static fields */

LoggerPtr Serializer::logger(Logger::getLogger("Serializer"));

const string Serializer::schemaMarker = "//--AVRO--";

const string AVRO_SCHEMA_KEY("avro.schema");
const string AVRO_CODEC_KEY("avro.codec");
const string AVRO_NULL_CODEC("null");

/* Methods */

Serializer::Serializer() {
}

Serializer::~Serializer() {
}

Serializer::Serializer(std::string configFilePath) :
        _configFilePath(boost::trim_copy(configFilePath)) {

    LOG4CXX_DEBUG(logger, "Schema established to = " << configFilePath);
    configure();
}

void Serializer::configFilePath(std::string configFilePath) {
    boost::trim(configFilePath);
    this->_configFilePath = configFilePath;
}

const string& Serializer::configFilePath() const {
    return this->_configFilePath;
}

void Serializer::configure() {

    if (_configFilePath.length() == 0) {
        LOG4CXX_WARN(logger, "No schema configuration file defined");
        return;
    }

    if (_configFilePath[0] != '/') {
        _configFilePath.insert(0, Constants::DEFAULT_SCHEMA_PATH);
    }

    path schemaPath(_configFilePath);
    string fullSchemaPath = system_complete(schemaPath).string();

    LOG4CXX_DEBUG(logger, "Full schema path: " << fullSchemaPath);

    ifstream schemaFile(fullSchemaPath);

    if (!schemaFile.is_open()) {
        string errorMessage("Unable to open file: " + fullSchemaPath + ". Changing to raw serialization.");
        LOG4CXX_WARN(logger, errorMessage);
        return;
    }

    loadMapper(schemaFile);
    schemaFile.close();
}

void Serializer::serialize(const string& entry, vector<uint8_t>& data) {

    avro::GenericDatum datum(_mapper);

    if (_mapper.pattern() != "") {

        _mapper.map(datum, entry);

        auto_ptr<avro::OutputStream> resultOutput = avro::memoryOutputStream();
        auto_ptr<avro::OutputStream> dataOutput = avro::memoryOutputStream();

        avro::EncoderPtr baseEncoder = avro::binaryEncoder();
        avro::EncoderPtr validatingEncoder = avro::validatingEncoder(_mapper, baseEncoder);

        _sync = makeSync();

        baseEncoder->init(*resultOutput);
        writeHeader(baseEncoder);

        baseEncoder->init(*dataOutput);
        avro::encode(*validatingEncoder, datum);
        validatingEncoder->flush();

        baseEncoder->init(*resultOutput);
        avro::encode(*baseEncoder, 1);
        int64_t byteCount = dataOutput->byteCount();
        avro::encode(*baseEncoder, byteCount);
        baseEncoder->flush();

        auto_ptr<avro::InputStream> inraw = avro::memoryInputStream(*dataOutput);

        copy(*inraw, *resultOutput);

        baseEncoder->init(*resultOutput);
        avro::encode(*baseEncoder, _sync);
        baseEncoder->flush();

        LOG4CXX_DEBUG(logger, "Result output size: " << resultOutput->byteCount());

        auto_ptr<avro::InputStream> testRead = avro::memoryInputStream(*resultOutput);
        avro::StreamReader reader(*testRead);

        while (reader.hasMore()) {
            data.push_back(reader.read());
        }

        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {
            hash<std::string> hash_fn;
            time_t now = time(NULL);

            ostringstream fileName;

            fileName << "/tmp/avro_" << hash_fn(ctime(&now)) << ".txt";

            std::auto_ptr<avro::OutputStream> fileStream = avro::fileOutputStream(fileName.str().data());
            testRead->backup(resultOutput->byteCount());

            copy(*testRead, *fileStream);
            fileStream->flush();
            fileStream.release();
        }

        LOG4CXX_DEBUG(logger, "Vector data size: " << data.size());
    }
    else { // Invalid mapper or mapping pattern not loaded, change to raw mode
        LOG4CXX_WARN(logger, "Invalid mapper or mapping pattern not loaded, changing to raw mode");
        copy(entry.begin(), entry.end(), back_inserter(data));
    }
}

void Serializer::loadMapper(istream &is) {

    if (!is.good()) {
        LOG4CXX_WARN(logger, "Invalid schema file. Changing to raw serialization.");
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
                    _mapper.pattern(what[1]);
                    LOG4CXX_DEBUG(logger, "Mapper pattern to use: " << _mapper.pattern());
                }
            }
        }

        avro::compileJsonSchema(is, _mapper);

        setMetadata(AVRO_CODEC_KEY, AVRO_NULL_CODEC);

        ostringstream oss;
        _mapper.toJson(oss);

        setMetadata(AVRO_SCHEMA_KEY, oss.str());

        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
            debugSchemaNode(_mapper);
        }
    }
    catch (const avro::Exception &e) {
        LOG4CXX_WARN(logger, "Unexpected AVRO error. Changing to raw mode.\nDetail: " << e.what());
    }
}

void Serializer::debugSchemaNode(const avro::ValidSchema &schema) const {
    const avro::NodePtr &root = schema.root();

    LOG4CXX_DEBUG(logger, "SCHEMA:"
            << "\nfullname: " << root->name()
            << "\nnames: " << root->names()
            << "\nleaves: " << root->leaves()
            << "\nschema: " << *root);
}

void Serializer::setMetadata(const string& key, const string& value) {
    LOG4CXX_DEBUG(logger, "Setting metadata key: " << key);

    vector<uint8_t> v(value.size());
    copy(value.begin(), value.end(), v.begin());
    _metadata[key] = v;

    LOG4CXX_TRACE(logger, "Metadata key value set to: " << value);
}

boost::mt19937 random_generator(static_cast<uint32_t>(time(0)));

static Magic magic = { { 'O', 'b', 'j', '\x01' } };

void Serializer::writeHeader(avro::EncoderPtr& e) {
    LOG4CXX_INFO(logger, "Write metadata header.");

    avro::encode(*e, magic);
    avro::encode(*e, _metadata);
    avro::encode(*e, _sync);

    e->flush();
}

DataBlockSync Serializer::makeSync() {
    DataBlockSync sync;

    for (size_t i = 0; i < sync.size(); ++i) {
        sync[i] = random_generator();
    }

    return sync;
}

