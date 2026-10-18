# MeleeReforged development rules

- Target KernelLoader Android `arm64-v8a`; keep each weapon profile separate from shared combat and rendering code.
- Resolve game methods from runtime metadata. The supplied dump archives contain identical `dump.cs` files; do not treat them as independent evidence or mix their RVAs with the PE export.
- Use the vendored matching TEFKernel declarations. Older KernelLoader headers omit fields from `patch_method_signature_t`; reject incomplete signatures before takeover.
- Native IL2CPP wrappers must forward the trailing `MethodInfo*`, including nongeneric methods. The supplied generic PatchLib hook backend does not do this reliably; do not reintroduce it here.
- Check required functions against the actual TPF registration table, not just header declarations. This Android `Asset<T>.Value` is a field, not a getter; the asset-generic helper is not exported through this kernel's TPF table.
- Keep `-Bsymbolic`: exported writable API slots must bind locally, not to same-named functions or slots in other plugins.
- Keep this module pinned before installing hooks: the current KernelLoader cleanup callback cannot veto unloading. Installing a changed binary requires restarting the game.
- Advance attacks on simulation ticks, including idle/recovery ticks. Drawing must not advance combo, trail age or particles.
- Action, collision and rendering must agree on resource readiness and the same attack trajectory. Keep the original sword texture visible and honor the supplied player draw context.
- Track NPC reset generations; do not identify respawns by object address alone. Preserve vanilla damage, immunity, crit and knockback; do not add a second damage path.
- Tests and ARM64 builds do not prove device startup, world entry, visual quality or multiplayer compatibility. First release remains a single-player test build.
