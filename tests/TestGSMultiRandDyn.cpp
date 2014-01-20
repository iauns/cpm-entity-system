
#include <entity-system/GenericSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <ctime>
#include <random>
#include <glm/glm.hpp>


// Global variable from tests/main.cpp -- the random seed that we should use.
extern uint64_t gRandomSeed;
extern uint64_t gRandomSeed;

namespace es = CPM_ES_NS;

namespace {

// We may want to enforce that these components have bson serialization members
// (possibly a static assert?).

typedef std::function<int()>    IntRandFun;
typedef std::function<double()> DoubleRandFun;

struct CompPosition
{
  CompPosition() {}
  CompPosition(IntRandFun /*irand*/, DoubleRandFun drand)
  {
    position = glm::vec3(drand(), drand(), drand());
  }

  bool checkEqual(const CompPosition* pos)
  {
    if (pos == nullptr) return false;
    return (position.x == pos->position.x)
        && (position.y == pos->position.y)
        && (position.z == pos->position.z);
  }

  //// Called when object is pooled / depooled. These are necessary because
  //// pooled objects don't, and can't, have an explicit construct / destructor.
  //void componentConstruct(uint64_t entityID)
  //{
  //  
  //}

  //void componentDestroy(uint64_t entityID)
  //{
  //  
  //}

  // What this 'struct' is all about -- the data.
  glm::vec3 position;
};

struct CompHomPos
{
  CompHomPos() {}
  CompHomPos(IntRandFun /*irand*/, DoubleRandFun drand)
  {
    position = glm::vec4(drand(), drand(), drand(), drand());
  }

  bool checkEqual(const CompHomPos* pos)
  {
    if (pos == nullptr) return false;
    return (position.x == pos->position.x)
        && (position.y == pos->position.y)
        && (position.z == pos->position.z)
        && (position.w == pos->position.w);
  }

  // DATA
  glm::vec4 position;
};

struct CompGameplay
{
  CompGameplay() {}
  CompGameplay(IntRandFun irand, DoubleRandFun /*drand*/)
  {
    health = irand();
    armor = irand();
  }

  bool checkEqual(const CompGameplay* gp)
  {
    if (gp == nullptr) return false;
    return (health == gp->health)
        && (armor == gp->armor);
  }

  // DATA
  int health;
  int armor;
};

struct CompTest1
{
  CompTest1() {}
  CompTest1(IntRandFun irand, DoubleRandFun /*drand*/)
  {
    t1 = irand();
  }

  bool checkEqual(const CompTest1* t)
  {
    if (t == nullptr) return false;
    return (t1 == t->t1);
  }

  int64_t t1;

};

struct CompTest2
{
  CompTest2() {}
  CompTest2(IntRandFun irand, DoubleRandFun drand)
  {
    t1 = irand();
    t2 = drand();
  }

  bool checkEqual(const CompTest2* t)
  {
    if (t == nullptr) return false;
    return (t1 == t->t1)
        && (t2 == t->t2);
  }

  int64_t t1;
  double t2;
};

struct CompTest3
{
  CompTest3() {}
  CompTest3(IntRandFun /*irand*/, DoubleRandFun drand)
  {
    v3 = glm::vec3(drand(), drand(), drand());
    v4 = glm::vec4(drand(), drand(), drand(), drand());
  }

  bool checkEqual(const CompTest3* t)
  {
    if (t == nullptr) return false;
    return (v3.x == t->v3.x)
        && (v3.y == t->v3.y)
        && (v3.z == t->v3.z)
        && (v4.x == t->v4.x)
        && (v4.y == t->v4.y)
        && (v4.z == t->v4.z)
        && (v4.w == t->v4.w);
  }

  glm::vec3 v3;
  glm::vec4 v4;
};

// Companent container that systems can use to check whether the components
// they are executing is correct.
template <typename T>
struct ComponentItem
{
  static ComponentItem<T>* mInstance;

  static ComponentItem<T>* instance()
  {
    if (mInstance == nullptr)
      mInstance = new ComponentItem<T>();

    return mInstance;
  }

  std::vector<uint64_t> failedComponents;  ///< A sorted list of all failed components.
  std::map<uint64_t, T> components;

  void clear()
  {
    failedComponents.clear();
    components.clear();
  }

  void generateOneRandomComponent(int entityID, std::shared_ptr<es::ESCore> core,
                                  IntRandFun irand, DoubleRandFun drand,
                                  IntRandFun gen100)
  {
    int entityGen = gen100();
    if (entityGen > 40)
    {
      T thisComponent = T(irand, drand);
      components.insert(std::make_pair(entityID, thisComponent));
      core->addComponent(entityID, thisComponent);
    }
    else
    {
      failedComponents.push_back(entityID);
    }
  }

