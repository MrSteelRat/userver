#include <gtest/gtest.h>

#include <fs/blocking/file_descriptor.hpp>
#include <fs/blocking/temp_directory.hpp>

using FileDescriptor = fs::blocking::FileDescriptor;

namespace {

std::string ReadContents(FileDescriptor& fd) {
  constexpr std::size_t kBlockSize = 4096;
  std::string result;
  std::size_t size = 0;

  while (true) {
    result.resize(size + kBlockSize);
    const auto s = fd.Read(result.data() + size, kBlockSize);
    if (s == 0) break;
    size += s;
  }

  result.resize(size);
  return result;
}

}  // namespace

TEST(FileDescriptor, OpenExisting) {
  const auto dir = fs::blocking::TempDirectory::Create();
  const auto path = dir.GetPath() + "/foo";

  auto fd =
      FileDescriptor::Open(path, {fs::blocking::OpenFlag::kWrite,
                                  fs::blocking::OpenFlag::kExclusiveCreate});
  EXPECT_NE(-1, fd.GetNative());

  EXPECT_EQ(0, fd.GetSize());

  EXPECT_NO_THROW(FileDescriptor::Open(path, fs::blocking::OpenFlag::kRead));

  EXPECT_THROW(FileDescriptor::OpenDirectory(path), std::system_error);
  EXPECT_NO_THROW(std::move(fd).Close());
}

TEST(FileDescriptor, FSyncClose) {
  const auto dir = fs::blocking::TempDirectory::Create();
  const auto path = dir.GetPath() + "/foo";

  auto fd =
      FileDescriptor::Open(path, {fs::blocking::OpenFlag::kWrite,
                                  fs::blocking::OpenFlag::kExclusiveCreate});
  EXPECT_NO_THROW(std::move(fd).Close());
  EXPECT_THROW(fd.FSync(), std::system_error);
  EXPECT_THROW(std::move(fd).Close(), std::system_error);
}

TEST(FileDescriptor, WriteRead) {
  const auto dir = fs::blocking::TempDirectory::Create();
  const auto path = dir.GetPath() + "/foo";
  const auto contents = "123456";

  auto fd =
      FileDescriptor::Open(path, {fs::blocking::OpenFlag::kWrite,
                                  fs::blocking::OpenFlag::kExclusiveCreate});
  EXPECT_NO_THROW(fd.Write(contents));
  std::move(fd).Close();

  auto fd2 = FileDescriptor::Open(path, fs::blocking::OpenFlag::kRead);
  EXPECT_EQ(contents, ReadContents(fd2));
}
