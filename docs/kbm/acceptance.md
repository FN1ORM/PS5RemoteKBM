# Console acceptance — owner-assisted tests

Status as of November 7: **partially completed by owner report**. PSN sign-in,
console listing, Android subnet routing, eight-digit registration and actual
cross-city video/keyboard/mouse control succeeded. PSN without the phone, audio
quality, routed wake, the new presets and prolonged stability still need live checks.
The numbered list remains a repeatable checklist; it is not a claim that every item
has passed. A build or local input test alone cannot prove streaming.
Never send PSN passwords, complete redirect URLs, tokens, registration keys or PINs
in chat. Enter them only into the browser/app yourself.

Record app/source version, Windows version, Intel driver, renderer/decoder, network
type and sanitized error text. For each step record pass/fail and what was actually
seen. Keep identifying details private. Use an unmodified retail PS5 with authorized
account and Remote Play enabled, initially awake with help available at its location.

1. Launch the separate app with its fresh settings. It must show no registered
   consoles. Open F2 → Test capture: W+D should tilt the left stick diagonally;
   moving the mouse should tilt the right stick and return to zero shortly after
   stopping. Verify both mouse triggers, buttons, modifiers and PS (P by default).
2. Hold W and a mouse button, then Alt+Tab. Return and verify released status and
   zero values. Repeat with Esc, minimizing, closing the panel and moving/resizing
   the stream window. F1 must be required to rearm after release.
3. Save a named input profile, change sensitivity and a binding, close/relaunch,
   select the profile and verify both settings and actual behavior were restored.
4. Sign in via the normal browser OAuth flow. Paste the redirect URL only into the
   app. Confirm PSN device listing. With **no prior console credentials**, click an
   unregistered PSN PS5 and attempt automatic registration. Record whether console
   authorization succeeds or the exact sanitized error. This is the remote-first
   experiment; do not mark it passed because a device name appeared.
5. If automatic registration fails, follow `vpn-registration.md`, verify the PS5's
   routed LAN address, and manually register PS5 with its current eight-digit PIN
   using its actual LAN IP (unicast registration). Confirm successful registration persists after restart.
6. Connect through PSN from the laptop's different-city network. Start at 720p60,
   H.264, modest bitrate (e.g. 8 Mbps), normal aspect ratio, SDR. Confirm **live PS5
   video**, not just the connection overlay, and **audible PS5 audio**. If hardware
   decode fails, test software decode at 720p30; separately record each result.
7. F1 capture. Navigate the PS5 UI using arrow keys, Space/C and P; launch a game
   without a controller. Verify WASD, diagonal normalization, mouse camera aim,
   LMB R2, RMB L2, Q/F shoulder buttons and other bindings inside that game.
8. Tune sensitivity/deadzone against that game's aim settings. Test invert Y,
   exponent, smoothing, output limit, recenter and walking tilt. Check no stuck
   controls after release or lost focus. Controller camera turn limits and game
   acceleration still apply; mouse-to-stick is not native PC mouse aim.
9. Verify fullscreen F11, aspect ratio, 30/60 FPS selection, volume/device selection,
   the existing stream statistics and readable logs. Use Task Manager during at
   least 5 minutes of gameplay to record actual CPU, working set and GPU decode use.
   Report frame drops/packet loss/latency only if measured by the client.
10. Disconnect and reconnect. Briefly interrupt the laptop network during a harmless
    game scene, restore it, and verify input releases and an actionable error or
    recovery occurs. Confirm video/audio after reconnect. Automatic recovery may
    depend on which connection phase failed; record actual behavior.
11. Put PS5 into Rest Mode with network wake enabled, then attempt PSN wake/reconnect.
    Also test the manual IP/VPN stream alternative. A VPN gateway must stay awake.
12. Log out in Settings → PSN, restart and verify no PSN access remains. Remove the
    console separately to delete its local registration data. If desired, use the
    console/account device-management UI yourself to revoke server authorization.

Overall success remains pending until steps 4 or 5 **and** 6–8 pass across the two
cities. Authentication, initial pairing and hardware observations require the owner.
