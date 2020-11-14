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
    ref_count_[key] = 1;
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
  uint16_t property = kUndefined;

  memcpy(name, raw, kNameSize);
  fh.name.assign(name);
  property =
      __builtin_bswap16(*(reinterpret_cast<const uint16_t *>(raw + kNameSize)));

  fh.length = property & 0xff8;
  fh.level = ((property & 0b110) >> 1);
  fh.in_use = property & 0b1;

  delete name;
  return (fh.length <= kFileSize) && (fh.level <= kMaxLevel);
}

int FileSystemManager::FindFile(const std::string &fname) {
  FileHeader fh;
  int res = kUndefined;
  char *fh_raw = new char[kFileHeaderSize];
  CacheEntry *ce = new CacheEntry;

  memset(&fh, 0, sizeof(FileHeader));

  if (!fs_stream_.is_open()) {
    res = kFSNotOpened;
    goto end;
  }
  Rewind();

  if (cache_.Hit(fname)) {
    Ref(fname);
    ce = cache_[fname];
    res = ce->offset;
    goto end;
  }

  // Iterate to find the file named "fname".
  while (!fs_stream_.eof()) {
    // Read file header.
    if (fs_stream_.readsome(fh_raw, kFileHeaderSize) != kFileHeaderSize) {
      res = kFileNotFound;
      break;
    }

    // Parse file header
    if (!ParseFileHeader(fh_raw, fh)) {
      res = kFSCorrupted;
      break;
    }

    if (fh.name == fname) {
      // Increase the reference counter of "fname".
      Ref(fname);

      // If the reference count of "fname" is bigger than of threshold,
      // put "fname" in to the cache.
      if ((cache_.Size() < 4 ||
           ref_count_[fname] > ref_count_[cache_.threshold()]) &&
          (!cache_.Hit(fname))) {
        ce->fheader.name.assign(fh.name);
        ce->fheader.in_use = fh.in_use;
        ce->fheader.level = fh.level;
        ce->fheader.length = fh.length;
        ce->offset = fs_stream_.tellg() - kFileHeaderSize;
        cache_.Put(fname, ce);
        for (auto const &[k, e] : cache_) {
          if (ref_count_[fname] > ref_count_[k]) cache_.set_threshold(k);
        }
      }
      res = (fs_stream_.tellg() - kFileHeaderSize);
      break;
    } else {
      fs_stream_.seekg(kFileSize - kFileHeaderSize, std::ios_base::cur);
    }
  }

end:
  Rewind();
  delete fh_raw;
  return res;  // File not found
}

int FileSystemManager::FindEmptyBlock() {
  // Initialize Variables
  FileHeader fh;
  int res = kUndefined;
  unsigned int off_empty_block = 0;
  char *fh_raw = new char[kFileHeaderSize];

  memset(&fh, 0, sizeof(fh));

  if (!fs_stream_.is_open()) {
    res = kFSNotOpened;
    goto end;
  }
  Rewind();

  while (!fs_stream_.eof()) {
    // Read the file header
    if (fs_stream_.readsome(fh_raw, kFileHeaderSize) != kFileHeaderSize) {
      res = off_empty_block;
      break;
    }

    // Parse the file header
    if (!ParseFileHeader(fh_raw, fh)) {
      res = kFSCorrupted;
      break;
    }

    if (fh.in_use == false) {
      res = off_empty_block;
      break;
    } else {
      off_empty_block += kFileSize;
      fs_stream_.seekg(kFileSize - kFileHeaderSize, std::ios::cur);
    }
  }

end:
  Rewind();
  delete fh_raw;
  return res;
}

