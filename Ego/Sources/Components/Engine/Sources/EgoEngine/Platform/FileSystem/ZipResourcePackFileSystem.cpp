#include "ZipResourcePackFileSystem.h"

#include "EgoCore/Hash/Crc32.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace
{
    constexpr uint32_t LocalFileHeaderSignature = 0x04034b50;
    constexpr uint32_t CentralDirectoryHeaderSignature = 0x02014b50;
    constexpr uint32_t EndOfCentralDirectorySignature = 0x06054b50;

    constexpr uint16_t ZipCompressionMethodStored = 0;
    constexpr uint16_t ZipCompressionMethodDeflated = 8;
    constexpr uint16_t ZipGeneralPurposeFlagEncrypted = 1 << 0;

    constexpr size_t LocalFileHeaderSize = 30;
    constexpr size_t CentralDirectoryHeaderSize = 46;
    constexpr size_t EndOfCentralDirectorySize = 22;
    constexpr size_t MaxZipCommentSize = 0xFFFF;

    constexpr size_t MaxHuffmanBits = 15;

    constexpr std::array<uint16_t, 29> DeflateLengthBases = {
        3, 4, 5, 6, 7, 8, 9, 10,
        11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115,
        131, 163, 195, 227, 258
    };

    constexpr std::array<uint8_t, 29> DeflateLengthExtraBits = {
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4,
        5, 5, 5, 5, 0
    };

    constexpr std::array<uint16_t, 30> DeflateDistanceBases = {
        1, 2, 3, 4, 5, 7, 9, 13,
        17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073,
        4097, 6145, 8193, 12289, 16385, 24577
    };

    constexpr std::array<uint8_t, 30> DeflateDistanceExtraBits = {
        0, 0, 0, 0, 1, 1, 2, 2,
        3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10,
        11, 11, 12, 12, 13, 13
    };

    bool CanRead(const ego::FileContent& _content, size_t _offset, size_t _size)
    {
        return _offset <= _content.size() && _size <= _content.size() - _offset;
    }

    uint16_t ReadUInt16LE(const uint8_t* _data)
    {
        return static_cast<uint16_t>(_data[0]) |
            (static_cast<uint16_t>(_data[1]) << 8);
    }

    uint32_t ReadUInt32LE(const uint8_t* _data)
    {
        return static_cast<uint32_t>(_data[0]) |
            (static_cast<uint32_t>(_data[1]) << 8) |
            (static_cast<uint32_t>(_data[2]) << 16) |
            (static_cast<uint32_t>(_data[3]) << 24);
    }

    uint16_t ReadUInt16LE(const ego::FileContent& _content, size_t _offset)
    {
        return ReadUInt16LE(_content.data() + _offset);
    }

    uint32_t ReadUInt32LE(const ego::FileContent& _content, size_t _offset)
    {
        return ReadUInt32LE(_content.data() + _offset);
    }

    uint16_t ReverseBits(uint16_t _value, uint8_t _bitCount)
    {
        uint16_t result = 0;
        for (uint8_t bitIndex = 0; bitIndex < _bitCount; ++bitIndex)
        {
            result = static_cast<uint16_t>((result << 1) | (_value & 1));
            _value = static_cast<uint16_t>(_value >> 1);
        }

        return result;
    }

    uint64_t PackDosDateTime(uint16_t _date, uint16_t _time)
    {
        return (static_cast<uint64_t>(_date) << 16) | _time;
    }

    uint32_t CalculateCrc32(const ego::FileContent& _content)
    {
        return ego::Crc32(
            _content.empty() ? "" : reinterpret_cast<const char*>(_content.data()),
            _content.size()
        );
    }

    bool FindEndOfCentralDirectory(const ego::FileContent& _content, size_t& _eocdOffset)
    {
        if (_content.size() < EndOfCentralDirectorySize)
        {
            return false;
        }

        const size_t lastOffset = _content.size() - EndOfCentralDirectorySize;
        const size_t firstOffset = _content.size() > EndOfCentralDirectorySize + MaxZipCommentSize ?
            _content.size() - EndOfCentralDirectorySize - MaxZipCommentSize :
            0;

        for (size_t offset = lastOffset;; --offset)
        {
            if (ReadUInt32LE(_content, offset) == EndOfCentralDirectorySignature)
            {
                const uint16_t commentLength = ReadUInt16LE(_content, offset + 20);
                if (offset + EndOfCentralDirectorySize + commentLength == _content.size())
                {
                    _eocdOffset = offset;
                    return true;
                }
            }

            if (offset == firstOffset)
            {
                break;
            }
        }

        return false;
    }

    class BitReader final
    {
    public:
        BitReader(const uint8_t* _data, size_t _size)
            : m_data(_data),
              m_size(_size)
        {}

        bool readBits(uint8_t _count, uint32_t& _value)
        {
            _value = 0;

            for (uint8_t bitIndex = 0; bitIndex < _count; ++bitIndex)
            {
                if (m_bitOffset >= m_size * 8)
                {
                    return false;
                }

                const size_t byteOffset = m_bitOffset / 8;
                const uint8_t bitOffset = static_cast<uint8_t>(m_bitOffset % 8);
                const uint32_t bit = (m_data[byteOffset] >> bitOffset) & 1;

                _value |= bit << bitIndex;
                ++m_bitOffset;
            }

            return true;
        }

        void alignToByte()
        {
            m_bitOffset = (m_bitOffset + 7) & ~static_cast<size_t>(7);
        }

        bool readAlignedUInt16(uint16_t& _value)
        {
            if ((m_bitOffset % 8) != 0)
            {
                return false;
            }

            const size_t byteOffset = m_bitOffset / 8;
            if (byteOffset > m_size || m_size - byteOffset < sizeof(uint16_t))
            {
                return false;
            }

            _value = ReadUInt16LE(m_data + byteOffset);
            m_bitOffset += sizeof(uint16_t) * 8;

            return true;
        }

        bool readAlignedBytes(size_t _size, ego::FileContent& _output, size_t _expectedOutputSize)
        {
            if ((m_bitOffset % 8) != 0)
            {
                return false;
            }

            const size_t byteOffset = m_bitOffset / 8;
            if (byteOffset > m_size || _size > m_size - byteOffset)
            {
                return false;
            }

            if (_output.size() > _expectedOutputSize || _size > _expectedOutputSize - _output.size())
            {
                return false;
            }

            _output.insert(_output.end(), m_data + byteOffset, m_data + byteOffset + _size);
            m_bitOffset += _size * 8;

            return true;
        }

    private:
        const uint8_t* m_data = nullptr;
        size_t m_size = 0;
        size_t m_bitOffset = 0;
    };

    class HuffmanTable final
    {
    public:
        bool build(const std::vector<uint8_t>& _codeLengths)
        {
            m_codesByLength = {};
            m_maxLength = 0;

            std::array<uint16_t, MaxHuffmanBits + 1> codeLengthCounts{};
            for (uint8_t codeLength : _codeLengths)
            {
                if (codeLength > MaxHuffmanBits)
                {
                    return false;
                }

                if (codeLength > 0)
                {
                    ++codeLengthCounts[codeLength];
                    m_maxLength = std::max(m_maxLength, codeLength);
                }
            }

            std::array<uint16_t, MaxHuffmanBits + 1> nextCodes{};
            uint16_t code = 0;
            for (size_t bits = 1; bits <= MaxHuffmanBits; ++bits)
            {
                code = static_cast<uint16_t>((code + codeLengthCounts[bits - 1]) << 1);
                nextCodes[bits] = code;
            }

            for (uint16_t symbol = 0; symbol < _codeLengths.size(); ++symbol)
            {
                const uint8_t codeLength = _codeLengths[symbol];
                if (codeLength == 0)
                {
                    continue;
                }

                const uint16_t canonicalCode = nextCodes[codeLength]++;
                const uint16_t reversedCode = ReverseBits(canonicalCode, codeLength);
                m_codesByLength[codeLength].push_back(HuffmanCode{reversedCode, symbol});
            }

            for (std::vector<HuffmanCode>& codes : m_codesByLength)
            {
                std::sort(
                    codes.begin(),
                    codes.end(),
                    [](const HuffmanCode& _left, const HuffmanCode& _right)
                    {
                        return _left.m_code < _right.m_code;
                    }
                );
            }

            return true;
        }

        bool decode(BitReader& _reader, uint16_t& _symbol) const
        {
            if (m_maxLength == 0)
            {
                return false;
            }

            uint32_t code = 0;
            for (uint8_t codeLength = 1; codeLength <= m_maxLength; ++codeLength)
            {
                uint32_t bit = 0;
                if (!_reader.readBits(1, bit))
                {
                    return false;
                }

                code |= bit << (codeLength - 1);

                const std::vector<HuffmanCode>& codes = m_codesByLength[codeLength];
                const auto codeIter = std::lower_bound(
                    codes.begin(),
                    codes.end(),
                    code,
                    [](const HuffmanCode& _left, uint32_t _code)
                    {
                        return _left.m_code < _code;
                    }
                );

                if (codeIter != codes.end() && codeIter->m_code == code)
                {
                    _symbol = codeIter->m_symbol;
                    return true;
                }
            }

            return false;
        }

    private:
        struct HuffmanCode final
        {
            uint16_t m_code = 0;
            uint16_t m_symbol = 0;
        };

        std::array<std::vector<HuffmanCode>, MaxHuffmanBits + 1> m_codesByLength;
        uint8_t m_maxLength = 0;
    };

    bool BuildFixedHuffmanTables(HuffmanTable& _literalLengthTable, HuffmanTable& _distanceTable)
    {
        std::vector<uint8_t> literalLengthCodeLengths(288, 0);
        for (size_t symbol = 0; symbol <= 143; ++symbol)
        {
            literalLengthCodeLengths[symbol] = 8;
        }
        for (size_t symbol = 144; symbol <= 255; ++symbol)
        {
            literalLengthCodeLengths[symbol] = 9;
        }
        for (size_t symbol = 256; symbol <= 279; ++symbol)
        {
            literalLengthCodeLengths[symbol] = 7;
        }
        for (size_t symbol = 280; symbol <= 287; ++symbol)
        {
            literalLengthCodeLengths[symbol] = 8;
        }

        std::vector<uint8_t> distanceCodeLengths(32, 5);

        return _literalLengthTable.build(literalLengthCodeLengths) &&
            _distanceTable.build(distanceCodeLengths);
    }

    bool ReadDynamicCodeLengths(
        BitReader& _reader,
        std::vector<uint8_t>& _literalLengthCodeLengths,
        std::vector<uint8_t>& _distanceCodeLengths
    )
    {
        uint32_t literalLengthCodeCountValue = 0;
        uint32_t distanceCodeCountValue = 0;
        uint32_t codeLengthCodeCountValue = 0;

        if (!_reader.readBits(5, literalLengthCodeCountValue) ||
            !_reader.readBits(5, distanceCodeCountValue) ||
            !_reader.readBits(4, codeLengthCodeCountValue))
        {
            return false;
        }

        const size_t literalLengthCodeCount = literalLengthCodeCountValue + 257;
        const size_t distanceCodeCount = distanceCodeCountValue + 1;
        const size_t codeLengthCodeCount = codeLengthCodeCountValue + 4;

        constexpr std::array<uint8_t, 19> CodeLengthCodeOrder = {
            16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
            11, 4, 12, 3, 13, 2, 14, 1, 15
        };

        std::vector<uint8_t> codeLengthCodeLengths(19, 0);
        for (size_t index = 0; index < codeLengthCodeCount; ++index)
        {
            uint32_t codeLength = 0;
            if (!_reader.readBits(3, codeLength))
            {
                return false;
            }

            codeLengthCodeLengths[CodeLengthCodeOrder[index]] = static_cast<uint8_t>(codeLength);
        }

        HuffmanTable codeLengthTable;
        if (!codeLengthTable.build(codeLengthCodeLengths))
        {
            return false;
        }

        std::vector<uint8_t> codeLengths(literalLengthCodeCount + distanceCodeCount, 0);
        uint8_t previousCodeLength = 0;
        size_t codeLengthIndex = 0;
        while (codeLengthIndex < codeLengths.size())
        {
            uint16_t symbol = 0;
            if (!codeLengthTable.decode(_reader, symbol))
            {
                return false;
            }

            if (symbol <= 15)
            {
                previousCodeLength = static_cast<uint8_t>(symbol);
                codeLengths[codeLengthIndex++] = previousCodeLength;
                continue;
            }

            uint32_t repeatExtraValue = 0;
            uint8_t repeatedCodeLength = 0;
            size_t repeatCount = 0;

            if (symbol == 16)
            {
                if (codeLengthIndex == 0 || !_reader.readBits(2, repeatExtraValue))
                {
                    return false;
                }

                repeatedCodeLength = previousCodeLength;
                repeatCount = repeatExtraValue + 3;
            }
            else if (symbol == 17)
            {
                if (!_reader.readBits(3, repeatExtraValue))
                {
                    return false;
                }

                repeatedCodeLength = 0;
                previousCodeLength = 0;
                repeatCount = repeatExtraValue + 3;
            }
            else if (symbol == 18)
            {
                if (!_reader.readBits(7, repeatExtraValue))
                {
                    return false;
                }

                repeatedCodeLength = 0;
                previousCodeLength = 0;
                repeatCount = repeatExtraValue + 11;
            }
            else
            {
                return false;
            }

            if (repeatCount > codeLengths.size() - codeLengthIndex)
            {
                return false;
            }

            std::fill(
                codeLengths.begin() + codeLengthIndex,
                codeLengths.begin() + codeLengthIndex + repeatCount,
                repeatedCodeLength
            );
            codeLengthIndex += repeatCount;
        }

        _literalLengthCodeLengths.assign(
            codeLengths.begin(),
            codeLengths.begin() + literalLengthCodeCount
        );
        _distanceCodeLengths.assign(
            codeLengths.begin() + literalLengthCodeCount,
            codeLengths.end()
        );

        return true;
    }

    bool BuildDynamicHuffmanTables(BitReader& _reader, HuffmanTable& _literalLengthTable, HuffmanTable& _distanceTable)
    {
        std::vector<uint8_t> literalLengthCodeLengths;
        std::vector<uint8_t> distanceCodeLengths;
        if (!ReadDynamicCodeLengths(_reader, literalLengthCodeLengths, distanceCodeLengths))
        {
            return false;
        }

        return _literalLengthTable.build(literalLengthCodeLengths) &&
            _distanceTable.build(distanceCodeLengths);
    }

    bool DecodeCompressedHuffmanBlock(
        BitReader& _reader,
        const HuffmanTable& _literalLengthTable,
        const HuffmanTable& _distanceTable,
        size_t _expectedOutputSize,
        ego::FileContent& _output
    )
    {
        while (true)
        {
            uint16_t literalLengthSymbol = 0;
            if (!_literalLengthTable.decode(_reader, literalLengthSymbol))
            {
                return false;
            }

            if (literalLengthSymbol <= 255)
            {
                if (_output.size() >= _expectedOutputSize)
                {
                    return false;
                }

                _output.push_back(static_cast<uint8_t>(literalLengthSymbol));
                continue;
            }

            if (literalLengthSymbol == 256)
            {
                return true;
            }

            if (literalLengthSymbol < 257 || literalLengthSymbol > 285)
            {
                return false;
            }

            const size_t lengthIndex = literalLengthSymbol - 257;
            uint32_t lengthExtraValue = 0;
            if (DeflateLengthExtraBits[lengthIndex] > 0 &&
                !_reader.readBits(DeflateLengthExtraBits[lengthIndex], lengthExtraValue))
            {
                return false;
            }

            const size_t copyLength = DeflateLengthBases[lengthIndex] + lengthExtraValue;

            uint16_t distanceSymbol = 0;
            if (!_distanceTable.decode(_reader, distanceSymbol) || distanceSymbol >= DeflateDistanceBases.size())
            {
                return false;
            }

            uint32_t distanceExtraValue = 0;
            if (DeflateDistanceExtraBits[distanceSymbol] > 0 &&
                !_reader.readBits(DeflateDistanceExtraBits[distanceSymbol], distanceExtraValue))
            {
                return false;
            }

            const size_t copyDistance = DeflateDistanceBases[distanceSymbol] + distanceExtraValue;
            if (copyDistance == 0 || copyDistance > _output.size())
            {
                return false;
            }

            if (_output.size() > _expectedOutputSize || copyLength > _expectedOutputSize - _output.size())
            {
                return false;
            }

            for (size_t index = 0; index < copyLength; ++index)
            {
                _output.push_back(_output[_output.size() - copyDistance]);
            }
        }
    }

    bool InflateRawDeflate(
        const uint8_t* _compressedData,
        size_t _compressedSize,
        size_t _expectedOutputSize,
        ego::FileContent& _output
    )
    {
        BitReader reader(_compressedData, _compressedSize);

        _output.clear();
        _output.reserve(_expectedOutputSize);

        bool isFinalBlock = false;
        while (!isFinalBlock)
        {
            uint32_t finalBlockValue = 0;
            uint32_t blockType = 0;
            if (!reader.readBits(1, finalBlockValue) || !reader.readBits(2, blockType))
            {
                _output.clear();
                return false;
            }

            isFinalBlock = finalBlockValue != 0;

            if (blockType == 0)
            {
                reader.alignToByte();

                uint16_t storedBlockSize = 0;
                uint16_t storedBlockSizeCheck = 0;
                if (!reader.readAlignedUInt16(storedBlockSize) ||
                    !reader.readAlignedUInt16(storedBlockSizeCheck) ||
                    storedBlockSize != static_cast<uint16_t>(~storedBlockSizeCheck))
                {
                    _output.clear();
                    return false;
                }

                if (!reader.readAlignedBytes(storedBlockSize, _output, _expectedOutputSize))
                {
                    _output.clear();
                    return false;
                }

                continue;
            }

            HuffmanTable literalLengthTable;
            HuffmanTable distanceTable;
            if (blockType == 1)
            {
                if (!BuildFixedHuffmanTables(literalLengthTable, distanceTable))
                {
                    _output.clear();
                    return false;
                }
            }
            else if (blockType == 2)
            {
                if (!BuildDynamicHuffmanTables(reader, literalLengthTable, distanceTable))
                {
                    _output.clear();
                    return false;
                }
            }
            else
            {
                _output.clear();
                return false;
            }

            if (!DecodeCompressedHuffmanBlock(reader, literalLengthTable, distanceTable, _expectedOutputSize, _output))
            {
                _output.clear();
                return false;
            }
        }

        if (_output.size() != _expectedOutputSize)
        {
            _output.clear();
            return false;
        }

        return true;
    }
}

