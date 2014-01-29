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
  virtual void addComponentContainer(BaseComponentContainer* component) = 0;

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

  /// Call this function at the beginning of every frame! It renormalizes all
  /// your components (adds / removes components). To call a system, execute the
  /// walkComponents function on BaseSystem. Most systems don't need a stable
  /// sort. But if you need to guarantee the relative order of multiple 
  /// components with the same entity ID, use stable sort.
  virtual void renormalize(bool stableSort = false);

  /// Removes all components associated with entity.
  virtual void removeEntity(uint64_t entityID);

  /// Adds a component. If a component container already exists, then that is
  /// used. Otherwise, a new component container is created and used.
  template <typename T>
  void addComponent(uint64_t entityID, const T& component)
  {
    /// \todo Ensure that we do not attempt to add components to a static
    ///       container -- write to cerr.
    if (entityID == 0)
    {
      std::cerr << "cpm-entity-system: Attempting to add a component of entityID 0! Not allowed." << std::endl;
      throw std::runtime_error("Attempting to add a component of entityID 0.");
      return;
    }

    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T>();
    ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
    concreteContainer->addComponent(entityID, component);
  }

  /// Adds a static component. Static components work exactly like normal
  /// components, except that they are not associated with an entityID and are
  /// fed into each system in the same manner. Use this to make 'global'
  /// components (for lighting, view camera position, input, etc...).
  /// Static components and regular components cannot be mixed.
  /// Returns the index of the static component.
  template <typename T>
  size_t addStaticComponent(const T& component)
  {
    // If the container isn't already marked as static, mark it and ensure
    // that it is empty.
    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T>();
    ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
    return concreteContainer->addStaticComponent(component);
  }

  /// Retrieves the list of static components. You can modify these values at
  /// will until renormalize is called. Use this to update the values of static
  /// components if you don't want to specifically write a system for that.
  /// Will return a nullptr if there are no static components.
  template <typename T>
  typename ComponentContainer<T>::ComponentItem* getStaticComponents()
  {
    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T>();
    ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
    return concreteContainer->getComponentArray();
  }

  template <typename T>
  T* getStaticComponent(int index = 0)
  {
    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T>();
    ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
    typename ComponentContainer<T>::ComponentItem* components = concreteContainer->getComponentArray();
    int numComp = concreteContainer->getNumComponents();
    if (components != nullptr && index < concreteContainer->getNumComponents())
      return &components[index].component;
    else
      return nullptr;
  }

private:

  template <typename T>
  BaseComponentContainer* ensureComponentArrayExists()
  {
    BaseComponentContainer* componentContainer = getComponentContainer(TemplateID<T>::getID());
    if (componentContainer == nullptr)
    {
      componentContainer = new ComponentContainer<T>();
      addComponentContainer(componentContainer);
    }
    return componentContainer;
  }

};

} // namespace CPM_ES_NS

#endif 
