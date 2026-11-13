# KeyStream — User Manual

Personal Windows client based on chiaki-ng • November 2026 • Connection, controls and troubleshooting guide

## 1. Start here: your working setup

You have already completed PSN sign-in, generated a token, registered your PS5 with an eight-digit pairing PIN, and successfully played from another city. Registration is saved. Opening the app tomorrow does not normally require another redirect URL or pairing PIN.

For the connection that you have proved works, the Android phone must remain connected to Tailscale and to the broadband Wi-Fi beside the PS5 throughout gameplay. Your Windows laptop must also be connected to Tailscale. Use the saved entry containing the PS5's private IP address.

The phone is the gateway carrying traffic between Tailscale and the PS5. The console cannot run Tailscale itself. Saving registration credentials does not replace that gateway.

You can now test the separate registered **Remote Connection via PSN** entry. If that works without the VPN, you can play without the phone. If the same unsupported-network error remains, use your working VPN path. Stock chiaki-ng also depends on successful network traversal for its PSN method. It does not universally need an Android phone after registration.

Your practical choices are:

| Method | What must remain available? | Phone needed? |
| --- | --- | --- |
| PSN connection | PS5 internet, laptop internet, compatible PSN connection establishment | No, if this method succeeds |
| Your current VPN connection | Laptop Tailscale, Android subnet router, PS5 LAN/Wi-Fi | Yes, while that phone is the only gateway |
| VPN through another gateway | An always-on supported router, computer or small server beside the PS5 | No Android phone; another gateway does its job |
| Manual internet connection | Suitable public addressing, controlled console router and correct port forwarding | No, when correctly configured and reachable |
| Same home network | Laptop and PS5 on the same reachable LAN | No |

No app update can guarantee a direct connection through every combination of carrier NAT, router and ISP. Replacing the phone with a permanent gateway is the dependable way to remove reliance on someone keeping their personal phone there, if PSN remains incompatible.

## 2. Open, update and close the app

1. Open your existing PS5 Remote KBM shortcut, or open the extracted `PS5RemoteKBM-win-x64` folder and double-click `PS5RemoteKBM.exe`.
2. Keep the executable with its DLLs, `platforms`, QML and other bundled folders. Do not run it from inside the ZIP or copy only the EXE elsewhere.
3. To update, close the app first and extract the new package into its own folder. You can also replace the existing portable installation while it is closed. Update your shortcut if you choose a new location.
4. Use the same Windows user account. Saved console registration and PSN credentials are stored separately from the portable executable.
5. To finish playing, release the mouse with Escape, open the stream menu with Ctrl+O, and disconnect. Choose to leave the console awake or put it into Rest Mode deliberately.

Closing the app does not delete pairing. The next connection needs a reachable, awake or wakeable console. A fully shut-down PS5 normally needs someone or another supported mechanism to turn it on.

For diagnostics without the video renderer, launch `PS5RemoteKBM.exe --connection-tools`. For the local input tester, use `--input-test`.

If rendering fails, launch `PS5RemoteKBM.exe --opengl`. This selects OpenGL in the active configuration. The fallback notification concerns graphics rendering on your laptop; it does not explain a PSN networking failure.

## 3. The four separate things called “connection”

**PSN sign-in** identifies your Sony account to PSN. You sign in in your browser and paste the returned redirect URL into the app. The app obtains tokens and your account identifier.

**Console registration** pairs this client with the PS5. The eight-digit Link Device code is a temporary registration code. The client saves the resulting console keys.

**Network reachability** determines whether packets can travel between laptop and console. PSN negotiation, a same-LAN route, a VPN subnet route or configured internet forwarding can supply this path.

**A streaming session** uses the saved pairing and reachable path to exchange video, audio and controller reports. It lasts only while the devices and network path remain available.

Success at an earlier stage does not prove a later stage. A visible PSN console can still be unreachable. A successfully registered console can still fail to stream if the phone leaves Wi-Fi. A successful phone ping does not prove the PS5 route works.

