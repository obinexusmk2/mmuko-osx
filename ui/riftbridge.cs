/*
 * riftbridge.cs  —  MMUKO-OS / RIFTLang Governance Bridge UI
 * Project   : OBINexus / MMUKO-OS / AEGIS
 * Protocol  : NSIGII Human Rights Verification + RIFTLang Governance
 * Version   : 2.0.0
 *
 * Architecture:
 *   .riftrc.0 → .rift.1 → .rift.2 → .rift.3
 *            → .rift.4 → .rift.5 → .rift.6 → .rift.7
 *   Each stage validated by rift-bridge governance relay.
 *   GosiLang .gs[0..7] modules coordinate polyglot runtime.
 *   NSIGII tripartite consent enforced at every stage boundary.
 *
 * Token Triplet Model  (from RIFTLang spec):
 *   token = (token_type, token_value, token_memory)
 *   Memory alignment declared BEFORE type or value.
 *
 * Usage:
 *   dotnet run              — full governance dashboard
 *   dotnet run --mode=boot  — ring boot simulation only
 *   dotnet run --mode=rift  — RIFT stage pipeline only
 *   dotnet run --mode=token — token stream viewer only
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using MmukoOS.RiftBridge.HumanRights;
using MmukoOS.RiftBridge.HumanRights.EvidencePackaging;
using MmukoOS.RiftBridge.HumanRights.GovernanceViewModels;
using MmukoOS.RiftBridge.HumanRights.TranscriptIngestion;

namespace MmukoOS.RiftBridge
{
    // =========================================================================
    // ENUMERATIONS  (NSIGII + RIFT + GosiLang)
    // =========================================================================

    public enum NSIGIIState : byte
    {
        Maybe = 0x00,   // Yellow — pending
        Yes   = 0x55,   // Green  — verified
        No    = 0xAA    // Red    — denied
    }

    public enum NSIGIIColor { Green = 0, Yellow = 1, Red = 2 }
    public enum DriftDirection { Towards = 0, Away = 1, Orthogonal = 2 }

    public enum RingBootPhase
    {
        Sparse = 0, Remember = 1, Active = 2,
        Verify = 3, Complete = 4, Failed = 5
    }

    public enum CompassDirection
    {
        North = 0, NorthEast = 1, East = 2,  SouthEast = 3,
        South = 4, SouthWest = 5, West = 6,  NorthWest = 7,
        Undefined = 8
    }

    /// <summary>RIFT compilation stages .riftrc.0 → .rift.7</summary>
    public enum RiftStage
    {
        Stage0_TokenSanitize  = 0,   // .riftrc.0  — token sanitization
        Stage1_Namespace      = 1,   // .rift.1    — namespace validation
        Stage2_TypeInference  = 2,   // .rift.2    — type + memory alignment
        Stage3_PolicyEnforce  = 3,   // .rift.3    — BitLexPolicy enforcement
        Stage4_ASTConstruct   = 4,   // .rift.4    — AST / DAG construction
        Stage5_Optimization   = 5,   // .rift.5    — entropy balancing / opt
        Stage6_Attestation    = 6,   // .rift.6    — Git-RAF / BLAKE3 attestation
        Stage7_HwDeploy       = 7    // .rift.7    — hardware deployment target
    }

    public enum RiftPolicyResult
    {
        Pass      = 0,   // Validation successful
        Warn      = 1,   // Log and continue
        Fail      = 2,   // Halt compilation
        Escalate  = 3    // Manual governance review
    }

    public enum RiftGovernanceEvent
    {
        StageEntry,
        StageExit,
        PolicyViolation,
        AttestationFail,
        GovernanceEscalation,
        EmergencyHalt,
        AntiGhostingAlert,
        ConsentLapse
    }

    /// <summary>
    /// Token execution mode: classical (deterministic) or quantum (superposed)
    /// </summary>
    public enum LexerMode { Classic = 0, Quantum = 1, Hybrid = 2 }

    /// <summary>GosiLang .gs[n] module types</summary>
    public enum GsiModuleType
    {
        CoreRuntime      = 0,   // .gs[0]
        LanguageBridge   = 1,   // .gs[1]
        CrossPlatform    = 2,   // .gs[2]
        SecurityBoundary = 3,   // .gs[3]
        Distributed      = 4,   // .gs[4]
        HwAbstraction    = 5,   // .gs[5]
        CryptoPrimitive  = 6,   // .gs[6]
        GovernancePolicy = 7    // .gs[7]
    }

    // =========================================================================
    // NSIGII CONSENT STATE  (tripartite: U + V + W)
    // =========================================================================

    public class ConsentState
    {
        public NSIGIIState UserConsent        { get; set; } = NSIGIIState.Maybe;
        public NSIGIIState InstitutionConsent { get; set; } = NSIGIIState.Maybe;
        public NSIGIIState ArbiterConsent     { get; set; } = NSIGIIState.Maybe;

        public NSIGIIColor   Color       { get; private set; } = NSIGIIColor.Yellow;
        public DriftDirection Drift      { get; private set; } = DriftDirection.Orthogonal;
        public double         Discriminant{ get; private set; } = 0.0;
        public bool           LivingLoop { get; private set; } = false;

        /// <summary>
        /// Drift discriminant b²-4ac:
        ///   > 0  → GREEN  → two solutions → healthy
        ///   = 0  → YELLOW → one solution  → equilibrium
        ///   &lt; 0  → RED    → no solution   → failing
        /// </summary>
        public NSIGIIColor Evaluate()
        {
            double a = Coeff(UserConsent);
            double b = Coeff(InstitutionConsent) * 2.0;
            double c = ArbiterConsent == NSIGIIState.Yes   ? 0.5  :
                       ArbiterConsent == NSIGIIState.Maybe ? 1.0  : 2.0;

            Discriminant = b * b - 4.0 * a * c;

            if (Discriminant > 1e-10)
                (Color, Drift, LivingLoop) = (NSIGIIColor.Green,  DriftDirection.Towards,    true);
            else if (Math.Abs(Discriminant) <= 1e-10)
                (Color, Drift, LivingLoop) = (NSIGIIColor.Yellow, DriftDirection.Orthogonal, true);
            else
                (Color, Drift, LivingLoop) = (NSIGIIColor.Red,    DriftDirection.Away,       false);

            return Color;
        }

        private static double Coeff(NSIGIIState s) => s switch
        {
            NSIGIIState.Yes   => 1.0,
            NSIGIIState.Maybe => 0.5,
            _                 => 0.0
        };

        public override string ToString() =>
            $"U={UserConsent} V={InstitutionConsent} W={ArbiterConsent}" +
            $" => {Color} (disc={Discriminant:+0.0000;-0.0000;0.0000}" +
            $" drift={Drift} loop={LivingLoop})";
    }

    // =========================================================================
    // RIFT TOKEN TRIPLET  (type, value, memory)
    // =========================================================================

    /// <summary>
    /// RIFTLang token: token = (token_type, token_value, token_memory)
    /// Memory alignment declared before type or value assignment.
    /// </summary>
    public class RiftToken
    {
        public string     TokenType   { get; set; } = "UNKNOWN";
        public string     TokenValue  { get; set; } = "";
        public int        MemoryBytes { get; set; } = 4096;
        public LexerMode  Mode        { get; set; } = LexerMode.Classic;
        public bool       Superposed  { get; set; } = false;
        public bool       Entangled   { get; set; } = false;
        public double     Entropy     { get; set; } = 0.0;

        // Classical binding: :=   Quantum binding: =:
        public bool IsDeferred => Mode == LexerMode.Quantum && Superposed;

        public string BindingSyntax => IsDeferred ? "=:" : ":=";

        public override string ToString() =>
            $"[{Mode,-8}] {TokenType,-12} {BindingSyntax} \"{TokenValue}\"" +
            $"  mem={MemoryBytes}b" +
            (Superposed ? " SUPER" : "") +
            (Entangled  ? " ENT"   : "") +
            (Entropy > 0 ? $" S={Entropy:F3}" : "");
    }

    // =========================================================================
    // RIFT STAGE CONTEXT
    // =========================================================================

    public class RiftStageContext
    {
        public RiftStage       Stage          { get; set; }
        public RiftPolicyResult PolicyResult  { get; set; } = RiftPolicyResult.Pass;
        public NSIGIIColor     ConsentColor   { get; set; } = NSIGIIColor.Yellow;
        public bool            Attested       { get; set; } = false;
        public bool            AntiGhosted    { get; set; } = true;
        public double          DurationMs     { get; set; } = 0;
        public string          GovernanceHash { get; set; } = "0x" + new string('0', 8);
        public List<string>    Events         { get; }      = new();

        public string StatusGlyph => PolicyResult switch
        {
            RiftPolicyResult.Pass     => "OK",
            RiftPolicyResult.Warn     => "!!",
            RiftPolicyResult.Fail     => "XX",
            RiftPolicyResult.Escalate => "??",
            _                         => "--"
        };

        public string StageName => Stage switch
        {
            RiftStage.Stage0_TokenSanitize => ".riftrc.0  Token Sanitize",
            RiftStage.Stage1_Namespace     => ".rift.1    Namespace",
            RiftStage.Stage2_TypeInference => ".rift.2    Type/Memory",
            RiftStage.Stage3_PolicyEnforce => ".rift.3    BitLexPolicy",
            RiftStage.Stage4_ASTConstruct  => ".rift.4    AST/DAG",
            RiftStage.Stage5_Optimization  => ".rift.5    Entropy Opt",
            RiftStage.Stage6_Attestation   => ".rift.6    Git-RAF",
            RiftStage.Stage7_HwDeploy      => ".rift.7    HW Deploy",
            _                              => "UNKNOWN"
        };
    }

    // =========================================================================
    // GOSILANG MODULE  (.gs[n])
    // =========================================================================

    public class GsiModule
    {
        public GsiModuleType Type      { get; set; }
        public string        Namespace { get; set; } = "";
        public string        Target    { get; set; } = "C";
        public bool          Protected { get; set; } = false;
        public bool          Verified  { get; set; } = false;
        public string        Hash      { get; set; } = "0x" + new string('0', 8);

        public string Label => $".gs[{(int)Type}] {Type,-18} {Namespace}";
    }

    // =========================================================================
    // ANTI-GHOSTING HEARTBEAT
    // =========================================================================

    public class AntiGhostingContext
    {
        public DateTime  StageEntryTime    { get; set; } = DateTime.UtcNow;
        public int       MaxDurationSecs   { get; set; } = 300;
        public int       HeartbeatInterval { get; set; } = 5;
        public int       MissedHeartbeats  { get; set; } = 0;
        public bool      IsGhosted         => MissedHeartbeats > 3;

        public string StatusLine =>
            IsGhosted
                ? $"[GHOST] Stage unresponsive! Missed={MissedHeartbeats}"
                : $"[LIVE]  Heartbeat OK  Missed={MissedHeartbeats}/3";
    }

    // =========================================================================
    // RING BOOT QUBIT
    // =========================================================================

    public class QubitRing
    {
        private static readonly string[] DirNames =
            { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
        private static readonly double[] SpinVals =
            { 0.7854, 1.0472, 1.5708, 3.1416, 6.2832, 1.5708, 1.0472, 0.7854 };

        public byte RawValue  { get; private set; }
        public int  BaseIndex { get; private set; }
        public bool[] Active  { get; } = new bool[8];
        public bool[] Super   { get; } = new bool[8];

        public void Init(byte raw)
        {
            RawValue  = raw;
            BaseIndex = (raw % 12) + 1;
            for (int i = 0; i < 8; i++)
            {
                Active[i] = ((raw >> i) & 1) == 1;
                Super[i]  = i is < 3 or > 4;
            }
        }

        public bool VerifyRotation()
        {
            byte r = RawValue;
            for (int i = 0; i < 8; i++) r = (byte)((r >> 1) | (r << 7));
            return r == RawValue;
        }

        public void Print()
        {
            for (int i = 0; i < 8; i++)
                Console.WriteLine(
                    $"  q[{i}] dir={DirNames[i],-3} " +
                    $"val={(Active[i] ? 1 : 0)} " +
                    $"spin={SpinVals[i]:F4}" +
                    (Super[i] ? " SUPER" : ""));
        }
    }

    // =========================================================================
    // RIFT GOVERNANCE DASHBOARD
    // =========================================================================

    public class RiftGovernanceDashboard
    {
        private readonly List<RiftStageContext> _stages   = new();
        private readonly List<GsiModule>        _modules  = new();
        private readonly List<RiftToken>        _tokens   = new();
        private readonly List<string>           _eventLog = new();
        private readonly ConsentState           _consent  = new();
        private readonly AntiGhostingContext    _ghost    = new();
        private          LexerMode              _mode     = LexerMode.Classic;
        private readonly TranscriptParserService _transcriptParser = new();
        private readonly HumanRightsGovernanceMapper _humanRightsMapper = new();
        private readonly EvidencePackageBuilder _evidencePackageBuilder = new();
        private readonly HumanRightsFirmwareUI _humanRightsPanel = new ConsoleHumanRightsPanel();

        // ---- Terminal helpers ----
        private static void Colored(string text, ConsoleColor fg,
                                     bool newline = true)
        {
            Console.ForegroundColor = fg;
            if (newline) Console.WriteLine(text);
            else         Console.Write(text);
            Console.ResetColor();
        }

        private static void Box(string text, ConsoleColor color)
        {
            int w = Math.Max(text.Length + 4, 52);
            string bar = new string('=', w);
            Colored(bar, color);
            Colored($"  {text}", color);
            Colored(bar, color);
        }

        private void Log(RiftGovernanceEvent ev, string msg)
        {
            string ts  = DateTime.UtcNow.ToString("HH:mm:ss.fff");
            string entry = $"[{ts}] {ev,-26} {msg}";
            _eventLog.Add(entry);

            ConsoleColor c = ev switch
            {
                RiftGovernanceEvent.PolicyViolation    => ConsoleColor.Yellow,
                RiftGovernanceEvent.AttestationFail    => ConsoleColor.Red,
                RiftGovernanceEvent.EmergencyHalt      => ConsoleColor.Red,
                RiftGovernanceEvent.AntiGhostingAlert  => ConsoleColor.Magenta,
                RiftGovernanceEvent.ConsentLapse       => ConsoleColor.Yellow,
                _ => ConsoleColor.DarkGray
            };
            Colored($"  EVT {entry}", c);
        }

        // ---- RIFT Stage pipeline ----
        private RiftStageContext SimulateStage(RiftStage s,
                                                NSIGIIState u,
                                                NSIGIIState v,
                                                NSIGIIState w)
        {
            var ctx = new RiftStageContext { Stage = s };

            Log(RiftGovernanceEvent.StageEntry, ctx.StageName);

            // Consent check
            _consent.UserConsent = u;
            _consent.InstitutionConsent = v;
            _consent.ArbiterConsent = w;
            var color = _consent.Evaluate();
            ctx.ConsentColor = color;

            // Policy result from consent
            ctx.PolicyResult = color switch
            {
                NSIGIIColor.Green  => RiftPolicyResult.Pass,
                NSIGIIColor.Yellow => RiftPolicyResult.Warn,
                NSIGIIColor.Red    => RiftPolicyResult.Fail,
                _                  => RiftPolicyResult.Escalate
            };

            // Anti-ghosting check
            ctx.AntiGhosted = !_ghost.IsGhosted;
            if (_ghost.IsGhosted)
                Log(RiftGovernanceEvent.AntiGhostingAlert,
                    $"Stage {(int)s} unresponsive");

            // Hash simulation
            ctx.GovernanceHash = $"0x{(s.GetHashCode() * 0xDEADBEEF) & 0xFFFFFFFF:X8}";
            ctx.Attested = ctx.PolicyResult == RiftPolicyResult.Pass;
            ctx.DurationMs = 12.5 * ((int)s + 1);

            ctx.Events.Add($"Consent: {_consent}");
            ctx.Events.Add($"Policy:  {ctx.PolicyResult}  Hash:{ctx.GovernanceHash}");

            if (ctx.PolicyResult == RiftPolicyResult.Fail)
                Log(RiftGovernanceEvent.PolicyViolation,
                    $"Stage {(int)s} FAIL — consent RED");
            else if (ctx.PolicyResult == RiftPolicyResult.Warn)
                Log(RiftGovernanceEvent.StageExit,
                    $"Stage {(int)s} WARN — continuing with caution");
            else
                Log(RiftGovernanceEvent.StageExit,
                    $"Stage {(int)s} PASS — attested");

            return ctx;
        }

        // ---- Token stream ----
        private void BuildTokenStream()
        {
            // Sample token stream showing classical and quantum tokens
            var stream = new List<(string type, string val, int mem,
                                    LexerMode mode, bool sup, bool ent, double ent_v)>
            {
                ("INT",       "42",                4096, LexerMode.Classic, false, false, 0.0),
                ("IDENTIFIER","ring_boot_init",    4096, LexerMode.Classic, false, false, 0.0),
                ("KEYWORD",   "!govern",           4096, LexerMode.Classic, false, false, 0.0),
                ("QBYTE",     "superpose(1,2,3)",  8,    LexerMode.Quantum, true,  true,  0.312),
                ("QROLE",     "entangle(u,v)",     8,    LexerMode.Quantum, true,  true,  0.198),
                ("OP",        ":=",                4096, LexerMode.Classic, false, false, 0.0),
                ("OP",        "=:",                8,    LexerMode.Quantum, true,  false, 0.0),
                ("KEYWORD",   "collapse()",        8,    LexerMode.Quantum, false, false, 0.0),
                ("ROLE",      "nsigii_probe",      4096, LexerMode.Classic, false, false, 0.0),
                ("QMATRIX",   "phase_lock[8]",     8,    LexerMode.Quantum, true,  true,  0.451),
            };

            _tokens.Clear();
            foreach (var t in stream)
                _tokens.Add(new RiftToken
                {
                    TokenType   = t.type,
                    TokenValue  = t.val,
                    MemoryBytes = t.mem,
                    Mode        = t.mode,
                    Superposed  = t.sup,
                    Entangled   = t.ent,
                    Entropy     = t.ent_v
                });
        }

        // ---- GosiLang modules ----
        private void BuildGsiModules()
        {
            var defs = new[]
            {
                (GsiModuleType.CoreRuntime,      "obn.runtime.core",      "C,Rust"),
                (GsiModuleType.LanguageBridge,   "obn.bridge.csharp",     "C#"),
                (GsiModuleType.CrossPlatform,    "obn.build.poly",        "C,Go"),
                (GsiModuleType.SecurityBoundary, "obn.security.nsigii",   "C"),
                (GsiModuleType.Distributed,      "obn.dist.gossip",       "Go"),
                (GsiModuleType.HwAbstraction,    "obn.hw.mmuko",          "C,ASM"),
                (GsiModuleType.CryptoPrimitive,  "obn.crypto.blake3",     "C"),
                (GsiModuleType.GovernancePolicy, "obn.policy.rift7",      "RIFT"),
            };

            _modules.Clear();
            foreach (var (t, ns, target) in defs)
                _modules.Add(new GsiModule
                {
                    Type      = t,
                    Namespace = ns,
                    Target    = target,
                    Protected = (int)t >= 3,
                    Verified  = (int)t <= 5,
                    Hash      = $"0x{(ns.GetHashCode() & 0xFFFF_FFFF):X8}"
                });
        }

        // ---- Rendering sections ----

        private void PrintHeader()
        {
            Console.Clear();
            Box("MMUKO-OS / RIFTLang Governance Bridge  v2.0.0", ConsoleColor.Cyan);
            Console.WriteLine($"  OBINexus AEGIS  |  Mode: {_mode,-8}" +
                              $"  |  UTC: {DateTime.UtcNow:yyyy-MM-dd HH:mm:ss}");
            Console.WriteLine($"  NSIGII: {_consent}");
            Console.WriteLine();
        }

        private void PrintStages()
        {
            Box("RIFT Compilation Pipeline  (.riftrc.0 -> .rift.7)", ConsoleColor.Blue);
            Console.WriteLine(
                $"  {"Stage",-26} {"Policy",-9} {"Consent",-7}" +
                $" {"Attest",-7} {"ms",6}  Hash");
            Console.WriteLine("  " + new string('-', 70));
            foreach (var s in _stages)
            {
                var fg = s.PolicyResult switch
                {
                    RiftPolicyResult.Pass     => ConsoleColor.Green,
                    RiftPolicyResult.Warn     => ConsoleColor.Yellow,
                    RiftPolicyResult.Fail     => ConsoleColor.Red,
                    RiftPolicyResult.Escalate => ConsoleColor.Magenta,
                    _                         => ConsoleColor.Gray
                };
                Colored(
                    $"  [{s.StatusGlyph}] {s.StageName,-26} " +
                    $"{s.ConsentColor,-7}  " +
                    $"{(s.Attested ? "ATTEST" : "PEND  ")}" +
                    $"  {s.DurationMs,5:F1}  {s.GovernanceHash}",
                    fg);
            }
            Console.WriteLine();
        }

        private void PrintTokenStream()
        {
            Box($"Token Stream  (mode={_mode})", ConsoleColor.DarkCyan);
            Console.WriteLine(
                $"  {"Mode",-9} {"Type",-12} {"Bind",-4} " +
                $"{"Value",-22} {"Mem",6}b  Flags");
            Console.WriteLine("  " + new string('-', 70));
            foreach (var t in _tokens)
            {
                var fg = t.Mode == LexerMode.Quantum
                    ? ConsoleColor.Magenta : ConsoleColor.Cyan;
                Colored($"  {t}", fg);
            }
            Console.WriteLine();
        }

        private void PrintGsiModules()
        {
            Box("GosiLang Module Registry  (.gs[0..7])", ConsoleColor.DarkYellow);
            Console.WriteLine(
                $"  {"Module",-38} {"Target",-10} " +
                $"{"IP-Prot",-8} {"Verif",-6}  Hash");
            Console.WriteLine("  " + new string('-', 70));
            foreach (var m in _modules)
            {
                var fg = m.Verified  ? ConsoleColor.Green :
                         m.Protected ? ConsoleColor.Yellow : ConsoleColor.DarkGray;
                Colored(
                    $"  {m.Label,-38} {m.Target,-10} " +
                    $"{(m.Protected ? "ChaCha20" : "NONE    "),-8} " +
                    $"{(m.Verified  ? "OK   " : "PEND ")}" +
                    $"  {m.Hash}",
                    fg);
            }
            Console.WriteLine();
        }

        private void PrintRingBoot(QubitRing[] mem)
        {
            Box("Ring Boot  (8-qubit compass, SPARSE->COMPLETE)", ConsoleColor.Green);
            Console.WriteLine($"  Vacuum: G=9.8000 | Lepton=0.9800 | Muon=0.0980");
            Console.WriteLine(
                $"  Frame: SouthWest | NSIGII: {_consent.Color}" +
                $" (disc={_consent.Discriminant:F4})");
            Console.WriteLine($"\n  Sample Byte[0] (raw=0x{mem[0].RawValue:X2}" +
                              $"  base={mem[0].BaseIndex}):");
            mem[0].Print();
            Console.WriteLine();
        }

        private void PrintDiamondTraversal()
        {
            Console.WriteLine("  Diamond traversal (nonlinear base resolution):");
            int[]    bases  = { 12,        6,            8,         4,
                                 10,        2,            1 };
            string[] prim   = { "SOUTH",   "SOUTHWEST",  "EAST",   "WEST",
                                  "SOUTHEAST","NORTHEAST","NORTH"  };
            string[] sec    = { "NORTH",   "EAST",       "WEST",   "EAST",
                                  "NORTH",   "WEST",      "SOUTH"  };
            for (int i = 0; i < bases.Length; i++)
                Console.WriteLine(
                    $"    Base {bases[i],2}  ->  {prim[i],-12} / {sec[i]}");
            Console.WriteLine();
        }

        private void PrintEventLog()
        {
            Box("Governance Event Log", ConsoleColor.DarkGray);
            var recent = _eventLog.TakeLast(12).ToList();
            foreach (var e in recent)
            {
                var fg = e.Contains("FAIL")       ? ConsoleColor.Red    :
                         e.Contains("WARN")       ? ConsoleColor.Yellow :
                         e.Contains("GHOST")      ? ConsoleColor.Magenta :
                         e.Contains("ESCALATE")   ? ConsoleColor.Magenta :
                                                    ConsoleColor.DarkGray;
                Colored($"  {e}", fg);
            }
            Console.WriteLine();
        }

        private void PrintAntiGhosting()
        {
            var fg = _ghost.IsGhosted ? ConsoleColor.Red : ConsoleColor.Green;
            Colored($"  Anti-Ghosting: {_ghost.StatusLine}", fg);
        }

        private void PrintHumanRightsPanel(HumanRightsStatusViewModel status,
                                           EvidencePackagePreview evidence)
        {
            Box("Human Rights Transcript Ingestion Panel", ConsoleColor.Magenta);
            _humanRightsPanel.RenderDiscriminantTimeline(status);
            _humanRightsPanel.RenderBreachThresholdIndicator(status);
            _humanRightsPanel.RenderEscalationReadiness(status);
            _humanRightsPanel.RenderEvidencePackagePreview(evidence);
            Console.WriteLine();
        }

        private static string ResolveTranscriptDirectory()
        {
            var cwdPath = Path.Combine(Directory.GetCurrentDirectory(), "transcript");
            if (Directory.Exists(cwdPath))
                return cwdPath;

            var repoRelative = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../transcript"));
            return Directory.Exists(repoRelative) ? repoRelative : cwdPath;
        }

        // ---- Entry: full pipeline ----
        public void Run(string mode)
        {
            var transcriptDirectory = ResolveTranscriptDirectory();
            var transcriptEvents = _transcriptParser.ParseDirectory(transcriptDirectory);
            var humanRightsStatus = _humanRightsMapper.MapToStatus(transcriptEvents, _consent);
            _ghost.MissedHeartbeats = humanRightsStatus.MissedHeartbeatsFromSilence;
            var evidencePreview = _evidencePackageBuilder.BuildPreview(transcriptEvents, humanRightsStatus);

            BuildTokenStream();
            BuildGsiModules();

            // Ring boot memory
            var mem = new QubitRing[16];
            for (int i = 0; i < 16; i++)
            {
                mem[i] = new QubitRing();
                mem[i].Init((byte)(i * 17 + 42));
            }

            // Initial consent (SPARSE: user present, institution pending, arbiter pending)
            _consent.UserConsent = NSIGIIState.Yes;
            _consent.InstitutionConsent = NSIGIIState.Maybe;
            _consent.ArbiterConsent = NSIGIIState.Maybe;
            _consent.Evaluate();

            // RIFT stage pipeline with escalating consent
            _stages.Add(SimulateStage(RiftStage.Stage0_TokenSanitize,
                NSIGIIState.Yes, NSIGIIState.Maybe, NSIGIIState.Maybe));
            _stages.Add(SimulateStage(RiftStage.Stage1_Namespace,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Maybe));
            _stages.Add(SimulateStage(RiftStage.Stage2_TypeInference,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Maybe));
            _stages.Add(SimulateStage(RiftStage.Stage3_PolicyEnforce,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Yes));
            _stages.Add(SimulateStage(RiftStage.Stage4_ASTConstruct,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Yes));
            _stages.Add(SimulateStage(RiftStage.Stage5_Optimization,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Yes));
            _stages.Add(SimulateStage(RiftStage.Stage6_Attestation,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Yes));
            _stages.Add(SimulateStage(RiftStage.Stage7_HwDeploy,
                NSIGIIState.Yes, NSIGIIState.Yes,   NSIGIIState.Yes));

            // Determine active lexer mode (quantum starts at stage 3+)
            int passedStages = _stages.Count(s =>
                s.PolicyResult is RiftPolicyResult.Pass or RiftPolicyResult.Warn);
            _mode = passedStages >= 4 ? LexerMode.Quantum :
                    passedStages >= 2 ? LexerMode.Hybrid  : LexerMode.Classic;

            // Render dashboard
            PrintHeader();

            if (mode is not "rift")
            {
                // Ring boot section
                Box("Ring Boot Sequence  (MMUKO-OS v2)", ConsoleColor.Cyan);
                Console.WriteLine();
                PrintRingBoot(mem);
                PrintDiamondTraversal();
            }

            if (mode is not "boot")
            {
                PrintStages();

                if (mode is not "rift")
                    PrintTokenStream();

                PrintGsiModules();
            }

            if (mode is not "boot" and not "rift")
                PrintEventLog();

            PrintHumanRightsPanel(humanRightsStatus, evidencePreview);

            PrintAntiGhosting();

            // Summary
            int pass = _stages.Count(s => s.PolicyResult == RiftPolicyResult.Pass);
            int warn = _stages.Count(s => s.PolicyResult == RiftPolicyResult.Warn);
            int fail = _stages.Count(s => s.PolicyResult == RiftPolicyResult.Fail);
            NSIGIIColor final = fail > 0 ? NSIGIIColor.Red :
                                warn > 0 ? NSIGIIColor.Yellow : NSIGIIColor.Green;

            Console.WriteLine();
            Box($"RIFT Pipeline: {pass} PASS  {warn} WARN  {fail} FAIL" +
                $"  =>  {final}", ConsoleColor.Cyan);
            Console.WriteLine(
                $"  Toolchain: riftlang.exe -> .so.a -> rift.exe -> gosilang");
            Console.WriteLine(
                $"  Build:     nlink -> polybuild -> mmuko-os");
            Console.WriteLine();
        }
    }

    // =========================================================================
    // NATIVE BRIDGE  (P/Invoke when libriftbridge present)
    // =========================================================================

    public sealed class NativeBridge : IDisposable
    {
        private const string Lib = "libriftbridge";
        private static readonly bool _available;

        static NativeBridge()
        {
            try { _available = NativeLibrary.TryLoad(Lib, out _); }
            catch { _available = false; }
        }

        public static bool IsAvailable => _available;

        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr riftbridge_app_create();
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern void riftbridge_app_destroy(IntPtr h);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern int riftbridge_app_init(IntPtr h);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern int riftbridge_app_frame(IntPtr h);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern void riftbridge_app_set_consent(
            IntPtr h, byte u, byte v, byte w);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern void riftbridge_app_set_phase(
            IntPtr h, int phase, int pass);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)]
        private static extern void riftbridge_app_shutdown(IntPtr h);

        private IntPtr _h;
        private bool   _disposed;

        public void Dispose()
        {
            if (!_disposed && _h != IntPtr.Zero)
            {
                riftbridge_app_destroy(_h);
                _h = IntPtr.Zero;
            }
            _disposed = true;
            GC.SuppressFinalize(this);
        }

        ~NativeBridge() => Dispose();
    }

    // =========================================================================
    // ENTRY POINT
    // =========================================================================

    public static class Program
    {
        public static void Main(string[] args)
        {
            // Parse --mode flag
            string mode = "full";
            foreach (var a in args)
                if (a.StartsWith("--mode="))
                    mode = a[7..].ToLower();

            Console.WriteLine("MMUKO-OS  Rift Bridge UI  v2.0.0");
            Console.WriteLine("OBINexus AEGIS  --  NSIGII Human Rights Protocol");
            Console.WriteLine("\"Don't just boot systems. Boot truthful ones.\"\n");

            if (NativeBridge.IsAvailable)
                Console.WriteLine("[NATIVE] libriftbridge detected — P/Invoke active");
            else
                Console.WriteLine("[MANAGED] Running pure managed simulation");

            Console.WriteLine();

            var dashboard = new RiftGovernanceDashboard();
            dashboard.Run(mode);
        }
    }
}
