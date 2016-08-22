namespace Microsoft.Azure.Devices.Client.Transport
{
    using System.Diagnostics;
    using System.Threading.Tasks;

    public class LatencyCounters
    {
        static int LatencyHighBound = 60000;
        public static int[] SendLatencyInMsCounters = new int[LatencyHighBound + 1];
        public static int[] OpenLatencyInMsCounters = new int[LatencyHighBound + 1];

        public static async Task RunAndMeasureLatencyAsync(Stopwatch stopwatch, Task asyncOperation, int[] latencyCounters)
        {
            stopwatch.Reset();
            stopwatch.Start();

            await asyncOperation;

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