ego::ZipResourcePackFileSystem::ZipResourcePackFileSystem(
    const FileSystem& _sourceFileSystem,
    const FileName& _packPath
)
{
    open(_sourceFileSystem, _packPath);
}

ego::ZipResourcePackFileSystem::~ZipResourcePackFileSystem()
{
    release();
}

bool ego::ZipResourcePackFileSystem::open(const FileSystem& _sourceFileSystem, const FileName& _packPath)
{
    FileContent content;
    if (!_sourceFileSystem.readFile(_packPath, content))
    {
        clear();
        return false;
    }

    return loadArchive(std::move(content), _packPath);
}

bool ego::ZipResourcePackFileSystem::open(const FileContent& _content, const FileName& _packPath)
{
    FileContent content = _content;
    return loadArchive(std::move(content), _packPath);
}

bool ego::ZipResourcePackFileSystem::isOpen() const
{
    return m_isOpen;
}

const ego::FileName& ego::ZipResourcePackFileSystem::getPackPath() const
{
    return m_packPath;
}

bool ego::ZipResourcePackFileSystem::init()
{
    m_isInitialized = true;
    return true;
}

void ego::ZipResourcePackFileSystem::release()
{
    clear();
}

bool ego::ZipResourcePackFileSystem::exists(const FileName& _path) const
{
    if (!m_isOpen)
    {
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath))
    {
        return false;
    }

    return normalizedPath.empty() || findEntry(normalizedPath);
}

