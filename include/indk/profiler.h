/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     23.08.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_PROFILER_H
#define INTERFERENCE_PROFILER_H

#include <functional>
#include <vector>
#include <indk/neuron.h>
#include <indk/neuralnet.h>

namespace indk {
    typedef std::function<void(indk::NeuralNet*)> ProfilerCallback;

    class Profiler {
    private:
    public:
        typedef enum {
            EventProcessed,
            EventTick
        } EventFlags;

        Profiler();
        static void doAttachCallback(indk::NeuralNet *object, int flag, indk::ProfilerCallback callback);
        static void doEmit(indk::NeuralNet *object, int flag);
        ~Profiler();
    };
}

#endif //INTERFERENCE_PROFILER_H
