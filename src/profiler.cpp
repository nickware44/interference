/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     23.08.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/profiler.h>
#include <map>

std::multimap<indk::NeuralNet*, std::pair<indk::ProfilerCallback, int>> Callbacks;

void indk::Profiler::doAttachCallback(indk::NeuralNet *object, int flag, indk::ProfilerCallback callback) {
    Callbacks.insert(std::make_pair(object, std::make_pair(callback, flag)));
}

void indk::Profiler::doEmit(indk::NeuralNet *object, int flag) {
    auto callback = Callbacks.equal_range(object);
    for (auto it = callback.first; it != callback.second; it++) {
        if (it->second.second == flag) it->second.first(object);
    }
}

indk::Profiler::~Profiler() {

}
