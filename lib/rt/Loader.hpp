#ifndef RTLOADER_HPP
#define RTLOADER_HPP

#include "rt/Format.hpp"
#include "rt/Executor.hpp"

#include <memory>

namespace rt {
  class LoadingFailed : std::exception {
    virtual const char* what() const throw() {
      return "loading failed";
    }
  };

  typedef std::shared_ptr<Executor> ExecutorPtr;

  class Loader;

  class LoadCallback {
  public:
    virtual ExecutorPtr load(Loader& loader) = 0;
    virtual ~LoadCallback() {}
  };

  class Loader {
  protected:
    static std::map<Kind, LoadCallback*> callbacks;

    Header header;
    const uint8_t* begin;
    const uint8_t* end;
    const uint8_t* curr;

    Loader(const uint8_t* data, size_t len);

  public:
    template <typename T>
    void readValue(T& t) {
      ensure(curr + sizeof(T) <= end);
      t = *reinterpret_cast<const T*>(curr);
      curr += sizeof(T);
    }

    const Header& getHeader() const;

    void readData(uint8_t* d, size_t len);
    void loadPredicate(Predicate& phi);

    void ensure(bool cond);

    static void addCallback(Kind kind, LoadCallback* callback);
    static ExecutorPtr load(const std::vector<uint8_t>& data);
    static ExecutorPtr load(const uint8_t* data, size_t len);
  };

} //namespace rt

#endif // RTLOADER_HPP
