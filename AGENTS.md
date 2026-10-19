# MeleeReforged development rules

- Target KernelLoader Android `arm64-v8a`; keep each weapon profile separate from shared combat and rendering code.
- Resolve game methods from runtime metadata. The supplied dump archives contain identical `dump.cs` files; do not treat them as independent evidence or mix their RVAs with the PE export.
- Use the vendored matching TEFKernel declarations. Older KernelLoader headers omit fields from `patch_method_signature_t`; reject incomplete signatures before takeover.
- Native IL2CPP wrappers must forward the trailing `MethodInfo*`, including nongeneric methods. The supplied generic PatchLib hook backend does not do this reliably; do not reintroduce it here.
- Check required functions against the actual TPF registration table, not just header declarations. This Android `Asset<T>.Value` is a field, not a getter; the asset-generic helper is not exported through this kernel's TPF table.
- Keep `-Bsymbolic`: exported writable API slots must bind locally, not to same-named functions or slots in other plugins.
- Require the kernel_mod_residency_v1 handshake before installing hooks; the matching KernelLoader retains this memdl mapping until process exit. Installing a changed binary requires restarting the game.
- Advance attacks on simulation ticks, including idle/recovery ticks. Drawing must not advance combo, trail age or particles.
- Action, collision and rendering must agree on resource readiness and the same attack trajectory. Keep the original sword texture visible and honor the supplied player draw context.
- Track NPC reset generations; do not identify respawns by object address alone. Keep melee on the original hit pipeline, scaling its arguments only. Sword waves are separate attacks and use verified Player.ApplyDamageToNPC once per NPC generation; never subtract NPC life directly.
- Tests and ARM64 builds do not prove device startup, world entry, visual quality or multiplayer compatibility. First release remains a single-player test build.
