#ifndef IAUNS_ENTITY_SYSTEM_ESCORE_BASE_HPP
#define IAUNS_ENTITY_SYSTEM_ESCORE_BASE_HPP

#include <map>
#include <list>
#include <iostream>
#include <stdexcept>
#include "BaseSystem.hpp"
#include "src/ComponentContainer.hpp"
#include "src/EmptyComponentContainer.hpp"

namespace CPM_ES_NS {

/// An example of creating a component container map using a std::map.
/// Feel free to use this class instead of rolling your own.
/// NOTE: This class is not meant to be used by itself! You should either use
/// the ESCore class or you should derive from this class and implement the 
/// necessary addComponent and addStaticComponent template functions. See ESCore
/// for an example implementation. I cannot enforce the existence of template
/// functions in a derived class, but this class is fairly useless without
/// the ability to add components.
class ESCoreBase
{
public:
  ESCoreBase();
  virtual ~ESCoreBase() {deleteAllComponentContainers();}

  /// Returns false if the component doesn't exist.
  bool hasComponentContainer(uint64_t componentID) const;

  /// When called, ESCoreBase takes ownership of \p component.
  /// Adds a new component to the system. If a component of the same
  /// TypeID already exists, the request is ignored.
  void addComponentContainer(BaseComponentContainer* componentCont, uint64_t componentID);

  /// Retrieves a base component container. Component is the output from
  /// the TemplateID class.
  BaseComponentContainer* getComponentContainer(uint64_t component);

  /// Clears out all component containers (deletes all entities).
  void clearAllComponentContainers();

  /// Clears out all component containers *immediately*.
  void clearAllComponentContainersImmediately();

  /// Call this function at the beginning or end of every frame. It renormalizes
  /// all your components (adds / removes components). To call a system, execute
  /// the walkComponents function on BaseSystem. Most systems don't need a
  /// stable sort. But if you need to guarantee the relative order of multiple 
  /// components with the same entity ID, use stable sort.
  virtual void renormalize(bool stableSort = false);

  /// Removes all components associated with entity.
  virtual void removeEntity(uint64_t entityID);

  /// Removes all components with their entityID equal to \p entityID, but only
  /// within the \p compTemplateID component (the compTemplateID identifier
  /// comes from the TemplateID<> class).
  void removeAllComponents(uint64_t entityID, uint64_t compTemplateID);

  template <typename T>
  void removeAllComponentsT(uint64_t entityID)
  {
    removeAllComponents(entityID, TemplateID<T>::getID());
  }

  /// Removes the component for \p entityID at the given index. Usually
  /// used with group systems that count the index until a particular
  /// component.
  void removeComponentAtIndex(uint64_t entityID, int32_t index, uint64_t templateID);

  template <typename T>
  void removeComponentAtIndexT(uint64_t entityID, int32_t index)
  {
    removeComponentAtIndex(entityID, index, TemplateID<T>::getID());
  }

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

  template <typename T>
  void removeFirstComponentT(uint64_t entityID)
  {
    removeFirstComponent(entityID, TemplateID<T>::getID());
  }

  /// Removes the last component with the given \p compTemplateID.
  void removeLastComponent(uint64_t entityID, uint64_t compTemplateID);

  template <typename T>
  void removeLastComponentT(uint64_t entityID)
  {
    removeLastComponent(entityID, TemplateID<T>::getID());
  }
  /// @}

  /// Retrieves the list of static components. You can modify these values at
  /// will until renormalize is called. Use this to update the values of static
  /// components if you don't want to specifically write a system for that.
  /// Will return a nullptr if there are no static components.
  template <typename T>
  typename ComponentContainer<T>::ComponentItem* getStaticComponents()
  {
    BaseComponentContainer* componentContainer = getComponentContainer(TemplateID<T>::getID());
    if (componentContainer != nullptr)
    {
      ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
      return concreteContainer->getComponentArray();
    }
    else
    {
      return nullptr;
    }
  }

