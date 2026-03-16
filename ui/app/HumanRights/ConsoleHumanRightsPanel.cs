using System;
using MmukoOS.RiftBridge.HumanRights.EvidencePackaging;
using MmukoOS.RiftBridge.HumanRights.GovernanceViewModels;

namespace MmukoOS.RiftBridge.HumanRights;

public sealed class ConsoleHumanRightsPanel : HumanRightsFirmwareUI
{
    public void RenderDiscriminantTimeline(HumanRightsStatusViewModel status)
    {
        Console.WriteLine("  Discriminant Timeline:");
        foreach (var point in status.Timeline)
            Console.WriteLine($"    {point.Timestamp:yyyy-MM-dd HH:mm} | {point.Institution,-28} | Δ={point.Discriminant:+0.000;-0.000;0.000}");
    }

    public void RenderBreachThresholdIndicator(HumanRightsStatusViewModel status)
    {
        var icon = status.BreachThresholdCrossed ? "[BREACH]" : "[OK]";
        Console.WriteLine($"  Breach Threshold: {icon} cutoff={status.BreachThreshold:+0.000;-0.000;0.000}");
    }

    public void RenderEscalationReadiness(HumanRightsStatusViewModel status)
    {
        var readiness = status.EscalationReady ? "READY" : "MONITORING";
        Console.WriteLine($"  Escalation Readiness: {readiness} (silence-heartbeats={status.MissedHeartbeatsFromSilence})");
    }

    public void RenderEvidencePackagePreview(EvidencePackagePreview preview)
    {
        Console.WriteLine("  Evidence Package Preview:");
        Console.WriteLine($"    Files: {string.Join(", ", preview.IncludedFiles)}");
        foreach (var finding in preview.InstitutionalFindings)
            Console.WriteLine($"    - {finding}");
        Console.WriteLine($"    Timeline points: {preview.TimelinePoints}");
    }
}
