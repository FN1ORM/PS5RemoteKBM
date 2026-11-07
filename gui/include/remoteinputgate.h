// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#pragma once

#include <chiaki/controller.h>
#include <cstdlib>

class RemoteInputGate
{
public:
    bool IsViewOnly() const { return view_only; }

    void SetViewOnly(bool enabled)
    {
        view_only = enabled;
        waiting_for_release = !enabled;
    }

    bool Suppresses(const ChiakiControllerState &state)
    {
        if (view_only)
            return true;
        if (!waiting_for_release)
            return false;

        // Raw SDL axes may rest slightly off center. Motion sensor data is
        // continuous and does not indicate a held control.
        constexpr int stick_center_tolerance = 4096;
        constexpr int trigger_release_tolerance = 4;
        if (!state.buttons && state.l2_state <= trigger_release_tolerance &&
            state.r2_state <= trigger_release_tolerance &&
            std::abs(state.left_x) <= stick_center_tolerance &&
            std::abs(state.left_y) <= stick_center_tolerance &&
            std::abs(state.right_x) <= stick_center_tolerance &&
            std::abs(state.right_y) <= stick_center_tolerance)
        {
            for (const auto &touch : state.touches)
                if (touch.id >= 0)
                    return true;
            waiting_for_release = false;
        }
        // Send one neutral report even on the frame that re-arms input.
        return true;
    }

private:
    bool view_only = false;
    bool waiting_for_release = false;
};
