#ifndef IAUNS_ENTITY_SYSTEM_GENERICSYSTEM_HPP
#define IAUNS_ENTITY_SYSTEM_GENERICSYSTEM_HPP

// GenericSystems are not associated with any one core. You can use a system
// with any number of cores. All important functions are static and don't
// use any stored state. You can create instances of this class and use
// the BaseSystem interface to iterate over all systems with the 
// walkComponentsOver override.

#include <iostream>
#include <array>
#include <set>          // Only used in a corner case of walkComponents where
                        // all components are optional.
#include <type_traits>
#include "ESCore.hpp"
#include "src/ComponentContainer.hpp"
#include "src/TemplateID.hpp"

namespace CPM_ES_NS {


namespace gs_detail
{

/// See: http://stackoverflow.com/questions/10766112/c11-i-can-go-from-multiple-args-to-tuple-but-can-i-go-from-tuple-to-multiple
template <typename C, typename F, typename Tuple, bool Done, int Total, int... N>
struct call_impl
{
  static void call(uint64_t sequence, C c, F f, Tuple && t)
  {
    call_impl<C, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(sequence, c, f, std::forward<Tuple>(t));
  }
};

template <typename C, typename F, typename Tuple, int Total, int... N>
struct call_impl<C, F, Tuple, bool(true), Total, N...>
{
  static void call(uint64_t sequence, C c, F f, Tuple && t)
  {
    // Call the variadic member function (c.*f) with expanded template
    // parameter list (std::get<N>(std::forward<Tuple>(t))...).
    (c->*f)(sequence, std::get<N>(std::forward<Tuple>(t))...);
  }
};

/// C = class type, F = function type.
/// c = class instance, f = member function pointer.
template <typename C, typename F, typename Tuple>
void call(uint64_t sequence, C c, F f, Tuple && t)
{
  typedef typename std::decay<Tuple>::type ttype;
  call_impl<C, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(sequence, c, f, std::forward<Tuple>(t));
}



template <typename... RTs>
struct init_impl;

template <typename RT, typename... RTs>
struct init_impl<RT, RTs...>
{
  static bool ensureContainersPresent(ESCore* core)
  {
    // Check to see if component is already present. If not, then we should
    // add the component default reserve, if any.
    if (!core->hasComponentContainer(TemplateID<RT>::getID()))
    {
      core->addComponentContainer(new ComponentContainer<RT>());
    }

    return init_impl<RTs...>::ensureContainersPresent(core);
  }
};

template <>
struct init_impl<>
{
  static bool ensureContainersPresent(ESCore* core)
  {
    return true;
  }
};



}

/// See: http://stackoverflow.com/questions/18986560/check-variadic-templates-parameters-for-uniqueness?lq=1
/// The mpl namespace contains everything to implement is_unique. It is used in
/// the static assertion at the beginning of GenericSystem below.
namespace mpl
{
template< class T > using invoke = typename T :: type ;
template< class C, class I, class E > using if_t     = invoke< std::conditional< C{}, I, E> >;
template< class T > struct id{};
struct empty{};
template< class A, class B > struct base : A, B {};
template< class B , class ... > struct is_unique_impl;
template< class B > struct is_unique_impl<B>: std::true_type{};
template< class B, class T, class ... U>
struct is_unique_impl<B, T, U...> : if_t< std::is_base_of< id<T>, B>, std::false_type, is_unique_impl< base<B,id<T>>, U...> >{};

template< class ...T >struct is_unique : is_unique_impl< empty, T ... > {};
} // mpl    


// Simple structure to group like components (components belonging to the
// same entity). Used when shouldGroupComponents returns true.
template <typename T>
struct ComponentGroup
{
  size_t numComponents;
  typename ComponentContainer<T>::ComponentItem* components;
};


/// Base class implementation of generic system. You do not need instances of
/// this class in order to use it. All important functions are static.
template <typename... Ts>
class GenericSystem : public BaseSystem
{
public:

  static_assert(mpl::is_unique<Ts...>::value, "GenericSystem does not allow duplicate component types.");

  GenericSystem()           {}
  virtual ~GenericSystem()  {}

