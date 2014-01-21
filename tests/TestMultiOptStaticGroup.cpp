
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

  void checkEqual(const CompPosition& pos)
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

  void checkEqual(const CompHomPos& pos)
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

  void checkEqual(const CompGameplay& gp)
  {
    EXPECT_EQ(health, gp.health);
    EXPECT_EQ(armor, gp.armor);
  }

  // DATA
  int health;
  int armor;
};

struct CompStaticLightDir
{
  CompStaticLightDir() {}
  CompStaticLightDir(const glm::vec3& dir) {lightDir = dir;}

  void checkEqual(const CompStaticLightDir& dir)
  {
    EXPECT_FLOAT_EQ(lightDir.x, dir.lightDir.x);
    EXPECT_FLOAT_EQ(lightDir.y, dir.lightDir.y);
    EXPECT_FLOAT_EQ(lightDir.z, dir.lightDir.z);
  }

  glm::vec3 lightDir;
};

std::vector<CompStaticLightDir> lightDirs = {
  glm::vec3(0.0, 1.0, 0.0)
};

struct CompStaticCamera
{
  CompStaticCamera() : dummy(0)         {}
  CompStaticCamera(int in) : dummy(in)  {}

  void checkEqual(const CompStaticCamera& in)
  {
    EXPECT_EQ(in.dummy, dummy);
  }

  int dummy;
};

std::vector<CompStaticCamera> cameras = {
  12
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
class BasicSystem : public es::GenericSystem<CompStaticLightDir, CompPosition, 
                                             CompStaticCamera, CompHomPos, CompGameplay>
{
public:

  static int homNull;
  static int posNull;

  static int homCall;
  static int posCall;

  static int numCall;

  static std::map<uint64_t, bool> invalidComponents;

  bool isComponentOptional(uint64_t templateID)
  {
    if (templateID == es::TemplateID<CompHomPos>::getID()) return true;
    else if (templateID == es::TemplateID<CompPosition>::getID()) return true;
    else return false;
  }

  bool shouldGroupComponents()
  {
    return true;
  }

  void groupExecute(uint64_t entityID,
                    const es::ComponentGroup<CompStaticLightDir>& dir,
                    const es::ComponentGroup<CompPosition>& pos,
                    const es::ComponentGroup<CompStaticCamera>& cam,
                    const es::ComponentGroup<CompHomPos>& homPos,
                    const es::ComponentGroup<CompGameplay>& gp) override
  {
    ++numCall;

    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      FAIL() << "BasicSystem attempt to execute on an invalid component." << std::endl;

    if (homPos.numComponents != 0) {homPos.components->component.checkEqual(homPosComponents[entityID]); ++homCall;}
    else ++homNull;

    if (pos.numComponents != 0) {pos.components->component.checkEqual(posComponents[entityID]); ++posCall;}
    else ++posNull;

    if (gp.numComponents != 0) gp.components->component.checkEqual(gameplayComponents[entityID]);

    // Check to ensure static components remain static.
    dir.components->component.checkEqual(lightDirs[0]);
    cam.components->component.checkEqual(cameras[0]);
  }
};

std::map<uint64_t, bool> BasicSystem::invalidComponents;
int BasicSystem::homNull = 0;
int BasicSystem::posNull = 0;

int BasicSystem::homCall = 0;
int BasicSystem::posCall = 0;

int BasicSystem::numCall = 0;

TEST(EntitySystem, MultiOptionalStaticGroupTest)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());

  // Add static light directions
  std::vector<size_t> lightDirIndices;
  for (auto it = lightDirs.begin(); it != lightDirs.end(); ++it)
  {
    lightDirIndices.push_back(core->addStaticComponent(*it));
  }
  int count = 0;
  for (size_t index : lightDirIndices)
  {
    EXPECT_EQ(index, count);
    ++count;
  }

  // Add static 'cameras'
  std::vector<size_t> cameraIndices;
  for (auto it = cameras.begin(); it != cameras.end(); ++it)
  {
    cameraIndices.push_back(core->addStaticComponent(*it));
  }
  count = 0;
  for (size_t index : cameraIndices)
  {
    EXPECT_EQ(index, count);
    ++count;
  }

  uint64_t id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  BasicSystem::invalidComponents.insert(std::make_pair(id, true));

  id = core->getNewEntityID();
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);
  
  std::shared_ptr<BasicSystem> sys(new BasicSystem());

  core->renormalize();
  sys->walkComponents(*core);

  EXPECT_EQ(1, BasicSystem::posNull);
  EXPECT_EQ(1, BasicSystem::homNull);

  EXPECT_EQ(2, BasicSystem::posCall);
  EXPECT_EQ(2, BasicSystem::homCall);

  EXPECT_EQ(3, BasicSystem::numCall);
}


}