bool ego::ZipResourcePackFileSystem::isFile(const FileName& _path) const
{
    if (!m_isOpen)
    {
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath) || normalizedPath.empty())
    {
        return false;
    }

    const Entry* entry = findEntry(normalizedPath);
    return entry && entry->m_type == FileSystemEntryType::File;
}

bool ego::ZipResourcePackFileSystem::isDirectory(const FileName& _path) const
{
    if (!m_isOpen)
    {
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath))
    {
        return false;
    }

    if (normalizedPath.empty())
    {
        return true;
    }

    const Entry* entry = findEntry(normalizedPath);
    return entry && entry->m_type == FileSystemEntryType::Directory;
}

ego::FileName ego::ZipResourcePackFileSystem::getWorkingDirectory() const
{
    return ToFileName(m_workingDirectory);
}

bool ego::ZipResourcePackFileSystem::setWorkingDirectory(const FileName& _path)
{
    if (!m_isOpen)
    {
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath))
    {
        return false;
    }

    if (!normalizedPath.empty())
    {
        const Entry* entry = findEntry(normalizedPath);
        if (!entry || entry->m_type != FileSystemEntryType::Directory)
        {
            return false;
        }
    }

    m_workingDirectory = std::move(normalizedPath);
    return true;
}

ego::FileName ego::ZipResourcePackFileSystem::getAbsolutePath(const FileName& _path) const
{
    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath))
    {
        return FileName();
    }

    return ToFileName(normalizedPath);
}

