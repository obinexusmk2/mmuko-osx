/*
 * RIFT Stage 555: RIFTBridge C# Wrapper
 * 
 * Provides C# interface for the RIFT pipeline using P/Invoke.
 * Compatible with .NET 6.0+ and Source Generators.
 */

using System;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;

namespace Rift
{
    /// <summary>
    /// RIFT Bridge for C# - Polyglot interface to RIFT pipeline
    /// </summary>
    public class RiftBridge : IDisposable
    {
        private IntPtr _bridge;
        private bool _disposed;

        #region Native API

        private const string DllName = "rift_pipeline";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr riftbridge_create_cs();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int riftbridge_initialize(IntPtr bridge);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int riftbridge_execute_pipeline_cs(IntPtr bridge, string input);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr riftbridge_get_error_cs(IntPtr bridge);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void riftbridge_destroy_cs(IntPtr bridge);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr riftbridge_version_string();

        #endregion

        #region Public API

        /// <summary>
        /// Creates a new RIFT Bridge instance
        /// </summary>
        public RiftBridge()
        {
            _bridge = riftbridge_create_cs();
            if (_bridge == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create RIFT Bridge");
            }
        }

        /// <summary>
        /// Initializes the bridge and NSIGII codec
        /// </summary>
        public void Initialize()
        {
            EnsureNotDisposed();
            int result = riftbridge_initialize(_bridge);
            if (result != 0)
            {
                throw new InvalidOperationException($"Failed to initialize: {ErrorMessage}");
            }
        }

        /// <summary>
        /// Executes the full RIFT pipeline on the input
        /// </summary>
        /// <param name="input">RIFT source code</param>
        public void ExecutePipeline(string input)
        {
            EnsureNotDisposed();
            int result = riftbridge_execute_pipeline_cs(_bridge, input);
            if (result != 0)
            {
                throw new InvalidOperationException($"Pipeline failed: {ErrorMessage}");
            }
        }

        /// <summary>
        /// Gets the last error message
        /// </summary>
        public string ErrorMessage
        {
            get
            {
                EnsureNotDisposed();
                IntPtr ptr = riftbridge_get_error_cs(_bridge);
                return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) : "Unknown error";
            }
        }

        /// <summary>
        /// Gets the RIFT version string
        /// </summary>
        public static string Version
        {
            get
            {
                IntPtr ptr = riftbridge_version_string();
                return ptr != IntPtr.Zero ? Marshal.PtrToStringAnsi(ptr) : "Unknown";
            }
        }

        /// <summary>
        /// Disposes the bridge and releases resources
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_bridge != IntPtr.Zero)
                {
                    riftbridge_destroy_cs(_bridge);
                    _bridge = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~RiftBridge()
        {
            Dispose(false);
        }

        private void EnsureNotDisposed()
        {
            if (_disposed)
            {
                throw new ObjectDisposedException(nameof(RiftBridge));
            }
        }

        #endregion
    }

    /// <summary>
    /// Hex Pattern Matcher using Source Generator compatible attributes
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public class RiftPatternAttribute : Attribute
    {
        public string Pattern { get; }
        public string Polar { get; }

        public RiftPatternAttribute(string pattern, string polar = "CSharp")
        {
            Pattern = pattern;
            Polar = polar;
        }
    }

    /// <summary>
    /// Capture group attribute for pattern matching
    /// </summary>
    [AttributeUsage(AttributeTargets.Property, AllowMultiple = false)]
    public class RiftCaptureAttribute : Attribute
    {
        public int Index { get; }
        public string Name { get; }

        public RiftCaptureAttribute(int index, string name)
        {
            Index = index;
            Name = name;
        }
    }

    /// <summary>
    /// Hex Pattern Matcher for [A-Z][0-9] patterns
    /// </summary>
    [RiftPattern("[A-Z][0-9]$")]
    public partial class HexMatcher
    {
        private static readonly Regex _regex = new Regex(@"([A-Z])([0-9])", RegexOptions.Compiled);

        /// <summary>
        /// Captured alpha component
        /// </summary>
        [RiftCapture(0, "args")]
        public char Alpha { get; private set; }

        /// <summary>
        /// Captured numeric component
        /// </summary>
        [RiftCapture(1, "vargs")]
        public int Numeric { get; private set; }

        /// <summary>
        /// Matches the input against the hex pattern
        /// </summary>
        /// <param name="input">Input string to match</param>
        /// <returns>True if pattern matched</returns>
        public bool Match(string input)
        {
            var match = _regex.Match(input);
            if (match.Success && match.Groups.Count >= 3)
            {
                Alpha = match.Groups[1].Value[0];
                Numeric = int.Parse(match.Groups[2].Value);
                return true;
            }
            return false;
        }

        /// <summary>
        /// Static match method
        /// </summary>
        [GeneratedRegex(@"[A-Z][0-9]$")]
        public static partial Regex GetHexPattern();
    }

    /// <summary>
    /// Token types for RIFT
    /// </summary>
    public enum TokenType
    {
        Unknown = 0,
        Int = 1,
        Role = 2,
        Mask = 3,
        Op = 4,
        QByte = 5,
        QRole = 6,
        QMatrix = 7,
        Identifier = 8,
        Keyword = 9,
        Literal = 10,
        Operator = 11,
        Delimiter = 12,
        Eof = 13
    }

    /// <summary>
    /// Token triplet structure
    /// </summary>
    public struct TokenTriplet
    {
        public TokenType Type { get; set; }
        public long IntValue { get; set; }
        public double FloatValue { get; set; }
        public string StringValue { get; set; }
        public uint Alignment { get; set; }
        public uint Size { get; set; }
        public uint Flags { get; set; }
        public uint ValidationBits { get; set; }

        public bool IsValid => (ValidationBits & 0x03) == 0x03;
        public bool IsLocked => (ValidationBits & 0x04) != 0;
    }
}