  bool walkEntity(ESCore& core, uint64_t entityID) override
  {
    /// \todo Remove excess calls to getComponentContainer. There should only
    ///       be one call made to getComponentContainer. Also think about
    ///       caching the component containers.
    if (sizeof...(Ts) == 0)
      return false;

    bool group = shouldGroupComponents();

    // Ensure all component containers have been created.
    // Even if a component cointainer is empty doesn't mean that it is not
    // associated with an optional component.
    gs_detail::init_impl<Ts...>::ensureContainersPresent(&core);

    std::array<int, sizeof...(Ts)> indices = { dynamic_cast<ComponentContainer<Ts>*>(core.getComponentContainer(TemplateID<Ts>::getID()))->getComponentItemIndexWithSequence(entityID)... };
    std::array<int, sizeof...(Ts)> nextIndices;
    std::array<bool, sizeof...(Ts)> isStatic = { core.getComponentContainer(TemplateID<Ts>::getID())->isStatic()... };
    std::array<int, sizeof...(Ts)> numComponents = { core.getComponentContainer(TemplateID<Ts>::getID())->getNumComponents()... };
    std::array<bool, sizeof...(Ts)> optionalComponents = { isComponentOptional(TemplateID<Ts>::getID())... }; // Detect optional components via overriden function call (simplest).

    std::tuple<typename ComponentContainer<Ts>::ComponentItem*...> componentArrays = std::make_tuple(
        dynamic_cast<ComponentContainer<Ts>*>(
            core.getComponentContainer(TemplateID<Ts>::getID()))->getComponentArray()...);

    std::tuple<Ts*...> values;  ///< Values that will be passed into execute.
    std::tuple<ComponentGroup<Ts>...> groupValues;  ///< Grouped values that will be passed into group execute.

    bool execute = true;
    for (int i = 0; i < indices.size(); i++)
    {
      // Note: For static components, getComponentItemIndexWithSequence always
      //       returns 0. Which is the correct index to start at.
      if (indices[i] == -1)
      {
        if (!optionalComponents[i])
        {
          execute = false;
          break;
        }
        else
        {
          // For optional components that we don't find, we set the index to the
          // size of the components array. When recursively executing, this
          // will continue the recursion with no extra branches as a result
          // of this component.
          indices[i] = numComponents[i];
        }
      }
    }

    if (execute)
    {
      if (group == false)
        RecurseExecute<0, Ts...>::exec(this, componentArrays, numComponents,
                                       indices, optionalComponents, isStatic,
                                       nextIndices, values, entityID);
      else
        GroupExecute<0, Ts...>::exec(this, componentArrays, numComponents,
                                     indices, optionalComponents, isStatic,
                                     nextIndices, groupValues, entityID);
      return true;
    }
    else
    {
      return false;
    }
  }

