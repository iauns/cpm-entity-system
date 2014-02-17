/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2014 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/// \author James Hughes
/// \date   February 2014

#include "ESCoreBase.hpp"

namespace CPM_ES_NS {

EmptyComponentContainer ESCoreBase::mEmptyContainer;

bool ESCoreBase::hasComponentContainer(uint64_t componentID) const
{
  auto it = mComponents.find(componentID);
  if (it == mComponents.end())
    return false;
  else
    return true;
}

/// When called, ESCoreBase takes ownership of \p component.
/// Adds a new component to the system. If a component of the same
/// TypeID already exists, the request is ignored.
void ESCoreBase::addComponentContainer(BaseComponentContainer* componentCont, uint64_t componentID)
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

/// Retrieves a base component container. Component is the output from
/// the TemplateID class.
BaseComponentContainer* ESCoreBase::getComponentContainer(uint64_t component)
{
  auto it = mComponents.find(component);
  if (it != mComponents.end())
    return (it->second);
  else
    return nullptr;
}

/// Clears out all component containers (deletes all entities).
void ESCoreBase::clearAllComponentContainers()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->removeAll();
}

void ESCoreBase::deleteAllComponentContainers()
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    delete iter->second;

  mComponents.clear();
}

/// Call this function at the beginning or end of every frame. It renormalizes
/// all your components (adds / removes components). To call a system, execute
/// the walkComponents function on BaseSystem. Most systems don't need a
/// stable sort. But if you need to guarantee the relative order of multiple 
/// components with the same entity ID, use stable sort.
void ESCoreBase::renormalize(bool stableSort)
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->renormalize(stableSort);
}

/// Removes all components associated with entity.
void ESCoreBase::removeEntity(uint64_t entityID)
{
  for (auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
    iter->second->removeSequence(entityID);
}

void ESCoreBase::removeFirstComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeFirstSequence(entityID);
}

void ESCoreBase::removeLastComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeLastSequence(entityID);
}

void ESCoreBase::removeComponentAtIndex(uint64_t entityID, int32_t index, uint64_t templateID)
{
  BaseComponentContainer* cont = getComponentContainer(templateID);
  cont->removeSequenceWithIndex(entityID, index);
}

void ESCoreBase::removeAllComponents(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeSequence(entityID);
}

} // namespace CPM_ES_NS


