# PS5 Remote KBM maintenance

- Keep the current Windows install at `dist/PS5RemoteKBM-win-x64`. Stage builds outside that folder, verify them, then update that same install.
- Preserve shipped versions with `kbm-*` Git tags and assets in the private `FN1ORM/PS5RemoteKBM` GitHub repository. Push the personal branch and release tag after each verified update.
- Keep the upstream chiaki-ng remote for fetching upstream changes. Exclude account data, logs, crash dumps, and local settings from Git and release assets.
