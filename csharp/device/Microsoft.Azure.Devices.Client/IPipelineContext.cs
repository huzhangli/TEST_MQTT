// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Microsoft.Azure.Devices.Client
{
    public interface IPipelineContext
    {
        void Set<T>(T value);

        void Set<T>(string key, T value);

        T Get<T>() where T : class;

        T Get<T>(string key) where T : class;

        T Get<T>(T defaultValue);

        T Get<T>(string key, T defaultValue);
    }
}