
#include <entity-system/GenericSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>

// Test construction / destruction (componentDestruct / componentDestruct).

namespace es = CPM_ES_NS;

namespace {

// We may want to enforce that these components have bson serialization members
// (possibly a static assert?).

struct CompPosition
{
  static int constructor;
  static int destructor;

  CompPosition() {}
  CompPosition(const glm::vec3& pos) {position = pos;}

  void checkEqual(const CompPosition& pos) const
  {
    EXPECT_FLOAT_EQ(position.x, pos.position.x);
    EXPECT_FLOAT_EQ(position.y, pos.position.y);
    EXPECT_FLOAT_EQ(position.z, pos.position.z);
  }

  void componentConstruct(uint64_t /*id*/)
  {
    ++constructor;
  }

  void componentDestruct(uint64_t /*id*/)
  {
    ++destructor;
  }

  // What this 'struct' is all about -- the data.
  glm::vec3 position;
};

int CompPosition::constructor = 0;
int CompPosition::destructor = 0;

struct CompHomPos
{
  static int constructor;
  static uint64_t lastID;

  CompHomPos() {}
  CompHomPos(const glm::vec4& pos) {position = pos;}

  void checkEqual(const CompHomPos& pos) const
  {
    EXPECT_FLOAT_EQ(position.x, pos.position.x);
    EXPECT_FLOAT_EQ(position.y, pos.position.y);
    EXPECT_FLOAT_EQ(position.z, pos.position.z);
    EXPECT_FLOAT_EQ(position.w, pos.position.w);
  }

  void componentConstruct(uint64_t id)
  {
    ++constructor;
    EXPECT_EQ(id, lastID + 1);
    lastID = id;
  }

  // DATA
  glm::vec4 position;
};

int CompHomPos::constructor = 0;
uint64_t CompHomPos::lastID = 0;

struct CompGameplay
{
  static int destructor;

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

  void componentDestruct(uint64_t /*id*/)
  {
    ++destructor;
  }

  // DATA
  int health;
  int armor;
};

int CompGameplay::destructor = 0;

// Component positions. associated with id. The first component is not used.
std::vector<CompPosition> posComponents = {
  glm::vec3(0.0, 0.0, 0.0),
  glm::vec3(1.0, 2.0, 3.0),
  glm::vec3(5.5, 6.0, 10.7),
  glm::vec3(1.5, 3.0, 107),
  glm::vec3(4.0, 7.0, 9.0),
  glm::vec3(2.92, 89.0, 4.0),
};

std::vector<CompHomPos> homPosComponents = {
  glm::vec4(0.0, 0.0, 0.0, 0.0),
  glm::vec4(1.0, 11.0, 41.0, 51.0),
  glm::vec4(2.0, 12.0, 42.0, 52.0),
  glm::vec4(3.0, 13.0, 43.0, 53.0),
  glm::vec4(4.0, 14.0, 44.0, 54.0),
  glm::vec4(5.0, 15.0, 45.0, 55.0),
};

std::vector<CompGameplay> gameplayComponents = {
  CompGameplay(0, 0),
  CompGameplay(45, 21),
  CompGameplay(23, 123),
  CompGameplay(99, 892),
  CompGameplay(73, 64),
  CompGameplay(23, 92),
};

// This basic system will apply, every frame, to entities with the CompPosition,
// CompHomPos, and CompGameplay components.
class BasicSystem : public es::GenericSystem<CompPosition, CompHomPos, CompGameplay>
{
public:

  static std::map<uint64_t, bool> invalidComponents;

  void execute(uint64_t entityID,
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

TEST(EntitySystem, BasicDestructionConstruction)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

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

  EXPECT_EQ(3, CompPosition::constructor);
  EXPECT_EQ(4, CompHomPos::constructor);

  core.reset();

  EXPECT_EQ(3, CompPosition::destructor);
  EXPECT_EQ(4, CompGameplay::destructor);
  
}

// Seeding:
// time(NULL)

}

