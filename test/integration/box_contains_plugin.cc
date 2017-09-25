/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "gazebo/test/ServerFixture.hh"
#include "gazebo/physics/physics.hh"
#include "gazebo/test/helper_physics_generator.hh"

using namespace gazebo;
class BoxContainsPluginTest : public ServerFixture
{
};

// Flag turned to true once box contains.
bool g_boxContains = false;

//////////////////////////////////////////////////
// Callback for box/contains topic
void boxCb(ConstIntPtr &_msg)
{
  g_boxContains = _msg->data() == 1;
}

//////////////////////////////////////////////////
TEST_F(BoxContainsPluginTest, Disable)
{
  this->Load("worlds/box_contains_plugin_demo.world", true);
  auto world = physics::get_world();
  ASSERT_NE(world , nullptr);

  // Get models
  auto drill = world->GetModel("drill");
  ASSERT_NE(drill , nullptr);

  // Subscribe to plugin notifications
  std::string prefix("/gazebo/default/drill/box/");
  auto boxSub = this->node->Subscribe(prefix + "contains", &boxCb);
  ASSERT_NE(boxSub , nullptr);

  // Check box doesn't contain yet
  EXPECT_FALSE(g_boxContains);

  // Place drill inside box
  drill->SetWorldPose(ignition::math::Pose3d(10.0, 10.0, 1.0, 0, 0, 0));

  // Give it time to fall
  world->Step(1000);

  // Verify we get a notification
  EXPECT_TRUE(g_boxContains);

  // Place drill outside box
  drill->SetWorldPose(ignition::math::Pose3d(0.0, 0.0, 1.0, 0, 0, 0));

  // Give it time to fall
  world->Step(1000);

  // Verify we get a notification
  EXPECT_FALSE(g_boxContains);

  // Disable plugin
  auto enablePub = this->node->Advertise<msgs::Int>(prefix + "enable");

  msgs::Int msg;
  msg.set_data(0);
  enablePub->Publish(msg);

  // Place drill inside box
  drill->SetWorldPose(ignition::math::Pose3d(10.0, 10.0, 1.0, 0, 0, 0));

  // Give it time to fall
  world->Step(1000);

  // Wait and see it doesn't notify now
  EXPECT_FALSE(g_boxContains);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
