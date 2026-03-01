#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "BitStreamWriter.h"

namespace {

static void pushByteLsbFirst(BitStreamWriter<std::ostringstream>& writer, uint8_t value) {
    for (uint32_t bit = 0; bit < 8; ++bit) {
        writer.pushbit((value >> bit) & 1U);
    }
}

static std::vector<uint8_t> toBytes(const std::string& data) {
    return std::vector<uint8_t>(data.begin(), data.end());
}

}  // namespace

TEST(BitStreamWriter, FlushOnEmptyProducesNoOutput) {
    std::ostringstream out;
    BitStreamWriter<std::ostringstream> writer(out);

    writer.flush();

    EXPECT_TRUE(out.str().empty());
}

TEST(BitStreamWriter, WritesSingleByteLsbFirst) {
    std::ostringstream out;
    BitStreamWriter<std::ostringstream> writer(out);

    pushByteLsbFirst(writer, 0xA5);
    writer.flush();

    auto bytes = toBytes(out.str());
    ASSERT_EQ(bytes.size(), 1U);
    EXPECT_EQ(bytes[0], 0xA5);
}

TEST(BitStreamWriter, WritesMultipleBytesInOrder) {
    std::ostringstream out;
    BitStreamWriter<std::ostringstream> writer(out);

    pushByteLsbFirst(writer, 0xAB);
    pushByteLsbFirst(writer, 0xCD);
    writer.flush();

    auto bytes = toBytes(out.str());
    ASSERT_EQ(bytes.size(), 2U);
    EXPECT_EQ(bytes[0], 0xAB);
    EXPECT_EQ(bytes[1], 0xCD);
}

TEST(BitStreamWriter, CommitsAt64BitBoundary) {
    std::ostringstream out;
    BitStreamWriter<std::ostringstream> writer(out);

    const std::vector<uint8_t> expected = {0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE};
    for (uint8_t value : expected) {
        pushByteLsbFirst(writer, value);
    }

    auto bytes = toBytes(out.str());
    ASSERT_EQ(bytes.size(), expected.size());
    EXPECT_EQ(bytes, expected);
}

TEST(BitStreamWriter, DestructorFlushesPendingFullBytes) {
    std::ostringstream out;
    {
        BitStreamWriter<std::ostringstream> writer(out);
        pushByteLsbFirst(writer, 0x5A);
    }

    auto bytes = toBytes(out.str());
    ASSERT_EQ(bytes.size(), 1U);
    EXPECT_EQ(bytes[0], 0x5A);
}

TEST(BitStreamWriter, FlushPadsPartialByteWithZeros) {
    std::ostringstream out;
    BitStreamWriter<std::ostringstream> writer(out);

    writer.pushbit(1U);
    writer.pushbit(0U);
    writer.pushbit(1U);
    writer.flush();

    auto bytes = toBytes(out.str());
    ASSERT_EQ(bytes.size(), 1U);
    EXPECT_EQ(bytes[0], 0x05);
}
