#ifndef IAUNS_ENTITY_SYSTEM_SPECIFICSYSTEM_HPP
#define IAUNS_ENTITY_SYSTEM_SPECIFICSYSTEM_HPP

#include "../GenericSystem.hpp"

namespace CPM_ES_NS {

/// This system executes any entities found within a general range. This is
/// much more efficient than walking a tagged component. But be very careful when
/// using this class. It adds corner cases to your design that you will need
/// to worry about (such as adding / removing objects from this specific
/// range).

/// More often than not, sever ESCores should be created representing various
/// other states if necessary.

template <typename... Ts>
class SpecificSystem : public GenericSystem<Ts...>
{
public:
  SpecificSystem();
  virtual ~SpecificSystem();
  
  /// Add an entity to update. By default, the system will not remove the
  /// entity when it fails to update. You can change this behavior by setting
  /// autoRemove = true. This will automatically remove the entity when
  /// it fails to update itself.
  void addEntity(uint64_t entityID, bool autoRemove = false);

  /// Removes an entity from the update cycle.
  void removeEntity(uint64_t entityID);

  /// If you have a pool of entities in a specific range which the pool should
  /// only apply to, then use this function. It is vastly more efficient at
  /// updating multiple entities than the 'addEntity' function above. But you
  /// are responsible for correctly managing entity ids so that they match
  /// the correct ranges.
  /// You can add as many ranges as you want, just be careful not to 
  void addEntityRange(uint64_t lower, uint64_t upper);

private:
};

} // namespace CPM_ES_NS 

#endif 
