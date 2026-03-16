using System.Collections.Generic;
using System.Linq;
using MmukoOS.RiftBridge.HumanRights.GovernanceViewModels;
using MmukoOS.RiftBridge.HumanRights.TranscriptIngestion;

namespace MmukoOS.RiftBridge.HumanRights.EvidencePackaging;

public record EvidencePackagePreview(
    IReadOnlyList<string> IncludedFiles,
    IReadOnlyList<string> InstitutionalFindings,
    int TimelinePoints);

public sealed class EvidencePackageBuilder
{
    public EvidencePackagePreview BuildPreview(
        IReadOnlyList<TranscriptExtraction> transcripts,
        HumanRightsStatusViewModel status)
    {
        var findings = transcripts
            .Select(t => $"{t.InstitutionName}: promises={t.PromiseTimeline.Count}, responses={t.ResponseTimestamps.Count}, silences={t.SilenceIntervals.Count}")
            .ToList();

        return new EvidencePackagePreview(
            IncludedFiles: transcripts.Select(t => t.SourceFile).ToList(),
            InstitutionalFindings: findings,
            TimelinePoints: status.Timeline.Count);
    }
}
