/// \author James Hughes
/// \date   December 2013

#ifndef ION_GAMEPLAY_TEMPLATEID_HPP
#define ION_GAMEPLAY_TEMPLATEID_HPP

#include <cstdint>

namespace CPM_ES_NS {

class TemplateIDHelper
{
public:

  static uint64_t getNewTypeID()
  {
    ++mCurrentTypeID;
    return mCurrentTypeID;
  }

private:
  static uint64_t mCurrentTypeID;
};

/// Simple templated class to extract a unique ID from types. Used mostly
/// for sorting purposes.
template <typename T>
class TemplateID
{
public:

  static uint64_t getID()
  {
    // Assign ourselves a new static ID if we don't already have one.
    if (mStaticTypeID == 0)
      mStaticTypeID = TemplateIDHelper::getNewTypeID();

    return mStaticTypeID;
  };

  static uint64_t mStaticTypeID;
};

template <typename T> uint64_t TemplateID<T>::mStaticTypeID = 0;

} // namespace CPM_ES_NS

#endif 
