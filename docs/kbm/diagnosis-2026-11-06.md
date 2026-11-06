# PSN registration reachability and renderer diagnosis — 2026-11-06

Inspected the supplied 43.2-second recording, renderer-fallback screenshot, recent
local session logs and the current client source. No account credentials were read
or changed. No registration, router, firewall or remote-PC settings were changed.

## Verified failure boundary

Repeated sessions in the 21:43–21:46 IST logs create the PSN session, send the
control offer and start the PSN session. Candidate checks then retry 20 times with
no first response. The final messages report a candidate-response timeout, no
reachable control candidate and failure to punch a hole for the control connection.
The recording shows automatic registration waiting and then the matching network
error. The console remains listed as eligible for automatic registration.

`StreamSession::ConnectPsnConnection()` returns the hole-punch error before starting
the streaming session's RUDP registration exchange. Therefore this is a **failure
to establish the console's direct UDP control path**, before pairing credentials
are exchanged. PSN sign-in/session API access works in these attempts; recreating
the login token is not indicated by this evidence. The UI's phrase "established
connection" must not be interpreted as a successful direct console connection.

NAT behavior, CGNAT, filtering/firewalls on either network, or console reachability
can cause unanswered candidates. These logs do not identify a particular ISP,
router, firewall or NAT type. UPnP discovery also times out, but that alone does not
prove why hole punching failed. No conclusion can yet be made about successful
clean-client PSN registration authorization on this console/firmware.

## Next tests and fallback

1. Have the person beside the console keep PS5 fully awake, confirm Remote Play is
   enabled and that the relevant PSN account is signed in. Retry once from a different
   laptop network, such as a mobile hotspot. Reuse saved authentication.
2. If unchanged, stop repeated PSN retries and configure the remote Windows PC as a
   Tailscale subnet router for the PS5 LAN address. Follow `vpn-registration.md`:
   route advertisement/approval, access policy, forwarding verification and default
   SNAT matter. A VPN installed only on the PCs does not make the PS5 reachable.
3. Verify the PS5 route, then use Settings → Consoles → Register New, PS5 LAN IP,
   PS5 target, broadcast disabled, the authenticated Account ID and the current
   eight-digit Link Device PIN. Type the PIN only into the app. Neither the laptop's
   public IP nor the remote PC's Tailscale IP belongs in the PS5 host field.
4. After successful pairing try PSN again, but **pairing through VPN does not repair
   the UDP hole-punch path**. Retain the manual IP entry and VPN gateway for streaming
   if PSN still fails. Gateway uptime and VPN bandwidth/relay behavior then matter.

A remote-PC-only local registration followed by copying a settings INI is not an
immediate workaround for this fork: credentials use current-user Windows DPAPI and
are not transferable between Windows users/computers through a normal export.

## Separate graphics warning

The screenshot explicitly reports repeated Vulkan swapchain failure and automatic
OpenGL fallback for that launch. This is a renderer failure, independent of the
UDP candidate timeout. Dismiss it and persist **Settings → Video → Renderer Backend
→ OpenGL**, accepting the app restart. The `--opengl` launch switch is another way
to persist that choice for the active profile. This does not change registration
or authentication. Decoder initialization in the network logs is not evidence
that a real gameplay frame was decoded or displayed.

## Sources

- Local source: `gui/src/streamsession.cpp:ConnectPsnConnection`,
  `gui/src/qmlbackend.cpp:checkPsnConnection`, and `lib/src/remote/holepunch.c`.
- [chiaki-ng remote connection limitations](https://chiaki-ng.github.io/chiaki-ng/setup/remoteconnection/)
- [Related upstream network-dependent failure report](https://github.com/chiaki-ng/chiaki-ng/issues/788)
- [Tailscale subnet routers](https://tailscale.com/docs/features/subnet-routers)
- [Tailscale Windows route setup](https://tailscale.com/docs/features/subnet-routers/how-to/setup?tab=windows)

These findings do not claim the VPN route, registration or live stream has passed;
those require access to the remote PC and PS5.
