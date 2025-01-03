/////////////////////////////////////////////////////////////////////////////
// Name:        neuralnet/neuralnet.cpp
// Purpose:     Neural net main class
// Author:      Nickolay Babbysh
// Created:     12.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <queue>
#include <thread>
#include <json.hpp>
#include <indk/neuralnet.h>
#include <indk/profiler.h>

typedef nlohmann::json json;

indk::NeuralNet::NeuralNet() {
    t = 0;
    StateSyncEnabled = false;
    LastUsedComputeBackend = -1;
    InterlinkService = nullptr;

    if (indk::System::getVerbosityLevel() > 1)
        std::cout << "Using default compute backend." << std::endl;

    indk::System::setComputeBackend(indk::System::ComputeBackends::Default);
}

indk::NeuralNet::NeuralNet(const std::string &path) {
    t = 0;
    StateSyncEnabled = false;
    LastUsedComputeBackend = -1;
    InterlinkService = nullptr;
    std::ifstream filestream(path);
    setStructure(filestream);

    if (indk::System::getVerbosityLevel() > 1)
        std::cout << "Using default compute backend." << std::endl;

    indk::System::setComputeBackend(indk::System::ComputeBackends::Default);
}

void indk::NeuralNet::doInterlinkInit(int port, int timeout) {
    InterlinkService = new indk::Interlink(port, timeout);

    indk::Profiler::doAttachCallback(this, indk::Profiler::EventFlags::EventTick, [this](indk::NeuralNet *nn) {
        auto neurons = getNeurons();
        for (uint64_t i = 0; i < neurons.size(); i++) {
            if (i >= InterlinkDataBuffer.size()) {
                InterlinkDataBuffer.emplace_back();
            }
            InterlinkDataBuffer[i].push_back(std::to_string(neurons[i]->doSignalReceive().second));
        }
    });

    indk::Profiler::doAttachCallback(this, indk::Profiler::EventFlags::EventProcessed, [this](indk::NeuralNet *nn) {
        doInterlinkSyncData();
    });

    if (InterlinkService->isInterlinked()) {
        if (!InterlinkService->getStructure().empty()) setStructure(InterlinkService->getStructure());
    }
}

void indk::NeuralNet::doInterlinkSyncStructure() {
    if (!InterlinkService || InterlinkService && !InterlinkService->isInterlinked()) return;
    InterlinkService -> doUpdateStructure(getStructure());
}

void indk::NeuralNet::doInterlinkSyncData() {
    if (!InterlinkService || InterlinkService && !InterlinkService->isInterlinked()) return;
    json j, jm;
    uint64_t in = 0;

    for (const auto &n: Neurons) {
        json jn, jnm;

        jn["name"] = n.second->getName();

        jnm["name"] = n.second->getName();
        jnm["total_time"] = n.second->getTime();
        jnm["output_signal"] = json::parse("[]");

        if (in < InterlinkDataBuffer.size()) {
            for (const auto& o: InterlinkDataBuffer[in]) {
                jnm["output_signal"].push_back(o);
            }
        }
        jm.push_back(jnm);

        for (int i = 0; i < n.second->getReceptorsCount(); i++) {
            json jr;
            auto r = n.second -> getReceptor(i);
            jr["sensitivity"] = r -> getSensitivityValue();

            auto scopes = r -> getReferencePosScopes();
            for (const auto &s: scopes) {
                json js;
                for (int p = 0; p < n.second->getDimensionsCount(); p++) {
                    js.push_back(s->getPositionValue(p));
                }
                jr["scopes"].push_back(js);
            }

            for (int p = 0; p < n.second->getDimensionsCount(); p++) {
                jr["phantom"].push_back(r->getPosf()->getPositionValue(p));
            }

            jn["receptors"].push_back(jr);
        }

        j["neurons"].push_back(jn);
        in++;
    }

    InterlinkDataBuffer.clear();
    InterlinkService -> doUpdateModelData(j.dump());
    InterlinkService -> doUpdateMetrics(jm.dump());
}

int64_t indk::NeuralNet::doFindEntry(const std::string& ename) {
    auto ne = std::find_if(Entries.begin(), Entries.end(), [ename](const std::pair<std::string, std::vector<std::string>>& e){
        return e.first == ename;
    });
    if (ne == Entries.end()) return -1;
    return std::distance(Entries.begin(), ne);
}

std::vector<float> indk::NeuralNet::doComparePatterns(int CompareFlag, int ProcessingMethod) {
    return doComparePatterns(std::vector<std::string>(), CompareFlag, ProcessingMethod);
}

std::vector<float> indk::NeuralNet::doComparePatterns(const std::string& ename, int CompareFlag, int ProcessingMethod) {
    auto en = Ensembles.find(ename);
    if (en != Ensembles.end()) {
        return doComparePatterns(en->second, CompareFlag, ProcessingMethod);
    }
    return {};
}

