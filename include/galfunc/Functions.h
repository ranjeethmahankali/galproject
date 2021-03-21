#pragma once

#include <string.h>
#include <functional>

namespace gal {
namespace func {

class Session
{
};

class IFunctor;

struct IDataRef
{
  virtual void*    data()   = 0;
  virtual uint32_t typeId() = 0;
  virtual IFunctor* owner()  = 0;
};

class IFuctor
{
public:
  virtual std::string name() const = 0;
  virtual void        run()        = 0;
};

}  // namespace func
}  // namespace gal