  void walkComponents(ESCore& core) override
  {
    if (sizeof...(Ts) == 0)
      return;

    // Ensure all component containers have been created.
    // Even if a component cointainer is empty doesn't mean that it is not
    // associated with an optional component.
    gs_detail::init_impl<Ts...>::ensureContainersPresent(&core);

    // Determine if we should group components together, or execute them
    // in a recursive manner. Component grouping calls a different derived
    // function.
    bool group = shouldGroupComponents();

    /// \todo Remove excess calls to getComponentContainer. There should only
    ///       be one call made to getComponentContainer. Also think about
    ///       caching the component containers.
    std::array<BaseComponentContainer*, sizeof...(Ts)> baseComponents = { core.getComponentContainer(TemplateID<Ts>::getID())... };
    std::array<int, sizeof...(Ts)> indices;
    std::array<int, sizeof...(Ts)> nextIndices;
    std::array<int, sizeof...(Ts)> numComponents;
    std::array<bool, sizeof...(Ts)> isStatic;
    std::array<bool, sizeof...(Ts)> optionalComponents = { isComponentOptional(TemplateID<Ts>::getID())... }; // Detect optional components via overriden function call (simplest).

    // Start off with std::numeric_limits max on lowest upper sequence,
    // and check for optional components. No optional components are allowed
    // to be the leading sequence. If all we find are optional components,
    // we write out a warning. We could also iterate through all components
    // and combinations in existence. We may implement this later and warn
    // the user about possible performance problems when doing this.
    uint64_t lowestUpperSequence = std::numeric_limits<uint64_t>::max();
    int leadingComponent = -1;
    int numOptionalComponents = 0;
    for (int i = 0; i < baseComponents.size(); ++i)
    {
      bool optional = optionalComponents[i];
      // An empty mandatory component results in an immediate termination
      // of the walk. Even if the component is static.
      if (baseComponents[i]->getNumComponents() == 0 && !optional)
        return;

      indices[i] = 0;
      nextIndices[i] = 0;
      numComponents[i] = baseComponents[i]->getNumComponents();
      isStatic[i] = baseComponents[i]->isStatic();

      // Optional and static components are not allowed to be leading components
      if (optional || isStatic[i])
        continue;

      if (baseComponents[i]->getUpperSequence() < lowestUpperSequence)
      {
        leadingComponent = i;
        lowestUpperSequence = baseComponents[i]->getUpperSequence();
      }
    }

    // Arrays containing components.
    std::tuple<typename ComponentContainer<Ts>::ComponentItem*...> componentArrays = std::make_tuple(
        dynamic_cast<ComponentContainer<Ts>*>(
            core.getComponentContainer(TemplateID<Ts>::getID()))->getComponentArray()...);

    std::tuple<Ts*...> values;                    ///< Values that will be passed into execute.
    std::tuple<ComponentGroup<Ts>...> groupValues;  ///< Grouped values that will be passed into group execute.

    // leadingComponent == -1 if and only if the number of optional and static
    // components == sizeof...(Ts). Because the if statement following the
    // "if (optional || isStatic) contitue;" statement will always succeed
    // due to the numeric_limits setting, leading to a valid value of
    // loading component.
    if (leadingComponent != -1)
    {
      // This clause is called when there is at least 1 non-optional
      // (mandatory) non-static component in the system.

      // Start off at the first target sequence and march forward from there.
      uint64_t targetSequence = baseComponents[leadingComponent]->getSequenceFromIndex(indices[leadingComponent]);
      while (targetSequence != 0)
      {
        // Find the target sequence in the other components.
        bool failed = false;
        for (int i = 0; i < baseComponents.size(); ++i)
        {
          uint64_t curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
          bool optional = optionalComponents[i];
          bool compStatic = isStatic[i];

          // Static components don't have associated sequences.
          // They are always at index 0 and always valid. Existance of some
          // static components were guaranteed above.
          if (!compStatic)  
          {
            // Note: when indices[i] = numComponents[i], we necessarily have an
            // optional component. This is because of the check for a zero number
            // of components above, in conjunction with the iteration logic
            // below. We always exit the function if we reach the end of a
            // mandatory component's array; but for an optional we break out and
            // continue.
            while (curSequence < targetSequence && indices[i] != numComponents[i])
            {
              ++indices[i];
              // Check to see if this is an optional component! If it is, then
              // we shouldn't return!
              if (indices[i] == numComponents[i])
              {
                if (!optional)
                  return;   // We are done -- this was a mandatory component and we reached the end of its list.
                else
                  break;    // Can't go any further.
              }
              curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
            }

            if (curSequence != targetSequence && !optional)
            {
              failed = true;
              break;
            }
          }
        }

        if (!failed)
        {
          // Execute with indices. This recursive execute will perform a cartesian
          // product.
          if (group == false)
          {
            if (!RecurseExecute<0, Ts...>::exec(this, componentArrays, 
                                                numComponents, indices, 
                                                optionalComponents, isStatic,
                                                nextIndices, values,
                                                targetSequence))
            {
              return; // We have reached the end of an array.
            }
          }
          else
          {
            if (!GroupExecute<0, Ts...>::exec(this, componentArrays,
                                              numComponents, indices,
                                              optionalComponents, isStatic,
                                              nextIndices, groupValues,
                                              targetSequence))
            {
              return;
            }
          }

          // Copy nextIndices into indices.
          indices = nextIndices;
        }

        // Find new target sequence.
        while (baseComponents[leadingComponent]->getSequenceFromIndex(indices[leadingComponent]) == targetSequence)
        {
          ++indices[leadingComponent];
          if (indices[leadingComponent] == numComponents[leadingComponent])
            return;   // We are done.
        }
        targetSequence = baseComponents[leadingComponent]->getSequenceFromIndex(indices[leadingComponent]);
      }
    }
    else
    {
      // This else clause is called when *all* components are optional or
      // static. Not a single one is mandatory other than the static components,
      // whose existence has already been tested above.

      // Now we walk in a different fashion than above.

      // Determine list of entity ids that need to be walked. We use these as
      // the list of target sequences to be used when walking all components.
      // We must keep the sequenced sorted. So we use a map to accomplish this.

      // Instead of a target sequence, we will use this list of sorted sequences
      // to walk the components.

      // Create the target sequence list by recursing through our parameters.
      /// \todo Optimize? Set will cause dynamic memory allocation. *But*, this
      ///       is only a corner case and very few, if any, systems need
      ///       this case where all components are optional.
      std::set<uint64_t> sequenceSet;
      GenSequenceSet<0, Ts...>::exec(sequenceSet, componentArrays,
                                     isStatic, numComponents);

      if (sequenceSet.size() != 0)
      {
        // Use the set as a list of target sequences when iterating over the
        // sequences.
        for (auto it = sequenceSet.begin(); it != sequenceSet.end(); ++it)
        {
          uint64_t targetSequence = *it;

          // Find the target sequence in the components.
          for (int i = 0; i < baseComponents.size(); ++i)
          {
            if (!isStatic[i])
            {
              uint64_t curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
              while (curSequence < targetSequence && indices[i] != numComponents[i])
              {
                ++indices[i];
                if (indices[i] == numComponents[i])
                  break;   // We are done.
                curSequence = baseComponents[i]->getSequenceFromIndex(indices[i]);
              }
            }

            // We are never going to fail finding this sequence in at least one
            // of the optional components.
          }

          // Execute with indices. This recursive execute will perform a cartesian
          // product.
          if (group == false)
          {
            if (!RecurseExecute<0, Ts...>::exec(this, componentArrays, 
                                                numComponents, indices, 
                                                optionalComponents, isStatic,
                                                nextIndices, values,
                                                targetSequence))
            {
              return; // We have reached the end of an array.
            }
          }
          else
          {
            if (!GroupExecute<0, Ts...>::exec(this, componentArrays,
                                              numComponents, indices,
                                              optionalComponents, isStatic,
                                              nextIndices, groupValues,
                                              targetSequence))
            {
              return; // We have reached the end of an arry
            }
          }

          // Copy nextIndices into indices.
          indices = nextIndices;
        }
      }
      else // if (sequenceSet.size() != 0)
      {
        // When the sequence set is zero, that means that we could have a corner
        // case where all of the components are static. If that is the case,
        // then we simply iterate over all static members, either as a group
        // or recursively. If there are any optional components, then we don't
        // execute at all.
        bool allStatic = true;
        for (int i = 0; i < sizeof...(Ts); ++i)
        {
          if (isStatic[i] == false)
          {
            allStatic = false;
            break;
          }
        }

        if (allStatic)
        {
          if (group == false)
          {
            if (!RecurseExecute<0, Ts...>::exec(this, componentArrays, 
                                                numComponents, indices, 
                                                optionalComponents, isStatic,
                                                nextIndices, values,
                                                BaseComponentContainer::StaticEntID))
            {
              return; // We have reached the end of an array.
            }
          }
          else
          {
            if (!GroupExecute<0, Ts...>::exec(this, componentArrays,
                                              numComponents, indices,
                                              optionalComponents, isStatic,
                                              nextIndices, groupValues,
                                              BaseComponentContainer::StaticEntID))
            {
              return; // We have reached the end of an arry
            }
          }
        }
      }
    }
  }

