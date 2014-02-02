#ifndef IAUNS_ENTITY_SYSTEM_CONTAINER_MAP_INTERFACE_HPP
#define IAUNS_ENTITY_SYSTEM_CONTAINER_MAP_INTERFACE_HPP

#include <functional>
#include <iostream>
#include <stdexcept>

#include "BaseSystem.hpp"
#include "src/ComponentContainer.hpp"

namespace CPM_ES_NS {

/// Abstract interface that contains all functions necessary for GenericSystem
/// to correctly iterate over a group of components.

class ContainerMapInterface
{
public:
  virtual ~ContainerMapInterface()    {}

  //----------------------------------------------------------------------------
  // Required functions
  //----------------------------------------------------------------------------

  /// When called, ContainerMapInterface should take ownership of the component
  /// container (BaseComponentContainer). Should issue a warning and refuse
  /// to add (and delete) \p component if there is already a pre-existing
  /// container. Used to automatically add containers via GenericSystem.
  /// containerTypeID is the TemplateID of the type being managed.
  virtual void addComponentContainer(BaseComponentContainer* component, uint64_t containerTypeID) = 0;

  /// Retrieves base component container. Used by GenericSystem to retrieve
  /// containers associated with particular components.
  virtual BaseComponentContainer* getComponentContainer(uint64_t component) = 0;

  /// Returns false if the container for the given \p component does not exist.
  /// GenericSystem uses this to ensure that containers for components are
  /// always present before system execution.
  virtual bool hasComponentContainer(uint64_t component) = 0;

  /// Iterates through all component containers calling the specified function
  /// for every component container. Mimics a forward iterator and is used
  /// to implement the 'renormalize' and 'removeEntity' functions for you.
  /// Feel free to override those functions and provide your own implementation,
  /// as these are extremelly thin functions.
  /// DEPRECATED.
  virtual void iterateOverContainers(std::function<void(BaseComponentContainer*)>& cb) = 0;

  //----------------------------------------------------------------------------
  // Utility functions implemented based on required functions
  //----------------------------------------------------------------------------

  /// Removes all components with their entityID equal to \p entityID, but only
  /// within the \p compTemplateID component (the compTemplateID identifier
  /// comes from the TemplateID<> class).
  void removeAllComponents(uint64_t entityID, uint64_t compTemplateID);

  /// NOTE: If you use either of the following functions, it is highly advised
  ///       that you renormalize with stableSort = true! This ensures that
  ///       your logic regarding what component comes first / last, when
  ///       entityIDs are the same, is valid. Stable sort is slower than a
  ///       regular sort, so only use it when necessary.
  /// @{
  /// Removes the first component with the given entityID associated with the
  /// given \p compTemplateID (the unique identifier for the component type,
  /// from the TemplateID<> class).
  void removeFirstComponent(uint64_t entityID, uint64_t compTemplateID);

  /// Removes the last component with the given \p compTemplateID.
  void removeLastComponent(uint64_t entityID, uint64_t compTemplateID);
  /// @}

};

} // namespace CPM_ES_NS

#endif 
