
#include <entity-system/GenericSystem.hpp>
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
class BasicSystem : public es::GenericSystem<false, CompStaticLightDir, CompStaticCamera>
{
public:

  static int numCall;

  void execute(es::ESCore&, uint64_t /* entityID */, const CompStaticLightDir* dir, const CompStaticCamera* cam) override
  {
    ++numCall;

    // Cehck to see if dir and cam are correct.
    switch (numCall)
    {
      case 1:
        dir->checkEqual(lightDirs[0]);
        cam->checkEqual(cameras[0]);
        break;

      case 2:
        dir->checkEqual(lightDirs[0]);
        cam->checkEqual(cameras[1]);
        break;

      case 3:
        dir->checkEqual(lightDirs[0]);
        cam->checkEqual(cameras[2]);
        break;

      case 4:
        dir->checkEqual(lightDirs[0]);
        cam->checkEqual(cameras[3]);
        break;

      case 5:
        dir->checkEqual(lightDirs[0]);
        cam->checkEqual(cameras[4]);
        break;

      case 6:
        dir->checkEqual(lightDirs[1]);
        cam->checkEqual(cameras[0]);
        break;

      case 7:
        dir->checkEqual(lightDirs[1]);
        cam->checkEqual(cameras[1]);
        break;

      case 8:
        dir->checkEqual(lightDirs[1]);
        cam->checkEqual(cameras[2]);
        break;

      case 9:
        dir->checkEqual(lightDirs[1]);
        cam->checkEqual(cameras[3]);
        break;

      case 10:
        dir->checkEqual(lightDirs[1]);
        cam->checkEqual(cameras[4]);
        break;
    }

  }
};

int BasicSystem::numCall = 0;

TEST(EntitySystem, StaticOnlyTest)
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

  EXPECT_EQ(10, BasicSystem::numCall);
}


}

