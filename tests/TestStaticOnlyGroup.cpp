
#include <entity-system/GenericSystem.hpp>
#include <entity-system/ESCore.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>

namespace es = CPM_ES_NS;

namespace {

// We may want to enforce that these components have bson serialization members
// (possibly a static assert?).

struct CompStaticLightDir
{
  CompStaticLightDir() {}
  CompStaticLightDir(const glm::vec3& dir) {lightDir = dir;}

  void checkEqual(const CompStaticLightDir& dir) const
  {
    EXPECT_FLOAT_EQ(lightDir.x, dir.lightDir.x);
    EXPECT_FLOAT_EQ(lightDir.y, dir.lightDir.y);
    EXPECT_FLOAT_EQ(lightDir.z, dir.lightDir.z);
  }

  glm::vec3 lightDir;
};

std::vector<CompStaticLightDir> lightDirs = {
  glm::vec3(0.0, 1.0, 0.0),
  glm::vec3(0.7333, 0.7333, 0.0),
};

struct CompStaticCamera
{
  CompStaticCamera() : dummy(0)         {}
  CompStaticCamera(int in) : dummy(in)  {}

  void checkEqual(const CompStaticCamera& in) const
  {
    EXPECT_EQ(in.dummy, dummy);
  }

  int dummy;
};

std::vector<CompStaticCamera> cameras = {
  12, 45, 982, 823, 53
};

// This basic system will apply, every frame, to entities with the CompPosition,
// CompHomPos, and CompGameplay components.
class BasicSystem : public es::GenericSystem<true, CompStaticLightDir, CompStaticCamera>
{
public:

  static int numCall;

  void groupExecute(es::CoreInt&, uint64_t /* entityID */,
                    const es::ComponentGroup<CompStaticLightDir>& lightDir,
                    const es::ComponentGroup<CompStaticCamera>& cam) override
  {
    ++numCall;

    for (size_t i = 0; i < lightDir.numComponents; ++i)
    {
      lightDir.components[i].component.checkEqual(lightDirs[i]);
    }

    for (size_t i = 0; i < cam.numComponents; ++i)
    {
      cam.components[i].component.checkEqual(cameras[i]);
    }
  }
};

int BasicSystem::numCall = 0;

TEST(EntitySystem, StaticOnlyGroupTest)
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

  std::shared_ptr<BasicSystem> sys(new BasicSystem());

  core->renormalize();
  sys->walkComponents(*core);

  EXPECT_EQ(1, BasicSystem::numCall);
}


}

