# Register across cities with a routed VPN

For your working **Android phone** gateway and everyday play, start with the
[full manual](user-manual.html), sections 9–11. The Windows/macOS alternatives
below are retained for people replacing the phone with another gateway.

The topology is **laptop → Tailscale → remote Windows PC → PS5 LAN IP**.
Installing Tailscale on the two PCs alone is insufficient. The PS5 cannot install
Tailscale; the remote PC must route traffic, stay awake and remain on the PS5's LAN.
This setup changes the remote gateway, not your laptop's Windows security settings.
Have the person with access to that PC perform the elevated commands below.

Record the PS5 IPv4 address from its network status and the remote PC's LAN interface
from `Get-NetIPConfiguration`. In the example the PS5 is `192.168.1.50` and the LAN
adapter is `Ethernet`; **replace both with the actual values**. Keep the PS5 address
stable using an existing DHCP reservation if available. No router port forwarding
is needed for this topology.

## Remote Windows 11 PC

1. Install [Tailscale](https://tailscale.com/download/windows), sign in to the same
   authorized tailnet on both PCs, and verify the remote PC reaches the awake PS5
   locally before attempting VPN registration. Confirm Remote Play is enabled.
2. In elevated PowerShell, inspect the interfaces and save their forwarding state:

   ```powershell
   Get-NetIPConfiguration
   Get-NetIPInterface -AddressFamily IPv4 |
       Select-Object InterfaceIndex,InterfaceAlias,Forwarding
   ```

3. Enable forwarding on the **identified** LAN and Tailscale interfaces. Do not
   enable forwarding on unrelated adapters. If your Tailscale adapter has a
   different alias, use that exact alias:

   ```powershell
   Set-NetIPInterface -InterfaceAlias 'Ethernet' -AddressFamily IPv4 -Forwarding Enabled
   Set-NetIPInterface -InterfaceAlias 'Tailscale' -AddressFamily IPv4 -Forwarding Enabled
   & "$env:ProgramFiles\Tailscale\tailscale.exe" set --advertise-routes=192.168.1.50/32
   ```

4. In the [Tailscale admin console](https://login.tailscale.com/admin/machines),
   approve this PC's advertised PS5 route. Also allow your laptop/user to access
   that destination in the tailnet access policy. Route approval and access grants
   are separate. Preserve existing policy rules; grant only the needed PS5 access.
5. Keep Tailscale's default subnet source NAT. The PS5 then replies to the remote
   PC's LAN address, without a router return route. Do not use the Linux-only
   `--snat-subnet-routes=false` switch on Windows. A custom non-NAT setup would
   require the LAN router to return tailnet traffic via the remote PC, which is
   unsuitable when router administration is unavailable.
6. Keep Windows Firewall enabled. The installed Tailscale service needs its normal
   network permissions, and any managed firewall must permit forwarding from the
   Tailscale interface to the specific PS5 address and stateful replies. Corporate
   endpoint policy may prevent this. Inspect dropped-packet logs and the actual
   Tailscale rules before adding narrow rules. Do not enable ICS or add generic
   WinNAT rules: they can conflict with other networking and Tailscale's NAT.
7. Recheck forwarding after a reboot and confirm the advertised route is still
   approved. Use the PC's power settings to keep it available during the session.

## Laptop verification and pairing

Run from ordinary PowerShell after installing and signing into Tailscale:

```powershell
& "$env:ProgramFiles\Tailscale\tailscale.exe" status
& "$env:ProgramFiles\Tailscale\tailscale.exe" ping REMOTE-PC-TAILSCALE-NAME
Find-NetRoute -RemoteIPAddress 192.168.1.50
Test-NetConnection 192.168.1.50 -Port 9295 -InformationLevel Detailed
```

A successful Tailscale ping to the **PC** only proves the first VPN hop. Verify
the selected route is the remote PS5 route, and test the PS5 address separately.
ICMP ping may be unanswered by the PS5; a failed ping alone does not prove routing
failure. TCP 9295 is useful when the awake console is listening but does not prove
UDP or registration. The definitive test is the app's registration response.

If both cities use `192.168.1.0/24`, advertising only `192.168.1.50/32` is more
specific than the laptop's LAN route. Verify route selection, especially if a local
device or the laptop itself has that exact IP. In that case change one network's
subnet if possible or use another network/hotspot for the laptop. Do not guess
that an overlapping route is reaching the intended console.

In the app sign into PSN through your browser so the Account ID is populated.
Use **Settings → Consoles → Register New**, choose PS5 and enter the **PS5 LAN IP**,
not the remote PC's VPN IP. In this build there is no separate broadcast checkbox:
entering the real PS5 IP instead of `255.255.255.255` automatically selects unicast
registration. Ask the person at the console
to open **Settings → System → Remote Play → Pair Device / Link Device** and type the current
eight-digit PIN directly into the app. Do not put the PIN, redirect URL, account
tokens or configuration export in chat. Repeat with a fresh PIN if it expires.

After registration, refresh PSN hosts and try the PSN console entry for ordinary
play. Keep the manual IP entry for VPN streaming if PSN NAT negotiation fails.
Discovery broadcasts and LAN wake packets normally do not traverse a routed VPN;
use PSN wake or ask the remote person to wake the PS5 initially. Rest-mode wake
requires the PS5's network wake and internet-in-rest settings.

## macOS gateway alternative

Use a Tailscale-supported macOS subnet router, advertise the PS5 `/32` with
`tailscale set --advertise-routes=192.168.1.50/32`, approve it and check policy as
above. Enable/verify IPv4 forwarding (`sysctl net.inet.ip.forwarding`; an authorized
administrator can use `sudo sysctl -w net.inet.ip.forwarding=1` where required by
the installed client). Avoid modifying pf/NAT rules unless packet evidence shows
the client's default forwarding/SNAT path is insufficient. Recheck after restart.

## Failure isolation and undo

If the PC cannot reach the PS5 locally, fix local network/Remote Play availability.
If Tailscale PC ping fails, fix tailnet authentication/connectivity. If PC ping
works but the route is wrong, fix advertisement/approval/overlap. If the route is
right but registration times out, check gateway forwarding, access rules, local
firewall, PS5 IP/PIN and awake state. DERP relay use can add latency; it does not
guarantee a usable streaming bandwidth. Radmin VPN alone also lacks the PS5 subnet
route and return NAT; no untested Radmin routing recipe is offered.

To undo, withdraw **only the route you added** (preserve any pre-existing advertised
routes), remove its approval/access grant, and restore forwarding values recorded
in step 2. `tailscale set --advertise-routes=` clears *all* advertised routes, so
use it only when this PS5 route was the gateway's sole route.

References, verified 2026-11-05: [Tailscale's Windows subnet-router and default SNAT documentation](https://tailscale.com/docs/features/subnet-routers?tab=windows),
[Microsoft Set-NetIPInterface](https://learn.microsoft.com/en-us/powershell/module/nettcpip/set-netipinterface).
These instructions are prepared for your topology; the remote PC/PS5 path has not
been exercised in this development session.
