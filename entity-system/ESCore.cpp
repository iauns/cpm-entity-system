#include "ESCore.hpp"
#include <iostream>

namespace CPM_ES_NS {

ESCore::ESCore() :
    mCurSequence(0)
{
}

ESCore::~ESCore()
{
  clearAllComponentContainers();
}

bool ESCore::hasComponentContainer(uint64_t componentID)
{
  auto it = mComponents.find(componentID);
  if (it == mComponents.end())
    return false;
  else
    return true;
}

BaseComponentContainer* ESCore::getComponentContainer(uint64_t component)
{
  auto it = mComponents.find(component);
  if (it != mComponents.end())
    return (it->second);
  else
    return nullptr;
}

void ESCore::addComponentContainer(BaseComponentContainer* componentCont)
{
  uint64_t componentID = componentCont->getComponentID();
  auto it = mComponents.find(componentID);
  if (it == mComponents.end())
  {
    mComponents.insert(std::make_pair(componentID, componentCont));
  }
  else
  {
    std::cerr << "cpm-entity-system - Warning: Attempting to add pre-existing component container!" << std::endl;
    delete componentCont;
  }
}

void ESCore::removeEntity(uint64_t entityID)
{
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
  {
    it->second->removeSequence(entityID);
  }
}

void ESCore::removeFirstComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeFirstSequence(entityID);
}

void ESCore::removeLastComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeLastSequence(entityID);
}

void ESCore::removeAllComponents(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeSequence(entityID);
}

void ESCore::clearAllComponentContainers()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    delete iter->second;

  mComponents.clear();
}

void ESCore::renormalize(bool stableSort)
{
  // Clean up each component container in the system. Any number of
  // components could have been added to the system, and placed in waiting
  // without being sorted into the container. Renormalizes ensures all
  // components that requested deletion are removed, and all added components
  // are sorted into the component container.
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
  {
    it->second->renormalize(stableSort);
  }
}

} // namespace CPM_ES_NS