/**
 * Compare neuron patterns (learning and recognition patterns) for all output neurons.
 * @return Vector of pattern difference values for each output neuron.
 */
std::vector<float> indk::NeuralNet::doComparePatterns(std::vector<std::string> nnames, int CompareFlag, int ProcessingMethod) {
    std::vector<float> PDiffR, PDiff;

    if (nnames.empty()) nnames = Outputs;
    for (const auto& O: nnames) {
        auto n = Neurons.find(O);
        if (n == Neurons.end()) break;
        auto P = n -> second -> doComparePattern(ProcessingMethod);
        PDiffR.push_back(std::get<0>(P));
    }

    switch (CompareFlag) {
        default:
        case indk::PatternCompareFlags::CompareDefault:
            return PDiffR;

        case indk::PatternCompareFlags::CompareNormalized:
            float PDRMin = PDiffR[std::distance(PDiffR.begin(), std::min_element(PDiffR.begin(), PDiffR.end()))];
            float PDRMax = PDiffR[std::distance(PDiffR.begin(), std::max_element(PDiffR.begin(), PDiffR.end()))] - PDRMin;
            for (auto &PDR: PDiffR) {
                if (PDRMax != 0) PDiff.push_back(1 - (PDR-PDRMin) / PDRMax);
                else PDiff.push_back(1);
            }
            return PDiff;
    }
}

void indk::NeuralNet::doCreateNewScope() {
    for (const auto& N: Neurons) N.second -> doCreateNewScope();
}

void indk::NeuralNet::doChangeScope(uint64_t scope) {
    for (const auto& N: Neurons) N.second -> doChangeScope(scope);
}

void indk::NeuralNet::doAddNewOutput(const std::string& name) {
    Outputs.push_back(name);
}

void indk::NeuralNet::doIncludeNeuronToEnsemble(const std::string& name, const std::string& ensemble) {
    auto en = Ensembles.find(ensemble);
    if (en != Ensembles.end()) {
        en -> second.push_back(name);
    } else {
        Ensembles.insert(std::make_pair(ensemble, std::vector<std::string>({name})));
    }
}

/**
 * Resets all neurons in the network.
 * See indk::Neuron::doReset() method for details.
 */
void indk::NeuralNet::doReset() {
    t = 0;
    for (const auto& N: Neurons) N.second -> doReset();
}

void indk::NeuralNet::doPrepare() {
    t = 0;
    for (const auto& N: Neurons) N.second -> doPrepare();
}

void indk::NeuralNet::doSignalProcessStart(const std::vector<std::vector<float>>& Xx, const EntryList& entries) {
    float value;
    int d = 0;

    int64_t dt = t;

    for (auto X: Xx) {
        int xi = 0;
        for (auto &e: entries) {
            for (auto &en: e.second) {
                auto n = Neurons.find(en);

                auto lto = Latencies.find(en);
                auto latencyto = lto != Latencies.end() ? lto->second : 0;
                auto waiting = n -> second -> getWaitingEntries();
                for (auto &we: waiting) {
                    auto nprev = Latencies.find(we);
                    if (nprev != Latencies.end() && nprev->second > latencyto) {
                        n -> second -> doSignalSendEntry(we, 0, 0);
                    }
                }

                if (n != Neurons.end())
                    n -> second -> doSignalSendEntry(e.first, X[xi], t);
            }
            xi++;
        }
        t++;
    }

    if (indk::System::getComputeBackendKind() == indk::System::ComputeBackends::OpenCL)
        indk::System::getComputeBackend() -> doWaitTarget();

    dt = t - dt;
    if (Xx.empty()) dt = 1;
    int64_t lt = 0;

    while (lt != dt) {
        d = 0;

        for (auto l: Links) {
            auto from = std::get<0>(l);
            auto to = std::get<1>(l);

            auto nfrom = (indk::Neuron*)std::get<2>(l);
            auto nto = (indk::Neuron*)std::get<3>(l);
            auto latency = std::get<4>(l);
            auto time = nto -> getTime();

            if (nto->getTime() == t) {
                d++;
                continue;
            }

            value = 0;
            if (!time && latency >= 0 || time) {
                auto shift = (bool)latency;
                if (latency > 0) shift = false;
                if (nfrom->getState(time-shift) != indk::Neuron::States::Computed) {
                    if (indk::System::getComputeBackendKind() == indk::System::ComputeBackends::OpenCL)
                        d++;
                    continue;
                }
                value = nfrom -> doSignalReceive(time-shift).second;
            }


            nto -> doSignalSendEntry(from, value, time);

            if (indk::System::getComputeBackendKind() == indk::System::ComputeBackends::OpenCL) {
                if (!Xx.empty() || Xx.empty() && time == t) {
                    d++;
                }
            } else if (time == t) {
                d++;
            }
        }

        if (d == Links.size()) {
            lt++;
        } else if (Xx.empty()) indk::System::getComputeBackend() -> doWaitTarget();
    }
}

