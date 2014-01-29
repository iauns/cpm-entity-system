#ifndef IAUNS_ENTITY_SYSTEM_ESCORE_HPP
#define IAUNS_ENTITY_SYSTEM_ESCORE_HPP

#include <map>
#include <list>
#include <iostream>
#include <stdexcept>
#include "BaseSystem.hpp"
#include "ContainerMapInterface.hpp"
#include "src/ComponentContainer.hpp"

namespace CPM_ES_NS {

/// An example of creating a component container map using a std::map.
/// Feel free to use this class instead of rolling your own.
class ESCore : public ContainerMapInterface
{
public:
  ESCore();
  virtual ~ESCore();

  /// Returns false if the component doesn't exist.
  bool hasComponentContainer(uint64_t component) override;

  /// When called, ESCore takes ownership of \p component.
  /// Adds a new component to the system. If a component of the same
  /// TypeID already exists, the request is ignored.
  void addComponentContainer(BaseComponentContainer* component) override;

  /// Retrieves a base component container. Component is the output from
  /// the TemplateID class.
  BaseComponentContainer* getComponentContainer(uint64_t component) override;

  /// Iterates over all containers. No ordering is presumed.
  void iterateOverContainers(std::function<void(BaseComponentContainer*)>& cb) override;

  /// Clears out all component containers (deletes all entities).
  void clearAllComponentContainers();

  /// Returns a, new, valid entity ID. This is a trivial function and should
  /// not be used if you are controlling entity ids using external code.
  uint64_t getNewEntityID() {return ++mCurSequence;}

private:

  std::map<uint64_t, BaseComponentContainer*> mComponents;
  uint64_t                                    mCurSequence;
};

} // namespace CPM_ES_NS 

#endif 
