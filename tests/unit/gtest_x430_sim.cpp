/**
 * @file gtest_x430_sim.cpp
 * @brief Simulator behaviour seen through the Control Table.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "x430_test_support.h"

#if X430_FEATURE_SIMULATOR

class X430SimTest : public X430Test
{
  protected:
    struct x430_sim sim;

    void SetUp() override
    {
        X430Test::SetUp();
        const struct x430_sim_config config = {121U, 35U};
        ASSERT_EQ(x430_sim_init(&sim, &dev[1], &config), X430_OK);
        const struct x430_device_config device_config = {5U, 0U, 0U, &x430_sim_hooks, &sim};
        ASSERT_EQ(x430_device_init(&dev[1], &device_config), X430_OK);
        ASSERT_EQ(x430_bus_attach(&bus, &dev[1]), X430_OK);
    }

    int32_t get(uint16_t address) { return x430_device_get(&dev[1], address); }

    /** Steps until Moving drops to 0 (or the time runs out); returns the time used. */
    uint32_t run_until_stopped(uint32_t limit_ms)
    {
        uint32_t elapsed = 0;
        do
        {
            x430_sim_step(&sim, 10);
            elapsed += 10;
        } while (((get(X430_ADDR_MOVING) != 0) || (elapsed < 50)) && (elapsed < limit_ms));
        return elapsed;
    }
};

TEST_F(X430SimTest, InitRejectsNull)
{
    EXPECT_EQ(x430_sim_init(nullptr, &dev[1], nullptr), X430_EINVAL);
    EXPECT_EQ(x430_sim_init(&sim, nullptr, nullptr), X430_EINVAL);
    x430_sim_step(nullptr, 10);
}

TEST_F(X430SimTest, ReportsConfiguredVoltageAndTemperature)
{
    x430_sim_step(&sim, 1);
    EXPECT_EQ(get(X430_ADDR_PRESENT_INPUT_VOLTAGE), 121);
    EXPECT_EQ(get(X430_ADDR_PRESENT_TEMPERATURE), 35);
}

TEST_F(X430SimTest, PositionMoveWithTrapezoidalProfile)
{
    EXPECT_EQ(write32(5, X430_ADDR_PROFILE_ACCELERATION, 20), 0);
    EXPECT_EQ(write32(5, X430_ADDR_PROFILE_VELOCITY, 100), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_POSITION, 2048), 0);
    x430_sim_step(&sim, 100);
    EXPECT_GT(get(X430_ADDR_PRESENT_POSITION), 0);
    EXPECT_EQ(get(X430_ADDR_MOVING), 1);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & X430_MOVING_STATUS_PROFILE_ONGOING,
              X430_MOVING_STATUS_PROFILE_ONGOING);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & 0x30, X430_MOVING_STATUS_PROFILE_TRAPEZOIDAL);
    EXPECT_LE(get(X430_ADDR_PRESENT_VELOCITY), 100);
    EXPECT_EQ(get(X430_ADDR_POSITION_TRAJECTORY), 2048);
    (void) run_until_stopped(10000);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), 2048);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 0);
    EXPECT_EQ(get(X430_ADDR_MOVING), 0);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & X430_MOVING_STATUS_IN_POSITION,
              X430_MOVING_STATUS_IN_POSITION);
}

TEST_F(X430SimTest, PositionMoveWithoutProfileUsesVelocityLimit)
{
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_POSITION, 4000), 0);
    x430_sim_step(&sim, 20);
    EXPECT_NEAR(get(X430_ADDR_PRESENT_VELOCITY), get(X430_ADDR_VELOCITY_LIMIT), 1);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & 0x30, X430_MOVING_STATUS_PROFILE_STEP);
    (void) run_until_stopped(20000);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), 4000);
    EXPECT_EQ(write32(5, X430_ADDR_PROFILE_VELOCITY, 10), 0);
    x430_sim_step(&sim, 1);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & 0x30, X430_MOVING_STATUS_PROFILE_RECTANGULAR);
    EXPECT_EQ(write32(5, X430_ADDR_PROFILE_VELOCITY, 0), 0);
    EXPECT_EQ(write32(5, X430_ADDR_PROFILE_ACCELERATION, 10), 0);
    x430_sim_step(&sim, 1);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & 0x30, X430_MOVING_STATUS_PROFILE_TRIANGULAR);
}

TEST_F(X430SimTest, TorqueOffHoldsAndTorqueOnDoesNotJump)
{
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_POSITION, 3000), 0); /* torque off: no motion */
    x430_sim_step(&sim, 100);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0); /* goal := present */
    EXPECT_EQ(get(X430_ADDR_GOAL_POSITION), 0);
    x430_sim_step(&sim, 100);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), 0);
    EXPECT_EQ(get(X430_ADDR_MOVING_STATUS) & X430_MOVING_STATUS_IN_POSITION,
              X430_MOVING_STATUS_IN_POSITION);
}

