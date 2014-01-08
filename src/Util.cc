/**
 * @file Util.cc
 * @brief Global utilities functions class.
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

#include "Util.hh"

using namespace std;
using namespace boost::filesystem;


path Util::getTempDirectoryPath() {
#   ifdef BOOST_POSIX_API
    const char* val = 0;

    (val = getenv("TMPDIR")) ||
        (val = getenv("TMP")) ||
        (val = getenv("TEMP")) ||
        (val = getenv("TEMPDIR"));

    path p((val != 0) ? val : "/tmp");

    if (p.empty() || !is_directory(p)) {
        errno = ENOTDIR;
        LOG_WARN("Invalid temporal directory: " << val);
        return p;
    }

    return p;

#   else  // Windows
    std::vector<path::value_type> buf(GetTempPathW(0, NULL));

    if (buf.empty() || GetTempPathW(buf.size(), &buf[0]) == 0) {
        if(!buf.empty()) ::SetLastError(ENOTDIR);
        // error(true, ec, "boost::filesystem::temp_directory_path");
        LOG_WARN("Invalid temporal directory: " << buf);
        return path();
    }

    buf.pop_back();

    path p(buf.begin(), buf.end());

    if (!is_directory(p)) {
        ::SetLastError(ENOTDIR);
        LOG_WARN("Invalid temporal directory: " << p.string());
        return path();
    }

    return p;
#   endif
}
