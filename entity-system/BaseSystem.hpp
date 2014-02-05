#ifndef IAUNS_ENTITY_SYSTEM_BASESYSTEM_HPP
#define IAUNS_ENTITY_SYSTEM_BASESYSTEM_HPP

namespace CPM_ES_NS {

class ESCoreBase;

class BaseSystem
{
public:
  BaseSystem()          {}
  virtual ~BaseSystem() {}
  
  virtual void walkComponents(ESCoreBase& core) = 0;
  virtual bool walkEntity(ESCoreBase& core, uint64_t entityID) = 0;
};

} // namespace CPM_ES_NS

#endif 
