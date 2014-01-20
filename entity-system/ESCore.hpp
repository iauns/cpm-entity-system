#ifndef IAUNS_ENTITY_SYSTEM_ESCORE_HPP
#define IAUNS_ENTITY_SYSTEM_ESCORE_HPP

#include <map>
#include <list>
#include <iostream>
#include "src/ComponentContainer.hpp"
#include "src/BaseSystem.hpp"

namespace CPM_ES_NS {

/// Entity system core.
class ESCore
{
public:
  ESCore();
  virtual ~ESCore();

  /// Returns false if the component doesn't exist.
  bool hasComponentContainer(uint64_t component);

  /// When called, ESCore takes ownership of \p component.
  /// Adds a new component to the system. If a component of the same
  /// TypeID already exists, the request is ignored.
  void addComponentContainer(BaseComponentContainer* component);

  /// Clears out all component containers (deletes all entities).
  void clearAllComponentContainers();

  /// Retrieves a base component container. Component is the output from
  /// the TemplateID class.
  BaseComponentContainer* getComponentContainer(uint64_t component);

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

  /// Removes all components associated with entity.
  void removeEntity(uint64_t entityID);

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

  /// Adds a static component. Static components work exactly like normal
  /// components, except that they are not associated with an entityID and are
  /// fed into each system in the same manner. Use this to make 'global'
  /// components (for lighting, view camera position, input, etc...).
  /// Static components and regular components cannot be mixed.
  template <typename T>
  void addStaticComponent(const T& component)
  {
    // If the container isn't already marked as static, mark it and ensure
    // that it is empty.
    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T>();
    ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
    concreteContainer->addStaticComponent(component);
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
  T* getFirstStaticComponent()
  {
    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T>();
    ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
    typename ComponentContainer<T>::ComponentItem* components = concreteContainer->getComponentArray();
    if (components != nullptr)
      return &components->component;
    else
      return nullptr;
  }

  /// Call this function at the beginning of every frame! It renormalizes all
  /// your components (adds / removes components).
  /// To call a system, execute the walkComponents function on BaseSystem.
  /// Most systems don't need a stable sort. But if you need to guarantee the
  /// relative order of multiple components with the same entity ID, use
  /// stable sort.
  void renormalize(bool stableSort = false);

  // OPTIONAL FUNCTION
  // All functions below this comment are entirely optional and provided only
  // for your conveinence. In most cases, you should probably re-implement
  // them in a form that is more appropriate for your purposes.

  /// Returns a, new, valid entity ID. This is a trivial function and should
  /// not be used if you are controlling entity ids using external code.
  uint64_t getNewEntityID() {return ++mCurSequence;}


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

  std::map<uint64_t, BaseComponentContainer*> mComponents;
  uint64_t                                    mCurSequence;
};

} // namespace CPM_ES_NS 

#endif 
