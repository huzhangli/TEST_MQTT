// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Microsoft.Azure.Devices.Client
{
    using System;
    using System.Collections.Generic;

    class DeviceClientPipelineBuilder : IDeviceClientPipelineBuilder
    {
        readonly LinkedList<Func<IPipelineContext, IDelegatingHandler>> pipeline = new LinkedList<Func<IPipelineContext, IDelegatingHandler>>();

        public IDeviceClientPipelineBuilder With(Func<IPipelineContext, IDelegatingHandler> delegatingHandlerCreator)
        {
            this.pipeline.AddLast(delegatingHandlerCreator);
            return this;
        }

        class Pipeline
        {
            
        }

        public IDelegatingHandler Build(IPipelineContext context)
        {
            LinkedListNode<Func<IPipelineContext, IDelegatingHandler>> currentHandlerFactoryNode = this.pipeline.First;
            while (currentHandlerFactoryNode.Next != null)
            {
                LinkedListNode<Func<IPipelineContext, IDelegatingHandler>> current = currentHandlerFactoryNode;
                Func<IPipelineContext, IDelegatingHandler> factory = current.Value;
                current.Value = ctx =>
                {
                    IDelegatingHandler delegatingHandler = factory(ctx);
                    delegatingHandler.ContinuationFactory = current.Next?.Value;
                    return delegatingHandler;
                };
                currentHandlerFactoryNode = currentHandlerFactoryNode.Next;
            }

            IDelegatingHandler root = this.pipeline.First.Value(context);
            return root;
        }
    }
}