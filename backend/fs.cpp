#include "fs.hpp"

#include <cstring>
#include <iostream>
#include <iterator>

/* FileSystemManager */

bool FileSystemManager::MountBLPfs(const std::string &fs) {
  fs_stream_.open(fs);

  if (fs_stream_.is_open()) {
    return true;
  }

  return false;
}

void FileSystemManager::Ref(const std::string &key) {
  if (ref_count_.find(key) == ref_count_.end()) {
    ref_count_[key] = 0;
  } else {
    ref_count_[key] += 1;

    if (key == cache_.threshold()) {
      for (auto const &[k, e] : cache_) {
        if (ref_count_[key] > ref_count_[k]) cache_.set_threshold(k);
      }
    }
  }
}

bool FileSystemManager::ParseFileHeader(const char *raw, FileHeader &fh) const {
  char *name = new char[kNameSize];
  uint16_t property = -1;

  memcpy(name, raw, kNameSize);
  fh.name = std::string(name);
  property =
      __builtin_bswap16(*(reinterpret_cast<const uint16_t *>(raw + kNameSize)));

  fh.length = property >> 3;
  fh.in_use = property >> 1;
  fh.level = property;

  return (fh.length <= kFileSize) && (fh.level <= kMaxLevel);
}

// It returns data offset, not header offset.
int FileSystemManager::FindFile(const std::string &fname) {
  FileHeader fh;
  CacheEntry ce;

  char *fh_raw = new char[kFileHeaderSize];

  if (!fs_stream_.is_open()) {
    return -1;
  }
  Rewind();

  if (cache_.Hit(fname)) {
    ce = cache_[fname];
    return ce.offset;
  }

  // Iterate to find the file named "fname".
  while (!fs_stream_.eof()) {
    // Read file header.
    if (fs_stream_.readsome(fh_raw, kFileHeaderSize) != kFileHeaderSize) {
      return -1;
    }

    // Parse file header
    if (!ParseFileHeader(fh_raw, fh)) {
      return -1;
    }

    if (fh.name == fname) {
      // Increase the reference counter of "fname".
      Ref(fname);

      // If the reference count of "fname" is bigger than of threshold,
      // put "fname" in to the cache.
      if (cache_.Size() == 0 ||
          ref_count_[fname] > ref_count_[cache_.threshold()]) {
        ce = {fh, fs_stream_.tellg()};
        cache_.Put(fname, ce);
        // Change threshold if the reference count of current threshold become
        // bigger than of any other entries.
      } else if (fname == cache_.threshold()) {
        for (auto [k, e] : cache_) {
          if (ref_count_[fname] > ref_count_[k]) cache_.set_threshold(k);
        }
      }
      return fs_stream_.tellg();
    } else {
      fs_stream_.seekg(kFileSize, std::ios_base::cur);
    }
  }

  return false;  // File not found
}

int FileSystemManager::CreateFile(const std::string &fname,
                                  const uint8_t level) {
  char name[8] = {
      0,
  };
  uint16_t property;

  if (!fs_stream_.is_open() || FindFile(fname) != kFileNotFound) {
    return false;
  }
  Rewind();

  fs_stream_.seekp(0, std::ios_base::end);

  // Write file name padded with null-bytes.
  if (fname.length() <= 8)
    fname.copy(name, fname.length());
  else
    return false;

  fs_stream_.write(name, kNameSize);

  // Write file property.
  // length(13bit) | level(2bit) | in-use(1bit)
  property = __builtin_bswap16(0 | ((level << 1) & 0b110) | 0b1);
  fs_stream_.write(reinterpret_cast<char *>(&property), 2);

  // Fill null-bytes
  std::fill_n(std::ostream_iterator<char>(fs_stream_), kFileSize, '\x00');

  return true;
}

int FileSystemManager::ReadFile(const std::string &fname, const uint32_t pos,
                                const uint32_t len) {
  if (!fs_stream_.is_open()) {
    return -1;
  }
}
int WriteFile(const std::string &fname, const uint32_t pos, const uint32_t len,
              const char &data);
bool RemoveFile(const std::string &fname);
int SizeFile(const std::string &fname);

/* File */

File::File(const std::string &name, const uint32_t length, const uint8_t level,
           bool in_use, const char *data) {
  fheader_.name.assign(name);

  fheader_.length = length;
  fheader_.level = level;
  fheader_.in_use = in_use;

  data_ = new char[fheader_.length];
  memcpy(data_, data, fheader_.length);
}

File::File(const char *raw) {
  fheader_.name = std::string(raw, 0, kNameSize);
  memcpy(reinterpret_cast<char *>(fheader_.length), raw + kNameSize,
         kLengthSize);

  fheader_.level = fheader_.length & 0x0006;
  fheader_.in_use = fheader_.length & 0x0001;
  fheader_.length &= 0xfff8;

  data_ = new char[fheader_.length];
  memcpy(data_, raw + kFileHeaderSize, fheader_.length);
}

/* Cache */

// TODO Remove least referenced entry.
void Cache::Put(const std::string &key, const CacheEntry &entry) {
  slots_.insert(make_pair(key, entry));
}

bool Cache::Get(const std::string &key, CacheEntry &entry) {
  if (Hit(key)) {
    entry = slots_[key];
    return true;
  } else {
    return false;
  }
}