## 4. How gameplay travels

The PS5 runs the game. It compresses video and audio and sends them to your laptop. The laptop decodes and displays them, captures your input when armed, and sends controller reports back. The game then produces the next frame.

```text
Your verified VPN path

Laptop app <-> Windows Tailscale <-> internet <-> Android Tailscale
                                                   |
                                              subnet routing
                                                   |
                                               broadband Wi-Fi
                                                   |
                                                  PS5

Possible PSN path

Laptop app <-> PSN signalling / negotiation <-> PS5 account presence
     |                                          |
     +--------- direct Remote Play path ---------+
                 if traversal succeeds
```

The phone is forwarding packets in the VPN arrangement; it does not run or decode the PS5 game for you. A Tailscale relay may sit between the two Tailscale devices if they cannot establish a direct peer path. This is separate from the app's PSN method.

Input response includes more than ping: game processing, capture/encoding, network travel, buffering, decoding and display all contribute. A phone ping is a useful network measurement, not a measurement of total input-to-picture delay.

## 5. Configure the PS5

Do this on the PS5 account you intend to use:

1. Open **Settings → System → Remote Play** and enable Remote Play.
2. Open **Settings → System → Power Saving → Features Available in Rest Mode**.
3. Enable **Stay Connected to the Internet** and **Enable Turning On PS5 from Network**.
4. Keep that console online and signed into the intended PSN account.
5. Prefer Ethernet between PS5 and broadband router when available. If using Wi-Fi, keep signal strong and avoid guest networks that isolate devices.
6. Record the PS5's IPv4 address from its network/connection status when using the manual VPN method. A router DHCP reservation prevents it changing later.