void indk::NeuralNet::doParseLinks(const EntryList& entries, const std::string& id) {
    if (id == PrepareID) return;

    Links.clear();

    NQueue nqueue;

    for (auto &e: entries) {
        for (auto &en: e.second) {
            nqueue.emplace(e.first, en, nullptr, 0);
        }
    }

    while (!nqueue.empty()) {
        auto i = nqueue.front();
        nqueue.pop();

        auto from = std::get<0>(i);
        auto to = std::get<1>(i);
        auto latency = std::get<3>(i);

        auto n = Neurons.find(to);

        if (n != Neurons.end()) {
            bool skip = false;
            for (auto l: Links) {
                if (from == std::get<0>(l) && to == std::get<1>(l)) {
                    skip = true;
                    break;
                }
            }
            if (skip) continue;
            auto nprev = Neurons.find(from);
            auto type = 0;
            if (nprev != Neurons.end()) Links.emplace_back(from, to, nprev->second, n->second, latency);

            auto nlinks = n -> second -> getLinkOutput();
            for (auto &nl: nlinks) {
                auto shift = 0;
                auto nlatency = 0;
                auto lnext = Latencies.find(nl);
                if (lnext != Latencies.end()) nlatency = lnext -> second;
                auto lto = Latencies.find(to);
                auto latencyto = lto != Latencies.end() ? lto->second : 0;
                shift = nlatency-latencyto;
                nqueue.emplace(to, nl, nullptr, shift);
            }
        }
    }

    std::sort(Links.begin(), Links.end(), [] (const indk::LinkDefinition& l1, const indk::LinkDefinition& l2) {
        if (std::get<4>(l1) < std::get<4>(l2)) return true;
        return false;
    });

//    std::cout << std::endl;
//    for (auto l: Links) {
//        std::cerr << std::get<0>(l) << " -> " << std::get<1>(l) << " " << std::get<4>(l) << std::endl;
//    }

    PrepareID = id;
}

void indk::NeuralNet::doSyncNeuronStates(const std::string &name) {
    auto s = StateSyncList.find(name);
    if (s != StateSyncList.end()) {
        auto n1 = Neurons.find(s->first);
        if (n1 != Neurons.end()) {
            for (const auto& v: s->second) {
                auto n2 = Neurons.find(v);
                if (n2 != Neurons.end()) {
                    for (int i = 0; i < n1->second->getReceptorsCount(); i++) {
                        auto pos = n1 -> second -> getReceptor(i) -> getPosf();
                        n2 -> second -> getReceptor(i) -> getPosf() -> setPosition(pos);
                    }
                    n2 -> second -> setTime(n1->second->getTime());
                }
            }
        }
    }
}

void indk::NeuralNet::doStructurePrepare() {
    doParseLinks(Entries, "all");
}

/**
 * Send signals to neural network and get output signals.
 * @param Xx Input data vector that contain signals.
 * @return Output signals.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doSignalTransfer(const std::vector<std::vector<float>>& Xx, const std::vector<std::string>& inputs) {
    std::vector<void*> v;
    std::vector<std::string> nsync;
    EntryList eentries;

    if (inputs.empty()) {
        doParseLinks(Entries, "all");
        eentries = Entries;
    } else {
        std::string eseq;
        for (const auto &e: inputs) {
            auto ne = doFindEntry(e);
            if (ne != -1) {
                eentries.emplace_back(Entries[ne]);
                eseq.append(e);
                if (StateSyncEnabled) {
                    for (auto &nname: Entries[ne].second) {
                        nsync.push_back(nname);
                    }
                }
            }
        }
        doParseLinks(eentries, eseq);
    }

    switch (indk::System::getComputeBackendKind()) {
        case indk::System::ComputeBackends::Default:
            doReserveSignalBuffer(1);
            for (auto &X: Xx) {
                doSignalProcessStart({X}, eentries);
                indk::Profiler::doEmit(this, indk::Profiler::EventFlags::EventTick);
            }
            break;

        case indk::System::ComputeBackends::Multithread:
            if (getSignalBufferSize() != Xx.size()) doReserveSignalBuffer(Xx.size());
            for (const auto &n: Neurons) v.push_back((void*)n.second);
            indk::System::getComputeBackend() -> doRegisterHost(v);
            doSignalProcessStart(Xx, eentries);
            indk::System::getComputeBackend() -> doWaitTarget();
            indk::System::getComputeBackend() -> doUnregisterHost();
            break;

        case indk::System::ComputeBackends::OpenCL:
            if (getSignalBufferSize() != Xx.size()) doReserveSignalBuffer(Xx.size());
            for (const auto &n: Neurons) v.push_back((void*)n.second);
            indk::System::getComputeBackend() -> doRegisterHost(v);
            for (auto &X: Xx) {
                doSignalProcessStart({X}, eentries);
            }
            doSignalProcessStart({}, eentries);
            indk::System::getComputeBackend() -> doUnregisterHost();
            break;
    }

    LastUsedComputeBackend = indk::System::getComputeBackendKind();
    indk::Profiler::doEmit(this, indk::Profiler::EventFlags::EventProcessed);

    if (!inputs.empty() && StateSyncEnabled) {
        for (const auto &name: nsync) {
            doSyncNeuronStates(name);
        }
    }

    return doSignalReceive();
}

/**
 * Send signals to neural network asynchronously.
 * @param Xx Input data vector that contain signals.
 * @param callback callback function for output signals.
 */
