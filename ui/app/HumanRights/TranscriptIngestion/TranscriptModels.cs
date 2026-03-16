using System;
using System.Collections.Generic;

namespace MmukoOS.RiftBridge.HumanRights.TranscriptIngestion;

public record PromiseMandateEntry(DateTime Timestamp, string Description);

public record TranscriptExtraction(
    string SourceFile,
    string InstitutionName,
    IReadOnlyList<PromiseMandateEntry> PromiseTimeline,
    IReadOnlyList<DateTime> ResponseTimestamps,
    IReadOnlyList<TimeSpan> SilenceIntervals);
