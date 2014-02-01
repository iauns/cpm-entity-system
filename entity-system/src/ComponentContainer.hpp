#ifndef IAUNS_ENTITY_SYSTEM_COMPONENTCONTAINER_HPP
#define IAUNS_ENTITY_SYSTEM_COMPONENTCONTAINER_HPP

#include <cstdint>
#include <vector>
#include <algorithm>
#include "TemplateID.hpp"
#include "BaseComponentContainer.hpp"

namespace CPM_ES_NS {

namespace cc_detail {

// SFINAE implementation of possible function calls inside of component
// structures.
template<class T>
auto maybe_component_construct(T& v, uint64_t sequence, int)
    -> decltype(v.componentConstruct(sequence), void())
{
  v.componentConstruct(sequence);
}

template<class T>
void maybe_component_construct(T&, size_t, long){}


template<class T>
auto maybe_component_destruct(T& v, uint64_t sequence, int)
    -> decltype(v.componentDestruct(sequence), void())
{
  v.componentDestruct(sequence);
}

template<class T>
void maybe_component_destruct(T&, size_t, long){}

template<class T>
auto maybe_component_name(const char** name, int)
    -> decltype(T::getName(), void())
{
  *name = T::getName();
}

template<class T>
void maybe_component_name(const char** name, long){}


template<class T>
auto maybe_component_serialize(T& v, ESSerialize& b, uint64_t sequence, int)
    -> decltype(v.serialize(b, sequence), void())
{
  v.serialize(b, sequence);
}

template<class T>
void maybe_component_serialize(T&, ESSerialize&, uint64_t sequence, long){}

}

/// Component container.
/// \todo Add maximum size caps to the container. Should also check size
///       caps for the number of removed components as well.
template <typename T>
class ComponentContainer : public BaseComponentContainer
{
public:
  ComponentContainer() :
      mIsStatic(false),
      mLastSortedSize(0),
      mUpperSequence(0),
      mLowerSequence(0)
  {
    /// \todo Extract information from global table regarding default / max size
    ///       of components.
    //mComponents.reserve();
  }

  virtual ~ComponentContainer()
  {
    // Remove all componnts, and if their destructors exist, call those
    // as well.
    for (auto it = mComponents.begin(); it != mComponents.begin() + mLastSortedSize; ++it)
    {
      cc_detail::maybe_component_destruct(it->component, it->sequence, 0);
    }
  }
  
  /// Retrieves the ID of our encapsulated component (T).
  uint64_t getComponentID() override {return TemplateID<T>::getID();}

  /// Retrieves the name of a component.
  const char* getComponentName() override
  {
    const char* name = nullptr;
    cc_detail::maybe_component_name<T>(&name, 0);
    return name;
  }

  /// Serializes all components within the system, given the serialization
  /// base type ESSerialize.
  void serializeComponents(ESSerialize& s) override
  {
    for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
    {
      cc_detail::maybe_component_serialize<T>(it->component, s, it->sequence, 0);
    }
  }

  /// Item that represents one component paired with a sequence.
  struct ComponentItem
  {
    ComponentItem() : sequence(0)
    {
    }

    ComponentItem(uint64_t seq, const T& comp)
    {
      this->sequence = seq;
      this->component = comp;
    }

    /// \todo Overload * operator to retrieve component. This is so we mimic
    ///       STL containers almost completely.

    uint64_t  sequence;   ///< Commonly used element in the first cacheline.
    T         component;  ///< Copy constructable component data.
  };


  static bool componentCompare(const ComponentItem& a, const ComponentItem& b)
  {
    return a.sequence < b.sequence;
  }

  /// Returns -1 if no component of the given sequence is found.
  int getComponentItemIndexWithSequence(uint64_t sequence)
  {
    if (mComponents.size() == 0)
      return -1;

    // You cannot search within static components. They are the same for all
    // entities. Therefore we return the first index, 0.
    if (isStatic())
      return 0;

    auto last = mComponents.begin() + mLastSortedSize;

    ComponentItem item;
    item.sequence = sequence;

    auto it = std::lower_bound(mComponents.begin(), last, item,
                               componentCompare);

    if (it->sequence == sequence)
    {
      return it - mComponents.begin();
    }
    else
    {
      return -1;
    }
  }

  ComponentItem* getComponentItemWithSequence(uint64_t sequence)
  {
    if (mComponents.size() == 0)
      return nullptr;

    if (isStatic())
      return &mComponents.front();

    auto last = mComponents.begin() + mLastSortedSize;

    // Unfortunately, we have to do this in order to take advantage of
    // STL algorithms (duplicate the exact component).
    ComponentItem item;
    item.sequence = sequence;

    // lower_bound uses a binary search to find the target component.
    // Our vector is always sorted, so we can pull this off.
    auto it = std::lower_bound(mComponents.begin(), last, item,
                               componentCompare);

    if (it->sequence == sequence)
    {
      return &(*it);
    }
    else
    {
      return nullptr;
    }
  }

