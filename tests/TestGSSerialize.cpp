
#include <entity-system/GenericSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>

namespace es = CPM_ES_NS;


namespace {

class GetNameCore;

class ESSerialize
{
public:

  std::shared_ptr<GetNameCore> mCore;
  ESSerialize(std::shared_ptr<GetNameCore> core) : mCore(core) {}

  static int mNumPosCalls;
  static int mNumHomCalls;
  static int mNumGamCalls;

  void pos_serializeVec3(const glm::vec3& vec, uint64_t entityID);
  void hom_serializeVec4(const glm::vec4& vec, uint64_t entityID);
  void gam_serialize(int health, int armor, uint64_t entityID);
};

int ESSerialize::mNumPosCalls = 0;
int ESSerialize::mNumHomCalls = 0;
int ESSerialize::mNumGamCalls = 0;

// We may want to enforce that these components have bson serialization members
// (possibly a static assert?).

struct CompPosition
{
  CompPosition() {}
  CompPosition(const glm::vec3& pos) {position = pos;}

  void checkEqual(const CompPosition& pos) const
  {
    EXPECT_FLOAT_EQ(position.x, pos.position.x);
    EXPECT_FLOAT_EQ(position.y, pos.position.y);
    EXPECT_FLOAT_EQ(position.z, pos.position.z);
  }

  // What this 'struct' is all about -- the data.
  glm::vec3 position;

  // Compile time polymorphic function
  void serialize(ESSerialize& s, uint64_t entityID)
  {
    s.pos_serializeVec3(position, entityID);
  }
};

struct CompHomPos
{
  CompHomPos() {}
  CompHomPos(const glm::vec4& pos) {position = pos;}

  void checkEqual(const CompHomPos& pos) const
  {
    EXPECT_FLOAT_EQ(position.x, pos.position.x);
    EXPECT_FLOAT_EQ(position.y, pos.position.y);
    EXPECT_FLOAT_EQ(position.z, pos.position.z);
    EXPECT_FLOAT_EQ(position.w, pos.position.w);
  }

  // DATA
  glm::vec4 position;

  // Compile time polymorphic function
  void serialize(ESSerialize& s, uint64_t entityID)
  {
    s.hom_serializeVec4(position, entityID);
  }
};

struct CompGameplay
{
  CompGameplay() : health(0), armor(0) {}
  CompGameplay(int healthIn, int armorIn)
  {
    this->health = healthIn;
    this->armor = armorIn;
  }

  void checkEqual(const CompGameplay& gp) const
  {
    EXPECT_EQ(health, gp.health);
    EXPECT_EQ(armor, gp.armor);
  }

  // DATA
  int health;
  int armor;

  // Compile time polymorphic function
  void serialize(ESSerialize& s, uint64_t entityID)
  {
    s.gam_serialize(health, armor, entityID);
  }
};

// Component positions. associated with id. The first component is not used.
static std::vector<CompPosition> posComponents = {
  glm::vec3(0.0, 0.0, 0.0),
  glm::vec3(1.0, 2.0, 3.0),
  glm::vec3(5.5, 6.0, 10.7),
  glm::vec3(1.5, 3.0, 107),
  glm::vec3(4.0, 7.0, 9.0),
  glm::vec3(2.92, 89.0, 4.0),
};

static std::vector<CompHomPos> homPosComponents = {
  glm::vec4(0.0, 0.0, 0.0, 0.0),
  glm::vec4(1.0, 11.0, 41.0, 51.0),
  glm::vec4(2.0, 12.0, 42.0, 52.0),
  glm::vec4(3.0, 13.0, 43.0, 53.0),
  glm::vec4(4.0, 14.0, 44.0, 54.0),
  glm::vec4(5.0, 15.0, 45.0, 55.0),
};

static std::vector<CompGameplay> gameplayComponents = {
  CompGameplay(0, 0),
  CompGameplay(45, 21),
  CompGameplay(23, 123),
  CompGameplay(99, 892),
  CompGameplay(73, 64),
  CompGameplay(23, 92),
};


class SerializeInterface
{
public:
  virtual void serialize(ESSerialize& s) = 0;
};

// http://stackoverflow.com/questions/12509962/static-assert-for-ensuring-design-contract
template <typename T>
class has_serialize_fun
{
    typedef char one;
    typedef long two;

    template <typename C> static one test( decltype(&C::serialize) ) ;
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// We can require that all components have get_name.
template <typename T>
class CustomContainer : public es::ComponentContainer<T>, public SerializeInterface
{
public:

