#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>

void copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    int srcFd = open(sourcePath.c_str(), O_RDONLY);
    if (srcFd == -1) {
        std::cerr << "cant open source" << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int destFd = open(destinationPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destFd == -1) {
        std::cerr << "cant open dest" << strerror(errno) << std::endl;
        close(srcFd);
        exit(EXIT_FAILURE);
    }

    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    ssize_t bytesRead, bytesWritten;
    off_t totalBytes = 0, dataBytes = 0, holeBytes = 0;

    while ((bytesRead = read(srcFd, buffer, bufferSize)) > 0) {
        for (ssize_t i = 0; i < bytesRead; ) {
            if (buffer[i] == '\0') {
                ssize_t holeStart = i;
                while (i < bytesRead && buffer[i] == '\0') ++i;
                off_t holeSize = i - holeStart;
                if (lseek(destFd, holeSize, SEEK_CUR) == -1) {
                    std::cerr << "lseek error" << strerror(errno) << std::endl;
                    close(srcFd);
                    close(destFd);
                    exit(EXIT_FAILURE);
                }
                holeBytes += holeSize;
            } else {
                ssize_t dataStart = i;
                while (i < bytesRead && buffer[i] != '\0') ++i;
                ssize_t dataSize = i - dataStart;
                bytesWritten = write(destFd, &buffer[dataStart], dataSize);
                if (bytesWritten == -1) {
                    std::cerr << "write error" << strerror(errno) << std::endl;
                    close(srcFd);
                    close(destFd);
                    exit(EXIT_FAILURE);
                }
                dataBytes += bytesWritten;
            }
        }
        totalBytes += bytesRead;
    }

    if (bytesRead == -1) {
        std::cerr << "read error" << strerror(errno) << std::endl;
        close(srcFd);
        close(destFd);
        exit(EXIT_FAILURE);
    }

    close(srcFd);
    close(destFd);

    std::cout << "copied" << totalBytes << " bytes (data: " 
              << dataBytes << ", hole: " << holeBytes << ")." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "./copy <source_file> <destination_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string sourcePath = argv[1];
    std::string destinationPath = argv[2];
    copyFile(sourcePath, destinationPath);

    return EXIT_SUCCESS;
}