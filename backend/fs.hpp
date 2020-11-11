#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <string>

const int kMagicSize = 4;
const int kNameSize = 8;
const int kLengthSize = 2;
const int kFileHeaderSize = kNameSize + kLengthSize;

const int kCacheSize = 4;

struct FileHeader {
  std::string name;
  unsigned int length : 13;
  unsigned int level : 2;
  unsigned int in_use : 1;
};

struct CacheEntry {
  FileHeader fheader;
  unsigned int offset;
};

class FileSystemManager {
 private:
  std::fstream fs_stream_;
  std::map<std::string, int> ref_count_;
  std::map<std::string, FileHeader> slot_;

  int FindFile(const std::string& fname);
  bool MountBLPfs(const std::string& fs);
  inline bool Rewind() {
    fs_stream_.seekg(0, std::ios_base::beg);
    fs_stream_.seekp(0, std::ios_base::beg);
  }

  bool Ref(const std::string& key);
  bool Hit(const std::string& key) { return slot_.find(key) != slot_.end(); }
  bool Put(const std::string& key, const FileHeader& entry);
  bool Get(const std::string& key);

 public:
  FileSystemManager(const std::string& fs);

  int CreateFile(const std::string& fname, const uint8_t level);
  int ReadFile(const std::string& fname, const uint32_t pos,
               const uint32_t len);
  int WriteFile(const std::string& fname, const uint32_t pos,
                const uint32_t len, const char& data);
  bool RemoveFile(const std::string& fname);
  int SizeFile(const std::string& fname);
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