void indk::NeuralNet::doSignalTransferAsync(const std::vector<std::vector<float>>& Xx, const std::function<void(std::vector<indk::OutputValue>)>& callback, const std::vector<std::string>& inputs) {
    std::function<void()> tCallback([this, Xx, callback, inputs] () {
        auto Y = doSignalTransfer(Xx, inputs);

        if (callback) {
            callback(Y);
        }
    });
    std::thread CallbackThread(tCallback);
    CallbackThread.detach();
}

/**
 * Start neural network learning process.
 * @param Xx Input data vector that contain signals for learning.
 * @return Output signals.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doLearn(const std::vector<std::vector<float>>& Xx, bool prepare, const std::vector<std::string>& inputs) {
    if (InterlinkService && InterlinkService->isInterlinked()) {
        InterlinkService -> doUpdateStructure(getStructure());
    }
    t = 0;
    setLearned(false);
    if (prepare) doPrepare();
    return doSignalTransfer(Xx, inputs);
}

/**
 * Recognize data by neural network.
 * @param Xx Input data vector that contain signals for recognizing.
 * @return Output signals.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doRecognise(const std::vector<std::vector<float>>& Xx, bool prepare, const std::vector<std::string>& inputs) {
    setLearned(true);
    if (prepare) {
        t = 0;
        doPrepare();
    }
    return doSignalTransfer(Xx, inputs);
}

/**
 * Start neural network learning process asynchronously.
 * @param Xx Input data vector that contain signals for learning.
 * @param callback Callback function for output signals.
 */
void indk::NeuralNet::doLearnAsync(const std::vector<std::vector<float>>& Xx, const std::function<void(std::vector<indk::OutputValue>)>& callback, bool prepare, const std::vector<std::string>& inputs) {
    setLearned(false);
    t = 0;
    if (prepare) doPrepare();
    doSignalTransferAsync(Xx, callback, inputs);
}

/**
 * Recognize data by neural network asynchronously.
 * @param Xx Input data vector that contain signals for recognizing.
 * @param callback Callback function for output signals.
 */
void indk::NeuralNet::doRecogniseAsync(const std::vector<std::vector<float>>& Xx, const std::function<void(std::vector<indk::OutputValue>)>& callback, bool prepare, const std::vector<std::string>& inputs) {
    setLearned(true);
    t = 0;
    if (prepare) doPrepare();
    doSignalTransferAsync(Xx, callback, inputs);
}

/**
 * Get output signals.
 * @return Output signals vector.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doSignalReceive(const std::string& ensemble) {
    std::vector<indk::OutputValue> ny;
    std::vector<std::string> elist;

    if (!ensemble.empty()) {
        auto en = Ensembles.find(ensemble);
        if (en == Ensembles.end()) return {};
        elist = en -> second;
        if (elist.empty()) return {};
    }

    for (const auto& oname: Outputs) {
        auto n = Neurons.find(oname);
        if (n != Neurons.end()) {
            if (!elist.empty()) {
                auto item = std::find_if(elist.begin(), elist.end(), [oname](const std::string &value) {
                    return oname == value;
                });
                if (item == elist.end()) continue;
            }

            ny.emplace_back(n->second->doSignalReceive().second, oname);
        }
    }
    return ny;
}

/**
 * Creates full copy of neuron.
 * @param from Source neuron name.
 * @param to Name of new neuron.
 * @param integrate Link neuron to the same elements as the source neuron.
 */
indk::Neuron* indk::NeuralNet::doReplicateNeuron(const std::string& from, const std::string& to, bool integrate) {
    PrepareID = "";

    auto n = Neurons.find(from);
    if (n == Neurons.end()) {
        if (indk::System::getVerbosityLevel() > 0)
            std::cout << "Neuron replication error: element " << from << " not found" << std::endl;
        return nullptr;
    }
    if (Neurons.find(to) != Neurons.end()) {
        if (indk::System::getVerbosityLevel() > 0)
            std::cout << "Neuron replication error: element " << to << " already exists" << std::endl;
        return nullptr;
    }

    auto nnew = new indk::Neuron(*n->second);
    nnew -> setName(to);
    Neurons.insert(std::make_pair(to, nnew));

    if (integrate) {
        auto entries = nnew -> getEntries();
        for (auto &e: entries) {
            auto ne = doFindEntry(e);
            if (ne != -1) {
                Entries[ne].second.push_back(to);
            } else {
                auto nfrom = Neurons.find(e);
                if (nfrom != Neurons.end()) {
                    nfrom -> second -> doLinkOutput(to);
                }
            }
        }

//        bool found = false;
//        for (auto& e: Ensembles) {
//            for (const auto &en: e.second) {
//                if (en == from) {
//                    e.second.push_back(to);
//                    found = true;
//                    break;
//                }
//            }
//            if (found) break;
//        }
    } else {
//        nnew ->  doClearEntries();
    }
    return nnew;
}