  template <typename T>
  T* getStaticComponent(int index = 0)
  {
    BaseComponentContainer* componentContainer = getComponentContainer(TemplateID<T>::getID());
    if (componentContainer != nullptr)
    {
      ComponentContainer<T>* concreteContainer = dynamic_cast<ComponentContainer<T>*>(componentContainer);
      typename ComponentContainer<T>::ComponentItem* components = concreteContainer->getComponentArray();
      int numComp = concreteContainer->getNumComponents();
      if (components != nullptr && index < concreteContainer->getNumComponents())
        return &components[index].component;
      else
        return nullptr;
    }
    else
    {
      return nullptr;
    }
  }

  /// Retrieve static dummy empty BaseComponentContainer implementation.
  static BaseComponentContainer* getEmptyContainer() {return static_cast<BaseComponentContainer*>(&mEmptyContainer);}

  // Note: getNewEntityID is a deprecated function that will be removed in a
  // future version of the entity system.

  /// Returns a, new, valid entity ID. This is a trivial function and should
  /// not be used if you are controlling entity ids using external code.
  uint64_t getNewEntityID() {return ++mCurSequence;}

protected:

  /// Deletes all component containers.
  void deleteAllComponentContainers();

  /// Adds a component. If a component container already exists, then that is
  /// used. Otherwise, a new component container is created and used.
  template <typename T, class CompCont = ComponentContainer<T>>
  void coreAddComponent(uint64_t entityID, const T& component)
  {
    if (entityID == 0)
    {
      std::cerr << "cpm-entity-system: Attempting to add a component of entityID 0! Not allowed." << std::endl;
      throw std::runtime_error("Attempting to add a component of entityID 0.");
      return;
    }

    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T, CompCont>();
    CompCont* concreteContainer = dynamic_cast<CompCont*>(componentContainer);
    concreteContainer->addComponent(entityID, component);
  }

  ///// Same function as above, but we bind to an rvalue reference.
  //template <typename T, class CompCont = ComponentContainer<T>>
  //void coreAddComponent(uint64_t entityID, T&& component)
  //{
  //  if (entityID == 0)
  //  {
  //    std::cerr << "cpm-entity-system: Attempting to add a component of entityID 0! Not allowed." << std::endl;
  //    throw std::runtime_error("Attempting to add a component of entityID 0.");
  //    return;
  //  }

  //  BaseComponentContainer* componentContainer = ensureComponentArrayExists<T, CompCont>();
  //  CompCont* concreteContainer = dynamic_cast<CompCont*>(componentContainer);
  //  concreteContainer->addComponent(entityID, std::move(component));
  //}

  /// Adds a static component. Static components work exactly like normal
  /// components, except that they are not associated with an entityID and are
  /// fed into each system in the same manner. Use this to make 'global'
  /// components (for lighting, view camera position, input, etc...).
  /// Static components and regular components cannot be mixed.
  /// Returns the index of the static component.
  template <typename T, class CompCont = ComponentContainer<T>>
  size_t coreAddStaticComponent(const T& component)
  {
    // If the container isn't already marked as static, mark it and ensure
    // that it is empty.
    BaseComponentContainer* componentContainer = ensureComponentArrayExists<T, CompCont>();
    CompCont* concreteContainer = dynamic_cast<CompCont*>(componentContainer);
    return concreteContainer->addStaticComponent(component);
  }

  ///// Same function as above, but we bind to an rvalue reference.
  //template <typename T, class CompCont = ComponentContainer<T>>
  //size_t coreAddStaticComponent(T&& component)
  //{
  //  // If the container isn't already marked as static, mark it and ensure
  //  // that it is empty.
  //  BaseComponentContainer* componentContainer = ensureComponentArrayExists<T, CompCont>();
  //  CompCont* concreteContainer = dynamic_cast<CompCont*>(componentContainer);
  //  return concreteContainer->addStaticComponent(std::move(component));
  //}

  template <typename T, class CompCont>
  BaseComponentContainer* ensureComponentArrayExists()
  {
    BaseComponentContainer* componentContainer = getComponentContainer(TemplateID<T>::getID());
    if (componentContainer == nullptr)
    {
      componentContainer = new CompCont();
      addComponentContainer(componentContainer, TemplateID<T>::getID());
    }
    return componentContainer;
  }

  std::map<uint64_t, BaseComponentContainer*> mComponents;
  uint64_t                                    mCurSequence;

  static EmptyComponentContainer mEmptyContainer;
};



} // namespace CPM_ES_NS 

#endif 
