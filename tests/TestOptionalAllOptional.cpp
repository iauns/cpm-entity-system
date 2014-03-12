
#include <entity-system/GenericSystem.hpp>
#include <entity-system/ESCore.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>

namespace es = CPM_ES_NS;

namespace {

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
};

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
class BasicSystem : public es::GenericSystem<false, CompPosition, CompHomPos, CompGameplay>
{
public:

  static int posNull;
  static int homNull;
  static int gameNull;

  static int posCall;
  static int homCall;
  static int gameCall;

  bool isComponentOptional(uint64_t /*templateID*/)
  {
    // Everything is optional -- this is a corner case test.
    return true;
  }

  void execute(es::ESCoreBase&, uint64_t entityID, const CompPosition* pos,
               const CompHomPos* homPos, const CompGameplay* gp) override
  {
    if (pos == nullptr) ++posNull;
    else {pos->checkEqual(posComponents[entityID]); ++posCall;}

    if (homPos == nullptr) ++homNull;
    else {homPos->checkEqual(homPosComponents[entityID]); ++homCall;}

    if (gp == nullptr) ++gameNull;
    else {gp->checkEqual(gameplayComponents[entityID]); ++gameCall;}

    if (entityID == 1)
    {
      EXPECT_EQ(nullptr, pos);
      EXPECT_EQ(nullptr, homPos);
      EXPECT_EQ(true, gp != nullptr);
    }

    if (entityID == 2)
    {
      EXPECT_EQ(nullptr, pos);
      EXPECT_EQ(true, homPos != nullptr);
      EXPECT_EQ(true, gp != nullptr);
    }

    if (entityID == 3)
    {
      EXPECT_EQ(nullptr, homPos);
      EXPECT_EQ(true, pos != nullptr);
      EXPECT_EQ(true, gp != nullptr);
    }

    if (entityID == 4)
    {
      EXPECT_EQ(nullptr, gp);
      EXPECT_EQ(true, pos != nullptr);
      EXPECT_EQ(true, homPos != nullptr);
    }
  }
};

int BasicSystem::posNull = 0;
int BasicSystem::homNull = 0;
int BasicSystem::gameNull = 0;

int BasicSystem::posCall = 0;
int BasicSystem::homCall = 0;
int BasicSystem::gameCall = 0;

TEST(EntitySystem, OptionalAllWalkCornerCase)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

  uint64_t id = core->getNewEntityID() - 1; // id = 1
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID() - 1;
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID() - 1;
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID() - 1;
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  
  std::shared_ptr<BasicSystem> sys(new BasicSystem());

  core->renormalize();
  sys->walkComponents(*core);

  EXPECT_EQ(2, BasicSystem::posNull);
  EXPECT_EQ(2, BasicSystem::homNull);
  EXPECT_EQ(1, BasicSystem::gameNull);

  EXPECT_EQ(2, BasicSystem::posCall);
  EXPECT_EQ(2, BasicSystem::homCall);
  EXPECT_EQ(3, BasicSystem::gameCall);
}

// Seeding:
// time(NULL)

}

