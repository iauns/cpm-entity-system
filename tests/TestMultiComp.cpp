
#include <entity-system/GenericSystem.hpp>
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
  glm::vec3(0.0, 0.0, 0.0),           // 0
  glm::vec3(1.0, 2.0, 3.0),           // 1
  glm::vec3(5.5, 6.0, 10.7),          // 2
  glm::vec3(1.5, 3.0, 107),           // 3
  glm::vec3(4.0, 7.0, 9.0),           // 4
  glm::vec3(2.92, 89.0, 4.0),         // 5
  glm::vec3(3.92, 9.0, 9.2),          // 6
};

std::vector<CompHomPos> homPosComponents = {
  glm::vec4(0.0, 0.0, 0.0, 0.0),      // 0
  glm::vec4(1.0, 11.0, 41.0, 51.0),   // 1
  glm::vec4(2.0, 12.0, 42.0, 52.0),   // 2
  glm::vec4(3.0, 13.0, 43.0, 53.0),   // 3
  glm::vec4(4.0, 14.0, 44.0, 54.0),   // 4
  glm::vec4(5.0, 15.0, 45.0, 55.0),   // 5
  glm::vec4(6.0, 5.0, 4.0, 8.0),      // 6
};

std::vector<CompGameplay> gameplayComponents = {
  CompGameplay(0, 0),                 // 0
  CompGameplay(45, 21),               // 1
  CompGameplay(23, 123),              // 2
  CompGameplay(99, 892),              // 3
  CompGameplay(73, 64),               // 4
  CompGameplay(23, 92),               // 5
  CompGameplay(3, 73),                // 6
};

// This basic system will apply, every frame, to entities with the CompPosition,
// CompHomPos, and CompGameplay components.
class BasicSystem : public es::GenericSystem<CompPosition, CompHomPos, CompGameplay>
{
public:

  static std::map<uint64_t, bool> invalidComponents;

  static int numEntity1Calls;
  static int numEntity3Calls;
  static int numEntity4Calls;

  void execute(uint64_t entityID, const CompPosition* pos,
               const CompHomPos* homPos, const CompGameplay* gp) override
  {
    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      FAIL() << "BasicSystem attempt to execute on an invalid component." << std::endl;

    if (entityID == 1)
    {
      if (numEntity1Calls == 0)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity1Calls == 1)
      {
        pos->checkEqual(posComponents[entityID + 2]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity1Calls == 2)
      {
        pos->checkEqual(posComponents[entityID + 4]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity1Calls == 3)
      {
        pos->checkEqual(posComponents[entityID + 5]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }

      ++numEntity1Calls;
    }
    else if (entityID == 3)
    {
      if (numEntity3Calls == 0)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity3Calls == 1)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID + 1]);
      }
      else if (numEntity3Calls == 2)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID + 2]);
      }
      else if (numEntity3Calls == 3)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID + 2]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity3Calls == 4)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID + 2]);
        gp->checkEqual(gameplayComponents[entityID + 1]);
      }
      else if (numEntity3Calls == 5)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID + 2]);
        gp->checkEqual(gameplayComponents[entityID + 2]);
      }
      ++numEntity3Calls;
    }
    else if (entityID == 4)
    {
      // Ensure we are called 4 times corresponding to the cartesian product
      // of the position components and the gameplay components.
      if (numEntity4Calls == 0)
      {
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity4Calls == 1)
      {
        // Those components that are mentioned last in the GenericSystem
        // template are iterated over first (depth first recursion).
        pos->checkEqual(posComponents[entityID]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID + 1]);
      }
      else if (numEntity4Calls == 2)
      {
        pos->checkEqual(posComponents[entityID + 1]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID]);
      }
      else if (numEntity4Calls == 3)
      {
        pos->checkEqual(posComponents[entityID + 1]);
        homPos->checkEqual(homPosComponents[entityID]);
        gp->checkEqual(gameplayComponents[entityID + 1]);
      }
      ++numEntity4Calls;
    }
    else
    {
      // Check the values contained in each of pos, homPos, and gp.
      pos->checkEqual(posComponents[entityID]);
      homPos->checkEqual(homPosComponents[entityID]);
      gp->checkEqual(gameplayComponents[entityID]);
    }
  }
};

std::map<uint64_t, bool> BasicSystem::invalidComponents;
int BasicSystem::numEntity1Calls = 0;
int BasicSystem::numEntity3Calls = 0;
int BasicSystem::numEntity4Calls = 0;

TEST(EntitySystem, MultiComp)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

  uint64_t id = core->getNewEntityID(); // 1
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, posComponents[id + 2]);  // 3
  core->addComponent(id, posComponents[id + 4]);  // 5
  core->addComponent(id, posComponents[id + 5]);  // 6
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID(); // 2
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, homPosComponents[id + 1]);
  core->addComponent(id, gameplayComponents[id]);
  core->addComponent(id, gameplayComponents[id + 2]);
  BasicSystem::invalidComponents.insert(std::make_pair(id, true));

  id = core->getNewEntityID(); // 3
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, homPosComponents[id + 2]);   // 5
  core->addComponent(id, gameplayComponents[id]);
  core->addComponent(id, gameplayComponents[id + 1]); // 4
  core->addComponent(id, gameplayComponents[id + 2]); // 5

  id = core->getNewEntityID(); // 4
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, posComponents[id + 1]); // 5
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);
  core->addComponent(id, gameplayComponents[id + 1]); // 5
  
  std::shared_ptr<BasicSystem> sys(new BasicSystem());

  core->renormalize(true);    // We want a stable sort
  sys->walkComponents(*core);

  // Ensure that the appropriate number of calls have been made.
  EXPECT_EQ(4, BasicSystem::numEntity1Calls);
  EXPECT_EQ(6, BasicSystem::numEntity3Calls);
  EXPECT_EQ(4, BasicSystem::numEntity4Calls);
}

// Seeding:
// time(NULL)

}

