
#include <entity-system/GenericSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <ctime>
#include <random>
#include <glm/glm.hpp>

namespace es = CPM_ES_NS;

// Global variable from tests/main.cpp -- the random seed that we should use.
extern uint64_t gRandomSeed;

namespace {

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

std::vector<CompPosition> posComponents;
std::vector<CompHomPos> homPosComponents;
std::vector<CompGameplay> gameplayComponents;

// This basic system will apply, every frame, to entities with the CompPosition,
// CompHomPos, and CompGameplay components.
class BasicSystem : public es::GenericSystem<CompPosition, CompHomPos, CompGameplay>
{
public:

  static std::map<uint64_t, bool> invalidComponents;
  static std::vector<uint64_t> calledEntities;

  void execute(uint64_t entityID,
               const CompPosition* pos, const CompHomPos* homPos,
               const CompGameplay* gp) override
  {
    //std::cout << "Executing entity: " << entityID << std::endl;

    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      FAIL() << "BasicSystem attempt to execute on an invalid component." << std::endl;

    calledEntities.push_back(entityID);

    // Check the values contained in each of pos, homPos, and gp.
    pos->checkEqual(posComponents[entityID]);
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);
  }
};

std::map<uint64_t, bool> BasicSystem::invalidComponents;
std::vector<uint64_t> BasicSystem::calledEntities;

TEST(EntitySystem, Rand)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

  std::mt19937 randEngine(gRandomSeed);

  std::vector<uint64_t> validEntities;


  const int numIterations = 400;
  const int numComponentsTotal = 500;

  std::uniform_int_distribution<int> dist100(0, 100);
  std::uniform_real_distribution<double> distReal(-100.0,100.0);

  auto gen100 = std::bind(dist100, std::ref(randEngine));
  auto genReal = std::bind(distReal, std::ref(randEngine));

  for (int i = 0; i < numIterations; ++i)
  {
    core->clearAllComponentContainers();

    BasicSystem::invalidComponents.clear();
    BasicSystem::calledEntities.clear();

    validEntities.clear();

    posComponents.clear();
    homPosComponents.clear();
    gameplayComponents.clear();

    // Test that adding BasicSystem also adds component containers for all data
    // components.
    std::shared_ptr<BasicSystem> sys(new BasicSystem);

    posComponents.emplace_back(glm::vec3(genReal(), genReal(), genReal()));
    homPosComponents.emplace_back(glm::vec4(genReal(), genReal(), genReal(), genReal()));
    gameplayComponents.emplace_back(0,0);
    for (int j = 0; j < numComponentsTotal; ++j)
    {
      bool failed = false;
      int entityID = j + 1;

      // Check to see if we should add this component to the core.
      posComponents.emplace_back(glm::vec3(genReal(), genReal(), genReal()));
      if (gen100() > 20)
      {
        // Add the component.
        core->addComponent(entityID, posComponents[entityID]);
      }
      else
      {
        failed = true;
        if (BasicSystem::invalidComponents.find(entityID) == BasicSystem::invalidComponents.end())
          BasicSystem::invalidComponents.insert(std::make_pair(entityID, true));
      }

      // Check to see if we should add this component to the core.
      homPosComponents.emplace_back(glm::vec4(genReal(), genReal(), genReal(), genReal()));
      if (gen100() > 20)
      {
        // Add the component.
        core->addComponent(entityID, homPosComponents[entityID]);
      }
      else
      {
        failed = true;
        if (BasicSystem::invalidComponents.find(entityID) == BasicSystem::invalidComponents.end())
          BasicSystem::invalidComponents.insert(std::make_pair(entityID, true));
      }

      // Check to see if we should add this component to the core.
      gameplayComponents.emplace_back(gen100(), gen100());
      if (gen100() > 20)
      {
        // Add the component.
        core->addComponent(entityID, gameplayComponents[entityID]);
      }
      else
      {
        failed = true;
        if (BasicSystem::invalidComponents.find(entityID) == BasicSystem::invalidComponents.end())
          BasicSystem::invalidComponents.insert(std::make_pair(entityID, true));
      }

      if (!failed)
      {
        validEntities.push_back(entityID);
      }
    }

    core->renormalize();
    sys->walkComponents(*core);

    // Ensure all systems which should have been called got called.
    if (validEntities.size() == BasicSystem::calledEntities.size())
    {
      // Check to see if the two vectors are equal.
      if (!std::equal(validEntities.begin(), validEntities.begin() + validEntities.size(),
                      BasicSystem::calledEntities.begin()))
        FAIL() << "Incorrect entities were called!";
    }
    else
    {
      FAIL() << "Failed to call valid entities.";
    }
  }
}

// Seeding:
// time(NULL)

}

