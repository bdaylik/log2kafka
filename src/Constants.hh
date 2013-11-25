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

#ifndef _LOG2KAFKA_CONSTANTS_HH_
#define _LOG2KAFKA_CONSTANTS_HH_

#include <string>

class Constants {
public:
    static const std::string DEFAULT_CLIENT_ID;
    static const std::string DEFAULT_HOST;
    static const std::size_t DEFAULT_PORT;
    static const std::string DEFAULT_TOPIC;
    static const std::string DEFAULT_SCHEMA;
    static const std::string DEFAULT_SCHEMA_PATH;
};

#endif /* _LOG2KAFKA_CONSTANTS_HH_ */