/**
 * Delete the neuron.
 * @param name Name of the neuron.
 */
void indk::NeuralNet::doDeleteNeuron(const std::string& name) {
    auto n = Neurons.find(name);
    if (n == Neurons.end()) return;
    delete n->second;
    Neurons.erase(n);
}

/**
 * Creates full copy of group of neurons.
 * @param from Source ensemble name.
 * @param to Name of new ensemble.
 * @param entries Copy entries during replication. So, if neuron `A1N1` (ensemble `A1`) has an entry `A1E1`
 * and you replicating to ensemble `A2`, a new entry `A2E1` will be added.
 */
void indk::NeuralNet::doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries) {
    json j;

    PrepareID = "";
    std::vector<std::string> enew;
    auto efrom = Ensembles.find(From);

    if (efrom != Ensembles.end()) {
        auto eto = Ensembles.find(To);
        auto lastname = efrom->second.back();

        std::map<std::string, std::string> newnames;
        for (auto &nn: efrom->second) {
            std::string nname;
            if (nn.substr(0, From.size()) == From) {
                nname = nn;
                nname.replace(0, From.size(), To);
            } else nname = To + nn;
            newnames.insert(std::make_pair(nn, nname));
        }

        for (auto &en: efrom->second) {
            json ji;

            auto n = Neurons.find(en);
            if (n != Neurons.end()) {
                auto nnew = new indk::Neuron(*n->second);
                std::string nname = newnames.find(en)->second;
                nnew -> setName(nname);

                auto entries = nnew -> getEntries();
                for (auto &e: entries) {
                    std::string ename = e;
                    auto r = newnames.find(e);
                    if (r != newnames.end()) {
                        nnew -> doReplaceEntryName(e, r->second);
                    } else if (CopyEntries) {
                        if (e.substr(0, From.size()) == From) {
                            ename = e;
                            ename.replace(0, From.size(), To);
                        } else ename = To + e;
                        nnew -> doReplaceEntryName(e, ename);
                    }

                    auto ne = doFindEntry(ename);
                    if (ne != -1) {
                        Entries[ne].second.push_back(nname);
                    } else if (CopyEntries && r == newnames.end()) {
                        std::vector<std::string> elinks;
                        elinks.push_back(nname);
                        j["entries"].push_back(ename);
                        Entries.emplace_back(ename, elinks);
                    }
                }

                auto outputlinks = nnew -> getLinkOutput();
                nnew -> doClearOutputLinks();
                for (auto &o: outputlinks) {
                    auto r = newnames.find(o);
                    if (r != newnames.end()) {
                        nnew -> doLinkOutput(r->second);
                    } else {
                        nnew -> doLinkOutput(o);
                    }
                }

                auto no = std::find(Outputs.begin(), Outputs.end(), en);
                if (no != Outputs.end()) {
                    Outputs.push_back(nname);
                    j["outputs"].push_back(nname);
                }

                auto nl = Latencies.find(en);
                if (nl != Latencies.end()) Latencies.insert(std::make_pair(nname, nl->second));

                Neurons.insert(std::make_pair(nname, nnew));

                entries = nnew -> getEntries();
                for (auto &e: entries) {
                    ji["input_signals"].push_back(e);
                }
                ji["old_name"] = en;
                ji["new_name"] = nname;
                j["neurons"].push_back(ji);

                auto sobject = StateSyncList.find(en);
                if (sobject == StateSyncList.end()) {
                    StateSyncList.insert(std::make_pair(en, std::vector<std::string>({nname})));
                } else {
                    sobject->second.push_back(nname);
                }

                if (eto == Ensembles.end()) {
                    enew.push_back(nname);
                } else {
                    eto->second.push_back(nname);
                }
            }
        }

        if (eto == Ensembles.end()) Ensembles.insert(std::make_pair(To, enew));
    }

    if (indk::System::getVerbosityLevel() > 1) {
        auto e = Ensembles.find(To);
        std::cout << "Entries: ";
        for (const auto& ne: Entries) {
            std::cout << ne.first << " ";
        }
        std::cout << std::endl;
        std::cout << e->first << " -";
        for (const auto& en: e->second) {
            std::cout << " " << en;
        }
        std::cout << std::endl;
        std::cout << "Outputs: ";
        for (const auto& o: Outputs) {
            std::cout << o << " ";
        }
        std::cout << std::endl;
    }
}

