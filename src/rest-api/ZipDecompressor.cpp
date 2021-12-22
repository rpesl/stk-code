#include <algorithm>
#include <cstring>
#include <fstream>
#include <list>
#include <string_view>
#include <vector>
#include <zlib.h>
#include "rest-api/ZipDecompressor.hpp"

namespace RestApi
{

namespace
{

struct File
{
    std::string filename;
    std::vector<char> data;
};

class ZipReader
{
public:
    ZipReader(
        std::string_view zip,
        uint32_t compressedSize,
        uint32_t uncompressedSize,
        uint16_t compressionMethod,
        std::string_view filename)
    : filename_(filename)
    , zip_(compressedSize)
    , compressionMethod_(compressionMethod)
    {
        stream_.zalloc = Z_NULL;
        stream_.zfree = Z_NULL;
        stream_.opaque = Z_NULL;
        stream_.avail_in = compressedSize;
        if (zip.length() < compressedSize)
            throw std::invalid_argument("Unexpected end of file");
        std::memcpy(zip_.data(), zip.data(), compressedSize);
        stream_.next_in = zip_.data();
        stream_.avail_out = uncompressedSize;
        checkError(inflateInit2(&stream_, -MAX_WBITS));
    }
    ZipReader(const ZipReader&) = delete;
    ZipReader(ZipReader&&) = delete;
    ~ZipReader() noexcept
    {
        inflateEnd(&stream_);
    }
    File read() const
    {
        if (compressionMethod_ == 0)
            return readUncompressed();
        if (compressionMethod_ == 8)
            return readCompressed();
        throw std::invalid_argument("Invalid compression type");
    }

private:
    File readUncompressed() const
    {
        std::vector<char> data(stream_.avail_in);
        std::memcpy(data.data(), zip_.data(), zip_.size());
        return {filename_, std::move(data)};
    }
    File readCompressed() const
    {
        std::vector<char> data(stream_.avail_out);
        stream_.next_out = reinterpret_cast<Bytef*>(data.data());
        checkError(inflate(&stream_, Z_NO_FLUSH));
        return {filename_, std::move(data)};
    }
    static void checkError(int code)
    {
        switch (code) {
            case Z_OK:
            case Z_STREAM_END:
                break;
            case Z_STREAM_ERROR:
                throw std::invalid_argument("zlib invalid parameters");
            case Z_DATA_ERROR:
                throw std::invalid_argument("zlib invalid or incomplete deflate data");
            case Z_MEM_ERROR:
                throw std::runtime_error("zlib out of memory");
            case Z_VERSION_ERROR:
                throw std::runtime_error("zlib version mismatch");
            default:
                throw std::runtime_error("Unknown zlib error");
        }
    }

private:
    mutable z_stream stream_;
    std::string filename_;
    std::vector<unsigned char> zip_;
    uint16_t compressionMethod_;
};

template<typename T, size_t S, size_t N>
T readValue(std::string_view view)
{
    static_assert(sizeof(T) == N);
    if (view.length() < S + N)
        throw std::invalid_argument("Header has not enough bytes");
    T value;
    std::memcpy(&value, view.substr(S, N).data(), N);
    return value;
}

class ZipArchiveReader
{
public:
    explicit ZipArchiveReader(const std::string& zip)
    {
        size_t index = zip.rfind("PK\5\6");
        if (index == std::string::npos)
            throw std::invalid_argument("Not a valid zip archive");
        std::string_view view(zip);
        parseEndOfCentralDirectoryRecord(view.substr(index), view);
    }
    [[nodiscard]]
    const std::list<ZipReader>& getFileReaders() const noexcept
    {
        return files_;
    }

private:
    void parseEndOfCentralDirectoryRecord(std::string_view record, std::string_view zip)
    {
        auto count = readValue<uint16_t, 8, 2>(record);
        auto start = readValue<uint32_t, 16, 4>(record);
        auto nextRecord = zip.substr(start);
        for (uint16_t i = 0; i < count; i++)
        {
            nextRecord = parseCentralDirectoryRecord(nextRecord, zip);
        }
    }
    std::string_view parseCentralDirectoryRecord(std::string_view record, std::string_view zip)
    {
        auto signature = readValue<uint32_t, 0, 4>(record);
        if (signature != 0x02014B50)
            throw std::invalid_argument("Invalid central directory file header");
        auto filenameLength = readValue<uint16_t, 28, 2>(record);
        auto extraLength = readValue<uint16_t, 30, 2>(record);
        auto commentLength = readValue<uint16_t, 32, 2>(record);
        auto compressedSize = readValue<uint32_t, 20, 4>(record);
        auto uncompressedSize = readValue<uint32_t, 24, 4>(record);
        auto start = readValue<uint32_t, 42, 4>(record);
        auto compressionMethod = readValue<uint16_t, 10, 2>(record);
        auto filename = record.substr(46, filenameLength);
        files_.emplace_back(
            skipLocalHeader(zip.substr(start)), compressedSize, uncompressedSize, compressionMethod, filename);
        return record.substr(46 + filenameLength + extraLength + commentLength);
    }
    static std::string_view skipLocalHeader(std::string_view record)
    {
        auto signature = readValue<uint32_t, 0, 4>(record);
        if (signature != 0x04034B50)
            throw std::invalid_argument("Invalid local file header");
        auto filenameLength = readValue<uint16_t, 26, 2>(record);
        auto extraLength = readValue<uint16_t, 28, 2>(record);
        return record.substr(30 + filenameLength + extraLength);
    }

private:
    std::list<ZipReader> files_;
};

void createDirectories(const std::filesystem::path& directory)
{
    if (!std::filesystem::exists(directory.parent_path()))
        createDirectories(directory.parent_path());
    std::filesystem::create_directory(directory);
}

void writeFile(const File& file, const std::filesystem::path& directory)
{
    auto path = directory / file.filename;
    if (path.filename().empty())
        return;
    if (!std::filesystem::exists(path.parent_path()))
        createDirectories(path.parent_path());
    std::ofstream stream(path, std::ios::binary);
    stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    stream.write(file.data.data(), static_cast<std::streamsize>(file.data.size()));
    stream.close();
}

void writeFiles(const std::vector<File>& files, const std::filesystem::path& directory)
{
    for (const auto& file : files)
    {
        writeFile(file, directory);
    }
}

std::filesystem::path getRootDirectory(const std::vector<File>& files, const std::filesystem::path& directory)
{
    std::vector<std::filesystem::path> directories;
    for (const auto& file : files)
    {
        auto path = directory / file.filename;
        if (path.filename().empty() && path.parent_path().parent_path() == directory)
            directories.emplace_back(path.parent_path());
    }
    if (directories.size() != 1)
        throw std::invalid_argument("zip archive must contain exactly one directory");
    return directories[0];
}

}

std::filesystem::path decompressZip(const std::string& zip, const std::filesystem::path& directory)
{
    ZipArchiveReader reader(zip);
    const auto& fileReaders = reader.getFileReaders();
    std::vector<File> files;
    files.reserve(fileReaders.size());
    std::transform(fileReaders.begin(), fileReaders.end(), std::back_inserter(files), [] (const auto& fileReader) { return fileReader.read(); });
    auto result = getRootDirectory(files, directory);
    if (std::filesystem::exists(result))
        throw std::invalid_argument("Directory \"" + std::string(result) + "\" already exists");
    writeFiles(files, directory);
    return result;
}

}
