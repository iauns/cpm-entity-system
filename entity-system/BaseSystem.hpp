#ifndef IAUNS_ENTITY_SYSTEM_BASESYSTEM_HPP
#define IAUNS_ENTITY_SYSTEM_BASESYSTEM_HPP

namespace CPM_ES_NS {

class CoreInt;

class BaseSystem
{
public:
  BaseSystem() {}
  virtual ~BaseSystem() {}
  
  virtual void walkComponents(CoreInt& core) = 0;
  virtual bool walkEntity(CoreInt& core, uint64_t entityID) = 0;

  virtual const char* getID() {return nullptr;}
};

} // namespace CPM_ES_NS

#endif 