  std::vector<uint64_t> getComponents()
  {
    std::vector<uint64_t> components = { TemplateID<Ts>::getID()... };
    return components;
  }

  template <int TupleIndex, typename... RTs>
  struct GenSequenceSet;

  template <int TupleIndex, typename RT, typename... RTs>
  struct GenSequenceSet<TupleIndex, RT, RTs...>
  {
    static void exec(std::set<uint64_t>& sequenceSet,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     const std::array<int, sizeof...(Ts)>& componentSizes)
    {
      typename ComponentContainer<RT>::ComponentItem* array = std::get<TupleIndex>(componentArrays);
      bool isStatic     = componentStatic[TupleIndex];

      // Only generate set of sequences if we do not have a static component.
      // Static componets always have 1 sequence, and they are handled
      // internally by the RecurseExecute and GroupExecute template structures.
      if (!isStatic)
      {
        // Loop over components at this RT.
        for (int i = 0; i < componentSizes[TupleIndex]; ++i)
        {
          sequenceSet.insert(array[i].sequence);
        }
      }

      // Recurse into corresponding container.
      GenSequenceSet<TupleIndex + 1, RTs...>::exec(
          sequenceSet, componentArrays, componentStatic, componentSizes);
    }
  };

  template <int TupleIndex>
  struct GenSequenceSet<TupleIndex>
  {
    static void exec(std::set<uint64_t>& /*sequenceSet*/,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& /* componentArrays */,
                     const std::array<bool, sizeof...(Ts)>& /* componentStatic */,
                     const std::array<int, sizeof...(Ts)>& /* componentSizes */)
    {}
  };

