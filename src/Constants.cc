/**
 * @file Constants.cc
 * @brief Global constants class.
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

#include "Constants.hh"

using namespace std;

bool Constants::LOG_VERBOSITY = false;
bool Constants::IS_TRACE_ENABLED = false;
bool Constants::IS_DEBUG_ENABLED = false;

const string Constants::DEFAULT_CLIENT_ID = "Log2Kafka Producer";
const string Constants::DEFAULT_CONFIG_PATH = "/etc/log2kafka/";
const int Constants::DEFAULT_CALLBACK_WAITING_TIMEOUT = 1000;
const string Constants::KAFKA_CLIENT_OPTION_PREFIX = "kafka.";
const string Constants::KAFKA_TOPIC_OPTION_PREFIX = "kafka_topic.";
