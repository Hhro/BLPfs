#include "fs.hpp"

#include <cstring>

int FileSystemManager::FindFile(const std::string& fname) {
  if (!fs_stream_.is_open()) {
    return -1;
  }
}

bool FileSystemManager::MountBLPfs(const std::string& fs) {
  char* magic;

  fs_stream_.open(fs);

  if (fs_stream_.is_open()) {
    magic = new char[kMagicSize];
    fs_stream_.read(magic, kMagicSize);

    if (memcmp(const_cast<const char*>(magic), "BLP\x7f", 4)) {
      fs_stream_.close();
      return false;
    } else {
      Rewind();
      return true;
    }
  }

  return false;
}

bool FileSystemManager::Ref(const std::string& key) {
  if (ref_count_.find(key) == ref_count_.end()) {
    ref_count_[key] = 0;
  } else {
    ref_count_[key] += 1;
  }
  return true;
}

bool FileSystemManager::Put(const std::string& key, const FileHeader& entry) {
  if (slot_.size() < kCacheSize) {
    slot_.insert(std::pair<std::string, FileHeader>(key, entry));
  } else {
    for (int i = 0; i < kCacheSize; i++) {
    }
  }
}

bool FileSystemManager::Get(const std::string& key) {}

int FileSystemManager::ReadFile(const std::string& fname, const uint32_t pos,
                                const uint32_t len) {
  if (!fs_stream_.is_open()) {
    return -1;
  }
}
int WriteFile(const std::string& fname, const uint32_t pos, const uint32_t len,
              const char& data);
bool RemoveFile(const std::string& fname);
int SizeFile(const std::string& fname);

File::File(const std::string& name, const uint32_t length, const uint8_t level,
           bool in_use, const char* data) {
  fheader_.name.assign(name);

  fheader_.length = length;
  fheader_.level = level;
  fheader_.in_use = in_use;

  data_ = new char[fheader_.length];
  memcpy(data_, data, fheader_.length);
}

File::File(const char* raw) {
  fheader_.name = std::string(raw, 0, kNameSize);
  memcpy(reinterpret_cast<char*>(fheader_.length), raw + kNameSize,
         kLengthSize);

  fheader_.level = fheader_.length & 0x0006;
  fheader_.in_use = fheader_.length & 0x0001;
  fheader_.length &= 0xfff8;

  data_ = new char[fheader_.length];
  memcpy(data_, raw + kFileHeaderSize, fheader_.length);
}