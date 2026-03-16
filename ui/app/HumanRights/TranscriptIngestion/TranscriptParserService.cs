using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;

namespace MmukoOS.RiftBridge.HumanRights.TranscriptIngestion;

public sealed class TranscriptParserService
{
    public IReadOnlyList<TranscriptExtraction> ParseDirectory(string transcriptDirectory)
    {
        if (!Directory.Exists(transcriptDirectory))
            return [];

        return Directory.EnumerateFiles(transcriptDirectory, "*.txt", SearchOption.TopDirectoryOnly)
            .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
            .Select(ParseFile)
            .ToList();
    }

    public TranscriptExtraction ParseFile(string path)
    {
        var lines = File.ReadAllLines(path);
        var institution = "Unknown Institution";
        var timeline = new List<PromiseMandateEntry>();
        var responses = new List<DateTime>();

        var inTimeline = false;
        var inResponses = false;

        foreach (var raw in lines)
        {
            var line = raw.Trim();

            if (line.StartsWith("Institution:", StringComparison.OrdinalIgnoreCase))
            {
                institution = line["Institution:".Length..].Trim();
                continue;
            }

            if (line.StartsWith("Promise Timeline:", StringComparison.OrdinalIgnoreCase))
            {
                inTimeline = true;
                inResponses = false;
                continue;
            }

            if (line.StartsWith("Response Timestamps:", StringComparison.OrdinalIgnoreCase))
            {
                inTimeline = false;
                inResponses = true;
                continue;
            }

            if (!line.StartsWith("- ", StringComparison.Ordinal))
                continue;

            var payload = line[2..].Trim();

            if (inTimeline && TrySplitTimestampAndDescription(payload, out var ts, out var desc))
                timeline.Add(new PromiseMandateEntry(ts, desc));
            else if (inResponses && TryParseTimestamp(payload, out var responseTs))
                responses.Add(responseTs);
        }

        var silence = ComputeSilenceIntervals(timeline.Select(t => t.Timestamp), responses);

        return new TranscriptExtraction(
            SourceFile: Path.GetFileName(path),
            InstitutionName: institution,
            PromiseTimeline: timeline,
            ResponseTimestamps: responses,
            SilenceIntervals: silence);
    }

    private static IReadOnlyList<TimeSpan> ComputeSilenceIntervals(IEnumerable<DateTime> promiseEvents, IEnumerable<DateTime> responses)
    {
        var ordered = promiseEvents.Concat(responses).OrderBy(t => t).ToList();
        var gaps = new List<TimeSpan>();

        for (var i = 1; i < ordered.Count; i++)
        {
            var gap = ordered[i] - ordered[i - 1];
            if (gap > TimeSpan.FromHours(12))
                gaps.Add(gap);
        }

        return gaps;
    }

    private static bool TrySplitTimestampAndDescription(string payload, out DateTime timestamp, out string description)
    {
        timestamp = default;
        description = string.Empty;

        var parts = payload.Split(':', 2);
        if (parts.Length != 2)
            return false;

        if (!TryParseTimestamp(parts[0].Trim(), out timestamp))
            return false;

        description = parts[1].Trim();
        return true;
    }

    private static bool TryParseTimestamp(string value, out DateTime timestamp)
    {
        return DateTime.TryParse(value, CultureInfo.InvariantCulture,
            DateTimeStyles.AdjustToUniversal | DateTimeStyles.AssumeUniversal,
            out timestamp);
    }
}