  /// \todo Use std::optional here when we have access to it.
  /// Retrieves component with the indicated sequence.
  const T* getComponentWithSequence(uint64_t sequence)
  {
    ComponentItem* item = getComponentItemWithSequence(sequence);
    if (item)
      &item->component;
    else
      return item;
  }

  /// Sorts in added components and removes deleted components.
  /// Neither of these operations (addition or deletion) take affect when the
  /// system is operating. This way, each timestep is completely deterministic.
  /// The same set of data is acted upon by all systems.
  void renormalize(bool stableSort) override
  {
    // Changes should come FIRST. Changes rely on direct indices to values.
    // No additions or removals should come before modifications.

    // Removals should come *after* addition of new components. This keeps
    // logic consistent within one frame in an entire entity was removed.
    // If an entity was adding components to itself, then subsequently deleted,
    // some of it's components would be left dangling if removal was performed
    // first. But, adding before removal has no side effects, especially if
    // stable sort is used. Then removal from the beginning or end of the list
    // is guaranteed to be consistent.

    if (mModifications.size() > 0)
    {
      // Sort modifications to maintain some semblance of cache friendliness
      // and to detect modification conflicts and resolve with priority.
      // Stable sort is not needed, a priority is used to determine who
      // resolves the modifications.
      std::sort(mModifications.begin(), mModifications.end(), modificationCompare);

      // Simple iteration through the modifications to apply them to our
      // components.
      size_t attemptIdx = 0;
      size_t numMods = mModifications.size();
      for (; attemptIdx != numMods;)
      {
        size_t resolvedIndex = attemptIdx;
        while (   (attemptIdx + 1) != numMods 
               && mModifications[attemptIdx + 1].componentIndex == mModifications[resolvedIndex].componentIndex)
        {
          ++attemptIdx;
          if (mModifications[attemptIdx].priority > mModifications[resolvedIndex].priority)
            resolvedIndex = attemptIdx;
        }

        // Now we have 1 fully resolved modification.
        if (mModifications[resolvedIndex].componentIndex < mComponents.size())
        {
          mComponents[mModifications[resolvedIndex].componentIndex].component = mModifications[resolvedIndex].value;
        }
        else
        {
          std::cerr << "cpm-entity-system - renormalize: Bad index!" << std::endl;
        }
        ++attemptIdx;
      }

      // Clear all modifications.
      mModifications.clear();
    }

    // Check to see if components were added. If so, then sort them into
    // our vector.
    if (mComponents.size() > 0)
    {
      if (mLastSortedSize != mComponents.size())
      {
        // Iterate through the components to-be-constructed array.
        auto it = mComponents.begin() + mLastSortedSize;
        for (; it != mComponents.end(); ++it)
        {
          // Construct added components
          cc_detail::maybe_component_construct(it->component, it->sequence, 0);
        }

        // Sort the entire vector (not just to mLastSortedSize).
        // We *always* stable sort static components. This way we guarantee
        // the correct ordering.
        if (!stableSort && !isStatic())
          std::sort(mComponents.begin(), mComponents.end(), componentCompare);
        else
          std::stable_sort(mComponents.begin(), mComponents.end(), componentCompare);

        mLastSortedSize = mComponents.size();
      }

      mLowerSequence = mComponents.front().sequence;
      mUpperSequence = mComponents.back().sequence;
    }
    else
    {
      mLastSortedSize = 0;
      mLowerSequence = 0;
      mUpperSequence = 0;
    }

    // Perform requested removals.
    if (mRemovals.size() > 0)
    {
      ComponentItem item;
      for (RemovalItem& rem : mRemovals)
      {
        auto last = mComponents.begin() + mLastSortedSize;
        item.sequence = rem.sequence;
        auto it = std::lower_bound(mComponents.begin(), last, item,
                                   componentCompare);

        if (rem.removeType == REMOVE_ALL)
        {
          while (it != mComponents.end() && it->sequence == rem.sequence)
          {
            // Call components destructor (we need to do this at the end).
            cc_detail::maybe_component_destruct(it->component, rem.sequence, 0);

            it = mComponents.erase(it);
            --mLastSortedSize;
          }
        }
        else if (rem.removeType == REMOVE_LAST)
        {
          auto priorIt = it;
          if (it != mComponents.end()) ++it;
          while (it != mComponents.end() && it->sequence == rem.sequence)
          {
            priorIt = it;
            ++it;
          }

          // Call components destructor (we need to do this at the end).
          if (priorIt != mComponents.end() && priorIt->sequence == rem.sequence)
          {
            cc_detail::maybe_component_destruct(priorIt->component, rem.sequence, 0);
            priorIt = mComponents.erase(priorIt);
            --mLastSortedSize;
          }
        }
        else // if (rem.removeType == REMOVE_FIRST)
        {
          if (it != mComponents.end() && it->sequence == rem.sequence)
          {
            cc_detail::maybe_component_destruct(it->component, rem.sequence, 0);
            it = mComponents.erase(it);
            --mLastSortedSize;
          }
        }
      }
      mRemovals.clear();
    }
  }

