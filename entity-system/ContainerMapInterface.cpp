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
/// \date   January 2014

#include "ContainerMapInterface.hpp"

namespace CPM_ES_NS {

void ContainerMapInterface::removeFirstComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeFirstSequence(entityID);
}

void ContainerMapInterface::removeLastComponent(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeLastSequence(entityID);
}

void ContainerMapInterface::removeAllComponents(uint64_t entityID, uint64_t compTemplateID)
{
  BaseComponentContainer* cont = getComponentContainer(compTemplateID);
  cont->removeSequence(entityID);
}

static void removeEntityImpl(BaseComponentContainer* cont, uint64_t entityID)
{
  cont->removeSequence(entityID);
}

void ContainerMapInterface::removeEntity(uint64_t entityID)
{
  iterateOverContainers(std::bind(removeEntityImpl, std::placeholders::_1, entityID));
}

static void renormalizeImpl(BaseComponentContainer* cont, bool stableSort)
{
  cont->renormalize(stableSort);
}

void ContainerMapInterface::renormalize(BaseComponentContainer* cont, bool stableSort)
{
  iterateOverContainers(std::bind(renormalizeImpl, std::placeholders::_1, stableSort));
}

} // namespace CPM_ES_NS

