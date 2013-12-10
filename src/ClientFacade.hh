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

#ifndef _LOG2KAFKA_CLIENT_FACADE_HH_
#define _LOG2KAFKA_CLIENT_FACADE_HH_

#include <string>
#include <cstring>
#include <ctime>
#include <functional>
#include <iomanip>
#include <memory>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <log4cxx/logger.h>

extern "C" {
#include <librdkafka/rdkafka.h>
}

#include "Constants.hh"
#include "Serializer.hh"

/**
 * Client connection facade class.
 */
class ClientFacade {
public:

    ClientFacade();
    virtual ~ClientFacade();

    /*-- getters/setters --*/

    /**
     * Set the Client ID.
     */
    void clientId(std::string clientId);

    /**
     * Set the message key.
     */
    void messageKey(std::string messageKey);

    /**
     * Set the hostname/ip.
     */
    void host(std::string host);

    /**
     * Set the host port.
     */
    void port(int port);

    /**
     * Set the topic and possibly the partition.
     *
     * The format expected of the string is *&lt;topic_name&gt;[:&lt;partition&gt;]*.
     * If omitted, a random partition will be selected.
     *
     * @pre The given topic value is not blank (empty or all spaces).
     */
    void topic(std::string topic);

    /**
     * Set the kafka compression codec to use.
     */
    void codec(std::string codec);

    /**
     * Configure an AVRO serializer instance according to the specified
     * configuration file.
     *
     * @param configFile the file path to the schema configuration and mapping
     */
    void serializer(const std::string& configFile);

    /*-- methods --*/

    /**
     * Prepare and establish the kafka client connection.
     */
    void connect();

    /**
     * Flush message queue.
     */
    void flush();

    /**
     * Send a message to kafka.
     *
     * @param message the message to be sent
     */
    void sendMessage(const std::string& message);

private:

    /*-- static fields --*/

    /**
     * Class logger.
     */
    static log4cxx::LoggerPtr logger;

    /*-- fields --*/

    /**
     * The kafka client handle.
     */
    rd_kafka_t* kafkaClient_;

    /**
     * The kafka configuration object;
     */
    rd_kafka_conf_t* kafkaConfig_;

    /**
     * Client identification.
     *
     * A user-specified string sent in each request to help trace calls. It
     * should logically identify the application making the request.
     */
    std::string clientId_;

    /**
     * Kafka messake key.
     */
    std::string messageKey_;

    /**
     * Broker hostname/ip.
     */
    std::string host_;

    /**
     * Broker port number.
     */
    int port_;

    /**
     * Topic name.
     */
    std::string topic_;

    /**
     * Topic partition.
     * (Default: -1, random selection)
     */
    int partition_;

    /**
     * Compression codec name.
     */
    std::string codec_;

    /**
     * Serializer object to use.
     */
    std::unique_ptr<Serializer> serializer_;

    /*-- static methods --*/

    /**
     * Message delivery report callback.
     * Called once for each message.
     *
     * @see rdkafka.h
     */
    static void deliverCallback(rd_kafka_t* rk, void* payload, size_t len, rd_kafka_resp_err_t error_code,
            void* opaque, void* msg_opaque);

    /*-- methods --*/

    /**
     * Initialize members with default values.
     *
     * @see Constants
     */
    void initDefaults();

    /**
     * Generate an unique correlation id for a request.
     */
    int generateCorrelationId();
};

#endif /* _LOG2KAFKA_CLIENT_FACADE_HH_ */