TEST_F(X430SimTest, VelocityModeRampsToGoalVelocity)
{
    EXPECT_EQ(write8(5, X430_ADDR_OPERATING_MODE, X430_MODE_VELOCITY), 0);
    EXPECT_EQ(write32(5, X430_ADDR_PROFILE_ACCELERATION, 50), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_VELOCITY, -100), 0);
    x430_sim_step(&sim, 10);
    const int32_t early = get(X430_ADDR_PRESENT_VELOCITY);
    EXPECT_LT(early, 0);
    EXPECT_GT(early, -100);
    x430_sim_step(&sim, 1000);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), -100);
    EXPECT_LT(get(X430_ADDR_PRESENT_POSITION), 0);
    EXPECT_EQ(get(X430_ADDR_VELOCITY_TRAJECTORY), -100);
    EXPECT_NE(get(X430_ADDR_PRESENT_PWM), 0);
}

TEST_F(X430SimTest, ReverseDriveModeAndHomingOffset)
{
    EXPECT_EQ(write8(5, X430_ADDR_DRIVE_MODE, X430_DRIVE_MODE_REVERSE), 0);
    EXPECT_EQ(write32(5, X430_ADDR_HOMING_OFFSET, 100), 0);
    x430_sim_step(&sim, 1);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), 100);
    EXPECT_EQ(write8(5, X430_ADDR_OPERATING_MODE, X430_MODE_EXTENDED_POSITION), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_POSITION, -500), 0);
    (void) run_until_stopped(20000);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), -500);
    EXPECT_EQ(sim.shaft_mpulse, 600 * 1000); /* (goal - offset) * -1 */
}

TEST_F(X430SimTest, PwmMode)
{
    EXPECT_EQ(write8(5, X430_ADDR_OPERATING_MODE, X430_MODE_PWM), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write16(5, X430_ADDR_GOAL_PWM, 885), 0);
    x430_sim_step(&sim, 10);
    EXPECT_NEAR(get(X430_ADDR_PRESENT_VELOCITY), get(X430_ADDR_VELOCITY_LIMIT), 1);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 0), 0);
    EXPECT_EQ(write16(5, X430_ADDR_PWM_LIMIT, 0), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    x430_sim_step(&sim, 10);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 0);
}

#if X430_MODEL_HAS_CURRENT
TEST_F(X430SimTest, CurrentMode)
{
    EXPECT_EQ(write8(5, X430_ADDR_OPERATING_MODE, X430_MODE_CURRENT), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write16(5, X430_ADDR_GOAL_CURRENT, get(X430_ADDR_CURRENT_LIMIT)), 0);
    x430_sim_step(&sim, 10);
    EXPECT_NEAR(get(X430_ADDR_PRESENT_VELOCITY), get(X430_ADDR_VELOCITY_LIMIT), 1);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 0), 0);
    EXPECT_EQ(write16(5, X430_ADDR_CURRENT_LIMIT, 0), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    x430_sim_step(&sim, 10);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 0);
}
#endif

TEST_F(X430SimTest, EventsStopTheModel)
{
    EXPECT_EQ(write8(5, X430_ADDR_OPERATING_MODE, X430_MODE_VELOCITY), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_VELOCITY, 50), 0);
    EXPECT_EQ(write8(5, X430_ADDR_BUS_WATCHDOG, 1), 0);
    x430_sim_step(&sim, 10);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 50);
    env.now += 1000;
    x430_bus_poll(&bus);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 0);
    EXPECT_EQ(get(X430_ADDR_GOAL_VELOCITY), 0);
    x430_sim_step(&sim, 10);
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 0);

    (void) transact(5, DXL2_INST_REBOOT, {});
    EXPECT_EQ(get(X430_ADDR_PRESENT_VELOCITY), 0);
    EXPECT_EQ(transact(5, DXL2_INST_FACTORY_RESET, {X430_FACTORY_RESET_EXCEPT_ID_BAUD})[0].error,
              0);
    x430_device_raise_hardware_error(&dev[1], X430_HW_ERROR_OVERHEATING);
    EXPECT_EQ(transact(5, DXL2_INST_BACKUP, {X430_BACKUP_STORE, 'C', 'T', 'R', 'L'}).size(), 1U);
}

TEST_F(X430SimTest, ClearMultiTurnKeepsTheAngle)
{
    EXPECT_EQ(write8(5, X430_ADDR_OPERATING_MODE, X430_MODE_EXTENDED_POSITION), 0);
    EXPECT_EQ(write8(5, X430_ADDR_TORQUE_ENABLE, 1), 0);
    EXPECT_EQ(write32(5, X430_ADDR_GOAL_POSITION, -9000), 0);
    (void) run_until_stopped(60000);
    ASSERT_EQ(get(X430_ADDR_PRESENT_POSITION), -9000);
    EXPECT_EQ(
        transact(5, DXL2_INST_CLEAR, {X430_CLEAR_MULTI_TURN, 0x44, 0x58, 0x4C, 0x22})[0].error, 0);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), -9000 + 3 * 4096);
    EXPECT_EQ(get(X430_ADDR_GOAL_POSITION), -9000 + 3 * 4096);
    x430_sim_step(&sim, 100);
    EXPECT_EQ(get(X430_ADDR_PRESENT_POSITION), -9000 + 3 * 4096);
}

#endif