bool ego::ZipResourcePackFileSystem::getEntryInfo(const FileName& _path, FileSystemEntryDesc& _entry) const
{
    if (!m_isOpen)
    {
        _entry = FileSystemEntryDesc();
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath))
    {
        _entry = FileSystemEntryDesc();
        return false;
    }

    if (normalizedPath.empty())
    {
        _entry = FileSystemEntryDesc();
        _entry.m_type = FileSystemEntryType::Directory;
        _entry.m_isReadOnly = true;
        return true;
    }

    const Entry* entry = findEntry(normalizedPath);
    if (!entry)
    {
        _entry = FileSystemEntryDesc();
        return false;
    }

    fillEntryDesc(*entry, _entry);
    return true;
}

bool ego::ZipResourcePackFileSystem::enumerate(
    const FileName& _directoryPath,
    FileSystemEntryCollection& _entries,
    bool _recursive
) const
{
    _entries.clear();

    if (!m_isOpen)
    {
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_directoryPath, normalizedPath))
    {
        return false;
    }

    if (!normalizedPath.empty())
    {
        const Entry* directoryEntry = findEntry(normalizedPath);
        if (!directoryEntry || directoryEntry->m_type != FileSystemEntryType::Directory)
        {
            return false;
        }
    }

    const std::string prefix = normalizedPath.empty() ? std::string() : normalizedPath + "/";
    for (const Entry& entry : m_entries)
    {
        if (entry.m_normalizedPath == normalizedPath)
        {
            continue;
        }

        std::string relativePath;
        if (prefix.empty())
        {
            relativePath = entry.m_normalizedPath;
        }
        else
        {
            if (entry.m_normalizedPath.rfind(prefix, 0) != 0)
            {
                continue;
            }

            relativePath = entry.m_normalizedPath.substr(prefix.size());
        }

        if (relativePath.empty())
        {
            continue;
        }

        if (!_recursive && relativePath.find('/') != std::string::npos)
        {
            continue;
        }

        FileSystemEntryDesc desc;
        fillEntryDesc(entry, desc);
        _entries.push_back(desc);
    }

    return true;
}