  void serialize(ESSerialize& s) override
  {
    // Enforce the contract that all components must have a serialize function.
    static_assert( has_serialize_fun<T>::value, "Component does not have a serialize function." );

    // In a real function, you will likely want to enforce that the getName
    // function exists.

    // Normally we would serialize a number of values here. In particular,
    // mLastSortedSize and removals / modifications.
    // But instead of serializing all of this information you can just simply
    // renormalize directly before the serialize call and forget about all
    // of that information.

    /// Serializes all component even if they have not been normalized into
    /// the system. See the destructor for ComponentContainer if you only
    /// want to serialize those components currently normalized into the system.
    /// Require the 
    for (auto it = es::ComponentContainer<T>::mComponents.begin(); 
         it != es::ComponentContainer<T>::mComponents.end(); ++it)
    {
      it->component.serialize(s, it->sequence);
    }
  }
  
};

class GetNameCore : public es::ESCoreBase
{
public:
  template <typename T>
  const char* getComponentName();

  template <typename T>
  void addComponent(uint64_t entityID, const T& component)
  {
    coreAddComponent<T, CustomContainer<T>>(entityID, component);
  }

  template <typename T>
  size_t addStaticComponent(const T& component)
  {
    return coreAddStaticComponent<T, CustomContainer<T>>(component);
  }

  // Runs through all components and serializes them.
  void serializeComponents(ESSerialize& s)
  {
    for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
    {
      dynamic_cast<SerializeInterface*>(it->second)->serialize(s);
    }
  }

};

// This basic system will apply, every frame, to entities with the CompPosition,
// CompHomPos, and CompGameplay components.
class BasicSystem : public es::GenericSystem<false, CompPosition, CompHomPos, CompGameplay>
{
public:

  static std::map<uint64_t, bool> invalidComponents;

  void execute(es::ESCoreBase&, uint64_t entityID,
               const CompPosition* pos, const CompHomPos* homPos,
               const CompGameplay* gp) override
  {
    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      FAIL() << "BasicSystem attempt to execute on an invalid component." << std::endl;

    // Check the values contained in each of pos, homPos, and gp.
    pos->checkEqual(posComponents[entityID]);
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);
  }
};

std::map<uint64_t, bool> BasicSystem::invalidComponents;

TEST(EntitySystem, TestSerialize)
{
  // Generate entity system core.
  std::shared_ptr<GetNameCore> core(new GetNameCore());

  uint64_t id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID();
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);
  BasicSystem::invalidComponents.insert(std::make_pair(id, true));

  id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);
  
  std::shared_ptr<BasicSystem> sys(new BasicSystem());

  core->renormalize();
  sys->walkComponents(*core);

  std::shared_ptr<ESSerialize> s(new ESSerialize(core));
  core->serializeComponents(*s);

  EXPECT_EQ(3, ESSerialize::mNumPosCalls);
  EXPECT_EQ(4, ESSerialize::mNumHomCalls);
  EXPECT_EQ(4, ESSerialize::mNumGamCalls);
}

// Seeding:
// time(NULL)

}

void ESSerialize::pos_serializeVec3(const glm::vec3& vec, uint64_t entityID)
{
  // Check to see that we have the right vector.
  EXPECT_FLOAT_EQ(posComponents[entityID].position.x, vec.x);
  EXPECT_FLOAT_EQ(posComponents[entityID].position.y, vec.y);
  EXPECT_FLOAT_EQ(posComponents[entityID].position.z, vec.z);
  ++mNumPosCalls;
}

void ESSerialize::hom_serializeVec4(const glm::vec4& vec, uint64_t entityID)
{
  EXPECT_FLOAT_EQ(homPosComponents[entityID].position.x, vec.x);
  EXPECT_FLOAT_EQ(homPosComponents[entityID].position.y, vec.y);
  EXPECT_FLOAT_EQ(homPosComponents[entityID].position.z, vec.z);
  EXPECT_FLOAT_EQ(homPosComponents[entityID].position.w, vec.w);
  ++mNumHomCalls;
}

void ESSerialize::gam_serialize(int health, int armor, uint64_t entityID)
{
  EXPECT_FLOAT_EQ(gameplayComponents[entityID].health, health);
  EXPECT_FLOAT_EQ(gameplayComponents[entityID].armor, armor);
  ++mNumGamCalls;
}