  template <int TupleIndex, typename... RTs>
  struct RecurseExecute;

  template <int TupleIndex, typename RT, typename... RTs>
  struct RecurseExecute<TupleIndex, RT, RTs...>
  {
    static bool exec(GenericSystem<Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<Ts*...>& input,
                     uint64_t targetSequence)
    {
      int arraySize     = componentSizes[TupleIndex];
      int currentIndex  = indices[TupleIndex];
      bool optional     = componentOptional[TupleIndex];
      bool isStatic     = componentStatic[TupleIndex];
      typename ComponentContainer<RT>::ComponentItem* array = std::get<TupleIndex>(componentArrays);

      // Check to see if we are at the end of the array. This will happen with
      // optional components.
      if (currentIndex == arraySize)
      {
        if (optional)
        {
          std::get<TupleIndex>(input) = nullptr;
        }
        else
        {
          // Terminate early if we are at the end of the array and are not an
          // optional component.
          return false;
        }
      }
      else
      {
        // Only pass target sequence 
        if (array[currentIndex].sequence != targetSequence || isStatic)
        {
          if (isStatic && arraySize > 0)
          {
            std::get<TupleIndex>(input) = &array[0].component;
          }
          else
          {
            if (!optional) std::cerr << "Generic System: invalid sequence on non-optional component!!" << std::endl;
            std::get<TupleIndex>(input) = nullptr;
          }
        }
        else
        {
          std::get<TupleIndex>(input) = &array[currentIndex].component;
        }
      }

      // Depth first this will perform one execution with the target sequence
      // we found.
      bool reachedEnd = false;
      if (!RecurseExecute<TupleIndex + 1, RTs...>::exec(ptr,
              componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence))
      {
        reachedEnd = true;
      }

      // Only increment current index if we are currently on the target
      // sequence. This *can* happen if we have an optional component that
      // passed nullptr in as its parameter.
      if (!isStatic)
      {
        if (array[currentIndex].sequence == targetSequence)
          ++currentIndex;

        if (currentIndex == arraySize)
        {
          if (nextIndices[TupleIndex] < currentIndex)
            nextIndices[TupleIndex] = currentIndex;

          // Terminate early in either case, but with different return values.
          // For optional components, reaching the end of the array does not
          // necessarily mean we should terminate the algorithm.
          if (optional)
            return true;
          else
            return false;
        }
        // Loop until we find a sequence that is not in our target.
        while (array[currentIndex].sequence == targetSequence)
        {
          std::get<TupleIndex>(input) = &array[currentIndex].component;
          // We don't need to check return value of RecurseExecute since any
          // lower component would have returned false on the first call. The
          // result of the first call is cached in reachedEnd.
          RecurseExecute<TupleIndex + 1, RTs...>::exec(
              ptr, componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence);

          ++currentIndex;
          if (currentIndex == arraySize)
          {
            if (optional)
              break;
            else
              return false;
          }
        }

        if (nextIndices[TupleIndex] < currentIndex)
          nextIndices[TupleIndex] = currentIndex;
      }
      else // if (!isStatic)
      {
        // Loop over static array (we've already executed the first element above).
        for (int i = 1; i < arraySize; ++i)
        {
          std::get<TupleIndex>(input) = &array[i].component;
          // We don't need to check return value of RecurseExecute since any
          // lower component would have returned false on the first call. The
          // result of the first call is cached in reachedEnd.
          RecurseExecute<TupleIndex + 1, RTs...>::exec(
              ptr, componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence);
        }
      }

      if (reachedEnd)
        return false;
      else
        return true;
    }
  };

  template <int TupleIndex>
  struct RecurseExecute<TupleIndex>
  {
    static bool exec(GenericSystem<Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<Ts*...>& input,
                     uint64_t targetSequence)
    {
      // Call our execute function with 'targetSequence' and 'input'.
      gs_detail::call(targetSequence, ptr, &GenericSystem<Ts...>::execute, input);
      return true;
    }
  };

  template <int TupleIndex, typename... RTs>
  struct GroupExecute;

