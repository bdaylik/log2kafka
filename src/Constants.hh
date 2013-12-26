/**
 * @file Constants.hh
 * @brief Global constants class header.
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

#pragma once

#ifndef _LOG2KAFKA_CONSTANTS_HH_
#define _LOG2KAFKA_CONSTANTS_HH_

#include <string>

/**
 * Global constants class.
 */
class Constants {
public:
    static bool LOG_VERBOSITY;
    static bool IS_TRACE_ENABLED;
    static bool IS_DEBUG_ENABLED;

    /**
     * Default Client ID value: Log2Kafka Producer
     */
    static const std::string DEFAULT_CLIENT_ID;

    /**
     * Default schema path: /etc/log2kafka
     *
     * Used if a relative value is given through the corresponding command
     * line argument.
     */
    static const std::string DEFAULT_SCHEMA_PATH;

    /**
     * Default message key: L2K
     */
    static const std::string DEFAULT_MESSAGE_KEY;

    /**
     * Default minimum amount of time that a kafka event callback will block
     * waiting for events: 1000 ms
     */
    static const int DEFAULT_CALLBACK_WAITING_TIMEOUT;

    /**
     * Default acknowledgement timeout: 2000 ms
     */
    static const int DEFAULT_TIMEOUT_ACKS;
};

#endif /* _LOG2KAFKA_CONSTANTS_HH_ */
