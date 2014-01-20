
#include <entity-system/GenericSystem.hpp>
#include <memory>
#include <glm/glm.hpp>

namespace es = CPM_ES_NS;

// We may want to enforce that these components have bson serialization members
// (possibly a static assert?).

struct CompPosition
{
  CompPosition() {}
  CompPosition(const glm::vec3& pos) {position = pos;}

  void checkEqual(const CompPosition& pos)
  {
    if (position.x != pos.position.x) std::cerr << "CompPosition - Incorrect position - x!" << std::endl;
    if (position.y != pos.position.y) std::cerr << "CompPosition - Incorrect position - y!" << std::endl;
    if (position.z != pos.position.z) std::cerr << "CompPosition - Incorrect position - z!" << std::endl;
  }

  // What this 'struct' is all about -- the data.
  glm::vec3 position;
};

struct CompHomPos
{
  CompHomPos() {}
  CompHomPos(const glm::vec4& pos) {position = pos;}

  void checkEqual(const CompHomPos& pos)
  {
    if (position.x != pos.position.x) std::cerr << "CompHomPos - Incorrect position! - x" << std::endl;
    if (position.y != pos.position.y) std::cerr << "CompHomPos - Incorrect position! - y" << std::endl;
    if (position.z != pos.position.z) std::cerr << "CompHomPos - Incorrect position! - z" << std::endl;
    if (position.w != pos.position.w) std::cerr << "CompHomPos - Incorrect position! - w" << std::endl;
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

  void checkEqual(const CompGameplay& gp)
  {
    if (health != gp.health) std::cerr << "CompGameplay - Incorrect health!" << std::endl;
    if (armor != gp.armor) std::cerr << "CompGameplay - Incorrect armor!" << std::endl;
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
class BasicSystem : public es::GenericSystem<CompPosition, CompHomPos, CompGameplay>
{
public:

  static std::map<uint64_t, bool> invalidComponents;

  void execute(uint64_t entityID, CompPosition* pos, CompHomPos* homPos, CompGameplay* gp) override
  {
    std::cout << "Executing " << entityID << std::endl;
    std::cout << "(" << pos->position.x << "," << pos->position.y << "," << pos->position.z << ")" << std::endl;
    std::cout << "(" << homPos->position.x << "," << homPos->position.y << "," << homPos->position.z << "," << homPos->position.w << ")" << std::endl;
    std::cout << "(" << gp->health << "," << gp->armor << ")" << std::endl;

    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      std::cerr << "BasicSystem attempt to execute on an invalid component." << std::endl;

    // Check the values contained in each of pos, homPos, and gp.
    pos->checkEqual(posComponents[entityID]);
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);
  }
};

std::map<uint64_t, bool> BasicSystem::invalidComponents;

void basicEntitySystemTest()
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

  // Test that adding BasicSystem also adds component containers for all data
  // components.
  core->addSystem(new BasicSystem());


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
  
  std::shared_ptr<BasicSystem> sys(new BasicSystem);

  core->renormalize();
  sys->walkComponents(*core);
}

// Seeding:
// time(NULL)