int FileSystemManager::CreateFile(const std::string &fname,
                                  const uint16_t fsize, const uint8_t flevel,
                                  const uint8_t ulevel) {
  // Initialize variables
  char name[8];
  int res = kUndefined;
  uint16_t property = kUndefined;

  memset(name, 0, kNameSize);

  if (!fs_stream_.is_open()) {
    res = kFSNotOpened;
    goto end;
  }

  Rewind();

  // File name must be unique
  if (FindFile(fname) >= 0) {
    res = kAlreadyExists;
    goto end;
  }

  // Check rule of no write-down & Level-range check
  if (flevel < ulevel || flevel > kMaxLevel || ulevel > kMaxLevel) {
    res = kNotPermitted;
    goto end;
  }

  // Length of the file name must not be longer than maximum name size.
  if (fname.length() > kNameSize) {
    res = kInvalid;
    goto end;
  }

  if (fsize > kFileSize) {
    res = kInvalid;
    goto end;
  }

  // Search empty block
  fs_stream_.seekp(FindEmptyBlock(), std::ios::beg);

  // Write file name padded with null-bytes.
  fname.copy(name, fname.length());

  fs_stream_.write(name, kNameSize);

  // Write file property.
  // length(13bit) | level(2bit) | in-use(1bit)
  property =
      __builtin_bswap16(((fsize + 8) & 0xff8) | ((flevel << 1) & 0b110) | 0b1);
  fs_stream_.write(reinterpret_cast<char *>(&property), 2);

  // Fill null-bytes
  std::fill_n(std::ostream_iterator<char>(fs_stream_), kFileSize, '\x00');

end:
  Rewind();
  return res;
}

int FileSystemManager::ReadFile(const std::string &fname, const uint32_t pos,
                                const uint32_t len, const uint8_t ulevel,
                                char *out) {
  int res = kUndefined;
  int offset = kUndefined;
  char *fh_raw = nullptr;
  char *data = nullptr;
  FileHeader fh;

  memset(&fh, 0, sizeof(FileHeader));

  if (!fs_stream_.is_open()) {
    return -1;
  }

  Rewind();
  if ((offset = FindFile(fname)) < 0) {
    res = kInvalid;
    goto end;
  }

  fs_stream_.seekg(offset, std::ios::beg);

  // Parse fileheader
  fh_raw = new char[kFileHeaderSize];
  fs_stream_.read(fh_raw, kFileHeaderSize);
  if (!ParseFileHeader(fh_raw, fh)) {
    res = kFSCorrupted;
    goto end;
  }

  // Check OOB
  if (pos + len > fh.length) {
    res = kOutOfBound;
    goto end;
  }

  // Check no read-up & Level-range check
  if (ulevel > fh.level || ulevel > kMaxLevel) {
    res = kNotPermitted;
    goto end;
  }

  // Move position
  fs_stream_.seekg(pos, std::ios::cur);

  // Read file
  data = new char[len];
  fs_stream_.read(data, len);

  if (!memcpy(out, data, len)) {
    res = len;
  }

end:
  Rewind();
  return res;
}

int FileSystemManager::WriteFile(const std::string &fname, const uint32_t pos,
                                 const uint32_t len, const char *data,
                                 const uint32_t ulevel) {
  int res = kUndefined;
  int offset = kUndefined;
  char *fh_raw = new char[kFileHeaderSize];
  FileHeader fh;

  memset(&fh, 0, sizeof(FileHeader));

  if (!fs_stream_.is_open()) {
    res = kFileNotFound;
    goto end;
  }

  Rewind();
  if ((offset = FindFile(fname)) < 0) {
    res = kInvalid;
    goto end;
  }

  fs_stream_.seekg(offset, std::ios::beg);

  // Parse fileheader
  fs_stream_.read(fh_raw, kFileHeaderSize);
  if (!ParseFileHeader(fh_raw, fh)) {
    res = kFSCorrupted;
    goto end;
  }

  // Check OOB
  if (pos + len > fh.length) {
    res = kOutOfBound;
    goto end;
  }

  // Check no write-down
  if (ulevel < fh.level && ulevel > kMaxLevel) {
    res = kNotPermitted;
    goto end;
  }

  // Move position
  fs_stream_.seekp(offset + kFileHeaderSize + pos, std::ios::beg);

  // Write data
  fs_stream_.write(data, len);
  res = len;

end:
  Rewind();
  delete fh_raw;
  return res;
}
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

// Remove least referenced entry.
void Cache::Put(const std::string &key, CacheEntry *&entry) {
  if (slots_.size() == kCacheSize) {
    delete slots_[threshold_];
    slots_.erase(threshold_);
  }

  threshold_.assign(key);
  slots_.insert(make_pair(key, entry));
}

bool Cache::Get(const std::string &key, CacheEntry *&entry) {
  if (Hit(key)) {
    entry = slots_[key];
    return true;
  } else {
    return false;
  }
}