  void genRandom(std::shared_ptr<es::ESCore> core, int numComponents,
                 IntRandFun irand, DoubleRandFun drand, IntRandFun gen100)
  {
    failedComponents.clear();
    components.clear();

    for (int i = 0; i < numComponents; ++i)
    {
      int entityID = i + 1;
      generateOneRandomComponent(entityID, core, irand, drand, gen100);
    }
  }

  void makeSequenceInvalid(uint64_t sequence)
  {
    if (!std::binary_search(failedComponents.begin(), failedComponents.end(), sequence))
    {
      // The sequence is *not* in our failed components. We need to insert the
      // failed component int failedComponents (just resort), and remove it from
      // components.
      failedComponents.push_back(sequence);
      std::sort(failedComponents.begin(), failedComponents.end());
      
      // Now erase sequence from components.
      components.erase(sequence);
    }
  }

  T* getComponent(uint64_t entityID)
  {
    auto comp = components.find(entityID);
    if (comp != components.end())
      return &comp->second;
    else
      return nullptr;
  }

  bool isFailedComponent(uint64_t id)
  {
    return std::binary_search(failedComponents.begin(), failedComponents.end(), id);
  }
};

template <typename T> ComponentItem<T>* ComponentItem<T>::mInstance = nullptr;

template <typename... Ts>
class TestSystem : public es::GenericSystem<Ts...>
{
public:
  void execute(uint64_t entityID, Ts*... args)
  {
    executedItems.push_back(entityID);
    
    // Detect any failed components that should not be tested.
    std::array<bool, sizeof...(Ts)> detectFailures = {{
      ComponentItem<Ts>::instance()->isFailedComponent(entityID)...
    }};
    for (size_t i = 0; i < sizeof...(Ts); ++i)
    {
      ASSERT_EQ(detectFailures[i], false);
    }

    // Ensure all components are their correct values.
    std::array<bool, sizeof...(Ts)> checks = {{args->checkEqual(ComponentItem<Ts>::instance()->getComponent(entityID))...}};
    for (size_t i = 0; i < sizeof...(Ts); ++i)
    {
      EXPECT_EQ(checks[i], true);
    }
  }

  // Run through all items that should have been executed and make sure they
  // were in our executed list.
  void ensureValidComponentsExecuted(int numComponents)
  {
    for (int i = 0; i < numComponents; ++i)
    {
      int entityID = i + 1;

      // Find out if each component is present, and hasn't failed.
      std::array<bool, sizeof...(Ts)> detectFailures = {{
        ComponentItem<Ts>::instance()->isFailedComponent(entityID)...
      }};

      bool validComponent = true;
      for (size_t j = 0; j < sizeof...(Ts); ++j)
      {
        if (detectFailures[j])
        {
          validComponent = false;
          break;
        }
      }

      // Attempt to find component in executed items list.
      bool binarySearchResult = std::binary_search(executedItems.begin(), executedItems.end(), entityID);
      if (binarySearchResult && !validComponent)
      {
        FAIL() << "Invalid component was executed.";
      }
      else if (!binarySearchResult && validComponent)
      {
        std::cout << "Entity: " << entityID << std::endl;
        FAIL() << "Component was not executed even though it was valid.";
      }
    }
  }

  void clearTestState()
  {
    executedItems.clear();
  }

  std::vector<uint64_t> executedItems;
};

TEST(EntitySystem, MultiRandDyn)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

  std::mt19937 randEngine(gRandomSeed);

  std::vector<uint64_t> validEntities;

  std::uniform_int_distribution<int> dist100(0, 100);
  std::uniform_int_distribution<int> distBool(0, 1);
  std::uniform_int_distribution<int> distEntity(1,3);
  std::uniform_real_distribution<double> distReal(-100.0,100.0);
  auto gen100 = std::bind(dist100, std::ref(randEngine));
  auto genReal = std::bind(distReal, std::ref(randEngine));
  auto genBool = std::bind(distBool, std::ref(randEngine));
  auto genEntity = std::bind(distEntity, std::ref(randEngine));

  const int numIterations = 60;
  int numComponentsTotal = 600;

  core->clearAllComponentContainers();

  ComponentItem<CompPosition>* c_pos      = ComponentItem<CompPosition>::instance();
  ComponentItem<CompHomPos>* c_homPos     = ComponentItem<CompHomPos>::instance();
  ComponentItem<CompGameplay>* c_gameplay = ComponentItem<CompGameplay>::instance();
  ComponentItem<CompTest1>* c_test1       = ComponentItem<CompTest1>::instance();
  ComponentItem<CompTest2>* c_test2       = ComponentItem<CompTest2>::instance();
  ComponentItem<CompTest3>* c_test3       = ComponentItem<CompTest3>::instance();

