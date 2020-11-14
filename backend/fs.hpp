#ifndef BLT_FS_HPP_
#define BLT_FS_HPP_

#include <fstream>
#include <map>
#include <queue>
#include <string>

const int kNameSize = 8;
const int kLengthSize = 2;
const int kFileHeaderSize = kNameSize + kLengthSize;
const int kFileSize = 256;
const int kCacheSize = 4;

// Levels
const int kUserLevel = 1;
const int kAdminLevel = 2;
const int kMaxLevel = 3;

// Return Codes
const int kSuccess = 0;
const int kFileNotFound = -1;
const int kFSCorrupted = -2;
const int kFSNotOpened = -3;
const int kOutOfBound = -4;
const int kNotPermitted = -5;
const int kAlreadyExists = -6;
const int kInvalid = -7;

// Variable Status
const int kUndefined = -1;

struct FileHeader {
  std::string name;
  uint16_t length;
  uint8_t level;
  uint8_t in_use;
};

struct CacheEntry {
  FileHeader fheader;
  uint8_t offset;
};

class File {
 private:
  FileHeader fheader_;
  char* data_;

 public:
  File(const std::string& name, const uint32_t length, const uint8_t level,
       bool in_use, const char* data);
  File(const char* raw);
  inline const std::string name() const { return fheader_.name; }
  inline const uint32_t length() const { return fheader_.length; }
  inline const uint8_t level() const { return fheader_.level; }
  inline const bool in_use() const { return fheader_.in_use; }
  inline const char* data() const { return data_; }
};

class Cache {
 private:
  typedef std::map<std::string, CacheEntry*> Slot;
  Slot slots_;

  std::string threshold_;

  // methods

 public:
  // getter
  const std::string& threshold() const { return threshold_; }
  inline const size_t Size() const { return slots_.size(); }

  // setter
  inline void set_threshold(const std::string& threshold) {
    threshold_.assign(threshold);
  }

  // iterators
  Slot::iterator begin() { return slots_.begin(); }
  Slot::iterator end() { return slots_.end(); }

  // get_item
  inline CacheEntry*& operator[](const std::string key) { return slots_[key]; }

  // methods
  bool Hit(const std::string& key) { return slots_.find(key) != slots_.end(); }
  void Put(const std::string& key, CacheEntry*& entry);
  bool Get(const std::string& key, CacheEntry*& entry);
};

class FileSystemManager {
 private:
  std::fstream fs_stream_;
  std::map<std::string, int> ref_count_;
  Cache cache_;

  int FindFile(const std::string& fname);
  int FindEmptyBlock();
  void Ref(const std::string& key);
  bool ParseFileHeader(const char* raw, FileHeader& fh) const;
  inline bool Rewind() {
    fs_stream_.seekg(0, std::ios_base::beg);
    fs_stream_.seekp(0, std::ios_base::beg);
  }

 public:
  bool MountBLPfs(const std::string& fs);
  int CreateFile(const std::string& fname, const uint16_t fsize,
                 const uint8_t flevel, const uint8_t ulevel);
  int ReadFile(const std::string& fname, const uint32_t pos, const uint32_t len,
               const uint8_t ulevel, char* out);
  int WriteFile(const std::string& fname, const uint32_t pos,
                const uint32_t len, const char* data, const uint32_t ulevel);
  bool RemoveFile(const std::string& fname);
  int SizeFile(const std::string& fname);
};

#endif
