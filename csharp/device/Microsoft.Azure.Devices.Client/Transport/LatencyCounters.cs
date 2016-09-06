namespace Microsoft.Azure.Devices.Client.Transport
{
    using System;
    using System.Diagnostics;
    using System.Threading.Tasks;

    public class LatencyCounters
    {
        static int LatencyHighBound = 60000;
        public static int[] SendLatencyTotalInMsCounters = new int[LatencyHighBound + 1];
        public static int[] SendLatencySuccessInMsCounters = new int[LatencyHighBound + 1];
        public static int[] OpenLatencyInMsCounters = new int[LatencyHighBound + 1];

        public static async Task RunAndMeasureLatencyAsync(Stopwatch stopwatch, Task asyncOperation, int[] latencyCounters, bool successOnly = false)
        {
            stopwatch.Reset();
            stopwatch.Start();

            try
            {
                await asyncOperation;
                if (successOnly)
                {
                    UpdateCounters(stopwatch, latencyCounters);
                }
            }
            catch (Exception ex)
            {
                Trace.TraceError(ex.ToString());
            }
            finally
            {
                if (!successOnly)
                {
                    UpdateCounters(stopwatch, latencyCounters);
                }
            }
        }

        static void UpdateCounters(Stopwatch stopwatch, int[] latencyCounters)
        {
            stopwatch.Stop();
            int elapsedInMs = (int)stopwatch.ElapsedMilliseconds;
            if (elapsedInMs <= LatencyHighBound)
            {
                latencyCounters[elapsedInMs]++;
            }
            else
            {
                latencyCounters[LatencyHighBound]++;
            }
        }
    }
}