void indk::NeuralNet::doReserveSignalBuffer(int64_t L) {
    for (auto &n: Neurons) {
        n.second -> doReserveSignalBuffer(L);
    }
}

void indk::NeuralNet::doClearCache() {
    PrepareID = "";
}

/**
 * Load neural network structure.
 * @param Stream Input stream of file that contains neural network structure in JSON format.
 */
void indk::NeuralNet::setStructure(std::ifstream &Stream) {
    if (!Stream.is_open()) {
        if (indk::System::getVerbosityLevel() > 0) std::cerr << "Error opening file" << std::endl;
        return;
    }
    std::string jstr;
    while (!Stream.eof()) {
        std::string rstr;
        getline(Stream, rstr);
        jstr.append(rstr);
    }
    setStructure(jstr);
}

/** \example samples/test/structure.json
 * Example of interference neural net structure. It can be used by NeuralNet class and indk::NeuralNet::setStructure method.
 */

/**
 * Load neural network structure.
 * @param Str JSON string that contains neural network structure.
 *
 * Format of neural network structure:
 * \code
 * {
 *      "entries": [<list of neural network entries>],
 *      "neurons": [{
 *          "name": <name of neuron>,
            "size": <size of neuron>,
            "dimensions": 3,
            "input_signals": [<list of input signals, it can be network entries or other neurons>],
            "ensemble": <the name of the ensemble to which the neuron will be connected>,
            "synapses": [{
                "entry": 0,
                "position": [100, 100, 100],
                "neurotransmitter": "activation",
                "k1": 1
            }],
            "receptors": [{
                "type": "cluster",
                "position": [100, 210, 100],
                "count": 15,
                "radius": 10
            }]
 *      }],
 *      "output_signals": [<list of output sources>],
 *      "name": "neural network structure name",
 *      "desc": "neural network structure description",
 *      "version": "neural network structure version"
 * }
 * \endcode
 *
 *
 * @note
 * Example of neural network structure can be found in the samples:
 * <a href="samples_2test_2structure_8json-example.html">test sample</a>,
 * <a href="samples_2test_2structure_8json-example.html">vision sample</a>
 *
 */
