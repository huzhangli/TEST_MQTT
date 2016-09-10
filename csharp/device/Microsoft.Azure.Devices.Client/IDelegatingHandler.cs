// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Microsoft.Azure.Devices.Client
{
    using System;
    using System.Collections.Generic;
    using System.Threading.Tasks;

    public interface IDelegatingHandler: IContinuationProvider<IDelegatingHandler>, IDisposable
    {
        Task AbandonAsync(string lockToken);
        Task CloseAsync();
        Task CompleteAsync(string lockToken);
        Task OpenAsync(bool explicitOpen);
        Task<Message> ReceiveAsync();
        Task<Message> ReceiveAsync(TimeSpan timeout);
        Task RejectAsync(string lockToken);
#if WINDOWS_UWP
        [Windows.Foundation.Metadata.DefaultOverload]
#endif
        Task SendEventAsync(Message message);
        Task SendEventAsync(IEnumerable<Message> messages);
    }
}