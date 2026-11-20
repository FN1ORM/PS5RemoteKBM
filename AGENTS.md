# KeyStream maintenance

- Keep the current Windows install at `dist/PS5RemoteKBM-win-x64`. Stage builds outside that folder and verify them before updating the install.
- Push verified updates to `personal/native-kbm` on the `personal` remote, the public `FN1ORM/PS5RemoteKBM` repository. Preserve shipped versions with `kbm-*` tags so they remain easy to restore.
- Use a GitHub-provided no-reply address for author and committer metadata in public commits. Keep the `origin` remote for fetching upstream chiaki-ng changes.
- Review staged changes and any release packages before publishing. Exclude account data, credentials, PINs, logs, crash dumps, local settings, and personal paths. Keep local builds out of Git and use GitHub Release assets for distributable packages.