  // We could easily streamline this using variadic templates.

  // Clear all component systems.
  c_pos->clear();
  c_homPos->clear();
  c_gameplay->clear();
  c_test1->clear();
  c_test2->clear();
  c_test3->clear();

  // Generate random components for all components.
  c_pos->genRandom(core, numComponentsTotal, gen100, genReal, gen100);
  c_homPos->genRandom(core, numComponentsTotal, gen100, genReal, gen100);
  c_gameplay->genRandom(core, numComponentsTotal, gen100, genReal, gen100);
  c_test1->genRandom(core, numComponentsTotal, gen100, genReal, gen100);
  c_test2->genRandom(core, numComponentsTotal, gen100, genReal, gen100);
  c_test3->genRandom(core, numComponentsTotal, gen100, genReal, gen100);

  auto testSystemA = new TestSystem<CompPosition, CompHomPos, CompGameplay>();
  auto testSystemB = new TestSystem<CompHomPos, CompGameplay>();
  auto testSystemC = new TestSystem<CompGameplay, CompTest2>();
  auto testSystemD = new TestSystem<CompTest1, CompTest2>();
  auto testSystemE = new TestSystem<CompPosition, CompTest3, CompTest1, CompTest2>();
  auto testSystemF = new TestSystem<CompPosition, CompHomPos, CompGameplay, CompTest1, CompTest2, CompTest3>();
  auto testSystemG = new TestSystem<CompPosition, CompHomPos, CompTest1, CompTest2, CompTest3>();
  auto testSystemH = new TestSystem<CompPosition, CompHomPos, CompTest1, CompTest2>();
  auto testSystemI = new TestSystem<CompPosition, CompTest1, CompTest2>();
  auto testSystemJ = new TestSystem<CompTest1, CompTest3>();

  auto executeAllSystems = [&]()
  {
    core->renormalize();
    testSystemA->walkComponents(*core);
    testSystemB->walkComponents(*core);
    testSystemC->walkComponents(*core);
    testSystemD->walkComponents(*core);
    testSystemE->walkComponents(*core);
    testSystemF->walkComponents(*core);
    testSystemG->walkComponents(*core);
    testSystemH->walkComponents(*core);
    testSystemI->walkComponents(*core);
    testSystemJ->walkComponents(*core);
  };

  for (int i = 0; i < numIterations; ++i)
  {
    // Clear executed components from systems.
    testSystemA->clearTestState();
    testSystemB->clearTestState();
    testSystemC->clearTestState();
    testSystemD->clearTestState();
    testSystemE->clearTestState();
    testSystemF->clearTestState();
    testSystemG->clearTestState();
    testSystemH->clearTestState();
    testSystemI->clearTestState();
    testSystemJ->clearTestState();

    // Execute one iteration.
    executeAllSystems();

    // Run tests to ensure valid components were executed.
    testSystemA->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemB->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemC->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemD->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemE->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemF->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemG->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemH->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemI->ensureValidComponentsExecuted(numComponentsTotal);
    testSystemJ->ensureValidComponentsExecuted(numComponentsTotal);

    // Add / remove components.

    // *Component addition*
    // We want to add to numComponentsTotal, and add the corresponding randomly
    // generated component (we just need to create a helper function that adds
    // the randomly generated components onto the end of the vectors).
    if (genBool())
    {
      int numEntities = genEntity();
      for (int j = 0; j < numEntities; ++j)
      {
        ++numComponentsTotal;

        c_pos->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        c_homPos->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        c_gameplay->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        c_test1->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        c_test2->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        c_test3->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
      }
    }

    // *Component removal*
    // Remove entity id from all valid lists, and make sure entity id is
    // removed from all components! Will need new functionality for deleting
    // entity id from all components. We don't touch num components total.
    if (genBool() && genBool())
    {
      int numEntities = genEntity();
      auto genToRemove = std::bind(std::uniform_int_distribution<uint64_t>(0, numComponentsTotal), randEngine);
      for (int j = 0; j < numEntities; ++j)
      {
        // Remove the entity id from the system.
        // We may happen upon entities we have already deleted, but that is
        // okay.
        uint64_t entityID = genToRemove();
        core->removeEntity(entityID);

        c_pos->makeSequenceInvalid(entityID);
        c_homPos->makeSequenceInvalid(entityID);
        c_gameplay->makeSequenceInvalid(entityID);
        c_test1->makeSequenceInvalid(entityID);
        c_test2->makeSequenceInvalid(entityID);
        c_test3->makeSequenceInvalid(entityID);
      }
    }

    // No need to add / remove systems. This is just testing add / remove from
    // lists.
  }
}

}