bool ego::ZipResourcePackFileSystem::createDirectory(const FileName&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::removeFile(const FileName&)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::removeDirectory(const FileName&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::remove(const FileName&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::copyFile(const FileName&, const FileName&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::move(const FileName&, const FileName&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::readFile(const FileName& _path, FileContent& _content) const
{
    _content.clear();

    if (!m_isOpen)
    {
        return false;
    }

    std::string normalizedPath;
    if (!resolvePath(_path, normalizedPath) || normalizedPath.empty())
    {
        return false;
    }

    const Entry* entry = findEntry(normalizedPath);
    if (!entry || entry->m_type != FileSystemEntryType::File)
    {
        return false;
    }

    if ((entry->m_generalPurposeFlags & ZipGeneralPurposeFlagEncrypted) != 0)
    {
        return false;
    }

    if (entry->m_uncompressedSize > std::numeric_limits<size_t>::max() ||
        entry->m_compressedSize > std::numeric_limits<size_t>::max())
    {
        return false;
    }

    const uint8_t* compressedData = nullptr;
    size_t compressedSize = 0;
    if (!getEntryData(*entry, compressedData, compressedSize))
    {
        return false;
    }

    const size_t uncompressedSize = static_cast<size_t>(entry->m_uncompressedSize);
    if (entry->m_compressionMethod == ZipCompressionMethodStored)
    {
        if (compressedSize != uncompressedSize)
        {
            return false;
        }

        _content.assign(compressedData, compressedData + compressedSize);
    }
    else if (entry->m_compressionMethod == ZipCompressionMethodDeflated)
    {
        if (!InflateRawDeflate(compressedData, compressedSize, uncompressedSize, _content))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (CalculateCrc32(_content) != entry->m_crc32)
    {
        _content.clear();
        return false;
    }

    return true;
}

bool ego::ZipResourcePackFileSystem::readTextFile(const FileName& _path, std::string& _content) const
{
    FileContent content;
    if (!readFile(_path, content))
    {
        _content.clear();
        return false;
    }

    if (content.empty())
    {
        _content.clear();
        return true;
    }

    _content.assign(reinterpret_cast<const char*>(content.data()), content.size());
    return true;
}

bool ego::ZipResourcePackFileSystem::writeFile(const FileName&, const FileContent&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::writeTextFile(const FileName&, const std::string&, bool)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::appendFile(const FileName&, const FileContent&)
{
    return false;
}

bool ego::ZipResourcePackFileSystem::appendTextFile(const FileName&, const std::string&)
{
    return false;
}

ego::FileSystemWatchID ego::ZipResourcePackFileSystem::watchDirectory(const FileName&, FileSystemWatchFilter, bool)
{
    return InvalidFileSystemWatchID;
}

bool ego::ZipResourcePackFileSystem::unwatchDirectory(FileSystemWatchID)
{
    return false;
}

void ego::ZipResourcePackFileSystem::updateDirectoryWatches()
{}

bool ego::ZipResourcePackFileSystem::IsPathSeparator(char _ch)
{
    return _ch == '/' || _ch == '\\';
}

bool ego::ZipResourcePackFileSystem::IsDirectoryName(const std::string& _path)
{
    return !_path.empty() && IsPathSeparator(_path.back());
}

std::string ego::ZipResourcePackFileSystem::GetParentPath(const std::string& _path)
{
    const size_t separatorPosition = _path.find_last_of('/');
    return separatorPosition == std::string::npos ? std::string() : _path.substr(0, separatorPosition);
}

bool ego::ZipResourcePackFileSystem::NormalizeArchivePath(
    const std::string& _path,
    std::string& _normalizedPath
)
{
    _normalizedPath.clear();

    if (!_path.empty() && IsPathSeparator(_path.front()))
    {
        return false;
    }

    std::vector<std::string> parts;
    size_t position = 0;
    while (position < _path.size())
    {
        while (position < _path.size() && IsPathSeparator(_path[position]))
        {
            ++position;
        }

        const size_t partStart = position;
        while (position < _path.size() && !IsPathSeparator(_path[position]))
        {
            ++position;
        }

        if (partStart == position)
        {
            continue;
        }

        const std::string part = _path.substr(partStart, position - partStart);
        if (part == ".")
        {
            continue;
        }

        if (part == "..")
        {
            return false;
        }

        parts.push_back(part);
    }

    for (size_t index = 0; index < parts.size(); ++index)
    {
        if (index > 0)
        {
            _normalizedPath += '/';
        }

        _normalizedPath += parts[index];
    }

    return true;
}

ego::FileName ego::ZipResourcePackFileSystem::ToFileName(const std::string& _path)
{
    return FileName(_path);
}

void ego::ZipResourcePackFileSystem::clear()
{
    m_archiveData.clear();
    m_entries.clear();
    m_entryIndices.clear();
    m_packPath.clear();
    m_workingDirectory.clear();
    m_isInitialized = false;
    m_isOpen = false;
}

bool ego::ZipResourcePackFileSystem::loadArchive(FileContent&& _content, const FileName& _packPath)
{
    clear();

    size_t eocdOffset = 0;
    if (!FindEndOfCentralDirectory(_content, eocdOffset))
    {
        return false;
    }

    const uint16_t diskNumber = ReadUInt16LE(_content, eocdOffset + 4);
    const uint16_t centralDirectoryDiskNumber = ReadUInt16LE(_content, eocdOffset + 6);
    const uint16_t diskEntryCount = ReadUInt16LE(_content, eocdOffset + 8);
    const uint16_t totalEntryCount = ReadUInt16LE(_content, eocdOffset + 10);
    const uint32_t centralDirectorySize = ReadUInt32LE(_content, eocdOffset + 12);
    const uint32_t centralDirectoryOffset = ReadUInt32LE(_content, eocdOffset + 16);

    if (diskNumber != 0 || centralDirectoryDiskNumber != 0 || diskEntryCount != totalEntryCount)
    {
        return false;
    }

    if (diskEntryCount == 0xFFFF ||
        centralDirectorySize == std::numeric_limits<uint32_t>::max() ||
        centralDirectoryOffset == std::numeric_limits<uint32_t>::max())
    {
        return false;
    }

    if (centralDirectoryOffset > eocdOffset || centralDirectorySize > eocdOffset - centralDirectoryOffset)
    {
        return false;
    }

    m_archiveData = std::move(_content);
    m_packPath = _packPath;

    size_t currentOffset = centralDirectoryOffset;
    for (uint16_t entryIndex = 0; entryIndex < totalEntryCount; ++entryIndex)
    {
        if (!CanRead(m_archiveData, currentOffset, CentralDirectoryHeaderSize) ||
            ReadUInt32LE(m_archiveData, currentOffset) != CentralDirectoryHeaderSignature)
        {
            clear();
            return false;
        }

        const uint16_t generalPurposeFlags = ReadUInt16LE(m_archiveData, currentOffset + 8);
        const uint16_t compressionMethod = ReadUInt16LE(m_archiveData, currentOffset + 10);
        const uint16_t lastWriteTime = ReadUInt16LE(m_archiveData, currentOffset + 12);
        const uint16_t lastWriteDate = ReadUInt16LE(m_archiveData, currentOffset + 14);
        const uint32_t crc32 = ReadUInt32LE(m_archiveData, currentOffset + 16);
        const uint32_t compressedSize = ReadUInt32LE(m_archiveData, currentOffset + 20);
        const uint32_t uncompressedSize = ReadUInt32LE(m_archiveData, currentOffset + 24);
        const uint16_t fileNameLength = ReadUInt16LE(m_archiveData, currentOffset + 28);
        const uint16_t extraFieldLength = ReadUInt16LE(m_archiveData, currentOffset + 30);
        const uint16_t fileCommentLength = ReadUInt16LE(m_archiveData, currentOffset + 32);
        const uint16_t fileDiskNumber = ReadUInt16LE(m_archiveData, currentOffset + 34);
        const uint32_t externalAttributes = ReadUInt32LE(m_archiveData, currentOffset + 38);
        const uint32_t localHeaderOffset = ReadUInt32LE(m_archiveData, currentOffset + 42);

        const size_t variableSize =
            static_cast<size_t>(fileNameLength) + extraFieldLength + fileCommentLength;
        if (!CanRead(m_archiveData, currentOffset + CentralDirectoryHeaderSize, variableSize) ||
            fileDiskNumber != 0 ||
            compressedSize == std::numeric_limits<uint32_t>::max() ||
            uncompressedSize == std::numeric_limits<uint32_t>::max() ||
            localHeaderOffset == std::numeric_limits<uint32_t>::max())
        {
            clear();
            return false;
        }

        std::string archivePath(
            reinterpret_cast<const char*>(m_archiveData.data() + currentOffset + CentralDirectoryHeaderSize),
            fileNameLength
        );

        std::string normalizedPath;
        if (!NormalizeArchivePath(archivePath, normalizedPath))
        {
            clear();
            return false;
        }

        const uint32_t unixMode = externalAttributes >> 16;
        const bool isDosDirectory = (externalAttributes & 0x10) != 0;
        const bool isUnixDirectory = (unixMode & 0170000) == 0040000;
        const bool isDirectory = IsDirectoryName(archivePath) || isDosDirectory || isUnixDirectory;

        if (!normalizedPath.empty())
        {
            if (!ensureDirectory(GetParentPath(normalizedPath)))
            {
                clear();
                return false;
            }

            Entry entry;
            entry.m_path = ToFileName(normalizedPath);
            entry.m_normalizedPath = normalizedPath;
            entry.m_type = isDirectory ? FileSystemEntryType::Directory : FileSystemEntryType::File;
            entry.m_generalPurposeFlags = generalPurposeFlags;
            entry.m_compressionMethod = compressionMethod;
            entry.m_crc32 = crc32;
            entry.m_compressedSize = isDirectory ? 0 : compressedSize;
            entry.m_uncompressedSize = isDirectory ? 0 : uncompressedSize;
            entry.m_localHeaderOffset = localHeaderOffset;
            entry.m_creationTime = PackDosDateTime(lastWriteDate, lastWriteTime);
            entry.m_lastAccessTime = entry.m_creationTime;
            entry.m_lastWriteTime = entry.m_creationTime;
            entry.m_isReadOnly = true;
            entry.m_isHidden = (externalAttributes & 0x02) != 0;

            if (!addEntry(std::move(entry)))
            {
                clear();
                return false;
            }
        }

        currentOffset += CentralDirectoryHeaderSize + variableSize;
    }

    if (currentOffset != centralDirectoryOffset + centralDirectorySize)
    {
        clear();
        return false;
    }

    m_isInitialized = true;
    m_isOpen = true;
    return true;
}

bool ego::ZipResourcePackFileSystem::addEntry(Entry&& _entry)
{
    const auto entryIter = m_entryIndices.find(_entry.m_normalizedPath);
    if (entryIter != m_entryIndices.end())
    {
        Entry& existingEntry = m_entries[entryIter->second];
        if (existingEntry.m_type != _entry.m_type)
        {
            return false;
        }

        if (existingEntry.m_isSynthetic && _entry.m_type == FileSystemEntryType::Directory)
        {
            existingEntry = std::move(_entry);
        }

        return true;
    }

    m_entryIndices.emplace(_entry.m_normalizedPath, m_entries.size());
    m_entries.push_back(std::move(_entry));

    return true;
}

bool ego::ZipResourcePackFileSystem::ensureDirectory(const std::string& _path)
{
    if (_path.empty())
    {
        return true;
    }

    const Entry* existingEntry = findEntry(_path);
    if (existingEntry)
    {
        return existingEntry->m_type == FileSystemEntryType::Directory;
    }

    if (!ensureDirectory(GetParentPath(_path)))
    {
        return false;
    }

    Entry entry;
    entry.m_path = ToFileName(_path);
    entry.m_normalizedPath = _path;
    entry.m_type = FileSystemEntryType::Directory;
    entry.m_isReadOnly = true;
    entry.m_isSynthetic = true;

    return addEntry(std::move(entry));
}

bool ego::ZipResourcePackFileSystem::resolvePath(const FileName& _path, std::string& _normalizedPath) const
{
    _normalizedPath.clear();

    const std::string path(_path.c_str());
    const bool isAbsolute = !path.empty() && IsPathSeparator(path.front());

    std::vector<std::string> parts;
    if (!isAbsolute && !m_workingDirectory.empty())
    {
        size_t position = 0;
        while (position < m_workingDirectory.size())
        {
            const size_t separatorPosition = m_workingDirectory.find('/', position);
            const size_t partEnd = separatorPosition == std::string::npos ?
                m_workingDirectory.size() :
                separatorPosition;

            parts.push_back(m_workingDirectory.substr(position, partEnd - position));

            if (separatorPosition == std::string::npos)
            {
                break;
            }

            position = separatorPosition + 1;
        }
    }

    size_t position = 0;
    while (position < path.size())
    {
        while (position < path.size() && IsPathSeparator(path[position]))
        {
            ++position;
        }

        const size_t partStart = position;
        while (position < path.size() && !IsPathSeparator(path[position]))
        {
            ++position;
        }

        if (partStart == position)
        {
            continue;
        }

        const std::string part = path.substr(partStart, position - partStart);
        if (part == ".")
        {
            continue;
        }

        if (part == "..")
        {
            if (parts.empty())
            {
                return false;
            }

            parts.pop_back();
            continue;
        }

        parts.push_back(part);
    }

    for (size_t index = 0; index < parts.size(); ++index)
    {
        if (index > 0)
        {
            _normalizedPath += '/';
        }

        _normalizedPath += parts[index];
    }

    return true;
}

const ego::ZipResourcePackFileSystem::Entry* ego::ZipResourcePackFileSystem::findEntry(
    const std::string& _path
) const
{
    const auto entryIter = m_entryIndices.find(_path);
    return entryIter == m_entryIndices.end() ? nullptr : &m_entries[entryIter->second];
}

ego::ZipResourcePackFileSystem::Entry* ego::ZipResourcePackFileSystem::findEntry(const std::string& _path)
{
    const auto entryIter = m_entryIndices.find(_path);
    return entryIter == m_entryIndices.end() ? nullptr : &m_entries[entryIter->second];
}

void ego::ZipResourcePackFileSystem::fillEntryDesc(const Entry& _entry, FileSystemEntryDesc& _desc) const
{
    _desc.m_path = _entry.m_path;
    _desc.m_type = _entry.m_type;
    _desc.m_size = _entry.m_type == FileSystemEntryType::File ? _entry.m_uncompressedSize : 0;
    _desc.m_creationTime = _entry.m_creationTime;
    _desc.m_lastAccessTime = _entry.m_lastAccessTime;
    _desc.m_lastWriteTime = _entry.m_lastWriteTime;
    _desc.m_isReadOnly = _entry.m_isReadOnly;
    _desc.m_isHidden = _entry.m_isHidden;
}

bool ego::ZipResourcePackFileSystem::getEntryData(
    const Entry& _entry,
    const uint8_t*& _data,
    size_t& _size
) const
{
    _data = nullptr;
    _size = 0;

    if (_entry.m_localHeaderOffset > std::numeric_limits<size_t>::max())
    {
        return false;
    }

    const size_t localHeaderOffset = static_cast<size_t>(_entry.m_localHeaderOffset);
    if (!CanRead(m_archiveData, localHeaderOffset, LocalFileHeaderSize) ||
        ReadUInt32LE(m_archiveData, localHeaderOffset) != LocalFileHeaderSignature)
    {
        return false;
    }

    const uint16_t fileNameLength = ReadUInt16LE(m_archiveData, localHeaderOffset + 26);
    const uint16_t extraFieldLength = ReadUInt16LE(m_archiveData, localHeaderOffset + 28);
    const size_t dataOffset =
        localHeaderOffset + LocalFileHeaderSize + static_cast<size_t>(fileNameLength) + extraFieldLength;

    if (_entry.m_compressedSize > std::numeric_limits<size_t>::max())
    {
        return false;
    }

    const size_t compressedSize = static_cast<size_t>(_entry.m_compressedSize);
    if (!CanRead(m_archiveData, dataOffset, compressedSize))
    {
        return false;
    }

    _data = m_archiveData.data() + dataOffset;
    _size = compressedSize;

    return true;
}
