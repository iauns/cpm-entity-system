#ifndef IAUNS_ENTITY_SYSTEM_BASECOMPONENTCONTAINER_HPP
#define IAUNS_ENTITY_SYSTEM_BASECOMPONENTCONTAINER_HPP

class ESSerialize;

namespace CPM_ES_NS {

// Component base class, used to verify component types at run-time.
class BaseComponentContainer
{
public:
  BaseComponentContainer()           {}
  virtual ~BaseComponentContainer()  {}
  
  virtual void renormalize(bool stableSort) = 0;

  /// Get the least sequence held by the component.
  virtual uint64_t getLowerSequence() = 0;

  /// Get the upper sequence held by the component.
  virtual uint64_t getUpperSequence() = 0;

  /// Get number of components.
  virtual uint64_t getNumComponents() = 0;

  /// Remove components identified with \p sequence. Every component associated
  /// with the sequnece (if there are multiple) will be removed.
  virtual void removeSequence(uint64_t sequence) = 0;

  /// Removes the first component found that is associated with 'sequence'.
  virtual void removeFirstSequence(uint64_t sequence) = 0;

  /// Removes the last component found that is associated with 'sequence'.
  virtual void removeLastSequence(uint64_t sequence) = 0;

  /// Retrieves the name of the component. Useful in drawing up serialization
  /// schemes to disk or between computers. Returns nullptr if the getName
  /// static function is not implemented by the component.
  /// DEPRECATED: This is application specific. Functionality can be replicated
  /// once we finalize a more flexible ComponentContainer system.
  virtual const char* getComponentName() = 0;

  /// Serializes all components associated with this BaseComponentContainer.
  /// SerializeBase is not defined by cpm-entity-system. It must be defined
  /// in your code if you use this function. The function that will be called
  /// on the component is: serialize(ESSerializeBase*, uint64_t entityID).
  /// This also serializes components that have not yet been normalized
  /// and inserted into the system. It is best to renormalize the system
  /// before calling this function.
  /// DEPRECATED: This is application specific. Functionality can be replicated
  /// once we finalize a more flexible ComponentContainer system.
  virtual void serializeComponents(ESSerialize& s) = 0;

  /// Returns true if the component system contains only static elements.
  /// These elements values are always the same regardless of the entity
  /// executing. Use for global values.
  virtual bool isStatic() = 0;

  /// Retrieves the sequence associated with the given index.
  /// Index must be in [0, getNumComponents()).
  /// Be careful when using this function, cache misses are likely if you
  /// aren't walking in-order.
  /// If the index is not present, the function returns 0 (an invalid sequence).
  virtual uint64_t getSequenceFromIndex(int index) = 0;

  static const int StaticEntID = 1;

};

} // namespace CPM_ES_NS 

#endif 
