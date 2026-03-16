using System;
using System.Collections.Generic;
using System.Linq;
using MmukoOS.RiftBridge.HumanRights.TranscriptIngestion;

namespace MmukoOS.RiftBridge.HumanRights.GovernanceViewModels;

public record GovernanceDiscriminantPoint(string Institution, DateTime Timestamp, double Discriminant);

public record HumanRightsStatusViewModel(
    IReadOnlyList<GovernanceDiscriminantPoint> Timeline,
    double BreachThreshold,
    bool BreachThresholdCrossed,
    bool EscalationReady,
    int MissedHeartbeatsFromSilence,
    int EvidenceDocumentCount);

public sealed class HumanRightsGovernanceMapper
{
    public HumanRightsStatusViewModel MapToStatus(
        IReadOnlyList<TranscriptExtraction> transcripts,
        ConsentState consentState)
    {
        var points = new List<GovernanceDiscriminantPoint>();

        foreach (var transcript in transcripts)
        {
            foreach (var ev in transcript.PromiseTimeline)
            {
                var disc = EstimateDiscriminant(transcript, ev.Timestamp);
                points.Add(new GovernanceDiscriminantPoint(transcript.InstitutionName, ev.Timestamp, disc));
            }
        }

        points = points.OrderBy(p => p.Timestamp).ToList();

        var worstDiscriminant = points.Count == 0 ? consentState.Discriminant : points.Min(p => p.Discriminant);
        var breachThreshold = -0.25;
        var crossed = worstDiscriminant <= breachThreshold;

        ApplyConsentProjection(consentState, worstDiscriminant);

        var silenceBursts = transcripts.SelectMany(t => t.SilenceIntervals).ToList();
        var missedHeartbeats = silenceBursts.Count(i => i >= TimeSpan.FromDays(2));
        var escalationReady = crossed || missedHeartbeats > 0;

        return new HumanRightsStatusViewModel(
            Timeline: points,
            BreachThreshold: breachThreshold,
            BreachThresholdCrossed: crossed,
            EscalationReady: escalationReady,
            MissedHeartbeatsFromSilence: missedHeartbeats,
            EvidenceDocumentCount: transcripts.Count);
    }

    private static double EstimateDiscriminant(TranscriptExtraction transcript, DateTime moment)
    {
        var lateResponses = transcript.ResponseTimestamps.Count(r => r > moment.AddDays(3));
        var silencePenalty = transcript.SilenceIntervals.Sum(s => s.TotalDays) * 0.08;
        return 0.75 - (lateResponses * 0.35) - silencePenalty;
    }

    private static void ApplyConsentProjection(ConsentState consent, double worstDiscriminant)
    {
        if (worstDiscriminant < 0)
        {
            consent.UserConsent = NSIGIIState.Yes;
            consent.InstitutionConsent = NSIGIIState.No;
            consent.ArbiterConsent = NSIGIIState.Maybe;
        }
        else
        {
            consent.UserConsent = NSIGIIState.Yes;
            consent.InstitutionConsent = NSIGIIState.Yes;
            consent.ArbiterConsent = NSIGIIState.Maybe;
        }

        consent.Evaluate();
    }
}