These Rest Mode options support waking the console for a new session. They do not keep the game running or the current stream alive while the PS5 is resting. See the Rest Mode section for the everyday sequence. Sony documents the console settings in its [Remote Play setup guide](https://remoteplay.dl.playstation.net/remoteplay/lang/en/1100004.html).

## 6. PSN token: what is saved and when to sign in again

Once the token-generation dialog reports success, close that dialog and return to the home page. The app stores the tokens locally and uses the refresh flow when needed. You do not normally repeat the browser sign-in each launch.

Sign in again if you explicitly log out, remove the settings, move to another Windows user/computer, Sony invalidates access, or the app reports that credentials have expired and cannot be refreshed. A network timeout alone is not a reason to regenerate tokens.

When sign-in is required:

1. Open Settings, then the Config section, and start PSN setup.
2. Use the normal external browser flow opened by the app.
3. Complete Sony sign-in yourself.
4. Copy the final redirect URL and paste it into the app's redirect field.
5. Run Setup and wait for the token-generation result.
6. Close the result dialog and refresh PSN hosts on the home page.

The redirect URL and tokens contain login material. Keep them out of messages, screenshots and shared logs. Tokens, console keys and saved console PINs in this fork use Windows current-user DPAPI protection. Copying a settings export to another PC does not provide portable decrypted credentials. Pair/sign in on the new machine instead.

The account ID is different from your display name and different from either pairing PIN. PS5 registration uses the encoded account ID populated by the PSN flow.

## 7. Understand the home-page entries

| Label | Meaning | What clicking it uses |
| --- | --- | --- |
| Discovered on network | The app received a console discovery response | The discovered network address |
| Saved IP / VPN route | You saved an address manually | That exact address and the selected registered console |
| Discovered + saved IP | A discovery result matches a saved manual host | The combined entry; this does not indicate two simultaneous streams |
| Remote Connection via PSN | The PSN console identity matches saved registration | PSN connection establishment |
| Automatic Registration Available | PSN lists this console but it is not paired here | A registration attempt, subject to network/account support |

The older wording “discovered + manual” means the same thing as “Discovered + saved IP.” It does not prove PSN works and does not mean another pairing is required.

Discovery is usually local broadcast traffic. Routed VPNs generally do not carry those broadcasts. A PS5 missing from discovery can still work through its saved IP. A manual entry may not show a live status or game title; that alone is not a failed connection.

The **Host** for your VPN method is the PS5's broadband LAN IP, such as `192.168.1.50`. It is not the Android phone's Tailscale address. The example address in this manual must be replaced with your actual PS5 address.

## 8. Registration with an eight-digit code

You have already done this. Repeat only when pairing has been removed or must be created on a new client/user.

1. Confirm Remote Play is enabled and the PS5 is awake.
2. Establish the network path. For your setup, connect both Tailscale apps, keep the phone on PS5 Wi-Fi, and confirm the PS5 route is approved.
3. In the Windows app, sign in to PSN if needed so the account ID is populated.
4. Open **Settings → Consoles → Register New**. Choose PS5.
5. Enter the PS5 LAN IP in Host. A real IP selects unicast registration; this build does not have a separate “disable broadcast” checkbox. Do not enter `255.255.255.255` across the VPN.
6. Ask the person at the PS5 to open **Settings → System → Remote Play → Link Device / Pair Device**. Wording can vary with system language/version.
7. Enter that current eight-digit code into **Remote Play PIN** in the app before it expires.
8. Leave the optional four-digit console login PIN blank unless that console account actually uses one. This is not the Link Device code.
9. Click Register. Read the result and close the log dialog after success.
10. Add or retain a manual host using the PS5 IP and select the newly registered console. Connect and confirm video/control.

If the code expires, obtain a fresh code and retry. Repeated network timeouts need a route/connectivity check, not more PIN attempts. Registration does not install Tailscale on the PS5.

## 9. Everyday play using your Android gateway

### Beside the PS5

1. Keep the PS5 connected to the same broadband network used for pairing.
2. Connect the Android phone to that broadband Wi-Fi. Mobile data alone does not give the phone access to the PS5 LAN.
3. Open Tailscale and connect to the correct tailnet.
4. Confirm the advertised PS5 route is still present. Keep the phone powered and let Tailscale remain active in the background; Android battery restrictions can interrupt it.
5. Turn on the PS5 if wake over the routed path is unreliable.

### On your laptop

1. Connect Tailscale to the same authorized tailnet.
2. Open KeyStream.
3. Select the saved PS5 IP entry. You do not need another Link Device PIN.
4. Wait for live video and audio.
5. Click/focus the stream and press F1. Confirm the title/overlay says input is captured.
6. Play. Escape releases the mouse. F1 captures it again after switching apps or resizing.
7. When finished, release input, open Ctrl+O, and disconnect. Decide whether to leave the console awake or put it to rest.

The phone can usually keep its screen off if its OS allows Tailscale to run, but this must be tested on that specific phone. Screen off is different from disconnecting Wi-Fi, stopping the VPN, force-closing the app or shutting the phone down. Avoid overheating during long sessions.

If the phone leaves home, disconnects, changes to another Wi-Fi, sleeps its VPN service or runs out of battery, your current path fails. A permanent gateway on the PS5 network avoids that dependence on the personal phone.

## 10. Test PSN after registration: can the phone be removed?

Run this controlled test when someone can wake the console if needed:

1. Finish/disconnect the working VPN session. Keep the saved registration and manual entry.
2. Have the PS5 awake and online.
3. Disconnect Tailscale on your laptop for this test so the VPN cannot provide the route. The helper can leave their phone untouched.
4. Open the app and refresh PSN hosts.
5. Select the registered entry labelled **Remote Connection via PSN**, rather than the saved private-IP entry.
6. Attempt a connection. If it works, play briefly and confirm video, audio and input. This proves a phone-free path on the tested networks.
7. If it fails with the unsupported-network message, note the error and return to the home page. Restore laptop Tailscale and use the saved PS5 IP to resume your proven method.
8. Do not delete registration or generate new tokens merely because this connection attempt failed.

Upstream chiaki-ng uses PSN to exchange connection information and attempts UDP traversal. It documents that some network types remain unsupported and provides a manual alternative. Your earlier failure occurred while establishing reachability, so pairing success alone does not establish that the same PSN attempt will now succeed. See [chiaki-ng remote connection documentation](https://chiaki-ng.github.io/chiaki-ng/setup/remoteconnection/).

A different laptop network, such as another broadband connection or a hotspot, can change the result. If the official Sony client works on exactly the same two networks while this fork does not, sanitized session logs may help investigate differences. It still does not guarantee that a small code change can fix traversal.

## 11. Set up or replace the Android subnet router

For the already-working phone, leave its current configuration intact. This section is for recovery or replacement.

1. Install a current Tailscale Android app and connect the phone to PS5 Wi-Fi.
2. Enroll it in your intended tailnet. A user appearing in the admin Users page does not prove that their phone has joined that network. Verify the phone appears in Machines/Devices.
3. If normal sign-in keeps selecting another tailnet, use the supported account/network-selection flow. The one-use auth-key enrollment method you used is another option: generate it in the intended tailnet and enter it privately in the phone app. Do not share the key in chat.
4. In Android Tailscale settings, open **Subnet routing**, add the PS5 IPv4 address as a `/32` route, for example `192.168.1.50/32`, and enable advertising. Menu placement can change between versions.
5. In your tailnet's admin console, approve that device's advertised route. Access rules must also allow your laptop/user to reach the PS5 destination.
6. Keep the phone connected to Tailscale and to the console's Wi-Fi. Verify PS5 reachability from the laptop separately from phone reachability.

A `/32` covers one PS5 address. If the PS5 later receives a different DHCP address, the old route and manual entry will no longer match it. Reserve its address or update both. Advertising the entire LAN is broader access than needed for one console.

An exit node is a different feature; you do not need to send all laptop internet traffic through the phone to reach a specifically routed PS5. Android subnet routing is documented in [Tailscale's setup guide](https://tailscale.com/docs/features/subnet-routers/how-to/setup?tab=android).

If both locations use the same LAN subnet, verify that the PS5 destination takes the intended VPN route. A host-specific `/32` can help, but an exact local address collision still needs attention. Test from another laptop network if uncertain.

## 12. Other ways to connect

**Same LAN:** place laptop and PS5 on a network where they can reach each other. Use discovery or the actual PS5 IP. Guest Wi-Fi/client isolation can prevent communication even when both devices show the same Wi-Fi name.

**Permanent VPN gateway:** replace the Android forwarding role with a supported always-on subnet router on the PS5 LAN. This can be a compatible router, small server or computer. Advertise/approve the route, verify access and use the same saved PS5 IP/registration. Check the gateway platform's actual routing support before buying hardware.

**Manual internet forwarding:** this requires authorized router administration and an inbound-reachable addressing arrangement at the PS5 location. Follow the PS5-specific port/protocol table in upstream's remote connection guide. Reserve the PS5 LAN address, configure only the required forwards to it, add the site's public endpoint as a manual host and test from outside that network. Never forward to the phone by guessing. CGNAT or another uncontrolled upstream router can make inbound forwarding ineffective. An ISP-assigned public address or a VPN may then be needed.

Do not put the PS5 in a router DMZ or disable firewalls as a blanket troubleshooting step. Automatic PSN setup and manual forwarding are separate paths. Neither is required just because the VPN stream was choppy.

## 13. Improve stability: choose a useful baseline

F4 opens **Connection and quality**. The presets apply to the next session and change **both Local PS5 and Remote PS5** video profiles. A private PS5 address reached through a VPN can select Local settings, despite crossing the internet. Choose the preset, apply it, disconnect and reconnect.

| Preset | Resolution / frame rate | Target bitrate | Use |
| --- | --- | --- | --- |
| Low bandwidth | 540p / 30 fps, H.264 SDR | 2 Mbps | Start here when higher settings stall or disconnect |
| Stable connection | 720p / 30 fps, H.264 SDR | 5 Mbps | Try when the 2 Mbps stream stays steady |
| Responsive | 720p / 60 fps, H.264 SDR | 8 Mbps | Prefer for aiming once the path sustains it |
| Clearer picture | 1080p / 60 fps, H.264 SDR | 15 Mbps | Test after the lower presets are stable |

If you see large smeared blocks, try **Request a fresh keyframe when video recovery fails** in **Settings → Stream** or in F4. The app asks the PS5 for a fresh reference picture when it cannot reconstruct a damaged frame. The picture can pause while it waits. Compare it on and off at the same bitrate to see which behavior you prefer.

These are starting points, not measured capacity recommendations for your current internet. The network also carries audio, input, protocol overhead and recovery data. Leave headroom rather than matching an internet speed test's peak throughput. The PS5 site's upload and the laptop site's download both matter.

For your Intel integrated-graphics laptop, start with H.264 SDR and modest resolution. Keep the existing decoder unless evidence points to decode trouble. OpenGL rendering and hardware video decoding are separate settings. A Vulkan display fallback does not automatically mean video decoding failed.

Use this order for improvements:

1. Wire the PS5 to broadband by Ethernet if possible.
2. Use Ethernet or strong, uncongested Wi-Fi on the laptop. The Android gateway still has its own Wi-Fi link in the present setup.
3. Keep the phone near its access point, powered and free of battery restrictions that stop routing. Avoid thermal throttling.
4. Pause large uploads, cloud backups, downloads and video streams at both locations while testing.
5. Check whether Tailscale settles on a direct peer connection or remains relayed.
6. Try the Stable preset, then Responsive if loss/stalls stay low.
7. Change one factor at a time and compare the same game/scene for several minutes.

More bitrate is not automatically better. It can turn a variable connection into loss and stalls. More buffering may hide jitter while increasing response time. Changing a “reported packet loss” limit does not repair real lost packets; do not treat that threshold as a network fix.

## 14. Measure direct/relay, ping and stream health

In F4, select **Load Tailscale peers**, choose the Android subnet router and run the peer check. These commands are read only. The test reports how the peer path is reached and its approximate round-trip time. It does not benchmark the PS5 or change route approval, access rules or VPN settings.

Tailscale may initially reply through DERP before negotiating a direct path. Persistent relaying can add delay or limit throughput; a direct path usually performs better. Tailscale explains the distinction in [connection types](https://tailscale.com/docs/reference/connection-types) and [performance troubleshooting](https://tailscale.com/docs/reference/troubleshooting/poor-performance-tailnet).

For manual diagnostics in ordinary PowerShell, replace all example addresses/names:

```powershell
& "$env:ProgramFiles\Tailscale\tailscale.exe" status
& "$env:ProgramFiles\Tailscale\tailscale.exe" ping --c=5 --until-direct=false PHONE-TAILSCALE-NAME
& "$env:ProgramFiles\Tailscale\tailscale.exe" netcheck
Find-NetRoute -RemoteIPAddress 192.168.1.50
ping -n 20 192.168.1.50
Test-NetConnection 192.168.1.50 -Port 9295 -InformationLevel Detailed
```

The phone probe checks the VPN peer path. The route command checks Windows' selected path to the PS5. A PS5 may ignore ICMP, especially while resting, so failed ICMP alone does not prove a broken route. TCP 9295 may be useful when the awake PS5 listens, but success does not prove UDP streaming or registration.

During gameplay, open Settings and enable **Show Stream Stats During Gameplay**. If the option is hard to find, look in the streaming/network settings near the packet-loss and FEC options. Reconnect/return to the stream as needed for the display to update.

| Observation | What to investigate |
| --- | --- |
| Rising stream packet loss with stalls | Congestion, unstable Wi-Fi, relaying, too much bitrate |
| Low loss but growing decoder/render queue or dropped frames | Laptop decode/render load, resolution, graphics backend |
| Smooth picture but late responses | Distance/RTT, game processing, buffering, mouse curve |
| Phone reachable, PS5 unreachable | Subnet route, approval/access, phone's Wi-Fi, PS5 power/IP |
| Everything works except PSN entry | PSN traversal path; use working manual VPN entry |

Different metrics describe different stages. Do not call a short smooth interval proof that the whole path is stable. Keep a small comparison record: connection path, preset, relay/direct result, observed packet loss, stalls and mouse response. Logs can contain device/network identifiers; review before sharing.

## 15. Keyboard and mouse controls

| Laptop input | PS5 control |
| --- | --- |
| F1 | Capture/release input while a stream is focused |
| Escape | Release mouse and neutralize held controls |
| F2 | Controls, profiles and local input test |
| F3 | Offline user manual |
| F4 | Connection diagnostics and video presets |
| Alt+F5 | Toggle Watch only on the current stream |
| Ctrl+O | Open or close the stream menu, including during keyboard capture |
| W / A / S / D | Left stick |
| Mouse movement | Right stick |
| Left / right mouse button | R2 / L2 |
| Space / C / E / R | Cross / Circle / Square / Triangle |
| Shift / V | L3 / R3 |
| Q / F | L1 / R1 |
| Tab / Enter | Touchpad click / Options |
| Arrow keys | D-pad |
| P | PS button |
| Ctrl | Partial left-stick walking tilt |

Control is deliberately unarmed until F1. Switching applications, losing focus, opening controls, minimizing, moving/resizing the window and disconnecting release capture and held controls. Return to the stream and press F1 again. Escape is for releasing the laptop mouse; use C for the game's Circle action.

F2 saves settings as you change them. To make a separate setup for a game, use **Save as** first, name it and change that profile. Input profiles are independent of the app's video/console configuration profiles. The local **Test capture** mode displays stick/button/trigger reports without sending them to a console. Escape exits capture.

Click a binding and press a key to change it. Clear removes it. You can also assign one of five mouse buttons. Escape and F1–F4 are reserved. Left/right Shift and Ctrl share Qt's corresponding key binding.

### Watch someone play on the PS5

1. Start Remote Play using your usual paired PS5 profile and connection route.
2. Press **Alt+F5** to switch to **Watch only**. You can also select **Watch** on the stream toolbar or press **Ctrl+O** and select **Watch only**.
3. Have the person at the PS5 press the **PS** button on their DualSense and assign it to a **different PS5 user profile** from the one your laptop uses. Keep the Remote Play profile signed in. They can play on their monitor while you watch and listen on the laptop, subject to the game's Remote Play support. [Sony's guide to local users during Remote Play](https://remoteplay.dl.playstation.net/remoteplay/lang/en/1100004.html)
4. To return to Remote Play controls, press **Alt+F5** again or choose **Resume control** in the stream menu. Release any held gamepad buttons, triggers or touches and center the sticks. Press **F1** if you want keyboard and mouse capture.

Watch only blocks gameplay input and mutes the laptop mic. You can turn the mic back on after resuming control. The toggle keeps the active video and audio session open. A fresh connection starts in normal Remote Play. If the local controller takes over the same PS5 profile, the console may end the Remote Play session. When you resume control, your input still belongs to the paired profile. Taking over the other person's character or save depends on the game and may require connecting under that profile.

## 16. Make aiming feel closer to a mouse

The PS5 receives right-stick movement from this client, not a native mouse aim API. Each game can impose turn-speed limits, deadzones, aim assist and acceleration. This client cannot universally provide PC-style one-to-one camera rotation.

An optional **Precise mouse preset** in F2 reduces the app's contribution to drifting after you stop moving. It uses a rolling eight-millisecond window, a linear response, no added app deadzone/anti-deadzone and no recenter tail. It preserves sensitivity, key bindings, walking tilt, inversion and maximum output. Your old profile is not changed until you choose the preset.

1. Use Save as to preserve your current profile.
2. Apply **Precise mouse preset**.
3. Test small movements and quick direction reversals in the local preview.
4. Try the same movements in the game. Set the game's controller deadzone low enough for small movements to register without causing drift from any actual connected controller.
5. Adjust X/Y sensitivity gradually. If large motions already hit the turn-speed ceiling, increasing sensitivity further makes precision harder.
6. Prefer a linear game response curve and reduced aim acceleration when the game offers those options. Compare aim assist on/off according to the game's behavior and your preference.
7. If motion feels noisy or inconsistent with your mouse/report cadence, increase smoothing modestly or return to **Smooth stick preset**.

The Smooth stick preset uses the earlier 16 ms sampling and 35 ms recenter tail. It changes the window/tail mode; it does not erase your key mappings. App anti-deadzone can help overcome a game deadzone but can also make tiny movements jump. Change it carefully after testing.

## 17. Rest Mode, waking and reconnecting

Entering Rest Mode stops the active game stream. The disconnect you observed is expected, even with network-in-rest settings enabled.

For daily use:

1. Finish the current session and put the console to rest if desired.
2. Leave broadband and the VPN gateway online if you plan to use that method later.
3. Next time, attempt Wake Up on the appropriate saved console or the supported PSN connection.
4. Wait for the console to boot before connecting. Do not repeatedly submit pairing codes.
5. If routed wake fails, ask the person at the console to turn it on, then connect normally. A working stream while awake does not prove wake packets can cross that route.

Broadcast discovery and broadcast wake commonly do not cross routed VPNs. Manual wake may work where the console accepts the app's unicast packet, and PSN wake depends on the PSN path. Test wake separately while someone can assist. Fully shutting the PS5 down is different from Rest Mode.

If the console repeatedly goes to rest during play, investigate local power commands, inactivity settings and HDMI-CEC power links as well as connectivity. The client cannot continue a game after the host has suspended it.

## 18. Keep the local monitor dark

Remote Play does not put the PS5 in a separate private game session. The awake console can still output its picture and sound locally. There is no supported HDMI-output-off command implemented in this client, and a monitor being off does not mean the PS5 has stopped generating HDMI output.

For a practical dark-screen setup without unplugging HDMI:

1. On PS5, open **Settings → System → HDMI**.
2. Disable **Enable HDMI Device Link**, or disable the relevant One-Touch Play/Power Off Link options if you want to retain other CEC behavior.
3. Turn the monitor/TV off, or switch it to another input.
4. Start Remote Play and verify the display stays off and the console stays awake.
5. If it still powers on or sleeps the PS5, inspect the monitor's own HDMI-CEC/auto-input/power settings.

This avoids screen illumination and CEC-triggered power changes. Someone at the PS5 can still turn the monitor back on or select its HDMI input. For actual isolation of the physical video path, external hardware/display control would be required; an app-only promise of HDMI privacy would be misleading.

The PS5's **Hide Remote Play Connection Status** option hides a status message, not the game picture. Do not confuse it with a privacy mode. HDMI-CEC power-link behavior is illustrated in [MSI's monitor guide](https://storage-asset.msi.com/global/picture/faq/10014069@2023-0818-0202-264112@%5BMNT%5D%20HDMI_CEC_function.pdf).

## 19. Fix common problems

| Symptom | Next steps |
| --- | --- |
| PS5 visible but PSN connection says unsupported network | Keep registration. Restore the working Tailscale route and use the PS5 IP entry. Test another laptop network if useful. |
| Saved IP worked yesterday and now fails | Check PS5 power/address, phone Wi-Fi and Tailscale, route approval/access and laptop route selection. |
| Phone ping works but PS5 does not | Verify the phone is still on console Wi-Fi and advertising the current PS5 IP; test the awake console separately. |
| Eight-digit pairing code fails | Confirm account/PS5 selection, use a fresh code, verify network reachability; distinguish the optional four-digit login PIN. |
| App asks for PSN login again | Check the exact credentials-expired message. Sign in if refresh has failed; do not do this merely for packet loss. |
| Token result or error text too long | New bounded dialogs wrap and scroll. Resize the main window; use the scrollbar for long content. |
| Vulkan fallback popup | OpenGL is the alternate graphics renderer. This is separate from NAT/PSN reachability; use --opengl if needed. |
| Video is choppy | Start with F4 Stable, reconnect, check relay/direct and stream loss/queue stats, reduce competing traffic and Wi-Fi hops. |
| Video is fine but controls do nothing | Focus the stream, press F1, check capture state, test the mapping with F2. |
| Camera keeps moving after mouse stops | Try the optional Precise preset and lower recenter/smoothing; check game acceleration/deadzone. |
| Input stops after resizing or Alt+Tab | Expected capture release. Focus the stream and press F1 again. |
| Rest Mode disconnected Remote Play | Expected. Wake the console and begin a new session. |
| Monitor turns on or monitor power puts PS5 to rest | Review HDMI Device Link/CEC and monitor power controls; the client does not disable HDMI output. |
| No audio | Check laptop output device/volume, app audio-disabled settings and PS5 audio. Video success alone does not validate audio. |
| Missing DLL / executable will not start | Fully extract the complete matching portable package; retain bundled folders. |

## 20. Saved data, safety and removal

The client uses its own `PS5RemoteKBM` settings namespace. Native controls use a separate `Controls` store. Tokens and pairing material are not stored in the portable EXE folder as plaintext credentials. Configuration exports retain protected data and should still be treated as private.

Logs normally reside under `%APPDATA%\PS5RemoteKBM\PS5RemoteKBM\log`. This fork redacts protocol hexdumps and credential-bearing account-lookup errors, but logs can still contain network/device identifiers. Review and sanitize before sharing. Never send a redirect URL, auth key, token, saved console key or PIN with a bug report.

Deleting the extracted program folder removes the portable files, not necessarily the saved configuration. Use app logout to remove local PSN tokens and remove registered consoles separately when you intend to forget pairing. Revoking a Tailscale invitation/key and removing a Tailscale device are different actions; do them in the relevant admin interface when no longer needed.

You do not need to uninstall the app or erase saved pairing to switch between PSN and VPN paths. Keep your known-working route while evaluating another method.

## 21. What has actually been verified

The owner reports successful eight-digit PS5 registration across cities through the Android subnet route, live connection and keyboard/mouse control. This establishes that the core workflow works on that tested setup.

Automated checks cover protocol components, native input behavior, protected credentials, input-profile persistence, credential-error redaction, audio/video initialization and responsive message geometry. Software checks do not measure the current internet link, guarantee a new game feels better or establish a phone-free PSN path.

The optional mouse preset, new quality presets and new UI require comparison during your own gameplay. Audio quality, routed wake reliability, long-session thermal/battery behavior and future network conditions should be tested separately. See the accompanying test-results document for the build's recorded checks.

## 22. Quick routine for tomorrow

1. PS5 online, awake or wakeable.
2. For your proven path: Android on PS5 Wi-Fi with Tailscale routing active; laptop Tailscale connected.
3. Open KeyStream and select Saved IP / VPN on the console card. No new redirect URL or registration PIN needed.
4. Start with Responsive; use Stable if the picture stalls. Reconnect after changing a preset.
5. F1 to capture, Escape to release, F2 controls, F3 this manual, F4 connection tools.
6. Leave the console awake while playing. Keep the monitor off separately with suitable CEC settings.
7. Disconnect or put the console to rest when finished.
