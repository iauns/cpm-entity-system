#ifndef IAUNS_ENTITY_SYSTEM_EMPTYCOMPONENTCONTAINER_HPP
#define IAUNS_ENTITY_SYSTEM_EMPTYCOMPONENTCONTAINER_HPP

namespace CPM_ES_NS {

// Component base class, used to verify component types at run-time.
class EmptyComponentContainer : public BaseComponentContainer
{
public:
  EmptyComponentContainer()           {}
  virtual ~EmptyComponentContainer()  {}
  
  void renormalize(bool stableSort) override {}
  uint64_t getLowerSequence() override {return 0;}
  uint64_t getUpperSequence() override {return 0;}
  uint64_t getNumComponents() override {return 0;}
  void removeSequence(uint64_t) override {}
  void removeFirstSequence(uint64_t) override {}
  void removeLastSequence(uint64_t) override {}
  bool isStatic() override {return false;}
  uint64_t getSequenceFromIndex(int) override {return 0;}
};

} // namespace CPM_ES_NS 

#endif 
