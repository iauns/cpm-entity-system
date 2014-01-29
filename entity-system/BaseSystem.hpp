#ifndef IAUNS_ENTITY_SYSTEM_BASESYSTEM_HPP
#define IAUNS_ENTITY_SYSTEM_BASESYSTEM_HPP

namespace CPM_ES_NS {

class ESCore;

class BaseSystem
{
public:
  BaseSystem() {}
  virtual ~BaseSystem() {}
  
  virtual void walkComponents(ESCore& core) = 0;
  virtual bool walkEntity(ESCore& core, uint64_t entityID) = 0;

  virtual uint64_t getUniqueID() {return 0;}
};

} // namespace CPM_ES_NS

#endif 
