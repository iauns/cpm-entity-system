
#include <entity-system/GenericSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <ctime>
#include <random>
#include <glm/glm.hpp>
#include <functional>

// This test randomly generates multiple components associated with each entity
// ID. This allows us to test that the recursive execution done by
// GenericSystem fufills its requirements.

// Global variable from tests/main.cpp -- the random seed that we should use.
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

  bool checkEqual(const CompPosition* pos) const
  {
    if (pos == nullptr) return false;
    // We do not perform a floating point comparison because we haven't
    // performed arithmetic on any of these stored values.
    bool b = (position.x == pos->position.x)
        && (position.y == pos->position.y)
        && (position.z == pos->position.z);
    return b;
  }

  bool checkEqual(const std::list<CompPosition>* list) const
  {
    if (list == nullptr) return false;

    for (auto it = list->begin(); it != list->end(); ++it)
      if (checkEqual(&(*it)))
        return true;

    // Print out the offending list of positions, and the desired position.
    std::cout << "Failed target pos: " << position.x << "," << position.y << ","
        << position.z << std::endl;

    std::cout << "List of candidates:" << std::endl;
    for (auto it = list->begin(); it != list->end(); ++it)
      std::cout << "Position: " << it->position.x << "," << it->position.y
          << "," << it->position.z << std::endl;

    return false;
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

  bool checkEqual(const CompHomPos* pos) const
  {
    if (pos == nullptr) return false;
    return (position.x == pos->position.x)
        && (position.y == pos->position.y)
        && (position.z == pos->position.z)
        && (position.w == pos->position.w);
  }

  bool checkEqual(const std::list<CompHomPos>* list) const
  {
    if (list == nullptr) return false;

    for (auto it = list->begin(); it != list->end(); ++it)
      if (checkEqual(&(*it)))
        return true;

    // Print out the offending list of positions, and the desired position.
    std::cout << "Failed target pos: " << position.x << "," << position.y << ","
        << position.z << "," << position.w << std::endl;

    std::cout << "List of candidates:" << std::endl;
    for (auto it = list->begin(); it != list->end(); ++it)
      std::cout << "Position: " << it->position.x << "," << it->position.y
          << "," << it->position.z << "," << it->position.w << std::endl;

    return false;
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

  bool checkEqual(const CompGameplay* gp) const
  {
    if (gp == nullptr) return false;
    return (health == gp->health)
        && (armor == gp->armor);
  }

  bool checkEqual(const std::list<CompGameplay>* list) const
  {
    if (list == nullptr) return false;

    for (auto it = list->begin(); it != list->end(); ++it)
      if (checkEqual(&(*it)))
        return true;

    // Print out failed targets
    std::cout << "Target health: " << health << " armor " << armor << std::endl;

    std::cout << "List of candidates:" << std::endl;
    for (auto it = list->begin(); it != list->end(); ++it)
      std::cout << "Health: " << it->health << " armor " << it->armor << std::endl;

    return false;
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

  bool checkEqual(const CompTest1* t) const
  {
    if (t == nullptr) return false;
    return (t1 == t->t1);
  }

  bool checkEqual(const std::list<CompTest1>* list) const
  {
    if (list == nullptr) return false;

    for (auto it = list->begin(); it != list->end(); ++it)
      if (checkEqual(&(*it)))
        return true;

    return false;
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

  bool checkEqual(const CompTest2* t) const
  {
    if (t == nullptr) return false;
    return (t1 == t->t1)
        && (t2 == t->t2);
  }

  bool checkEqual(const std::list<CompTest2>* list) const
  {
    if (list == nullptr) return false;

    for (auto it = list->begin(); it != list->end(); ++it)
      if (checkEqual(&(*it)))
        return true;

    return false;
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

  bool checkEqual(const CompTest3* t) const
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

  bool checkEqual(const std::list<CompTest3>* list) const
  {
    if (list == nullptr) return false;

    for (auto it = list->begin(); it != list->end(); ++it)
      if (checkEqual(&(*it)))
        return true;

    return false;
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
  std::map<uint64_t, std::list<T>> components;
  uint64_t highestEntityID;

  ComponentItem() : highestEntityID(0) {}

  void clear()
  {
    highestEntityID = 0;
    failedComponents.clear();
    components.clear();
  }

  void generateOneRandomComponent(uint64_t entityID, std::shared_ptr<es::ESCore> core,
                                  IntRandFun irand, DoubleRandFun drand,
                                  IntRandFun gen100)
  {
    if (entityID > highestEntityID) highestEntityID = entityID;

    int entityGen = gen100();
    std::list<T> componentList;
    if (getComponentList(entityID) != nullptr)
      componentList = *getComponentList(entityID);
    if (entityGen > 35)
    {
      T thisComponent = T(irand, drand);
      componentList.push_back(thisComponent);

      // Just in case there are already any pre-existing components.
      components.erase(entityID);

      components.insert(std::make_pair(entityID, componentList));
      core->addComponent(entityID, thisComponent);

      if (isFailedComponent(entityID))
        makeSequenceValid(entityID);
    }
    else
    {
      if (getComponentList(entityID) == nullptr && !isFailedComponent(entityID))
      {
        failedComponents.push_back(entityID);
        std::sort(failedComponents.begin(), failedComponents.end());
      }
    }
  }

  void generateMultipleRandomComponents(uint64_t entityID, std::shared_ptr<es::ESCore> core,
                                        IntRandFun irand, DoubleRandFun drand,
                                        IntRandFun gen100)
  {
    if (entityID > highestEntityID) highestEntityID = entityID;

    int entityGen = gen100();
    int numComponents = entityGen % 6 + 1;
    int numCompGenerated = 0;
    std::list<T> componentList;
    if (getComponentList(entityID) != nullptr)
      componentList = *getComponentList(entityID);
    for (int i = 0; i < numComponents; ++i)
    {
      if (gen100() > 20)
      {
        T thisComponent = T(irand, drand);
        componentList.push_back(thisComponent);
        core->addComponent(entityID, thisComponent);
        ++numCompGenerated;
      }
    }

    if (numCompGenerated != 0)
    {
      // Just in case there are already any pre-existing components.
      components.erase(entityID);
      components.insert(std::make_pair(entityID, componentList));

      // Ensure we are not on the failed components.
      if (isFailedComponent(entityID))
        makeSequenceValid(entityID);
    }
    else
    {
      if (getComponentList(entityID) == nullptr && !isFailedComponent(entityID))
      {
        failedComponents.push_back(entityID);
        std::sort(failedComponents.begin(), failedComponents.end());
      }
    }
  }

  void genRandom(std::shared_ptr<es::ESCore> core, uint64_t numComponents,
                 IntRandFun irand, DoubleRandFun drand, IntRandFun gen100)
  {
    failedComponents.clear();
    components.clear();

    for (uint64_t i = 0; i < numComponents; ++i)
    {
      uint64_t entityID = i + 1;
      if (gen100() > 30) generateOneRandomComponent(entityID, core, irand, drand, gen100);
      else               generateMultipleRandomComponents(entityID, core, irand, drand, gen100);
    }
  }

  void makeSequenceInvalid(uint64_t sequence)
  {
    if (!std::binary_search(failedComponents.begin(), failedComponents.end(), sequence))
    {
      // The sequence is *not* in our failed components. We need to insert the
      // failed component int failedComponents (just re-sort), and remove it from
      // components.
      EXPECT_EQ(false, isFailedComponent(sequence));
      failedComponents.push_back(sequence);
      std::sort(failedComponents.begin(), failedComponents.end());
      
      // Now erase sequence from components.
      components.erase(sequence);
    }
  }

  void makeSequenceValid(uint64_t sequence)
  {
    // lower_bound uses a binary search to find the target component.
    // Our vector is always sorted, so we can pull this off.
    auto it = std::lower_bound(failedComponents.begin(), failedComponents.end(), sequence);
    bool didErase = false;
    if (*it == sequence)
    {
      failedComponents.erase(it);
      didErase = true;
    }
    if (isFailedComponent(sequence) == true)
    {
      if (didErase)
        std::cout << "FAILED EVEN WITH ERASE." << std::endl;
      EXPECT_EQ(false, isFailedComponent(sequence));
    }
  }

  void removeFirstComponentFromList(uint64_t sequence)
  {
    std::list<T>* compList = getComponentList(sequence);
    if (compList != nullptr)
    {
      compList->pop_front();
      if (compList->size() == 0)
      {
        // Make this sequence invalid.
        makeSequenceInvalid(sequence);
      }
      else
      {
        EXPECT_EQ(compList->size(), getComponentList(sequence)->size());
      }
    }
  }

  void removeLastComponentFromList(uint64_t sequence)
  {
    std::list<T>* compList = getComponentList(sequence);
    if (compList != nullptr)
    {
      compList->pop_back();
      if (compList->size() == 0)
      {
        // Make this sequence invalid.
        makeSequenceInvalid(sequence);
      }
      else
      {
        EXPECT_EQ(compList->size(), getComponentList(sequence)->size());
      }
    }
  }

  std::list<T>* getComponentList(uint64_t entityID)
  {
    auto comp = components.find(entityID);
    if (comp != components.end())
      return &comp->second;
    else
      return nullptr;
  }

  bool hasComponents(uint64_t entityID)
  {
    return (getComponentList(entityID) != nullptr);
  }

  bool isFailedComponent(uint64_t id)
  {
    return std::binary_search(failedComponents.begin(), failedComponents.end(), id);
  }
};

template <typename T> ComponentItem<T>* ComponentItem<T>::mInstance = nullptr;

template <typename... Ts>
class TestSystem : public es::GenericSystem<false, Ts...>
{
public:

  // Calculate the number of executions required for this system, based on the
  // number of current entities and components.
  uint64_t getNumRecursiveExecutions()
  {
    // Retrieve highest entity ID.
    uint64_t highestEntityID = std::max({ComponentItem<Ts>::instance()->highestEntityID...});
    uint64_t totalNumExecutions = 0;

    for (uint64_t i = 0; i <= highestEntityID; ++i)
    {
      // Check to see if all components exist for this entity ID.
      std::array<bool, sizeof...(Ts)> validComponents = {{ ComponentItem<Ts>::instance()->hasComponents(i)... }};
      bool isValid = true;
      for (size_t j = 0; j < sizeof...(Ts); ++j)
      {
        if (!validComponents[j])
        {
          isValid = false;
          break;
        }
      }

      if (isValid)
      {
        std::array<size_t, sizeof...(Ts)> numComponents = {{ ComponentItem<Ts>::instance()->getComponentList(i)->size()... }};
        int executionsForThisComponent = numComponents[0];
        for (size_t j = 1; j < sizeof...(Ts); ++j)
        {
          executionsForThisComponent *= static_cast<uint64_t>(numComponents[j]);
        }
        totalNumExecutions += executionsForThisComponent;
      }
    }
    return totalNumExecutions;
  }

  void execute(es::ESCore&, uint64_t entityID, const Ts*... args)
  {
    executedItems.push_back(entityID);
    ++numExecutions;

    // Detect any failed components that should not be tested.
    std::array<bool, sizeof...(Ts)> detectFailures = {{
      ComponentItem<Ts>::instance()->isFailedComponent(entityID)...
    }};
    for (size_t i = 0; i < sizeof...(Ts); ++i)
    {
      if (detectFailures[i] == true)
        ASSERT_EQ(detectFailures[i], false);
    }

    // Ensure all components are their correct values.
    std::array<bool, sizeof...(Ts)> checks = {{args->checkEqual(ComponentItem<Ts>::instance()->getComponentList(entityID))...}};
    for (size_t i = 0; i < sizeof...(Ts); ++i)
    {
      if (checks[i] == false)
        ASSERT_EQ(checks[i], true);
    }
  }

  // Run through all items that should have been executed and make sure they
  // were in our executed list.
  void ensureValidComponentsExecuted(uint64_t numComponents)
  {
    for (uint64_t i = 0; i < numComponents; ++i)
    {
      uint64_t entityID = i + 1;

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
    numExecutions = 0;
  }

  // The number of times this test system called 'execute'.
  std::vector<uint64_t> executedItems;
  static uint64_t numExecutions;
};

template <typename... Ts> uint64_t TestSystem<Ts...>::numExecutions = 0;

TEST(EntitySystem, MultiRandDynGroupRec)
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
  uint64_t numComponentsTotal = 500;

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
    core->renormalize(true);            // We need stable sort for removal of 
                                        // single components (first or last).
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

    // Check the number of executions against expected number of executions.
    EXPECT_EQ(testSystemA->getNumRecursiveExecutions(), testSystemA->numExecutions);
    EXPECT_EQ(testSystemB->getNumRecursiveExecutions(), testSystemB->numExecutions);
    EXPECT_EQ(testSystemC->getNumRecursiveExecutions(), testSystemC->numExecutions);
    EXPECT_EQ(testSystemD->getNumRecursiveExecutions(), testSystemD->numExecutions);
    EXPECT_EQ(testSystemE->getNumRecursiveExecutions(), testSystemE->numExecutions);
    EXPECT_EQ(testSystemF->getNumRecursiveExecutions(), testSystemF->numExecutions);
    EXPECT_EQ(testSystemG->getNumRecursiveExecutions(), testSystemG->numExecutions);
    EXPECT_EQ(testSystemH->getNumRecursiveExecutions(), testSystemH->numExecutions);
    EXPECT_EQ(testSystemI->getNumRecursiveExecutions(), testSystemI->numExecutions);
    EXPECT_EQ(testSystemJ->getNumRecursiveExecutions(), testSystemJ->numExecutions);

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

        if (gen100() > 30)  c_pos->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        else                c_pos->generateMultipleRandomComponents(numComponentsTotal, core, gen100, genReal, gen100);

        if (gen100() > 35)  c_homPos->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        else                c_homPos->generateMultipleRandomComponents(numComponentsTotal, core, gen100, genReal, gen100);

        if (gen100() > 40)  c_gameplay->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        else                c_gameplay->generateMultipleRandomComponents(numComponentsTotal, core, gen100, genReal, gen100);

        if (gen100() > 35)  c_test1->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        else                c_test1->generateMultipleRandomComponents(numComponentsTotal, core, gen100, genReal, gen100);

        if (gen100() > 30)  c_test2->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        else                c_test2->generateMultipleRandomComponents(numComponentsTotal, core, gen100, genReal, gen100);

        if (gen100() > 25)  c_test3->generateOneRandomComponent(numComponentsTotal, core, gen100, genReal, gen100);
        else                c_test3->generateMultipleRandomComponents(numComponentsTotal, core, gen100, genReal, gen100);
      }
    }

    // Insert components at random entityIDs 10% of the time.
    if (gen100() < 10)
    {
      for (uint64_t j = 1; j < numComponentsTotal; ++j)
      {
        if (gen100() < 8)
        {
          if (gen100() < 20)
          {
            if (gen100() > 30)  c_pos->generateOneRandomComponent(j, core, gen100, genReal, gen100);
            else                c_pos->generateMultipleRandomComponents(j, core, gen100, genReal, gen100);
          }

          if (gen100() < 20)
          {
            if (gen100() > 35)  c_homPos->generateOneRandomComponent(j, core, gen100, genReal, gen100);
            else                c_homPos->generateMultipleRandomComponents(j, core, gen100, genReal, gen100);
          }

          if (gen100() < 20)
          {
            if (gen100() > 40)  c_gameplay->generateOneRandomComponent(j, core, gen100, genReal, gen100);
            else                c_gameplay->generateMultipleRandomComponents(j, core, gen100, genReal, gen100);
          }

          if (gen100() < 20)
          {
            if (gen100() > 35)  c_test1->generateOneRandomComponent(j, core, gen100, genReal, gen100);
            else                c_test1->generateMultipleRandomComponents(j, core, gen100, genReal, gen100);
          }

          if (gen100() < 20)
          {
            if (gen100() > 30)  c_test2->generateOneRandomComponent(j, core, gen100, genReal, gen100);
            else                c_test2->generateMultipleRandomComponents(j, core, gen100, genReal, gen100);
          }

          if (gen100() < 20)
          {
            if (gen100() > 25)  c_test3->generateOneRandomComponent(j, core, gen100, genReal, gen100);
            else                c_test3->generateMultipleRandomComponents(j, core, gen100, genReal, gen100);
          }
        }
      }
    }

    // *Component removal*
    // Remove entity id from all valid lists, and make sure entity id is
    // removed from all components! Will need new functionality for deleting
    // entity id from all components. We don't touch num components total.
    auto genToRemove = std::bind(std::uniform_int_distribution<uint64_t>(0, numComponentsTotal), randEngine);
    auto genRandComp = std::bind(std::uniform_int_distribution<uint64_t>(0, 5), randEngine);

    if (genBool() && genBool())
    {
      int numEntities = genEntity();
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

    // Remove entire sequences from components containers.
    if (gen100() < 30)
    {
      int numEntities = genEntity();
      for (int j = 0; j < numEntities; ++j)
      {
        uint64_t entityID = genToRemove();
        uint64_t compToRemove = genRandComp();

        switch (compToRemove)
        {
          case 0:
            core->removeAllComponents(entityID, es::TemplateID<CompPosition>::getID());
            c_pos->makeSequenceInvalid(entityID);
            break;

          case 1:
            core->removeAllComponents(entityID, es::TemplateID<CompHomPos>::getID());
            c_homPos->makeSequenceInvalid(entityID);
            break;

          case 2:
            core->removeAllComponents(entityID, es::TemplateID<CompGameplay>::getID());
            c_gameplay->makeSequenceInvalid(entityID);
            break;

          case 3:
            core->removeAllComponents(entityID, es::TemplateID<CompTest1>::getID());
            c_test1->makeSequenceInvalid(entityID);
            break;

          case 4:
            core->removeAllComponents(entityID, es::TemplateID<CompTest2>::getID());
            c_test2->makeSequenceInvalid(entityID);
            break;

          case 5:
            core->removeAllComponents(entityID, es::TemplateID<CompTest3>::getID());
            c_test3->makeSequenceInvalid(entityID);
            break;
        }
      }
    }

    // Remove a series of 'first' components (we are using stable sort, so
    // we have guarantees here).
    if (gen100() < 30)
    {
      int numEntities = genEntity();
      for (int j = 0; j < numEntities; ++j)
      {
        uint64_t entityID = genToRemove();
        uint64_t compToRemove = genRandComp();
        switch (compToRemove)
        {
          case 0:
            core->removeFirstComponent(entityID, es::TemplateID<CompPosition>::getID());
            c_pos->removeFirstComponentFromList(entityID);
            break;

          case 1:
            core->removeFirstComponent(entityID, es::TemplateID<CompHomPos>::getID());
            c_homPos->removeFirstComponentFromList(entityID);
            break;

          case 2:
            core->removeFirstComponent(entityID, es::TemplateID<CompGameplay>::getID());
            c_gameplay->removeFirstComponentFromList(entityID);
            break;

          case 3:
            core->removeFirstComponent(entityID, es::TemplateID<CompTest1>::getID());
            c_test1->removeFirstComponentFromList(entityID);
            break;

          case 4:
            core->removeFirstComponent(entityID, es::TemplateID<CompTest2>::getID());
            c_test2->removeFirstComponentFromList(entityID);
            break;

          case 5:
            core->removeFirstComponent(entityID, es::TemplateID<CompTest3>::getID());
            c_test3->removeFirstComponentFromList(entityID);
            break;
        }
      }
    }

    // Remove a series of 'last' components.
    if (gen100() < 30)
    {
      int numEntities = genEntity();
      for (int j = 0; j < numEntities; ++j)
      {
        uint64_t entityID = genToRemove();
        uint64_t compToRemove = genRandComp();
        switch (compToRemove)
        {
          case 0:
            core->removeLastComponent(entityID, es::TemplateID<CompPosition>::getID());
            c_pos->removeLastComponentFromList(entityID);
            break;

          case 1:
            core->removeLastComponent(entityID, es::TemplateID<CompHomPos>::getID());
            c_homPos->removeLastComponentFromList(entityID);
            break;

          case 2:
            core->removeLastComponent(entityID, es::TemplateID<CompGameplay>::getID());
            c_gameplay->removeLastComponentFromList(entityID);
            break;

          case 3:
            core->removeLastComponent(entityID, es::TemplateID<CompTest1>::getID());
            c_test1->removeLastComponentFromList(entityID);
            break;

          case 4:
            core->removeLastComponent(entityID, es::TemplateID<CompTest2>::getID());
            c_test2->removeLastComponentFromList(entityID);
            break;

          case 5:
            core->removeLastComponent(entityID, es::TemplateID<CompTest3>::getID());
            c_test3->removeLastComponentFromList(entityID);
            break;
        }
      }
    }

  }

}

}