  /// Get the least sequence held by the component.
  uint64_t getLowerSequence() override { return mLowerSequence; }

  /// Get the upper sequence held by the component.
  uint64_t getUpperSequence() override { return mUpperSequence; }

  /// Retrieves the number of sorted components.
  uint64_t getNumComponents() override { return mLastSortedSize; }

  /// Retrieves the sequence associated with a particular index.
  uint64_t getSequenceFromIndex(int index) override
  {
    if (index < 0 || index >= mLastSortedSize)
      return 0;

    return mComponents[index].sequence;
  }

  /// Adds the component to the end of our components list. It will only become
  /// available upon renormalization (which usually occurs at the end of
  /// a frame).
  void addComponent(uint64_t sequence, const T& component)
  {
    // Add the component to the end of mComponents and wait for a renormalize.
    if (isStatic() == true)
    {
      std::cerr << "Attempting to add entityID component to a static component container!" << std::endl;
      throw std::runtime_error("Attempting to add entityID component to static component container!");
      return;
    }
    mComponents.push_back(ComponentItem(sequence, component));
  }

  /// Returns the index the static component was added at.
  size_t addStaticComponent(const T& component)
  {
    if (isStatic() == false)
    {
      if (mComponents.size() > 0)
      {
        std::cerr << "Cannot add static components to a container that already has";
        std::cerr << " non-static\ncomponents!" << std::endl;
        throw std::runtime_error("Cannot add static components to an entityID component container!");
        return -1;
      }
      else
      {
        setStatic(true);
      }
    }
    size_t newIndex = mComponents.size();
    mComponents.push_back(ComponentItem(StaticEntID, component));
    return newIndex;
  }

  void removeSequence(uint64_t sequence) override
  {
    /// \todo Check size of mRemovals. Ensure it is not greater than the size
    ///       mComponents is allowed to grow to.
    mRemovals.emplace_back(sequence, REMOVE_ALL);
  }

  void removeFirstSequence(uint64_t sequence) override
  {
    mRemovals.emplace_back(sequence, REMOVE_FIRST);
  }

  void removeLastSequence(uint64_t sequence) override
  {
    mRemovals.emplace_back(sequence, REMOVE_LAST);
  }

  ComponentItem* getComponentArray()
  {
    if (mComponents.size() != 0)
      return &mComponents[0];
    else
      return nullptr;
  }

  void modifyIndex(const T& val, size_t index, int priority)
  {
    mModifications.emplace_back(val, index, priority);
  }

  /// Retrieves the active size of the vector backing this component container.
  /// Used only for debugging purposes (see addStaticComponent in ESCore).
  size_t getSizeOfBackingContainer()  {return mComponents.size();}

  bool isStatic() override    {return mIsStatic;}
  void setStatic(bool truth)  {mIsStatic = truth;}

  int mLastSortedSize;                ///< Unsorted elements can be added to the end
                                      ///< of mComponents. This represents the last
                                      ///< sorted element inside mComponents.
  int mUpperSequence;                 ///< Largest sequence in the list.
  int mLowerSequence;                 ///< Smallest sequence in the list.

  bool mIsStatic;                     ///< True if this container contains static
                                      ///< component data.

  /// \todo Look into possibly optimizing binary search by having a separate
  ///       vector containing component sequences. We are at less of a risk
  ///       of cache hits that way.
  ///       This simple implementation is to just sort mComponents, and copy
  ///       over the sequences. But I don't know if this will help or hurt
  ///       overall performance, so we should look into that later.

  enum REMOVAL_TYPE
  {
    REMOVE_ALL,
    REMOVE_FIRST,
    REMOVE_LAST
  };

  struct RemovalItem
  {
    RemovalItem(uint64_t sequenceIn, REMOVAL_TYPE removeTypeIn) :
        sequence(sequenceIn),
        removeType(removeTypeIn)
    {}

    uint64_t      sequence;
    REMOVAL_TYPE  removeType;
  };

  struct ModificationItem
  {
    ModificationItem(const T& val, size_t idx, int pri) :
        value(val),
        componentIndex(idx),
        priority(pri)
    {}

    T value;
    size_t componentIndex; 
    int priority;
  };

  static bool modificationCompare(const ModificationItem& a, const ModificationItem& b)
  {
    return a.componentIndex < b.componentIndex;
  }

  std::vector<ComponentItem>    mComponents;    ///< All components currently in the system.
  std::vector<RemovalItem>      mRemovals;      ///< An array of objects to remove during
                                                ///< renormalization.
  std::vector<ModificationItem> mModifications; ///< An array of objects whose values need
                                                ///< to be updated during renormalization.
};

} // namespace CPM_ES_NS 

#endif 
