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

void ESCore::addComponentContainer(BaseComponentContainer* componentCont, uint64_t componentID)
{
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

void ESCore::iterateOverContainers(std::function<void(BaseComponentContainer*)>& cb)
{
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
  {
    cb(it->second);
  }
}

void ESCore::clearAllComponentContainers()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    delete iter->second;

  mComponents.clear();
}

void ESCore::renormalize(bool stableSort)
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->renormalize(stableSort);
}

void ESCore::removeEntity(uint64_t entityID)
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->removeSequence(entityID);
}

} // namespace CPM_ES_NS

