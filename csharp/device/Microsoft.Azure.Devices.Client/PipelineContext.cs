// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Microsoft.Azure.Devices.Client
{
    using System.Collections.Generic;

    class PipelineContext: IPipelineContext
    {
        readonly Dictionary<string, object> context = new Dictionary<string, object>();

        public void Set<T>(T value)
        {
            this.Set(typeof(T).Name, value);
        }

        public void Set<T>(string key, T value)
        {
            this.context[key] = value;
        }

        public T Get<T>() where T : class
        {
            return this.Get<T>(typeof(T).Name);
        }

        public T Get<T>(string key) where T : class
        {
            object value;
            if (this.context.TryGetValue(key, out value))
            {
                return value as T;
            }
            return null;
        }

        public T Get<T>(T defaultValue)
        {
            return this.Get(typeof(T).Name, defaultValue);
        }

        public T Get<T>(string key, T defaultValue)
        {
            object value;
            if (this.context.TryGetValue(key, out value))
            {
                return value is T ? (T)value : defaultValue;
            }
            return defaultValue;
        }
    }
}