  template <int TupleIndex, typename RT, typename... RTs>
  struct GroupExecute<TupleIndex, RT, RTs...>
  {
    static bool exec(GenericSystem<Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<ComponentGroup<Ts>...>& input,
                     uint64_t targetSequence)
    {
      int arraySize     = componentSizes[TupleIndex];
      int currentIndex  = indices[TupleIndex];
      bool optional     = componentOptional[TupleIndex];
      bool isStatic     = componentStatic[TupleIndex];
      typename ComponentContainer<RT>::ComponentItem* array = std::get<TupleIndex>(componentArrays);
      bool endOfArray   = false;

      // Check to see if we are at the end of the array. This will happen with
      // optional components. Static components will never enter this case.
      if (currentIndex == arraySize)
      {
        if (optional)
        {
          std::get<TupleIndex>(input).numComponents = 0;
          std::get<TupleIndex>(input).components = nullptr;
        }
        else
        {
          // Terminate early if we are at the end of the array and are not an
          // optional component (this works for isStatic as well).
          return false;
        }
      }
      else
      {
        // Only pass target sequence 
        if (array[currentIndex].sequence != targetSequence || isStatic)
        {
          if (isStatic && arraySize > 0)
          {
            std::get<TupleIndex>(input).numComponents = arraySize;
            std::get<TupleIndex>(input).components = &array[0];
          }
          else
          {
            if (!optional) std::cerr << "Generic System: invalid sequence on non-optional or static component!!" << std::endl;
            std::get<TupleIndex>(input).numComponents = 0;
            std::get<TupleIndex>(input).components = nullptr;
          }
        }
        else
        {
          /// This is the only case in which we need to determine the number
          /// of components.
          int numComponents = 0;

          // Set value before we reassign currentIndex.
          std::get<TupleIndex>(input).components = &array[currentIndex];

          // Loop until we find a sequence that is not in our target.
          while (array[currentIndex].sequence == targetSequence)
          {
            ++currentIndex;
            ++numComponents;
            if (currentIndex == arraySize)
            {
              endOfArray = true;
              break;
            }
          }

          // Now set how many components should exist.
          std::get<TupleIndex>(input).numComponents = numComponents;
        }
      }

      // Depth first this will perform one execution with the target sequence
      // we found.
      if (!GroupExecute<TupleIndex + 1, RTs...>::exec(ptr,
              componentArrays, componentSizes, indices, componentOptional,
              componentStatic, nextIndices, input, targetSequence))
      {
        return false;
      }

      if (nextIndices[TupleIndex] < currentIndex)
        nextIndices[TupleIndex] = currentIndex;

      if (endOfArray && !optional)
        return false;
      else
        return true;
    }
  };

  template <int TupleIndex>
  struct GroupExecute<TupleIndex>
  {
    static bool exec(GenericSystem<Ts...>* ptr,
                     const std::tuple<typename ComponentContainer<Ts>::ComponentItem*...>& componentArrays,
                     const std::array<int, sizeof...(Ts)>& componentSizes,
                     const std::array<int, sizeof...(Ts)>& indices,
                     const std::array<bool, sizeof...(Ts)>& componentOptional,
                     const std::array<bool, sizeof...(Ts)>& componentStatic,
                     std::array<int, sizeof...(Ts)>& nextIndices,
                     std::tuple<ComponentGroup<Ts>...>& input,
                     uint64_t targetSequence)
    {
      // Call our execute function with 'targetSequence' and 'input'.
      gs_detail::call(targetSequence, ptr, &GenericSystem<Ts...>::groupExecute, input);
      return true;
    }
  };


  /// Derived classes should implement one of the two following functions,
  /// depending on if shouldGroupComponents is true or false.
  virtual void execute(uint64_t entityID, Ts*... vs)                          {}
  virtual void groupExecute(uint64_t entityID, const ComponentGroup<Ts>&... groups)  {}

  /// Override and return true from this function if you want grouped components
  /// fed into groupExecute, instead of individual components fed into execute.
  /// If you don't use grouped components, all possible combinations of 
  /// components will be fed to execute if there are multiple components.
  virtual bool shouldGroupComponents()                    {return false;}

  /// This function should be overriden and return true for all components
  /// which may be optional.
  virtual bool isComponentOptional(uint64_t templateID)   {return false;}
};


} // namespace CPM_ES_NS

#endif