void indk::NeuralNet::setStructure(const std::string &Str) {
    for (const auto& N: Neurons) delete N.second;
    PrepareID = "";
    Entries.clear();
    Outputs.clear();
    Latencies.clear();
    Neurons.clear();
    Ensembles.clear();
    StateSyncList.clear();

    try {
        auto j = json::parse(Str);
        //std::cout << j.dump(4) << std::endl;
        Name = j["name"].get<std::string>();
        Description = j["desc"].get<std::string>();
        Version = j["version"].get<std::string>();

        std::multimap<std::string, std::string> links;
        for (auto &jneuron: j["neurons"].items()) {
            auto nname = jneuron.value()["name"].get<std::string>();
            for (auto &jinputs: jneuron.value()["input_signals"].items()) {
                auto iname = jinputs.value().get<std::string>();
                links.insert(std::make_pair(iname, nname));
            }
        }

        for (auto &jentry: j["entries"].items()) {
            auto ename = jentry.value().get<std::string>();
            std::vector<std::string> elinks;
            auto l = links.equal_range(ename);
            for (auto it = l.first; it != l.second; it++) {
                elinks.push_back(it->second);
                if (indk::System::getVerbosityLevel() > 1) std::cout << ename << " -> " << it->second << std::endl;
            }
            Entries.emplace_back(ename, elinks);
        }

        for (auto &joutput: j["output_signals"].items()) {
            auto oname = joutput.value().get<std::string>();
            if (indk::System::getVerbosityLevel() > 1) std::cout <<  "Output " << oname << std::endl;
            Outputs.push_back(oname);
        }

//        for (auto &l: links) {
//            std::cout << l.first << " - " << l.second << std::endl;
//        }

        for (auto &jneuron: j["neurons"].items()) {
            auto nname = jneuron.value()["name"].get<std::string>();
            auto nsize = jneuron.value()["size"].get<unsigned int>();
            auto ndimensions = jneuron.value()["dimensions"].get<unsigned int>();
            if (jneuron.value()["latency"] != nullptr) {
                auto nlatency = jneuron.value()["latency"].get<int>();
                if (indk::System::getVerbosityLevel() > 1) std::cout << nname << " with latency " << nlatency << std::endl;
                Latencies.insert(std::make_pair(nname, nlatency));
            }
            std::vector<std::string> nentries;
            for (auto &jent: jneuron.value()["input_signals"].items()) {
                nentries.push_back(jent.value().get<std::string>());
            }
            auto *N = new indk::Neuron(nsize, ndimensions, 0, nentries);

            if (jneuron.value()["ensemble"] != nullptr) {
                auto ename = jneuron.value()["ensemble"].get<std::string>();
                auto e = Ensembles.find(ename);
                if (e == Ensembles.end()) {
                    std::vector<std::string> en;
                    en.push_back(nname);
                    Ensembles.insert(std::make_pair(ename, en));
                } else {
                    e->second.push_back(nname);
                }
            }

            for (auto &jsynapse: jneuron.value()["synapses"].items()) {
                std::vector<float> pos;
                if (ndimensions != jsynapse.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jsynapse.value()["position"].items()) {
                    pos.push_back(jposition.value().get<float>());
                }
                float k1 = 1.2;
                if (jsynapse.value()["k1"] != nullptr) k1 = jsynapse.value()["k1"].get<float>();
                unsigned int tl = 0;
                if (jsynapse.value()["tl"] != nullptr) tl = jsynapse.value()["tl"].get<unsigned int>();
                int nt = 0;
                if (jsynapse.value()["neurotransmitter"] != nullptr) {
                    if (jsynapse.value()["neurotransmitter"].get<std::string>() == "deactivation")
                        nt = 1;
                }
                if (jsynapse.value()["type"] != nullptr && jsynapse.value()["type"].get<std::string>() == "cluster") {
                    auto sradius = jsynapse.value()["radius"].get<unsigned int>();
                    N -> doCreateNewSynapseCluster(pos, sradius, k1, tl, nt);
                } else {
                    auto sentryid = -1;
                    if (jsynapse.value()["entry"] != nullptr) {
                        sentryid = jsynapse.value()["entry"].get<unsigned int>();
                    } else {
                        std::cout << "Error: entry number must be set" << std::endl;
                        return;
                    }
                    auto sentry = jneuron.value()["input_signals"][sentryid];
                    N -> doCreateNewSynapse(sentry, pos, k1, tl, nt);
                }


            }
            for (auto &jreceptor: jneuron.value()["receptors"].items()) {
                std::vector<float> pos;
                if (ndimensions != jreceptor.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jreceptor.value()["position"].items()) {
                    pos.push_back(jposition.value().get<float>());
                }
                if (jreceptor.value()["type"] != nullptr && jreceptor.value()["type"].get<std::string>() == "cluster") {
                    auto rcount = jreceptor.value()["count"].get<unsigned int>();
                    auto rradius = jreceptor.value()["radius"].get<unsigned int>();
                    N -> doCreateNewReceptorCluster(pos, rradius, rcount);

                    for (auto i = N->getReceptorsCount()-rcount; i < N ->getReceptorsCount(); i++) {
                        auto r = N -> getReceptor(i);
                        for (auto &jscope: jreceptor.value()["scopes"].items()) {
                            pos.clear();
                            for (auto &jposition: jscope.value().items()) {
                                pos.push_back(jposition.value().get<float>());
                            }
                            N -> doCreateNewScope();
                            r -> setPos(new indk::Position(nsize, pos));
                        }
                    }
                } else {
                    N -> doCreateNewReceptor(pos);
                    for (auto &jscope: jreceptor.value()["scopes"].items()) {
                        pos.clear();
                        for (auto &jposition: jscope.value().items()) {
                            pos.push_back(jposition.value().get<float>());
                        }
                        auto r = N -> getReceptor(N->getReceptorsCount()-1);
                        r -> doCreateNewScope();
                        r -> setPos(new indk::Position(nsize, pos));
                    }
                }
            }

            auto l = links.equal_range(nname);
            for (auto it = l.first; it != l.second; it++) {
                N -> doLinkOutput(it->second);
                if (indk::System::getVerbosityLevel() > 1) std::cout << nname << " -> " << it->second << std::endl;
            }

            N -> setName(nname);
            Neurons.insert(std::make_pair(nname, N));
        }

        if (indk::System::getVerbosityLevel() > 1) {
            for (const auto &e: Ensembles) {
                std::cout << e.first << " -";
                for (const auto &en: e.second) {
                    std::cout << " " << en;
                }
                std::cout << std::endl;
            }
        }

        if (InterlinkService && InterlinkService->isInterlinked()) {
            InterlinkService -> setStructure(Str);
        }
    } catch (std::exception &e) {
        if (indk::System::getVerbosityLevel() > 0) std::cerr << "Error parsing structure: " << e.what() << std::endl;
    }
}

/**
 * Set neural network to `learned` state.
 * @param LearnedFlag
 */
void indk::NeuralNet::setLearned(bool LearnedFlag) {
    for (const auto& N: Neurons) {
        N.second -> setLearned(LearnedFlag);
    }
}

/**
 * Enable neuron state synchronization.
 * @param enable
 */
void indk::NeuralNet::setStateSyncEnabled(bool enabled) {
    StateSyncEnabled = enabled;
}

/**
 * Check if neural network is in learned state.
 * @return
 */
bool indk::NeuralNet::isLearned() {
    for (const auto& N: Neurons) {
        if (!N.second -> isLearned()) return false;
    }
    return true;
}

