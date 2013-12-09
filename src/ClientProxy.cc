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

#include "ClientProxy.hh"

using namespace std;
using namespace log4cxx;

namespace kafka = LibKafka;

LoggerPtr ClientProxy::logger(Logger::getLogger("ClientProxy"));

ClientProxy::ClientProxy() {
    initDefaults();
}

ClientProxy::~ClientProxy() {
}

void ClientProxy::clientId(std::string clientId) {
    this->clientId_ = clientId;
}

void ClientProxy::messageKey(std::string messageKey) {
    this->messageKey_ = messageKey;
}

void ClientProxy::host(std::string host) {
    this->host_ = host;
}

void ClientProxy::port(int port) {
    this->port_ = port;
}

void ClientProxy::topic(std::string topic) {
    vector<string> fields;
    int partition = 0;

    // TODO: document pre condition: topic name present

    boost::split(fields, topic, boost::is_any_of(":"));

    if (fields.size() >= 2) {
        try {
            partition = boost::lexical_cast<int>(fields[1]);
        }
        catch (exception& e) {
            LOG4CXX_WARN(logger, "Invalid partition value: " << fields[1] << ". Defaulting to: " << partition);
        }
    }

    this->topics_.insert(make_pair(fields[0], partition));
}

void ClientProxy::requiredAcks(int requiredAcks) {
    this->requiredAcks_ = requiredAcks;
}

void ClientProxy::timeoutAcks(int timeoutAcks) {
    this->timeoutAcks_ = timeoutAcks;
}

void ClientProxy::serializer(const string& configFile) {
    unique_ptr<Serializer> serializer(new Serializer(configFile));
    this->serializer_ = move(serializer);
}

void ClientProxy::initDefaults() {
    this->clientId_ = Constants::DEFAULT_CLIENT_ID;
    this->requiredAcks_ = Constants::DEFAULT_REQUIRED_ACKS;
    this->timeoutAcks_ = Constants::DEFAULT_TIMEOUT_ACKS;
}

kafka::Message* ClientProxy::createMessage(const auto_ptr<avro::OutputStream>& data) {

    int crc = 0;
    int offset = -1;
    signed char magicByte = 1;
    signed char attributes = 0; // last three bits must be zero to disable gzip compression

    // Copy message key

    int keyLength = this->messageKey_.length();
    uint8_t* key = new uint8_t[keyLength];

    memcpy(key, this->messageKey_.data(), keyLength);

    // Copy message value

    uint64_t valueLength = data->byteCount();
    uint8_t* value = new uint8_t[valueLength];

    auto_ptr<avro::InputStream> dataInput = avro::memoryInputStream(*data);
    avro::StreamReader reader(*dataInput);
    reader.readBytes(value, valueLength);

    if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
        LOG4CXX_DEBUG(logger, "Message to be sent:");
        LOG4CXX_DEBUG(logger, "MESSAGE BEGIN");
        cout.write(reinterpret_cast<const char*>(value), valueLength);
        cout << endl;
        LOG4CXX_DEBUG(logger, "MESSAGE END");
    }

    //memcpy(value, message.data(), valueLength);

    return new kafka::Message(crc, magicByte, attributes, keyLength, key, valueLength, value, offset);
}

kafka::ProduceMessageSet* ClientProxy::createMessageSet(int partition,
        const auto_ptr<avro::OutputStream>& data) {

    vector<kafka::Message*> messages;

    kafka::Message *kafkaMessage = createMessage(data);
    messages.push_back(kafkaMessage);

    // sizeof(offset) + sizeof(messageSize) + messageSize
    int messageSetSize = sizeof(long int) + sizeof(int) + kafkaMessage->getWireFormatSize(false);

    kafka::MessageSet* messageSet = new kafka::MessageSet(messageSetSize, messages);
    messageSetSize = messageSet->getWireFormatSize(false);

    return new kafka::ProduceMessageSet(partition, messageSetSize, messageSet);
}

kafka::TopicNameBlock<kafka::ProduceMessageSet>* ClientProxy::createRequestTopicNameBlock(const string& topic,
        int partition, const auto_ptr<avro::OutputStream>& data) {

    int numMessages = 1;

    kafka::ProduceMessageSet** messageSetArray = new kafka::ProduceMessageSet*[numMessages];
    messageSetArray[0] = createMessageSet(partition, data);

    return new kafka::TopicNameBlock<kafka::ProduceMessageSet>(topic, numMessages, messageSetArray);
}

kafka::ProduceRequest* ClientProxy::createProduceRequest(const auto_ptr<avro::OutputStream>& data) {

    int correlationId = generateCorrelationId();
    vector<std::string>::size_type numTopics = this->topics_.size();

    kafka::TopicNameBlock<kafka::ProduceMessageSet>** topicArray =
            new kafka::TopicNameBlock<kafka::ProduceMessageSet>*[numTopics];

    int i = 0;

    for (topicmap::iterator it = this->topics_.begin(); it != this->topics_.end(); ++it) {
        topicArray[i] = createRequestTopicNameBlock(it->first, it->second, data);
        i++;
    }

    return new kafka::ProduceRequest(correlationId, clientId_, requiredAcks_, timeoutAcks_, numTopics,
            topicArray, false);
}

void ClientProxy::sendMessage(const string& message) {

    LOG4CXX_DEBUG(logger, "Message: " << message);

    auto_ptr<avro::OutputStream> dataOutput = avro::memoryOutputStream();

    /* Prepare message */

    if (message.length() == 0) {
        LOG4CXX_WARN(logger, "Empty message entry discarded");
        return;
    }

    if (serializer_) { // Use serialization mode
        LOG4CXX_DEBUG(logger, "Schema defined. Using serialization mode");

        try {
            serializer_->serialize(message, dataOutput);
        }
        catch (exception& e) {
            //copy(message.begin(), message.end(), back_inserter(preparedMessage));
            LOG4CXX_ERROR(logger, "Using raw mode due to unexpected exception: " << e.what());
        }
    }
    else { // Use raw mode
           //copy(message.begin(), message.end(), back_inserter(preparedMessage));
        LOG4CXX_DEBUG(logger, "No schema defined. Using raw mode");
    }

    /* Send request */

    unique_ptr<kafka::ProduceRequest> pr1(createProduceRequest(dataOutput));
    //kafka::ProduceRequest *pr1 = createProduceRequest(dataOutput);

    if (pr1) {
        LOG4CXX_DEBUG(logger, "Request:\n" << pr1.get());

        if (!client_) {
            client_.reset(new kafka::Client(host_, port_));
        }

        unique_ptr<kafka::ProduceResponse> pr2(client_->sendProduceRequest(pr1.get()));
        // kafka::ProduceResponse *pr2 = client_->sendProduceRequest(pr1);

        if (pr2) {
            LOG4CXX_DEBUG(logger, "Response received");
            LOG4CXX_DEBUG(logger, "Response detail:\n" << pr2.get());

            if (pr2->hasErrorCode()) {
                LOG4CXX_DEBUG(logger, "Unexpected sendProduceRequest error: " << pr2->hasErrorCode());
            }

            // delete pr2;
        }
        else {
            LOG4CXX_DEBUG(logger, "Response is null");
        }

        // delete pr1;
    }
}

int ClientProxy::generateCorrelationId() {
    hash<std::string> hash_fn;
    time_t now = time(NULL);
    return hash_fn(ctime(&now));
}
