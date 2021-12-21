//
// Created by Karan Shah on 12/7/21.
//

#ifndef NETWORK_MONITOR_FILE_DOWNLOADER_H
#define NETWORK_MONITOR_FILE_DOWNLOADER_H

#include <filesystem>
#include <string>

namespace NetworkMonitor {

/*! \brief Download a file from a remote HTTPS URL.
 *
 *  \param destination The full path and filename of the output file. The path
 *                     to the file must exist.
 *  \param caCertFile  The path to a cacert.pem file to perform certificate
 *                     verification in an HTTPS connection.
 */
    bool DownloadFile(
            const std::string& fileUrl,
            const std::filesystem::path& destination,
            const std::filesystem::path& caCertFile = {}
    );

} // namespace NetworkMonitor

#endif //NETWORK_MONITOR_FILE_DOWNLOADER_H