/**
 * Get neuron by name.
 * @param NName Neuron name.
 * @return indk::Neuron object pointer.
 */
indk::Neuron* indk::NeuralNet::getNeuron(const std::string& NName) {
    auto N = Neurons.find(NName);
    if (N != Neurons.end()) return N->second;
    return nullptr;
}

/**
 * Get neuron list.
 * @return Vector of indk::Neuron object pointers.
 */
std::vector<indk::Neuron*> indk::NeuralNet::getNeurons() {
    std::vector<indk::Neuron*> neurons;
    for (auto &n: Neurons) {
        neurons.push_back(n.second);
    }
    return neurons;
}

/**
 * Get count of neurons in neural network.
 * @return Count of neurons.
 */
uint64_t indk::NeuralNet::getNeuronCount() {
    return Neurons.size();
}

/**
 * Get neural network structure in JSON format.
 * @return JSON string that contains neural network structure.
 */
std::string indk::NeuralNet::getStructure(bool minimized) {
    json j;

    for (const auto& e: Entries) {
        j["entries"].push_back(e.first);
    }

    for (const auto& n: Neurons) {
        json jn;
        jn["name"] = n.second -> getName();
        jn["size"] = n.second -> getXm();
        jn["dimensions"] = n.second -> getDimensionsCount();

        auto nentries = n.second -> getEntries();
        for (const auto& ne: nentries) {
            jn["input_signals"].push_back(ne);
        }
        for (const auto& en: Ensembles) {
            for (const auto& nen: en.second) {
                if (nen == n.second->getName()) {
                    jn["ensemble"] = en.first;
                    break;
                }
            }
        }

        for (int i = 0; i < n.second->getEntriesCount(); i++) {
            auto ne = n.second -> getEntry(i);
            for (int s = 0; s < ne->getSynapsesCount(); s++) {
                auto ns = ne -> getSynapse(s);
                json js;
                js["entry"] = i;
                js["k1"] = ns -> getk1();

                switch (ns->getNeurotransmitterType()) {
                    case 0:
                        js["neurotransmitter"] = "activation";
                        break;
                    case 1:
                        js["neurotransmitter"] = "deactivation";
                        break;
                }

                for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                    js["position"].push_back(ns->getPos()->getPositionValue(p));
                }
                jn["synapses"].push_back(js);
            }
        }

        for (int r = 0; r < n.second->getReceptorsCount(); r++) {
            auto nr = n.second ->getReceptor(r);
            json jr;
            jr["type"] = "single";
            for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                jr["position"].push_back(nr->getPos0()->getPositionValue(p));
            }

            json jscopes;
            auto scopes = nr -> getReferencePosScopes();
            for (const auto &s: scopes) {
                json jscope;
                for (int p = 0; p < s->getDimensionsCount(); p++) {
                    jscope.push_back(s->getPositionValue(p));
                }
                jscopes.push_back(jscope);
            }
            if (!scopes.empty())
                jr["scopes"] = jscopes;

            jn["receptors"].push_back(jr);
        }

        auto l = Latencies.find(n.second->getName());
        if (l != Latencies.end()) {
            jn["latency"] = l->second;
        }

        j["neurons"].push_back(jn);
    }

    for (const auto& o: Outputs) {
        j["output_signals"].push_back(o);
    }

    j["name"] = Name;
    j["desc"] = Description;
    j["version"] = Version;

    if (indk::System::getVerbosityLevel() > 2) {
        std::cout << j.dump(4) << std::endl;
    }

    return minimized ? j.dump() : j.dump(4);
}

/**
 * Get neural network structure name.
 * @return String that contains name.
 */
std::string indk::NeuralNet::getName() {
    return Name;
}

/**
 * Get neural network structure description.
 * @return String that contains description.
 */
std::string indk::NeuralNet::getDescription() {
    return Description;
}

/**
 * Get neural network structure version.
 * @return String that contains version.
 */
std::string indk::NeuralNet::getVersion() {
    return Version;
}

/**
 * Get group of neurons by name.
 * @param ename Ensemble name.
 * @return Vector of indk::Neuron object pointers.
 */
std::vector<indk::Neuron*> indk::NeuralNet::getEnsemble(const std::string& ename) {
    auto e = Ensembles.find(ename);
    if (e != Ensembles.end()) {
        std::vector<indk::Neuron*> neurons;
        for (const auto& en: e->second) {
            neurons.push_back(getNeuron(en));
        }
        return neurons;
    }
    return {};
}

int64_t indk::NeuralNet::getSignalBufferSize() {
    int64_t size = -1;
    for (auto &n: Neurons) {
        auto nbsize = n.second -> getSignalBufferSize();
        if (size != -1 && nbsize >= size) continue;
        size = nbsize;
    }
    return size;
}

indk::NeuralNet::~NeuralNet() {
    for (const auto& N: Neurons) delete N.second;
}
