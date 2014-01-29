/// \author James Hughes
/// \date   December 2013

#ifndef IAUNS_CPM_ENTITY_SYSTEM_TEMPLATEID_HPP
#define IAUNS_CPM_ENTITY_SYSTEM_TEMPLATEID_HPP

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

  static void setCounter(uint64_t newCount) {mCurrentTypeID = newCount;}

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
