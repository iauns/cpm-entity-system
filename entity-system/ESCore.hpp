#ifndef IAUNS_ENTITY_SYSTEM_ESCORE_HPP
#define IAUNS_ENTITY_SYSTEM_ESCORE_HPP

#include <map>
#include <list>
#include <iostream>
#include <stdexcept>
#include "BaseSystem.hpp"
#include "CoreInt.hpp"
#include "src/ComponentContainer.hpp"

namespace CPM_ES_NS {

/// An example of creating a component container map using a std::map.
/// Feel free to use this class instead of rolling your own.
//template <template<typename T> typename Container = ComponentContainer>
class ESCore : public CoreInt
{
public:
  ESCore();
  virtual ~ESCore();

  /// Returns false if the component doesn't exist.
  bool hasComponentContainer(uint64_t component) override;

  /// When called, ESCore takes ownership of \p component.
  /// Adds a new component to the system. If a component of the same
  /// TypeID already exists, the request is ignored.
  void addComponentContainer(BaseComponentContainer* component, uint64_t containerID) override;

  /// Retrieves a base component container. Component is the output from
  /// the TemplateID class.
  BaseComponentContainer* getComponentContainer(uint64_t component) override;

  /// Iterates over all containers. No ordering is presumed.
  /// DEPRECATED
  void iterateOverContainers(std::function<void(BaseComponentContainer*)>& cb) override;

  /// Clears out all component containers (deletes all entities).
  void clearAllComponentContainers();

  /// Call this function at the beginning or end of every frame. It renormalizes
  /// all your components (adds / removes components). To call a system, execute
  /// the walkComponents function on BaseSystem. Most systems don't need a
  /// stable sort. But if you need to guarantee the relative order of multiple 
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

  // Note: getNewEntityID is a deprecated function that will be removed in a
  // future version of the entity system.

  /// Returns a, new, valid entity ID. This is a trivial function and should
  /// not be used if you are controlling entity ids using external code.
  uint64_t getNewEntityID() {return ++mCurSequence;}

protected:

  template <typename T>
  BaseComponentContainer* ensureComponentArrayExists()
  {
    BaseComponentContainer* componentContainer = getComponentContainer(TemplateID<T>::getID());
    if (componentContainer == nullptr)
    {
      componentContainer = new ComponentContainer<T>();
      addComponentContainer(componentContainer, TemplateID<T>::getID());
    }
    return componentContainer;
  }

  std::map<uint64_t, BaseComponentContainer*> mComponents;
  uint64_t                                    mCurSequence;
};

} // namespace CPM_ES_NS 

#endif 
