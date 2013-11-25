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

#ifndef _LOG2KAFKA_CLIENT_PROXY_HH_
#define _LOG2KAFKA_CLIENT_PROXY_HH_

#include <string>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <vector>

#include <libkafka/Client.h>
#include <log4cxx/logger.h>

#include "Constants.hh"

namespace kafka = LibKafka;

class ClientProxy {
public:

    std::string clientId;
    std::string host;
    std::size_t port;
    int requiredAcks;
    int timeout;

    std::vector<std::string> topics;

    ClientProxy();

    ClientProxy(const std::string& host, std::size_t port, const std::string& topic);

    virtual ~ClientProxy();

    kafka::Message* createMessage(const std::string& message);

    kafka::ProduceMessageSet* createMessageSet(const std::string& message);

    kafka::TopicNameBlock<kafka::ProduceMessageSet>* createRequestTopicNameBlock(const std::string& topic,
            const std::string& message);

    kafka::ProduceRequest* createProduceRequest(const std::string& message);

    void sendMessage(const std::string& message);

private:
    static log4cxx::LoggerPtr logger;

    void initDefaults();
    int getCorrelationId();
};

#endif /* _LOG2KAFKA_CLIENT_PROXY_HH_ */
